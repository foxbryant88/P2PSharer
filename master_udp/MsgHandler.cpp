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

//将收到的消息缓存
void CMsgHandler::CacheMsgData(const RECIEVE_DATA &rdata)
{
	m_lockMsgData.lock();
	m_vMsgData.push_back(rdata);
	m_lockMsgData.unlock();
}

//从缓存取数据
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
		m_errmsg.format("bind本地端口失败,err:%s", acl::last_serror());
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
			m_errmsg.format("设置远端地址：%s失败, err:%d", recieve.peerAddr.buf(), acl::last_error());
			g_serlog.error1(m_errmsg);
			MessageBox(NULL, m_errmsg, "Error", MB_OK);
		}

		DealMsg(msg, &m_SockStream);
	}

	return NULL;
}

//消息处理
void CMsgHandler::DealMsg(MSGDef::TMSG_HEADER *msg, acl::socket_stream *sock)
{
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

	default:
		break;
	}

}

//客户端登录消息
void CMsgHandler::ProcUserLoginMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock)
{

	MSGDef::TMSG_USERLOGIN *pUserLogin = (MSGDef::TMSG_USERLOGIN*)pMsgHeader;
	 
	m_errmsg.clear();
	m_errmsg.format("收到%s登录消息, ip:%s\r\n", pUserLogin->PeerInfo.szMAC, sock->get_peer(true));
	g_serlog.msg1(m_errmsg);
	printf(m_errmsg);

	//int nNum = pUserLogin->PeerInfo.nAddrNum;
// 	pUserLogin->PeerInfo.IPAddr[nNum] = sock->get_peer(true);
//	pUserLogin->PeerInfo.IPAddr = sock->get_peer(true);
	acl::string ip = sock->get_peer(true);
	memcpy(pUserLogin->PeerInfo.IPAddr, ip, ip.length());
	//pUserLogin->PeerInfo.nAddrNum++;
	pUserLogin->PeerInfo.dwActiveTime = GetTickCount();   // 登陆的时间为活跃时间

	//加入在线列表
	m_lockUserList.lock();
	if (m_lstOnlineUser.GetAPeer(pUserLogin->PeerInfo.szMAC) == NULL)
		m_lstOnlineUser.AddPeer(pUserLogin->PeerInfo);
	m_lockUserList.unlock();

	//回复登录确认消息
	MSGDef::TMSG_USERLOGINACK tMsgUserLogAck(pUserLogin->PeerInfo);
	if (!SendData(&tMsgUserLogAck, sizeof(tMsgUserLogAck), sock, pUserLogin->PeerInfo.IPAddr))
	{
		m_errmsg.format("向%s回复登录确认消息失败！\r\n", ip);
		g_serlog.msg1(m_errmsg);
		printf("%s", m_errmsg);
		return;
	}

	printf("回复数据成功！目标：%s.当前用户数：%d\r\n", ip.buf(), m_lstOnlineUser.GetCurrentSize());
}

//客户端请求转发P2P打洞消息
void CMsgHandler::ProcP2PConnectMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock)
{
	MSGDef::TMSG_P2PCONNECT *msg = (MSGDef::TMSG_P2PCONNECT *)pMsgHeader;

	m_errmsg.format("收到来自：%s的打洞消息，目标：%s！\r\n", sock->get_peer(true), msg->ConnToAddr);
	g_serlog.msg1(m_errmsg);

	//根据MAC地址查找目标IP
	Peer_Info *peer = NULL;
	m_lockUserList.lock();
	peer = m_lstOnlineUser.GetAPeer(msg->PeerInfo.szMAC);
	m_lockUserList.unlock();

	if (peer == NULL)
	{
		m_errmsg.format("找不到%s的目标IP，打洞失败！\r\n", msg->ConnToAddr);
		g_serlog.msg1(m_errmsg);
		printf(m_errmsg);
		return;
	}

	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(msg, sizeof(MSGDef::TMSG_P2PCONNECT), sock, peer->IPAddr))
		{
			g_serlog.msg1("向[%s]转发P2P打洞消息成功！\r\n", peer->IPAddr);
			return;
		}

		Sleep(1000);
	}
}

//客户端注销登录消息
void CMsgHandler::ProcLogoutMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock)
{
	MSGDef::TMSG_USERLOGOUT *msg = (MSGDef::TMSG_USERLOGOUT *)pMsgHeader;

	m_errmsg.clear();
	m_errmsg.format("收到%s注销消息,ip:%s\r\n", msg->PeerInfo.szMAC, sock->get_peer(true));
	g_serlog.msg1(m_errmsg);
	printf(m_errmsg);

	m_lockUserList.lock();
	m_lstOnlineUser.DeleteAPeer(msg->PeerInfo.szMAC);
	m_lockUserList.unlock();
}

//收到客户端确认存活消息
void CMsgHandler::ProcActiveMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock)
{
	MSGDef::TMSG_USERACTIVEQUERY *msg = (MSGDef::TMSG_USERACTIVEQUERY *)pMsgHeader;
	printf("收到%s存活确认消息, IP: %s \n", msg->PeerInfo.szMAC, msg->PeerInfo.IPAddr);

	m_lockUserList.lock();
	Peer_Info *peer = m_lstOnlineUser.GetAPeer(msg->PeerInfo.szMAC);
	if (NULL != peer)
	{
		peer->dwActiveTime = GetTickCount();
	}
	m_lockUserList.unlock();
}

//维护在线列表
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
				printf("------删除下线客户端-----, MAC:%s, IP: %s \n", pPeerInfo->szMAC, pPeerInfo->IPAddr);
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
				if (SendData(&tUserActiveQuery, sizeof(tUserActiveQuery), &m_SockStream, pPeerInfo->IPAddr))
					printf("Sending Active Ack Message To %s\n", pPeerInfo->IPAddr);
			}
		}
	}
}

//向指定地址发送数据
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

		g_serlog.error1("向[%s]发送数据失败,err:%d", addr, acl::last_error());
	}
	else
		g_serlog.error1("设置远程地址[%s]失败,err:%d", addr, acl::last_error());

	m_lockSocket.unlock();
	
	return false;
}