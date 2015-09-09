#include "stdafx.h"
#include "ServerEX.h"


static int __timeout = 10;

acl::log g_cli_exlog;

ServerEX::ServerEX()
{
	g_cli_exlog.open("serEx.log", "ServerEx");

	m_bExit = false;

	m_objFlagMgr = new CFlagMgr;
	m_objReciever = new CFileClient;
}


ServerEX::~ServerEX()
{
	m_bExit = true;
	
}

//��ʼ������UDP����¼����˵�ַ
bool ServerEX::Init(const char* addr)
{
	if (NULL == m_objFlagMgr)
	{
		g_cli_exlog.fatal1("��Ƕ�����Ϊ�գ�");
		return false;
	}

	if (NULL == m_objReciever)
	{
		g_cli_exlog.fatal1("�ļ����ն�����Ϊ�գ�");
		return false;
	}

	server_addr_ = addr;
	acl::acl_cpp_init();

	if (!m_sockstream.bind_udp("0.0.0.0:0"))
	{
		g_cli_exlog.fatal1("�󶨱���UDP�˿�ʧ�ܣ�%d", acl::last_error());
		return false;
	}

	g_cli_exlog.msg1("�󶨱���UDP�˿�:%s", m_sockstream.get_local(true));
	memcpy(m_peerInfo.IPAddr_Local, m_sockstream.get_local(true), MAX_ADDR_LENGTH);
	memcpy(m_peerInfo.szMAC, GetMacAddr().c_str(), MAX_MACADDR_LEN);

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
// 		case  eMSG_P2PDATAACK:
// 			break;
		//case eMSG_REQFILE:
		//	break;
		//case eMSG_REQFILEACK:
		//	break;
		case eMSG_GETBLOCKS:
			break;
// 		case eMSG_GETBLOCKSACK:
// 			break;
		case eMSG_USERACTIVEQUERY:
			ProcMsgUserActiveQuery(msg, &m_sockstream);
			break;
		case eMSG_GETUSERCLIENTIPACK:
			ProcMsgGetUserClientAck(msg);
			break;
		case eMsg_FILEBLOCKDATA:
			ProcMsgFileBlockData(msg);
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

	acl::string loginFlag = m_objFlagMgr->GetFlag(FORMAT_FLAG_LOGIN, server_addr_);
	m_objFlagMgr->SetFlag(loginFlag, 0);

	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(&tMsgUserLogin, sizeof(tMsgUserLogin), &m_sockstream, server_addr_))
		{
			for (int iWait = 0; iWait < 20; iWait++)
			{
				if (m_objFlagMgr->WaitFlag(loginFlag))
				{
					g_cli_exlog.msg1("��¼�ɹ���");
					m_objFlagMgr->RMFlag(loginFlag);
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

	//���õ�¼�ɹ����
	m_objFlagMgr->SetFlag(m_objFlagMgr->GetFlag(FORMAT_FLAG_LOGIN, server_addr_), 1);

	m_errmsg.format("��¼�ɹ�������IP��%s", m_peerInfo.IPAddr);
	g_cli_exlog.msg1(m_errmsg);
	//MessageBox(NULL, m_errmsg, "client", MB_OK);
}

//��������ת��P2P������
bool ServerEX::SendMsg_P2PConnect(const char *mac)
{
	Peer_Info *peer = m_lstUser.GetAPeer(mac);
	if (NULL == peer)
	{
		if (!SendMsg_GetIPofMAC(mac))
		{
			g_cli_exlog.msg1("��ȡ[%s]��IP��ַʧ�ܣ�", mac);
			return false;
		}
	}

	MSGDef::TMSG_P2PCONNECT tMsgConnect(m_peerInfo);
	memcpy(tMsgConnect.ConnToAddr, peer->IPAddr, strlen(peer->IPAddr));

	//��ʼ�����ͱ��
	acl::string flag = m_objFlagMgr->GetFlag(FORMAT_FLAG_P2PCONN, peer->IPAddr);
	m_objFlagMgr->SetFlag(flag, 0);

	//����Ŀ�귢�ʹ�
	for (int i = 0; i < 3; i++)
	{
		if (SendData(&tMsgConnect, sizeof(tMsgConnect), &m_sockstream, peer->IPAddr))
			break;
	}

	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(&tMsgConnect, sizeof(tMsgConnect), &m_sockstream, server_addr_) && m_objFlagMgr->WaitFlag(flag))
		{
			g_cli_exlog.msg1("������[%s]����P2P��ת����Ϣ�ɹ�", server_addr_);
			m_objFlagMgr->RMFlag(flag);
			return true;
		}

		Sleep(1000);
	}

	m_errmsg.format("������[%s]����P2P��ת����Ϣʧ��,err:%d", m_sockstream.get_peer(true), acl::last_error());
	MessageBox(NULL, m_errmsg, "client", MB_OK);
	return false;
}

