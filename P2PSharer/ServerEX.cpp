#include "stdafx.h"
#include "ServerEX.h"


static int __timeout = 10;

acl::log logEx;

ServerEX::ServerEX()
{
	logEx.open("mylog.log", "ServerEx");
}


ServerEX::~ServerEX()
{
}

//初始化服务器连接
bool ServerEX::InitServer(const char* addr)
{
	addr_ = addr;
	acl::acl_cpp_init();

	return true;
}

//发送在线消息
bool ServerEX::SendMsg_Online()
{
	acl::string data = "MSG_ONLINE";
	DAT_HDR req_hdr;
	req_hdr.len = htonl(data.length());        //上线消息只有消息头，所以长度直接设置为消息长度
	ACL_SAFE_STRNCPY(req_hdr.cmd, data, sizeof(req_hdr.cmd));

	if (!WriteDataToServer(&req_hdr, sizeof(req_hdr)))
	{
		MessageBox(NULL, "发送在线消息失败!", "error", MB_OK);
		logEx.error1("发送在线消息失败!");

		return false;
	}

	logEx.msg1("发送在线消息成功!");

	return true;
}

//发送请求连接其它客户端命令
bool ServerEX::SendCMD_CONN(acl::string &target)
{
	acl::string data = "CMD_CONN";
	DAT_HDR req_hdr;
	req_hdr.len = htonl(target.length());                      //消息头中指定目标客户端地址的长度
	ACL_SAFE_STRNCPY(req_hdr.cmd, data, sizeof(req_hdr.cmd));

	if (!WriteDataToServer(&req_hdr, sizeof(req_hdr)))
	{
		MessageBox(NULL, "请求P2P连接时，发送消息头失败!", "error", MB_OK);
		logEx.error1("请求P2P连接时，发送消息头失败!");
		return false;
	}

	if (!WriteDataToServer((void *)target.c_str(), target.length()))
	{
		MessageBox(NULL, "请求P2P连接时，发送目标地址失败!", "error", MB_OK);
		logEx.error1("请求P2P连接时，发送目标地址失败!");
		return false;
	}

	return true;
}


//向服务端写入信息
bool ServerEX::WriteDataToServer(const void* data, size_t size)
{
	if (!conn_.alive())
	{
		if (conn_.open(addr_, __timeout, __timeout) == false)
		{
			msg_.format("connect %s error!", addr_.c_str());
			MessageBox(NULL, msg_.c_str(), "error", MB_OK);
			return false;
		}
	}

	if (conn_.write(data, size) == -1)
	{
		MessageBox(NULL, "write data to server failed!", "error", MB_OK);
		return false;
	}

	return true;
}