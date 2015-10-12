#include "stdafx.h"
#include "MsgHandler.h"
#include "CommonDefine.h"

CMsgHandler::CMsgHandler()
{
	m_vMsgData.clear();
	m_bExit = false;
}


CMsgHandler::~CMsgHandler()
{
	m_bExit = true;
}

//��ʼ��
void CMsgHandler::Init(acl::socket_stream *Sock)
{
	m_pSock = Sock;
}

//���յ�����Ϣ����
void CMsgHandler::CacheMsgData(const RECIEVE_DATA &rdata)
{
	m_lockMsgData.lock();
	m_vMsgData.push_back(rdata);
	m_lockMsgData.unlock();
}

//�ӻ���ȡ����
bool CMsgHandler::GetMsgData(RECIEVE_DATA &rdata)
{
	if (m_vMsgData.empty())
	{
		return false;
	}

	m_lockMsgData.lock();
	rdata = m_vMsgData.at(0);
	m_vMsgData.erase(m_vMsgData.begin());
	m_lockMsgData.unlock();

	return true;
}

void* CMsgHandler::run()
{
	RECIEVE_DATA recieve;
	if (!m_SockStream.bind_udp("0.0.0.0:0"))
	{
		m_errmsg.format("bind���ض˿�ʧ��,err:%s", acl::last_serror());
		ShowError(m_errmsg);
	}

	while (!m_bExit)
	{
		if (!GetMsgData(recieve))
		{
			Sleep(20);
			continue;
		}
		
		MSGDef::TMSG_HEADER *msg = (MSGDef::TMSG_HEADER *)recieve.data.buf();
		const char *addr = (const char *)recieve.peerAddr.buf();
		if (!m_SockStream.set_peer(addr))
		{
			m_errmsg.format("����Զ�˵�ַ��%sʧ��, err:%d", recieve.peerAddr.buf(), acl::last_error());
			g_serlog.error1(m_errmsg);
			ShowError(m_errmsg);
		}

		DealMsg(msg, &m_SockStream);
	}

	return NULL;
}

//��Ϣ����
void CMsgHandler::DealMsg(MSGDef::TMSG_HEADER *msg, acl::socket_stream *sock)
{
// 	m_pSock = sock;

	switch (msg->cMsgID)
	{
	case eMSG_USERLOGIN:
		ProcUserLoginMsg(msg, sock);
		break;

	case eMSG_P2PCONNECT:
		ProcP2PConnectMsg(msg, sock);
		break;

	case eMSG_USERLOGOUT:
		ProcLogoutMsg(msg, sock);
		break;

	case eMSG_USERACTIVEQUERY:
		ProcActiveMsg(msg, sock);
		break;

	case eMSG_GETUSERCLIENTIP:
		ProcGetUserClientIP(msg, sock);
		break;

	case eMSG_GETBLOCKS2:
		ProcMsgGetBlocks(msg, sock);
		break;

	case eMsg_FILEBLOCKDATA2:
		ProcMsgFileBlockData(msg, sock);
		break;

	default:
		break;
	}
}

//�ͻ��˵�¼��Ϣ
void CMsgHandler::ProcUserLoginMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock)
{

	MSGDef::TMSG_USERLOGIN *pUserLogin = (MSGDef::TMSG_USERLOGIN*)pMsgHeader;
	 
	m_errmsg.clear();
	m_errmsg.format("�յ�%s��¼��Ϣ, ip:%s\r\n", pUserLogin->PeerInfo.szMAC, sock->get_peer(true));
	g_serlog.msg1(m_errmsg);
	printf(m_errmsg);

	acl::string ip = sock->get_peer(true);
	int nNum = pUserLogin->PeerInfo.nAddrNum;
	memcpy(pUserLogin->PeerInfo.arrAddr[nNum].IPAddr, ip.c_str(), ip.length());
	pUserLogin->PeerInfo.nAddrNum++;
	pUserLogin->PeerInfo.dwActiveTime = GetTickCount();   // ��½��ʱ��Ϊ��Ծʱ��

	//���������б�
	m_lockUserList.lock();
	if (m_lstOnlineUser.GetAPeer(pUserLogin->PeerInfo.szMAC) == NULL)
		m_lstOnlineUser.AddPeer(pUserLogin->PeerInfo);
	m_lockUserList.unlock();


	//�ظ���¼ȷ����Ϣ
	MSGDef::TMSG_USERLOGINACK tMsgUserLogAck(pUserLogin->PeerInfo);
	if (!SendData(&tMsgUserLogAck, sizeof(tMsgUserLogAck), sock, ip))
	{
		m_errmsg.format("��%s�ظ���¼ȷ����Ϣʧ�ܣ�\r\n", ip);
		g_serlog.msg1(m_errmsg);
		printf("%s", m_errmsg);
		return;
	}

	printf("�ظ����ݳɹ���Ŀ�꣺%s.��ǰ�û�����%d\r\n", ip.buf(), m_lstOnlineUser.GetCurrentSize());
}

