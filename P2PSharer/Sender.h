/////////////////////////////////////
//CSender���ڿ�������񷽷�����������
/////////////////////////////////////

#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CSender :
	public acl::thread
{
public:
	CSender();
	~CSender();

	//��ʼ��
	bool Init(const char *toaddr);

	//����һ�����ݿ���Ÿ�����������
	bool PushTask(std::vector<DWORD> &blockNums);

	void *run();

private:

};

