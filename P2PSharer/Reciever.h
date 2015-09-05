#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CReciever :
	public acl::thread
{
public:
	CReciever();
	~CReciever();

	//��ʼ���ļ���Ϣ
	bool Init(acl::ofstream &files, char *md5, DWORD filesize);

	//��������,�����ڴ��ɵ��������룬��ģ�鴦����Ϻ��ͷ�
	void CacheData(const char *data);

	void *run();

	//ֹͣ�������ļ����Ѵ�����Ϣ
	void Stop();

private:
	//�ӻ���ȡ����
	const char *GetData();

// 	//��������
// 	void DataProcess()
	std::vector<const char *> m_vdata;
	acl::locker m_lockvdata;

	acl::ofstream *m_fstream;
	char m_md5[32];
	DWORD m_dwFileSize;

	bool m_bStop;
};