//�ͻ�������ת��P2P����Ϣ
void CMsgHandler::ProcP2PConnectMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock)
{
	MSGDef::TMSG_P2PCONNECT *msg = (MSGDef::TMSG_P2PCONNECT *)pMsgHeader;

	m_errmsg.format("�յ�������Դ��%s, Ŀ�꣺%s", sock->get_peer(true), msg->szMAC);
	printf("%s\n", m_errmsg.buf());
	g_serlog.msg1(m_errmsg);

	//����MAC��ַ����Ŀ��IP
	Peer_Info *peer = NULL;
	m_lockUserList.lock();
	peer = m_lstOnlineUser.GetAPeer(msg->szMAC);
	m_lockUserList.unlock();

	if (peer == NULL)
	{
		m_errmsg.format("�û�[%s]�Ѿ����ߣ���ʧ�ܣ�\r\n", msg->szMAC);
		g_serlog.msg1(m_errmsg);
		printf(m_errmsg);
		return;
	}

	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(msg, sizeof(MSGDef::TMSG_P2PCONNECT), sock, peer->arrAddr[peer->nAddrNum - 1].IPAddr))
		{
			g_serlog.msg1("��[MAC:%s IP:%s]ת��P2P����Ϣ�ɹ���\r\n", msg->szMAC, peer->arrAddr[peer->nAddrNum - 1].IPAddr);
			return;
		}

		Sleep(1000);
	}
}

//�ͻ���ע����¼��Ϣ
void CMsgHandler::ProcLogoutMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock)
{
	MSGDef::TMSG_USERLOGOUT *msg = (MSGDef::TMSG_USERLOGOUT *)pMsgHeader;

	m_errmsg.clear();
	m_errmsg.format("�յ�%sע����Ϣ,ip:%s\r\n", msg->PeerInfo.szMAC, sock->get_peer(true));
	g_serlog.msg1(m_errmsg);
	printf(m_errmsg);

	m_lockUserList.lock();
	m_lstOnlineUser.DeleteAPeer(msg->PeerInfo.szMAC);
	m_lockUserList.unlock();
}

//�յ��ͻ���ȷ�ϴ����Ϣ
void CMsgHandler::ProcActiveMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock)
{
	MSGDef::TMSG_USERACTIVEQUERY *msg = (MSGDef::TMSG_USERACTIVEQUERY *)pMsgHeader;
	printf("�յ�%s���ȷ����Ϣ, IP: %s \n", msg->PeerInfo.szMAC, msg->PeerInfo.arrAddr[msg->PeerInfo.nAddrNum - 1].IPAddr);

	m_lockUserList.lock();
	Peer_Info *peer = m_lstOnlineUser.GetAPeer(msg->PeerInfo.szMAC);
	if (NULL != peer)
	{
		peer->dwActiveTime = GetTickCount();
	}
	m_lockUserList.unlock();
}

