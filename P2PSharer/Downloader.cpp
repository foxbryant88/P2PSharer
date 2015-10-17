#include "stdafx.h"
#include "Downloader.h"
#include <io.h>


CDownloader::CDownloader()
{
	m_dwLastBlockPos = 0;
	m_bExit = false;
	m_dwProviderLastUpdateTime = 0;
}


CDownloader::~CDownloader()
{
}

//��ʼ��
bool CDownloader::Init(T_LOCAL_FILE_INFO &fileinfo, acl::socket_stream &sock, CRedisClient *redis, HWND hNotifyWnd)
{
	m_fileInfo.filemd5 = fileinfo.filemd5;
	m_fileInfo.filename = fileinfo.filename;
	m_fileInfo.filesize = fileinfo.filesize;
	m_fileInfo.fullpath = fileinfo.fullpath;

	//m_sock = &sock;
	m_redis = redis;
	m_hWndRecieveNotify = hNotifyWnd;

	acl::string localpath = "d:\\p2pfile\\";
	if (_access(localpath.c_str(), 0) != 0)
	{
		if (acl_make_dirs(localpath.c_str(), 0755) == -1)
		{
			g_clientlog.error1("create dirs: %s error: %s",
				localpath.c_str(), acl::last_serror());

			ShowError("����Ŀ¼ʧ�ܣ�");
			return false;
		}
	}

	localpath += m_fileInfo.filename;

	//����ָ����С���ļ�
	if (!m_fstream.create(localpath.c_str()))
	{
		g_clientlog.error1("�����ļ�[%s]ʧ�ܣ�err:%d", localpath, acl::last_error());
		return false;
	}
	m_fstream.fseek(m_fileInfo.filesize, SEEK_SET);
	//m_fstream.write(0);
	
	m_objReciever = new CFileClient;
	m_objReciever->Init(m_fstream, m_fileInfo.filemd5.c_str(), m_fileInfo.filesize, m_hWndRecieveNotify);
	m_objReciever->start();
}

//���ӿ������ؽڵ�
void CDownloader::AddProvider(acl::string &addr, acl::string &md5)
{
	//�Ѵ��������
	for (int i = 0; i < m_vProviderMACs.size(); ++i)
	{
		if (m_vProviderMACs[i] == addr)
		{
			return;
		}
	}

	m_vProviderMACs.push_back(addr);

	CReqSender *psender = new CReqSender;
	psender->Init(addr, g_serEx.GetSockStream(), md5, this);
	psender->start();

	m_lockSenderObject.lock();
	m_vObjSender.push_back(psender);
	m_lockSenderObject.unlock();
}

//�����յ��ķֿ�����,ת����FileReciver
void CDownloader::Recieve(MSGDef::TMSG_FILEBLOCKDATA *data)
{
	BLOCK_DATA_INFO *block = &data->info;

	m_lockFileBlocks.lock();
	std::map<DWORD, DWORD>::iterator itTmp = m_mapFileBlocks.find(block->dwBlockNumber);

	if (itTmp != m_mapFileBlocks.end())
	{
		BLOCK_DATA_INFO *info = new BLOCK_DATA_INFO;            //��д���ļ�����CFileClient�ͷ�
		memcpy(info, block, sizeof(BLOCK_DATA_INFO));

		m_objReciever->CacheData((void *)info);

		//���ѳɹ����صķ�Ƭ��Ŵ��б����Ƴ�
		m_mapFileBlocks.erase(itTmp);
	}

	m_lockFileBlocks.unlock();
}

//�Ƴ�����ʧ�ܵķ��Ͷ���(ֻ�Ƴ������ͷ��ڴ棬CReqSender�������ɾ��)
void CDownloader::RemoveFailConnSender(CReqSender *pSender)
{
	m_lockSenderObject.lock();
	std::vector<CReqSender *>::iterator it = m_vObjSender.begin();
	for (; it != m_vObjSender.end(); ++it)
	{
		if (*it == pSender)
		{
			m_vObjSender.erase(it);
			break;
		}
	}

	if (m_vObjSender.size() == 0)
	{
		PostMessage(m_hWndRecieveNotify, UM_DOWNLOAD_ABORT, NULL, (LPARAM)m_fileInfo.filemd5.c_str());
	}
	m_lockSenderObject.unlock();
}

