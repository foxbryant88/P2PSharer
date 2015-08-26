#include "stdafx.h"
#include "ServerEX.h"


static int __timeout = 10;

acl::log logEx;

ServerEX::ServerEX()
{
	logEx.open("mylog.log", "ServerEx");

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

	if (!m_sockstream.bind_udp("0.0.0.0:8888"))
	{
		logEx.fatal1("�󶨱���UDP�˿�ʧ�ܣ�%d", acl::last_error());
		return false;
	}

	logEx.msg1("�󶨱���UDP�˿�:%s", m_sockstream.get_local(true));
 
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
			g_log.error1("��ȡ����ʧ�ܣ�%s", m_sockstream.get_peer(true));
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
			break;
		default:
			g_log.error1("�������Ϣ���ͣ�%d", msg->cMsgID);
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

		logEx.error1("��[%s]��������ʧ��,err:%d", addr, acl::last_error());
	}
	else
    	logEx.error1("����Զ�̵�ַ[%s]ʧ��,err:%d", addr, acl::last_error());

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
			Sleep(200);
			if (m_bLoginSucc)
			{
				logEx.msg1("��¼�ɹ���");
				return true;
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
	g_log.msg1(m_errmsg);
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
			logEx.msg1("������[%s]����P2P��ת����Ϣ�ɹ�", server_addr_);
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
	logEx.msg1("�յ�%sȷ�ϴ򶴳ɹ�����Ϣ", stream->get_peer(true));

	MSGDef::TMSG_P2PCONNECTACK *msg = (MSGDef::TMSG_P2PCONNECTACK *)data;
	
	//�򶴳ɹ��Ŀͻ��˴����б�
	m_lockListUser.lock();
	m_lstUser.AddPeer(msg->PeerInfo);
	m_lockListUser.unlock();

	m_mapFlags[stream->get_peer(true)] = 1;
}

//�յ�����P2P���ӣ��򶴣�����Ϣ
void ServerEX::ProcMsgP2PConnect(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_P2PCONNECT *msg = (MSGDef::TMSG_P2PCONNECT*)data;

	logEx.msg1("�յ�����[%s]�Ĵ�����׼���ظ���", msg->PeerInfo.IPAddr);

	MSGDef::TMSG_P2PCONNECTACK tMsgP2PConnectAck(m_peerInfo);
	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(&tMsgP2PConnectAck, sizeof(tMsgP2PConnectAck), &m_sockstream, msg->PeerInfo.IPAddr))
		{
			logEx.msg1("��[%s]�ظ�ȷ�ϳɹ�", msg->PeerInfo.IPAddr);
			return ;
		}

		Sleep(1000);
	}

	m_errmsg.format("��¼�ɹ�������IP��%s", m_peerInfo.IPAddr);
	g_log.msg1(m_errmsg);
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
				if (SendData((void*)data, strlen(data), &m_sockstream, toaddr))
				{
					logEx.msg1("��%s�������ݳɹ���", toaddr);
					return true;
				}

				Sleep(1000);
			}
		}

		//���δ���ӻ���ʧ����ʼ��
		SendMsg_P2PConnect(toaddr);
		Sleep(500);
	}
}