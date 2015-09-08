#include "stdafx.h"
#include "FileServer.h"
#include <io.h>


CFileServer::CFileServer()
{
}


CFileServer::~CFileServer()
{
	Stop();
}

//初始化，打开文件流
bool CFileServer::Init(const char *fullpath, int blocksize)
{
	m_nBlockSize = blocksize;
	if (_access(fullpath, 4) != -1)
	{
		if (m_fstream.open_read(fullpath))
		{
			return true;
		}
	}

	g_clientlog.error1("打开文件[%s]失败，err:%d", fullpath, acl::last_error());
	return false;
}

//获取指定分块的数据 length传入时表示想要获取的字节数
//返回时表示实际获取字节数
bool CFileServer::GetBlockData(DWORD dwPos, void *buf, int &len)
{
	if (m_fstream.fseek(dwPos * m_nBlockSize, SEEK_SET) >= 0)
	{
		int iRet = m_fstream.read(buf, len);
		if (iRet > 0)
		{
			len = iRet;
			return true;
		}
	}

	return false;
}

//结束
void CFileServer::Stop()
{
	if (m_fstream.opened())
	{
		m_fstream.close();
	}
}