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
bool CDownloader::Init(T_LOCAL_FILE_INFO &fileinfo, acl::socket_stream &sock)
{
	m_fileInfo.filemd5 = fileinfo.filemd5;
	m_fileInfo.filename = fileinfo.filename;
	m_fileInfo.filesize = fileinfo.filesize;
	m_fileInfo.fullpath = fileinfo.fullpath;

	m_sock = &sock;

	acl::string localpath = "d:\\p2pfile\\";
	localpath += m_fileInfo.filename;

	//����ָ����С���ļ�
	if (!m_fstream->create(localpath))
	{
		g_clientlog.error1("�����ļ�[%s]ʧ�ܣ�err:%d", localpath, acl::last_error());
		return false;
	}
	m_fstream->fseek(m_fileInfo.filesize);
	m_fstream->write(0);
	
	m_objReciever = new CFileReciever;
	m_objReciever->Init(m_fstream, m_fileInfo.filemd5.c_str(), m_fileInfo.filesize);
}

//���ӿ������ؽڵ�
void CDownloader::AddProvider(const char *addr)
{
	m_vProvider.push_back(addr);

	CReqSender *psender = new CReqSender;
	psender->Init(addr, *m_sock);

	m_vObjSender.push_back(psender);
}

//�����յ��ķֿ�����,ת����FileReciver
void CDownloader::Recieve(const char *data)
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
	while (!m_bExit)
	{
		SplitFileSizeIntoBlockMap();


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
	std::map<DWORD, DWORD>::iterator itTmp = m_mapFileBlocks.begin();
	for (; itTmp != m_mapFileBlocks.end(); ++itTmp)
	{
		if (blockNums.size() > MAX_REQUEST_BLOCKS_COUNT)
		{
			break;
		}

		blockNums.push_back(itTmp->first);
		itTmp->second = GetTickCount();        //�ѷ�����
	}

	return true;
}