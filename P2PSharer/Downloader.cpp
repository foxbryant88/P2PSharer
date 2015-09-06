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

//初始化
bool CDownloader::Init(T_LOCAL_FILE_INFO &fileinfo, acl::socket_stream &sock)
{
	m_fileInfo.filemd5 = fileinfo.filemd5;
	m_fileInfo.filename = fileinfo.filename;
	m_fileInfo.filesize = fileinfo.filesize;
	m_fileInfo.fullpath = fileinfo.fullpath;

	m_sock = &sock;

	acl::string localpath = "d:\\p2pfile\\";
	localpath += m_fileInfo.filename;

	//创建指定大小的文件
	if (!m_fstream->create(localpath))
	{
		g_clientlog.error1("创建文件[%s]失败！err:%d", localpath, acl::last_error());
		return false;
	}
	m_fstream->fseek(m_fileInfo.filesize);
	m_fstream->write(0);
	
	m_objReciever = new CFileReciever;
	m_objReciever->Init(m_fstream, m_fileInfo.filemd5.c_str(), m_fileInfo.filesize);
}

//增加可用下载节点
void CDownloader::AddProvider(const char *addr)
{
	m_vProvider.push_back(addr);

	CReqSender *psender = new CReqSender;
	psender->Init(addr, *m_sock);

	m_vObjSender.push_back(psender);
}

//处理收到的分块数据,转发给FileReciver
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

		//从分片列表中移除
		m_mapFileBlocks.erase(itTmp);
	}
}

//控制分片及请求
void *CDownloader::run()
{
	while (!m_bExit)
	{
		SplitFileSizeIntoBlockMap();


	}

	return NULL;
}

//停止
void CDownloader::Stop()
{
	m_bExit = true;
}

//对文件进行分片
bool CDownloader::SplitFileSizeIntoBlockMap()
{
	//分片不足一半时重新分配
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

//获取一批分块
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
		itTmp->second = GetTickCount();        //已分配标记
	}

	return true;
}