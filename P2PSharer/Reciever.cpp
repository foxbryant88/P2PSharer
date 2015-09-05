#include "stdafx.h"
#include "Reciever.h"


CReciever::CReciever()
{
	m_dwFileSize = 0;
	memset(m_md5, 0, 32);
	m_bStop = false;
}


CReciever::~CReciever()
{
}


//��ʼ���ļ���Ϣ
bool CReciever::Init(acl::ofstream &filestream, char *md5, DWORD filesize)
{
	m_fstream = &filestream;
	memcpy(m_md5, md5, 32);
	m_dwFileSize = filesize;
}

//��������,�����ڴ��ɵ��������룬��ģ�鴦����Ϻ��ͷ�
void CReciever::CacheData(const char *data)
{
	m_lockvdata.lock();
	m_vdata.push_back(data);
	m_lockvdata.unlock();
}

//�ӻ���ȡ����
const char *CReciever::GetData()
{
	if (m_vdata.empty())
	{
		return NULL;
	}

	const char *data = NULL;
	m_lockvdata.lock();
	data = m_vdata.at(0);
	m_vdata.erase(m_vdata.begin());
	m_lockvdata.unlock();

	return data;
}

//ֹͣ�������ļ����Ѵ�����Ϣ
void CReciever::Stop()
{
	m_bStop = true;
}

void *CReciever::run()
{
	const char *data = NULL;
	while (!m_bStop)
	{
		data = GetData();
		if (NULL == data)
		{
			Sleep(10);
			continue;
		}

		MSGDef::TMSG_FILEBLOCKDATA *msg = (MSGDef::TMSG_FILEBLOCKDATA *)data;
		BLOCK_DATA_INFO *block = &msg->info;
		if (!_stricmp(m_md5, block->md5))
		{
			m_fstream->fseek(block->dwBlockNumber * EACH_BLOCK_SIZE, SEEK_SET);
			m_fstream->write((void*)block->data, EACH_BLOCK_SIZE, true);
		}

		delete msg;
	}

	//�ر��ļ����´λ����´򿪽���д
	m_fstream->close();

	return NULL;
}