#include "stdafx.h"
#include "ServerEX.h"


static int __timeout = 10;

acl::log g_cli_exlog;

ServerEX::ServerEX()
{
	g_cli_exlog.open("mylog.log", "ServerEx");

	m_bLoginSucc = false;
	m_bExit = false;
}


ServerEX::~ServerEX()
{
	m_bExit = true;
	
}

//��ʼ������UDP����¼����˵�ַ
bool ServerEX::Init(const char* addr)
{
	server_addr_ = addr;
	acl::acl_cpp_init();

	if (!m_sockstream.bind_udp("0.0.0.0:0"))
	{
		g_cli_exlog.fatal1("�󶨱���UDP�˿�ʧ�ܣ�%d", acl::last_error());
		return false;
	}

	g_cli_exlog.msg1("�󶨱���UDP�˿�:%s", m_sockstream.get_local(true));
	memcpy(m_peerInfo.IPAddr_Local, m_sockstream.get_local(true), MAX_ADDR_LENGTH);
 
	m_sockstream.set_rw_timeout(0);

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
			g_cli_exlog.error1("��ȡ����ʧ�ܣ�%s", m_sockstream.get_peer(true));
			continue;
		}

		msg = (MSGDef::TMSG_HEADER*)buf;
		switch (msg->cMsgID)
		{
		case eMSG_USERLOGINACK:
			ProcMsgUserLoginAck(msg);
			break;
		case eMSG_P2PCONNECT:
			ProcMsgP2PConnect(msg);
			break;
		case eMSG_P2PCONNECTACK:
			ProcMsgP2PConnectAck(msg, &m_sockstream);
			break;
		case eMSG_P2PDATA:
			ProcMsgP2PData(msg);
			break;
		case  eMSG_P2PDATAACK:
			break;
		case eMSG_REQFILE:
			break;
		case eMSG_REQFILEACK:
			break;
		case eMSG_GETBLOCKS:
			break;
		case eMSG_GETBLOCKSACK:
			break;
		case eMSG_USERACTIVEQUERY:
			ProcMsgUserActiveQuery(msg, &m_sockstream);
			break;
		default:
			g_cli_exlog.error1("�������Ϣ���ͣ�%d", msg->cMsgID);
			break;
		}
	}

	return NULL;
}

//��ָ����ַ��������
bool ServerEX::SendData(void *data, size_t size, acl::socket_stream *stream, const char *addr)
{
	m_lockSockStream.lock();

	if (stream->set_peer(addr))
	{
		if (stream->write(data, size) > 0)
		{
			m_lockSockStream.unlock();
			return true;
		}

		g_cli_exlog.error1("��[%s]��������ʧ��,err:%d", addr, acl::last_error());
	}
	else
    	g_cli_exlog.error1("����Զ�̵�ַ[%s]ʧ��,err:%d", addr, acl::last_error());

	m_lockSockStream.unlock();

	return false;
}

//���͵�¼��Ϣ
bool ServerEX::SendMsg_UserLogin()
{
	MSGDef::TMSG_USERLOGIN tMsgUserLogin(m_peerInfo);
	//memcpy(tMsgUserLogin.PeerInfo.P2PAddr, "test", 5);
	//memcpy(tMsgUserLogin.PeerInfo.IPAddr, "test1", 6);

	m_bLoginSucc = false;
	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(&tMsgUserLogin, sizeof(tMsgUserLogin), &m_sockstream, server_addr_))
		{
			for (int iWait = 0; iWait < 20; iWait++)
			{
				if (m_bLoginSucc)
				{
					g_cli_exlog.msg1("��¼�ɹ���");
					return true;
				}
				Sleep(100);
			}
		}

		Sleep(1000);
	}

	m_errmsg.format("��%s���͵�¼��Ϣʧ��,err:%d", m_sockstream.get_peer(true), acl::last_error());
	MessageBox(NULL, m_errmsg, "client", MB_OK);
	return false;
}

//��¼ȷ����Ϣ
void ServerEX::ProcMsgUserLoginAck(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_USERLOGINACK *msg = (MSGDef::TMSG_USERLOGINACK*)data;

	m_peerInfo = msg->PeerInfo;

	m_bLoginSucc = true;

	m_errmsg.format("��¼�ɹ�������IP��%s", m_peerInfo.IPAddr);
	g_cli_exlog.msg1(m_errmsg);
	MessageBox(NULL, m_errmsg, "client", MB_OK);
}

//��������ת��P2P������
bool ServerEX::SendMsg_P2PConnect(const char *addr)
{
	MSGDef::TMSG_P2PCONNECT tMsgConnect(m_peerInfo);
	memcpy(tMsgConnect.ConnToAddr, addr, strlen(addr));

	//��ʼ�����ͱ��
	acl::string flag;
	flag.format(FORMAT_FLAG_P2PCONN, addr);
	m_mapFlags[flag] = 0;

	//����Ŀ�귢�ʹ�
	for (int i = 0; i < 3; i++)
	{
		if (SendData(&tMsgConnect, sizeof(tMsgConnect), &m_sockstream, addr))
			break;
	}

	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(&tMsgConnect, sizeof(tMsgConnect), &m_sockstream, server_addr_) && WaitFlag(flag))
		{
			g_cli_exlog.msg1("������[%s]����P2P��ת����Ϣ�ɹ�", server_addr_);
			return true;
		}

		Sleep(1000);
	}

	m_errmsg.format("������[%s]����P2P��ת����Ϣʧ��,err:%d", m_sockstream.get_peer(true), acl::last_error());
	MessageBox(NULL, m_errmsg, "client", MB_OK);
	return false;
}


