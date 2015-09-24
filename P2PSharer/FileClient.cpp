#include "stdafx.h"
#include "FileClient.h"


CFileClient::CFileClient()
{
	m_dwFileSize = 0;
	memset(m_md5, 0, 33);
	m_bStop = false;
	m_dwRecievedBlocks = 0;
}


CFileClient::~CFileClient()
{
}


//初始化文件信息
bool CFileClient::Init(acl::ofstream &filestream, char *md5, DWORD filesize, HWND hNotifyWnd)
{
	m_fstream = &filestream;
	memcpy(m_md5, md5, 33);
	m_dwFileSize = filesize;
	m_hWndProgressNotify = hNotifyWnd;

	return true;
}

//缓存数据,参数内存由调用者申请，本模块处理完毕后释放
void CFileClient::CacheData(void *data)
{
	m_lockvdata.lock();
	m_vdata.push_back(data);
	m_lockvdata.unlock();
}

//从缓存取数据
void *CFileClient::GetData()
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
void CFileClient::Stop()
{
	m_bStop = true;
}

void *CFileClient::run()
{
	void *data = NULL;
	DWORD dwTotalBlocks = ceil(m_dwFileSize / (double)EACH_BLOCK_SIZE);
	acl::string progressTip;

	while (!m_bStop)
	{
		data = GetData();
		if (NULL == data)
		{
			Sleep(10);
			continue;
		}

		ShowMsg("即将对收到的数据分块写入文件！");

		BLOCK_DATA_INFO *block = (BLOCK_DATA_INFO *)data;
		if (!_stricmp(m_md5, block->md5))
		{
			m_fstream->fseek(block->dwBlockNumber * EACH_BLOCK_SIZE, SEEK_SET);
			int iret = m_fstream->write((void*)block->data, block->datalen, true);

			m_dwRecievedBlocks++;
			float fprogress = ((double)m_dwRecievedBlocks / dwTotalBlocks) * 100;
			progressTip.format("%0.2f%%", fprogress);
			PostMessage(m_hWndProgressNotify, UM_UPDATE_DOWNLOAD_PROGRESS, (WPARAM)progressTip.c_str(), (LPARAM)m_md5);

			acl::string errmsg;
			errmsg.format("写入%d个字节", iret);
			ShowMsg(errmsg);
		}

		delete block;

		//文件接收完成
		if (m_dwRecievedBlocks == dwTotalBlocks)
		{
			//ShowError("文件下载完成！");
			break;
		}
	}

	//关闭文件，下次会重新打开接着写
	m_fstream->close();
	PostMessage(m_hWndProgressNotify, UM_DOWNLOAD_FINISHED, NULL, (LPARAM)m_md5);

	delete this;
	return NULL;
}