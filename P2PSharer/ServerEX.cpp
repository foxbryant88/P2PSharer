#include "stdafx.h"
#include "ServerEX.h"


static int __timeout = 10;

acl::log logEx;

ServerEX::ServerEX()
{
	logEx.open("mylog.log", "ServerEx");

	m_bExit = false;
}


ServerEX::~ServerEX()
{
	m_bExit = true;
}

//��ʼ������UDP����¼����˵�ַ
bool ServerEX::Init(const char* addr)
{
	addr_ = addr;
	acl::acl_cpp_init();

	if (!m_sockstream.bind_udp("127.0.0.1:0"))
	{
		logEx.fatal1("�󶨱���UDP�˿�ʧ�ܣ�%d", acl::last_error());
		return false;
	}

	logEx.msg1("�󶨱���UDP�˿�:%s", m_sockstream.get_local(true));
 
	m_sockstream.set_rw_timeout(0);

	SendMsg_UserLogin();

	return true;
}

void* ServerEX::run()
{
	char buf[1300];
	MSGDef::TMSG_HEADER *msg;

	while (!m_bExit)
	{
		memset(buf, 0, sizeof(buf));
		int iret = m_sockstream.read(buf, sizeof(buf), false);
		if (iret < 0)
		{
			g_log.error1("��ȡ����ʧ�ܣ�%s", m_sockstream.get_peer(true));
			continue;
		}

		msg = (MSGDef::TMSG_HEADER*)buf;
		switch (msg->cMsgID)
		{
		case eMSG_USERLOGINACK:
			ProcMsgUserLoginAck(msg);
			break;
		default:
			break;
		}
		//MSGDef::TMSG_USERLOGINACK *Ack = (MSGDef::TMSG_USERLOGINACK*)buf;
		//m_errmsg.format("�յ����ԣ�%s�����ݣ�%s", m_sockstream.get_peer(true), buf);
		//MessageBox(NULL, m_errmsg, "client", MB_OK);
		//m_errmsg = "fds";
	}

	return NULL;
}

//���͵�¼��Ϣ
bool ServerEX::SendMsg_UserLogin()
{
	if (!m_sockstream.set_peer(addr_))
	{
		MessageBox(NULL, "�������ӷ�����ʧ��", "client", MB_OK);
		return false;
	}

	MSGDef::TMSG_USERLOGIN tMsgUserLogin(m_peerInfo);
	memcpy(tMsgUserLogin.PeerInfo.P2PAddr, "test", 5);
	memcpy(tMsgUserLogin.PeerInfo.IPAddr, "test1", 6);

	if (m_sockstream.write(&tMsgUserLogin, sizeof(tMsgUserLogin), true) == -1)
	{
		m_errmsg.format("��%s����������Ϣʧ��,err:%d", m_sockstream.get_peer(true), acl::last_error());
		MessageBox(NULL, m_errmsg, "client", MB_OK);
		return false;
	}

	return true;
}

//��¼ȷ����Ϣ
void ServerEX::ProcMsgUserLoginAck(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_USERLOGINACK *msg = (MSGDef::TMSG_USERLOGINACK*)data;

	m_peerInfo = msg->PeerInfo;
	m_errmsg.format("��¼�ɹ�������IP��%s", m_peerInfo.IPAddr);
	g_log.msg1(m_errmsg);
	MessageBox(NULL, m_errmsg, "client", MB_OK);
}

//�����������������ͻ�������
bool ServerEX::SendCMD_CONN(acl::string &target)
{
// 	acl::string data = "CMD_CONN";
// 	DAT_HDR req_hdr;
// 	req_hdr.len = htonl(target.length());                      //��Ϣͷ��ָ��Ŀ��ͻ��˵�ַ�ĳ���
// 	ACL_SAFE_STRNCPY(req_hdr.cmd, data, sizeof(req_hdr.cmd));
// 
// 	if (!WriteDataToServer(&req_hdr, sizeof(req_hdr)))
// 	{
// 		MessageBox(NULL, "����P2P����ʱ��������Ϣͷʧ��!", "error", MB_OK);
// 		logEx.error1("����P2P����ʱ��������Ϣͷʧ��!");
// 		return false;
// 	}
// 
// 	if (!WriteDataToServer((void *)target.c_str(), target.length()))
// 	{
// 		MessageBox(NULL, "����P2P����ʱ������Ŀ���ַʧ��!", "error", MB_OK);
// 		logEx.error1("����P2P����ʱ������Ŀ���ַʧ��!");
// 		return false;
// 	}

	return true;
}


//������д����Ϣ
bool ServerEX::WriteDataToServer(const void* data, size_t size)
{
	if (!m_sockstream.alive())
	{		
		if (!m_sockstream.set_peer(addr_))
		{
			m_errmsg.format("������δ�򿪣����÷�������ַʧ��!");
			MessageBox(NULL, m_errmsg.c_str(), "client", MB_OK);
			return false;
		}
	}
	acl::string buf = "hello, world";
	size = buf.length();
	if (m_sockstream.write(buf, true) == -1)
	{
		m_errmsg.format("��������ʧ�ܣ�%d!", acl::last_error());
		MessageBox(NULL, m_errmsg, "client", MB_OK);
		return false;
	}

	return true;
}