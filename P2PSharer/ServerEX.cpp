#include "stdafx.h"
#include "ServerEX.h"

static int __timeout = 10;

ServerEX::ServerEX()
{
}


ServerEX::~ServerEX()
{
}

//��ʼ������������
bool ServerEX::InitServer(const char* addr)
{
	addr_ = addr;
	acl::acl_cpp_init();

	return true;
}

//����������Ϣ
bool ServerEX::SendMsg_Online()
{
	acl::string data = "ONLINE";
	DAT_HDR req_hdr;
	req_hdr.len = htonl(data.length());
	ACL_SAFE_STRNCPY(req_hdr.cmd, data, sizeof(req_hdr.cmd));

	if (!WriteDataToServer(&req_hdr, sizeof(req_hdr)))
	{
		MessageBox(NULL, "����������Ϣʧ��!", "error", MB_OK);
		return false;
	}

	return true;
}

//������д����Ϣ
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