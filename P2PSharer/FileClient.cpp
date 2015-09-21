#include "stdafx.h"
#include "FileClient.h"


CFileClient::CFileClient()
{
	m_dwFileSize = 0;
	memset(m_md5, 0, 33);
	m_bStop = false;
}


CFileClient::~CFileClient()
{
}


//��ʼ���ļ���Ϣ
bool CFileClient::Init(acl::ofstream &filestream, char *md5, DWORD filesize)
{
	m_fstream = &filestream;
	memcpy(m_md5, md5, 33);
	m_dwFileSize = filesize;

	return true;
}

//��������,�����ڴ��ɵ��������룬��ģ�鴦����Ϻ��ͷ�
void CFileClient::CacheData(void *data)
{
	m_lockvdata.lock();
	m_vdata.push_back(data);
	m_lockvdata.unlock();
}

//�ӻ���ȡ����
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

//ֹͣ�������ļ����Ѵ�����Ϣ
void CFileClient::Stop()
{
	m_bStop = true;
}

void *CFileClient::run()
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

		ShowMsg("�������յ������ݷֿ�д���ļ���");

		BLOCK_DATA_INFO *block = (BLOCK_DATA_INFO *)data;
		if (!_stricmp(m_md5, block->md5))
		{
			m_fstream->fseek(block->dwBlockNumber * EACH_BLOCK_SIZE, SEEK_SET);
			int iret = m_fstream->write((void*)block->data, block->datalen, true);

			acl::string errmsg;
			errmsg.format("д��%d���ֽ�", iret);
			ShowMsg(errmsg);
		}

		delete block;
	}

	//�ر��ļ����´λ����´򿪽���д
	m_fstream->close();

	return NULL;
}