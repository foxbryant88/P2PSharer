/////////////////////////////////////
//�ļ������࣬����Խ��յ����ļ��ֿ�����ƴװ
/////////////////////////////////////
#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CFileClient :
	public acl::thread
{
public:
	CFileClient();
	~CFileClient();

	//��ʼ���ļ���Ϣ
	bool Init(acl::ofstream &files, char *md5, DWORD filesize);

	//��������,�����ڴ��ɵ��������룬��ģ�鴦����Ϻ��ͷ�
	void CacheData(void *data);

	void *run();

	//ֹͣ�������ļ����Ѵ�����Ϣ
	void Stop();

private:
	//�ӻ���ȡ����
	void *GetData();

// 	//��������
// 	void DataProcess()
	std::vector<void *> m_vdata;
	acl::locker m_lockvdata;

	acl::ofstream *m_fstream;
	char m_md5[33];
	DWORD m_dwFileSize;

	bool m_bStop;
};