//���Ʒ�Ƭ������
void *CDownloader::run()
{
	std::vector<DWORD> vBlocks;
	int iGetBlockRes = 0;

	while (!m_bExit)
	{
		//���ڿ��õ����ؽڵ�
		if (UpdateServiceProvider())
		{
			//ȷ����Ƭ�������з�Ƭ
			SplitFileSizeIntoBlockMap();


			//Ϊÿ�����Ͷ���ָ����������
			m_lockSenderObject.lock();
			for (int i = 0; i < m_vObjSender.size(); i++)
			{
				if ((iGetBlockRes = GetBlocks(vBlocks)) == 1)
				{
					if (m_vObjSender[i]->PushTask(vBlocks))
					{
						vBlocks.clear();
					}
				}
			}
			m_lockSenderObject.unlock();

			//����ʱ�ķ�Ƭ����(��δ�·���Ƭʱ�Ŵ���
			if (iGetBlockRes == 2)
			{
				DealTimeoutBlockRequests();
			}
		}

		Sleep(100);
	}

	//ֹͣ���ռ���Ƭ�������
	m_objReciever->Stop();
//	delete m_objReciever;

	m_lockSenderObject.lock();
	for (int i = 0; i < m_vObjSender.size(); i++)
	{
		m_vObjSender[i]->Stop();
	}
	m_vObjSender.clear();
	m_lockSenderObject.unlock();

	return NULL;
}

//ֹͣ
void CDownloader::Stop()
{
	m_bExit = true;
}

//���ļ����з�Ƭ
bool CDownloader::SplitFileSizeIntoBlockMap()
{
	//��Ƭ����һ��ʱ���·���
	if (m_mapFileBlocks.size() < MAX_CACHE_BLOCKS / 2)
	{
		m_lockFileBlocks.lock();
		for (int i = m_mapFileBlocks.size(); i < MAX_CACHE_BLOCKS; i++)
		{
			if (m_dwLastBlockPos * EACH_BLOCK_SIZE > m_fileInfo.filesize)
			{
				ShowMsg("�Ѵﵽ���һ����Ƭ��");			
				break;
			}

			m_mapFileBlocks[m_dwLastBlockPos] = 0;
			m_dwLastBlockPos++;
		}
		m_lockFileBlocks.unlock();
	}
	
	return true;
}

//��ȡһ���ֿ� 0����Ƭ������Ϊ�� 1���ɹ� 2����δ�·���Ƭ
int CDownloader::GetBlocks(std::vector<DWORD> &blockNums)
{
	//�ֿ黺���б����޷ֿ���ȡ�ֿ�ʧ��
	if (m_mapFileBlocks.empty())
	{
		return 0;
	}

	//�ֿ��б�ǿգ�������δ�����CFileSender����ִ�С�ֱ�ӷ���
	if (blockNums.size() > 0)
	{
		return 1;
	}

	bool bDispatch = false;
	m_lockFileBlocks.lock();
	std::map<DWORD, DWORD>::iterator itTmp = m_mapFileBlocks.begin();
	for (; itTmp != m_mapFileBlocks.end(); ++itTmp)
	{
		if (blockNums.size() == MAX_REQUEST_BLOCKS_COUNT)
		{
			break;
		}

		//��Ƭʱ���Ϊ0��ʾδ����
		if (itTmp->second == 0)
		{
			blockNums.push_back(itTmp->first);
			itTmp->second = GetTickCount();        //�ѷ�����
			bDispatch = true;
		}
	}
	m_lockFileBlocks.unlock();

	if (bDispatch)
		return 1;
	else
		return 2;
}

//�Գ���5����δ��Ӧ�ķ�Ƭ�������Ƭʱ���Ϊ0
//��������������
void CDownloader::DealTimeoutBlockRequests()
{
	static DWORD dwCheckTime = GetTickCount();

	if (GetTickCount() - dwCheckTime > BLOCK_REQUEST_TIME_OUT)
	{
		dwCheckTime = GetTickCount();

		m_lockFileBlocks.lock();
		std::map<DWORD, DWORD>::iterator itTmp = m_mapFileBlocks.begin();
		for (; itTmp != m_mapFileBlocks.end(); ++itTmp)
		{
			if (itTmp->second - dwCheckTime > BLOCK_REQUEST_TIME_OUT)
			{
				itTmp->second = 0;       //������0
			}
		}
		m_lockFileBlocks.unlock();
	}
}

//ÿ��1������������һ�η���ڵ�
bool CDownloader::UpdateServiceProvider(void)
{
	if (GetTickCount() - m_dwProviderLastUpdateTime > UPDATE_SERVICE_PROVIDER_TIME)
	{
		m_dwProviderLastUpdateTime = GetTickCount();

		std::vector<acl::string> vRes;
		int num = m_redis->GetResourceOwnersID(m_fileInfo.filemd5, vRes);
		for (int i = 0; i < num; i++)
		{
// 			acl::string ipAddr;
// 			if (g_serEx.SendMsg_GetIPofMAC(vRes[i], ipAddr))
// 			{
				AddProvider(vRes[i], m_fileInfo.filemd5);
//			}
		}
	}

	if (m_vProviderMACs.size() > 0)
		return true;

	return false;
}