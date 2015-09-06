#include "stdafx.h"
#include "FileReciever.h"


CFileReciever::CFileReciever()
{
	m_dwFileSize = 0;
	memset(m_md5, 0, 32);
	m_bStop = false;
}


CFileReciever::~CFileReciever()
{
}


//初始化文件信息
bool CFileReciever::Init(acl::ofstream &filestream, char *md5, DWORD filesize)
{
	m_fstream = &filestream;
	memcpy(m_md5, md5, 32);
	m_dwFileSize = filesize;
}

//缓存数据,参数内存由调用者申请，本模块处理完毕后释放
void CFileReciever::CacheData(void *data)
{
	m_lockvdata.lock();
	m_vdata.push_back(data);
	m_lockvdata.unlock();
}

//从缓存取数据
void *CFileReciever::GetData()
{
	if (m_vdata.empty())
	{
		return NULL;
	}

	void *data = NULL;
	m_lockvdata.lock();
	data = m_vdata.at(0);
	m_vdata.erase(m_vdata.begin());
	m_lockvdata.unlock();

	return data;
}

//停止，缓存文件的已处理信息
void CFileReciever::Stop()
{
	m_bStop = true;
}

void *CFileReciever::run()
{
	void *data = NULL;
	while (!m_bStop)
	{
		data = GetData();
		if (NULL == data)
		{
			Sleep(10);
			continue;
		}

		BLOCK_DATA_INFO *block = (BLOCK_DATA_INFO *)data;
		if (!_stricmp(m_md5, block->md5))
		{
			m_fstream->fseek(block->dwBlockNumber * EACH_BLOCK_SIZE, SEEK_SET);
			m_fstream->write((void*)block->data, EACH_BLOCK_SIZE, true);
		}

		delete block;
	}

	//关闭文件，下次会重新打开接着写
	m_fstream->close();

	return NULL;
}