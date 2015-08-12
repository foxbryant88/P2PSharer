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
	acl::string data = "MSG_ONLINE";
	DAT_HDR req_hdr;
	req_hdr.len = htonl(data.length());        //������Ϣֻ����Ϣͷ�����Գ���ֱ������Ϊ��Ϣ����
	ACL_SAFE_STRNCPY(req_hdr.cmd, data, sizeof(req_hdr.cmd));

	if (!WriteDataToServer(&req_hdr, sizeof(req_hdr)))
	{
		MessageBox(NULL, "����������Ϣʧ��!", "error", MB_OK);
		logEx.error1("����������Ϣʧ��!");

		return false;
	}

	logEx.msg1("����������Ϣ�ɹ�!");

	return true;
}

//�����������������ͻ�������
bool ServerEX::SendCMD_CONN(acl::string &target)
{
	acl::string data = "CMD_CONN";
	DAT_HDR req_hdr;
	req_hdr.len = htonl(target.length());                      //��Ϣͷ��ָ��Ŀ��ͻ��˵�ַ�ĳ���
	ACL_SAFE_STRNCPY(req_hdr.cmd, data, sizeof(req_hdr.cmd));

	if (!WriteDataToServer(&req_hdr, sizeof(req_hdr)))
	{
		MessageBox(NULL, "����P2P����ʱ��������Ϣͷʧ��!", "error", MB_OK);
		logEx.error1("����P2P����ʱ��������Ϣͷʧ��!");
		return false;
	}

	if (!WriteDataToServer((void *)target.c_str(), target.length()))
	{
		MessageBox(NULL, "����P2P����ʱ������Ŀ���ַʧ��!", "error", MB_OK);
		logEx.error1("����P2P����ʱ������Ŀ���ַʧ��!");
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