//�յ�ȷ�ϴ򶴳ɹ�����Ϣ
void ServerEX::ProcMsgP2PConnectAck(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream)
{
	g_cli_exlog.msg1("�յ�%sȷ�ϴ򶴳ɹ�����Ϣ", stream->get_peer(true));

	MSGDef::TMSG_P2PCONNECTACK *msg = (MSGDef::TMSG_P2PCONNECTACK *)data;
	
	//�򶴳ɹ��Ŀͻ��˴����б�
	m_lockListUser.lock();
	m_lstUser.AddPeer(msg->PeerInfo);
	m_lockListUser.unlock();

	acl::string flag;
	flag.format(FORMAT_FLAG_P2PCONN, msg->PeerInfo.IPAddr);
	m_mapFlags[flag] = 1;

}

//�յ�����P2P���ӣ��򶴣�����Ϣ
void ServerEX::ProcMsgP2PConnect(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_P2PCONNECT *msg = (MSGDef::TMSG_P2PCONNECT*)data;
	m_errmsg.format("�յ�����[%s]�Ĵ�����׼���ظ���\r\n", msg->PeerInfo.IPAddr);
	g_cli_exlog.msg1(m_errmsg);
	MessageBox(NULL, m_errmsg, "OK", MB_OK);

	MSGDef::TMSG_P2PCONNECTACK tMsgP2PConnectAck(m_peerInfo);
	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(&tMsgP2PConnectAck, sizeof(tMsgP2PConnectAck), &m_sockstream, msg->PeerInfo.IPAddr))
		{
			g_cli_exlog.msg1("��[%s]�ظ�ȷ�ϳɹ�", msg->PeerInfo.IPAddr);
			return ;
		}

		//if (SendData(&tMsgP2PConnectAck, sizeof(tMsgP2PConnectAck), &m_sockstream, msg->PeerInfo.IPAddr_Local))
		//{
		//	logEx.msg1("��[%s]�ظ�ȷ�ϳɹ�", msg->PeerInfo.IPAddr);
		//	return;
		//}

		Sleep(1000);
	}

	m_errmsg.format("��¼�ɹ�������IP��%s", m_peerInfo.IPAddr);
	g_cli_exlog.msg1(m_errmsg);
	MessageBox(NULL, m_errmsg, "client", MB_OK);

}

//ѭ��������Ƿ�Ϊ1 
//�ɹ�����true ����false
bool ServerEX::WaitFlag(const acl::string &flag)
{
#define WAIT_RETRY 100

	for (int i = 0; i < WAIT_RETRY; i++)
	{
		if (m_mapFlags[flag] == 1)
		{
			return true;
		}

		Sleep(20);
	}

	return false;
}

//����P2P���ݣ�������
bool ServerEX::SendMsg_P2PData(const char *data, const char *toaddr)
{
	for (int iRetry = 0; iRetry < 3; iRetry++)
	{
		Peer_Info *peer = m_lstUser.GetAPeer(toaddr);
		if (peer != NULL)
		{
			//���Է�������
			for (int i = 0; i < 3; i++)
			{
				MSGDef::TMSG_P2PDATA tMsgP2PData(m_peerInfo);
				memcpy(tMsgP2PData.szMsg, data, strlen(data));
				if (SendData(&tMsgP2PData, sizeof(tMsgP2PData), &m_sockstream, toaddr))
				{
					g_cli_exlog.msg1("��%s�������ݳɹ���", toaddr);
					return true;
				}

				Sleep(1000);
			}

			ShowMsg("�����Ѳ����ã��������´򶴣�");
		}

		//���δ���ӻ���ʧ����ʼ��
		SendMsg_P2PConnect(toaddr);
		Sleep(500);
	}
}

//�����յ������������ͻ���������
void ServerEX::ProcMsgP2PData(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_P2PDATA *msg = (MSGDef::TMSG_P2PDATA *)data;
	MessageBox(NULL, msg->szMsg, "OK", MB_OK);
}

//�յ���ѯ�Ƿ�����Ϣ
void ServerEX::ProcMsgUserActiveQuery(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream)
{
	MSGDef::TMSG_USERACTIVEQUERY tMsgUserActive(m_peerInfo);
	//ShowMsg("�յ��������Ϣ!");

	if (!SendData(&tMsgUserActive, sizeof(tMsgUserActive), stream, server_addr_))
	{
		ShowError("�ظ������Ϣʧ�ܣ�");
	}
}