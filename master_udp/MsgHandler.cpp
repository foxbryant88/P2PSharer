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
	if (!m_SockStream.bind_udp("127.0.0.1:0"))
	{
		m_errmsg.format("bind���ض˿�ʧ��,err:%s", acl::last_serror());
		MessageBox(NULL, m_errmsg, "server", MB_OK);
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
			g_runlog.error1(m_errmsg);
			MessageBox(NULL, m_errmsg, "Error", MB_OK);
		}

		switch (msg->cMsgID)
		{
		case eMSG_USERLOGIN:
			ProcUserLoginMsg(msg, m_SockStream);
			break;

		case eMSG_P2PCONNECT:
			ProcP2PConnectMsg(msg, m_SockStream);
			break;

		case eMSG_USERLOGOUT:
			ProcLogoutMsg(msg, m_SockStream);
			break;

		case eMSG_USERACTIVEQUERY:
			ProcActiveMsg(msg, m_SockStream);
			break;

		default:
			break;
		}
	}

	return NULL;
}

//�ͻ��˵�¼��Ϣ
void CMsgHandler::ProcUserLoginMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock)
{
	m_errmsg.clear();
	m_errmsg.format("�յ�%s��¼��Ϣ\r\n", sock.get_peer(true));
	g_runlog.msg1(m_errmsg);
	printf(m_errmsg);

	MSGDef::TMSG_USERLOGIN *pUserLogin = (MSGDef::TMSG_USERLOGIN*)pMsgHeader;
	 
	int nNum = pUserLogin->PeerInfo.nAddrNum;
// 	pUserLogin->PeerInfo.IPAddr[nNum] = sock->get_peer(true);
//	pUserLogin->PeerInfo.IPAddr = sock->get_peer(true);
	acl::string ip = sock.get_peer(true);
	memcpy(pUserLogin->PeerInfo.IPAddr, ip, ip.length());
	//pUserLogin->PeerInfo.nAddrNum++;
	pUserLogin->PeerInfo.dwActiveTime = GetTickCount();   // ��½��ʱ��Ϊ��Ծʱ��

	//���������б�
	m_lockUserList.lock();
	m_lstOnlineUser.AddPeer(pUserLogin->PeerInfo);
	m_lockUserList.unlock();

	//�ظ���¼ȷ����Ϣ
	MSGDef::TMSG_USERLOGINACK tMsgUserLogAck(pUserLogin->PeerInfo);
 	if (sock.write((void*)&tMsgUserLogAck, sizeof(tMsgUserLogAck), true) == -1)
	{
		m_errmsg.format("��%s�ظ���¼ȷ����Ϣʧ�ܣ�\r\n", ip);
		g_runlog.msg1(m_errmsg);
		printf("%s", m_errmsg);
	}
}

//�ͻ�������ת��P2P������Ϣ
void CMsgHandler::ProcP2PConnectMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock)
{

}

//�ͻ���ע����¼��Ϣ
void CMsgHandler::ProcLogoutMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock)
{

}

//�ͻ���ע����¼��Ϣ
void CMsgHandler::ProcActiveMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock)
{

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
			if (dwTick - pPeerInfo->dwActiveTime >= 2 * 15 * 1000 + 600)
			{
				printf("Delete A Non-Active client, IP: %s \n", pPeerInfo->IPAddr);
				m_lockUserList.lock();
				m_lstOnlineUser.DeleteAPeer(pPeerInfo->IPAddr);
				m_lockUserList.unlock();
				--i;
			}
			else
			{
				MSGDef::TMSG_USERACTIVEQUERY tUserActiveQuery;
				m_SockStream.set_peer(pPeerInfo->IPAddr);
				m_SockStream.write(&tUserActiveQuery, sizeof(tUserActiveQuery));
				printf("Sending Active Ack Message To %s\n", pPeerInfo->IPAddr);
			}
		}
	}
}