#include "stdafx.h"
#include "Downloader.h"


CDownloader::CDownloader()
{
	m_dwLastBlockPos = 0;
	m_bExit = false;
}


CDownloader::~CDownloader()
{
}

//��ʼ��
bool CDownloader::Init(T_LOCAL_FILE_INFO &fileinfo, acl::socket_stream &sock, CRedisClient *redis)
{
	m_fileInfo.filemd5 = fileinfo.filemd5;
	m_fileInfo.filename = fileinfo.filename;
	m_fileInfo.filesize = fileinfo.filesize;
	m_fileInfo.fullpath = fileinfo.fullpath;

	m_sock = &sock;
	m_redis = redis;

	acl::string localpath = "d:\\p2pfile\\";
	localpath += m_fileInfo.filename;

	//����ָ����С���ļ�
	if (!m_fstream->create(localpath))
	{
		g_clientlog.error1("�����ļ�[%s]ʧ�ܣ�err:%d", localpath, acl::last_error());
		return false;
	}
	m_fstream->fseek(m_fileInfo.filesize, SEEK_SET);
	m_fstream->write(0);
	
	m_objReciever = new CFileReciever;
	m_objReciever->Init(*m_fstream, m_fileInfo.filemd5.c_str(), m_fileInfo.filesize);
}

//���ӿ������ؽڵ�
void CDownloader::AddProvider(acl::string &addr)
{
	//�Ѵ��������
	for (int i = 0; i < m_vProvider.size(); ++i)
	{
		if (m_vProvider[i] == addr)
		{
			break;;
		}
	}

	m_vProvider.push_back(addr);

	CReqSender *psender = new CReqSender;
	psender->Init(addr, *m_sock);

	m_vObjSender.push_back(psender);
}

//�����յ��ķֿ�����,ת����FileReciver
void CDownloader::Recieve(void *data)
{
	MSGDef::TMSG_FILEBLOCKDATA *msg = (MSGDef::TMSG_FILEBLOCKDATA *)data;
	BLOCK_DATA_INFO *block = &msg->info;
	std::map<DWORD, DWORD>::iterator itTmp = m_mapFileBlocks.find(block->dwBlockNumber);

	if (itTmp != m_mapFileBlocks.end())
	{
		BLOCK_DATA_INFO *info = new BLOCK_DATA_INFO;
		memcpy(info, block, sizeof(BLOCK_DATA_INFO));

		m_objReciever->CacheData((void *)info);

		//�ӷ�Ƭ�б����Ƴ�
		m_mapFileBlocks.erase(itTmp);
	}
}

//���Ʒ�Ƭ������
void *CDownloader::run()
{
	std::vector<DWORD> vBlocks;

	while (!m_bExit)
	{
		//���ڿ��õ����ؽڵ�
		if (UpdateServiceProvider())
		{
			//ȷ����Ƭ�������з�Ƭ
			SplitFileSizeIntoBlockMap();


			//Ϊÿ�����Ͷ���ָ����������
			for (int i = 0; i < m_vObjSender.size(); i++)
			{
				if (GetBlocks(vBlocks))
				{
					if (m_vObjSender[i]->PushTask(vBlocks))
					{
						vBlocks.clear();
					}
				}
			}

			//����ʱ�ķ�Ƭ����
			DealTimeoutBlockRequests();
		}
	}

	//ֹͣ���ռ���Ƭ�������
	m_objReciever->Stop();
	for (int i = 0; i < m_vObjSender.size(); i++)
	{
		m_vObjSender[i]->Stop();
	}

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
		for (int i = m_mapFileBlocks.size(); i < MAX_CACHE_BLOCKS; i++)
		{
			if (m_dwLastBlockPos * EACH_BLOCK_SIZE > m_fileInfo.filesize)
			{
				//���һ����Ƭ�Ĵ���������
				//��������������������
				break;
			}

			m_mapFileBlocks[m_dwLastBlockPos * EACH_BLOCK_SIZE] = 0;
		}
	}
	
	return true;
}

//��ȡһ���ֿ�
bool CDownloader::GetBlocks(std::vector<DWORD> &blockNums)
{
	if (blockNums.empty())
	{
		return false;
	}

	std::map<DWORD, DWORD>::iterator itTmp = m_mapFileBlocks.begin();
	for (; itTmp != m_mapFileBlocks.end(); ++itTmp)
	{
		if (blockNums.size() > MAX_REQUEST_BLOCKS_COUNT)
		{
			break;
		}

		//��Ƭʱ���Ϊ0��ʾδ����
		if (itTmp->second == 0)
		{
			blockNums.push_back(itTmp->first);
			itTmp->second = GetTickCount();        //�ѷ�����
		}
	}

	return true;
}

//�Գ���5����δ��Ӧ�ķ�Ƭ�������Ƭʱ���Ϊ0
//��������������
void CDownloader::DealTimeoutBlockRequests()
{
	static DWORD dwCheckTime = GetTickCount();

	if (GetTickCount() - dwCheckTime > BLOCK_REQUEST_TIME_OUT)
	{
		dwCheckTime = GetTickCount();

		std::map<DWORD, DWORD>::iterator itTmp = m_mapFileBlocks.begin();
		for (; itTmp != m_mapFileBlocks.end(); ++itTmp)
		{
			if (itTmp->second - dwCheckTime > BLOCK_REQUEST_TIME_OUT)
			{
				itTmp->second = 0;       //������0
			}
		}
	}
}

//ÿ��1������������һ�η���ڵ�
bool CDownloader::UpdateServiceProvider(void)
{
	static DWORD dwLastUpdate = GetTickCount();

	if (GetTickCount() - dwLastUpdate > UPDATE_SERVICE_PROVIDER_TIME)
	{
		std::vector<acl::string> vRes;
		int num = m_redis->GetResourceOwners(m_fileInfo.filemd5, vRes);
		for (int i = 0; i < num; i++)
		{
			AddProvider(vRes[i]);
		}
	}

	if (m_vProvider.size() > 0)
		return true;

	return false;
}