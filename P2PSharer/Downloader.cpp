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

	//创建指定大小的文件
	if (!m_fstream->create(localpath))
	{
		g_clientlog.error1("创建文件[%s]失败！err:%d", localpath, acl::last_error());
		return false;
	}
	m_fstream->fseek(m_fileInfo.filesize, SEEK_SET);
	m_fstream->write(0);
	
	m_objReciever = new CFileReciever;
	m_objReciever->Init(*m_fstream, m_fileInfo.filemd5.c_str(), m_fileInfo.filesize);
}

//增加可用下载节点
void CDownloader::AddProvider(acl::string &addr)
{
	//已存在则不添加
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

//处理收到的分块数据,转发给FileReciver
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

		//从分片列表中移除
		m_mapFileBlocks.erase(itTmp);
	}
}

//控制分片及请求
void *CDownloader::run()
{
	std::vector<DWORD> vBlocks;

	while (!m_bExit)
	{
		//存在可用的下载节点
		if (UpdateServiceProvider())
		{
			//确保分片队列中有分片
			SplitFileSizeIntoBlockMap();


			//为每个发送对象指定发送任务
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

			//处理超时的分片请求
			DealTimeoutBlockRequests();
		}
	}

	//停止接收及分片请求对象
	m_objReciever->Stop();
	for (int i = 0; i < m_vObjSender.size(); i++)
	{
		m_vObjSender[i]->Stop();
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
				//最后一个分片的处理！！！！
				//。。。。。。。。。。
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

		//分片时间戳为0表示未请求
		if (itTmp->second == 0)
		{
			blockNums.push_back(itTmp->first);
			itTmp->second = GetTickCount();        //已分配标记
		}
	}

	return true;
}

//对超过5分钟未响应的分片重置其分片时间戳为0
//以允许重新请求
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
				itTmp->second = 0;       //重新置0
			}
		}
	}
}

//每隔1分钟重新搜索一次服务节点
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