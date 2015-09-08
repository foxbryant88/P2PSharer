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

//��ʼ�������ļ���
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

	g_clientlog.error1("���ļ�[%s]ʧ�ܣ�err:%d", fullpath, acl::last_error());
	return false;
}

//��ȡָ���ֿ������ length����ʱ��ʾ��Ҫ��ȡ���ֽ���
//����ʱ��ʾʵ�ʻ�ȡ�ֽ���
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

//����
void CFileServer::Stop()
{
	if (m_fstream.opened())
	{
		m_fstream.close();
	}
}