//ά�������б�
void CMsgHandler::MaintainUserlist()
{
	DWORD dwTick = GetTickCount();
	Peer_Info* pPeerInfo;
	int nCurrentsize = m_lstOnlineUser.GetCurrentSize();
	for (int i = 0; i < nCurrentsize; ++i)
	{
		if (NULL != (pPeerInfo = m_lstOnlineUser[i]))
		{
			int nNum = pPeerInfo->nAddrNum - 1;
			if (dwTick - pPeerInfo->dwActiveTime >= 15 * 1000 + 600)
			{
				printf("------ɾ�����߿ͻ���-----, MAC:%s, IP: %s \n", pPeerInfo->szMAC, pPeerInfo->arrAddr[nNum].IPAddr);
				m_lockUserList.lock();
				m_lstOnlineUser.DeleteAPeer(pPeerInfo->szMAC);
				m_lockUserList.unlock();
				--i;
			}
			else
			{
				MSGDef::TMSG_USERACTIVEQUERY tUserActiveQuery;
				//m_SockStream.set_peer(pPeerInfo->IPAddr);
				//m_SockStream.write(&tUserActiveQuery, sizeof(tUserActiveQuery));
				//printf("Sending Active Ack Message To %s\n", pPeerInfo->IPAddr);
				if (m_pSock != NULL)
				{
					if (SendData(&tUserActiveQuery, sizeof(tUserActiveQuery), m_pSock, pPeerInfo->arrAddr[nNum].IPAddr))
						printf("��ͻ���[%s]���ʹ������Ϣ\n", pPeerInfo->arrAddr[nNum].IPAddr);
				}
			}
		}
	}
}

//��ָ����ַ��������
bool CMsgHandler::SendData(void *data, size_t size, acl::socket_stream *stream, const char *addr)
{
	m_lockSocket.lock();

	if (stream->set_peer(addr))
	{
		if (stream->write(data, size) > 0)
		{
			m_lockSocket.unlock();
			return true;
		}

		g_serlog.error1("��[%s]��������ʧ��,err:%d", addr, acl::last_error());
	}
	else
		g_serlog.error1("����Զ�̵�ַ[%s]ʧ��,err:%d", addr, acl::last_error());

	m_lockSocket.unlock();
	
	return false;
}

//�յ�����ָ���ͻ���IP����Ϣ
void CMsgHandler::ProcGetUserClientIP(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock)
{
	MSGDef::TMSG_GETUSERCLIENTIP *msg = (MSGDef::TMSG_GETUSERCLIENTIP *)pMsgHeader;
	printf("�յ�����[%s] IP��ַ����Ϣ\n", msg->szMAC);
	
	Peer_Info *peer = m_lstOnlineUser.GetAPeer(msg->szMAC);
	if (NULL != peer)
	{
    	MSGDef::TMSG_GETUSERCLIENTIPACK tMsgGetClientIPAck(*peer);
		SendData(&tMsgGetClientIPAck, sizeof(tMsgGetClientIPAck), sock, msg->PeerInfo.arrAddr[msg->PeerInfo.nAddrNum - 1].IPAddr);
	}
	else
	{
		MSGDef::TMSG_GETUSERCLIENTIPACK tMsgGetClientIPAck;
		SendData(&tMsgGetClientIPAck, sizeof(tMsgGetClientIPAck), sock, msg->PeerInfo.arrAddr[msg->PeerInfo.nAddrNum - 1].IPAddr);
	}
}

//�յ������������ݿ����Ϣ(ת����
void CMsgHandler::ProcMsgGetBlocks(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream)
{
	MSGDef::TMSG_GETBLOCKS2 *msg = (MSGDef::TMSG_GETBLOCKS2 *)data;
	Peer_Info *peer = m_lstOnlineUser.GetAPeer(msg->szDestMAC);
	if (NULL != peer)
	{
		printf("�յ�ת����������������Ϣ������Ŀ��ͻ��˳ɹ���\r\n");
		SendData(msg, sizeof(MSGDef::TMSG_GETBLOCKS2), stream, peer->arrAddr[peer->nAddrNum - 1].IPAddr);
		return;
	}

	printf("�յ�ת����������������Ϣ������Ŀ��ͻ���ʧ�ܣ�\r\n");
	//system("pause");
}

//�յ��ļ���������(ת����
void CMsgHandler::ProcMsgFileBlockData(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream)
{
	MSGDef::TMSG_FILEBLOCKDATA2 *msg = (MSGDef::TMSG_FILEBLOCKDATA2 *)data;

	printf("�յ��������ݿ飬��ת����ip:%s��\r\n", msg->srcIPAddr);
	//system("pause");
	SendData(msg, sizeof(MSGDef::TMSG_FILEBLOCKDATA2), stream, msg->srcIPAddr);
}