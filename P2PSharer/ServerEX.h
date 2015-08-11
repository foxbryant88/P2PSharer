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

	//初始化服务器连接
	bool InitServer(const char* addr);

	//发送在线消息
	bool SendMsg_Online();

	//发送其它。。。


private:
	acl::string addr_;
	int length_;                //数据长度
	char *req_dat_;
	char *res_dat_;
	int res_max_;
	acl::string msg_;           //错误信息
	acl::socket_stream conn_;

	//向服务端写入信息
	bool WriteDataToServer(const void* data, size_t size);
};

