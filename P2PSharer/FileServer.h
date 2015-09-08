/////////////////////////////////////
//�ļ��ṩ���࣬�����ṩԴ�ļ��ֿ�����
/////////////////////////////////////

#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CFileServer
{
public:
	CFileServer();
	~CFileServer();

	//��ʼ�������ļ���
	bool Init(const char *fullpath, int blocksize);

	//��ȡָ���ֿ������ length����ʱ��ʾ��Ҫ��ȡ���ֽ���
	//����ʱ��ʾʵ�ʻ�ȡ�ֽ���
	bool GetBlockData(DWORD dwPos, void *buf, int &len);

	//����
	void Stop();


private:
	int m_nBlockSize;             //ÿһ�ֿ��С
	acl::ifstream m_fstream;
};

