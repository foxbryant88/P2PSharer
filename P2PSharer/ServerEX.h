#pragma once
#include "define.h"
#include <iostream>
#include "lib_acl.h"
#include "acl_cpp\lib_acl.hpp"

class ServerEX
{
public:
	ServerEX();
	~ServerEX();

	//��ʼ������������
	bool InitServer(const char* addr);

	//����������Ϣ
	bool SendMsg_Online();

	//��������������


private:
	acl::string addr_;
	int length_;                //���ݳ���
	char *req_dat_;
	char *res_dat_;
	int res_max_;
	acl::string msg_;           //������Ϣ
	acl::socket_stream conn_;

	//������д����Ϣ
	bool WriteDataToServer(const void* data, size_t size);
};