//�����ȡָ��MAC��IP��ַ���Ա���д�ͨ�ŵȲ���
bool ServerEX::SendMsg_GetIPofMAC(const char *mac)
{
	if (NULL != m_lstUser.GetAPeer(mac))
	{
		return true;
	}

	MSGDef::TMSG_GETUSERCLIENTIP tMsgGetClientIP(m_peerInfo);
			
	memcpy(tMsgGetClientIP.szMAC, mac, strlen(mac));

	//��ʼ�����ͱ��
	acl::string flag = m_objFlagMgr->GetFlag(FORMAT_FLAG_GETCLIENTIP, mac);
	m_objFlagMgr->SetFlag(flag, 0);	

	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(&tMsgGetClientIP, sizeof(tMsgGetClientIP), &m_sockstream, server_addr_) && m_objFlagMgr->WaitFlag(flag))
		{
			g_cli_exlog.msg1("������[%s]��������[%s]��IP��ַ�ɹ�", server_addr_, mac);
			m_objFlagMgr->RMFlag(flag);

			m_errmsg.format("MAC [%s] ��IPΪ��%s", mac, m_lstUser.GetAPeer(mac)->IPAddr);
			MessageBox(NULL, m_errmsg, "OK", MB_OK);

			return true;
		}

		Sleep(1000);
	}

	m_errmsg.format("������[%s]��������[%s]��IP��ַʧ��", server_addr_, mac);
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

	//���ô򶴳ɹ����
	m_objFlagMgr->SetFlag(m_objFlagMgr->GetFlag(FORMAT_FLAG_P2PCONN, msg->PeerInfo.IPAddr), 1);
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

acl::socket_stream &ServerEX::GetSockStream()
{
	return m_sockstream;
}

//�յ�������ָ��MAC��IP
void ServerEX::ProcMsgGetUserClientAck(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_GETUSERCLIENTIPACK *msg = (MSGDef::TMSG_GETUSERCLIENTIPACK *)data;

	//�����û��б�
	m_lockListUser.lock();
	m_lstUser.AddPeer(msg->PeerInfo);
	m_lockListUser.unlock();

	//��������ɹ����
	acl::string flag = m_objFlagMgr->GetFlag(FORMAT_FLAG_GETCLIENTIP, msg->PeerInfo.szMAC);
	m_objFlagMgr->SetFlag(flag, 1);
}

//�յ��ļ���������
void ServerEX::ProcMsgFileBlockData(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_FILEBLOCKDATA *msg = (MSGDef::TMSG_FILEBLOCKDATA *)data;

	std::map<acl::string, CDownloader *>::iterator itTemp = g_mapFileDownloader.find(msg->info.md5);
	if (itTemp != g_mapFileDownloader.end())
	{
		itTemp->second->Recieve(msg);
	}
}

//�յ������������ݿ����Ϣ
void ServerEX::ProcMsgGetBlocks(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream)
{
	MSGDef::TMSG_GETBLOCKS *msg = (MSGDef::TMSG_GETBLOCKS *)data;
	void *buf = new char[EACH_BLOCK_SIZE];
	
start:

	std::map<acl::string, CFileServer *>::iterator itTmp = g_mapFileServer.find(msg->FileBlock.md5);
	if (itTmp != g_mapFileServer.end())
	{
		for (int i = 0; i < sizeof(msg->FileBlock.block) / sizeof(DWORD); i++)
		{
			DWORD dwPos = msg->FileBlock.block[i];
			if (dwPos == 0)
			{
				break;
			}

			int len = EACH_BLOCK_SIZE;
			memset(buf, 0, EACH_BLOCK_SIZE);
			if (itTmp->second->GetBlockData(dwPos, buf, len))
			{
				SendData(buf, len, stream, stream->get_peer());
			}
		}
	}
	else
	{
		acl::string fullPath;
		fullPath.url_decode(g_resourceMgr->GetFileFullPath(msg->FileBlock.md5));
		CFileServer *pFileServer = new CFileServer;
		if (!pFileServer->Init(fullPath, EACH_BLOCK_SIZE))
		{
			g_clientlog.error1("׼���ļ�[%s]ʧ��!", fullPath);
			return;
		}

		g_mapFileServer[msg->FileBlock.md5] = pFileServer;
		goto start;
	}
}
