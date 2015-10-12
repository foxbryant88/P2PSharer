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

//初始化
void CMsgHandler::Init(acl::socket_stream *Sock)
{
	m_pSock = Sock;
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
			m_errmsg.format("设置远端地址：%s失败, err:%d", recieve.peerAddr.buf(), acl::last_error());
			g_serlog.error1(m_errmsg);
			ShowError(m_errmsg);
		}

		DealMsg(msg, &m_SockStream);
	}

	return NULL;
}

//消息处理
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

//客户端登录消息
void CMsgHandler::ProcUserLoginMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock)
{

	MSGDef::TMSG_USERLOGIN *pUserLogin = (MSGDef::TMSG_USERLOGIN*)pMsgHeader;
	 
	m_errmsg.clear();
	m_errmsg.format("收到%s登录消息, ip:%s\r\n", pUserLogin->PeerInfo.szMAC, sock->get_peer(true));
	g_serlog.msg1(m_errmsg);
	printf(m_errmsg);

	acl::string ip = sock->get_peer(true);
	int nNum = pUserLogin->PeerInfo.nAddrNum;
	memcpy(pUserLogin->PeerInfo.arrAddr[nNum].IPAddr, ip.c_str(), ip.length());
	pUserLogin->PeerInfo.nAddrNum++;
	pUserLogin->PeerInfo.dwActiveTime = GetTickCount();   // 登陆的时间为活跃时间

	//加入在线列表
	m_lockUserList.lock();
	if (m_lstOnlineUser.GetAPeer(pUserLogin->PeerInfo.szMAC) == NULL)
		m_lstOnlineUser.AddPeer(pUserLogin->PeerInfo);
	m_lockUserList.unlock();


	//回复登录确认消息
	MSGDef::TMSG_USERLOGINACK tMsgUserLogAck(pUserLogin->PeerInfo);
	if (!SendData(&tMsgUserLogAck, sizeof(tMsgUserLogAck), sock, ip))
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

	m_errmsg.format("收到打洞请求，源：%s, 目标：%s", sock->get_peer(true), msg->szMAC);
	printf("%s\n", m_errmsg.buf());
	g_serlog.msg1(m_errmsg);

	//根据MAC地址查找目标IP
	Peer_Info *peer = NULL;
	m_lockUserList.lock();
	peer = m_lstOnlineUser.GetAPeer(msg->szMAC);
	m_lockUserList.unlock();

	if (peer == NULL)
	{
		m_errmsg.format("用户[%s]已经下线，打洞失败！\r\n", msg->szMAC);
		g_serlog.msg1(m_errmsg);
		printf(m_errmsg);
		return;
	}

	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(msg, sizeof(MSGDef::TMSG_P2PCONNECT), sock, peer->arrAddr[peer->nAddrNum - 1].IPAddr))
		{
			g_serlog.msg1("向[MAC:%s IP:%s]转发P2P打洞消息成功！\r\n", msg->szMAC, peer->arrAddr[peer->nAddrNum - 1].IPAddr);
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
	printf("收到%s存活确认消息, IP: %s \n", msg->PeerInfo.szMAC, msg->PeerInfo.arrAddr[msg->PeerInfo.nAddrNum - 1].IPAddr);

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
			int nNum = pPeerInfo->nAddrNum - 1;
			if (dwTick - pPeerInfo->dwActiveTime >= 15 * 1000 + 600)
			{
				printf("------删除下线客户端-----, MAC:%s, IP: %s \n", pPeerInfo->szMAC, pPeerInfo->arrAddr[nNum].IPAddr);
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
						printf("向客户端[%s]发送存活检测消息\n", pPeerInfo->arrAddr[nNum].IPAddr);
				}
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

//收到请求指定客户端IP的消息
void CMsgHandler::ProcGetUserClientIP(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock)
{
	MSGDef::TMSG_GETUSERCLIENTIP *msg = (MSGDef::TMSG_GETUSERCLIENTIP *)pMsgHeader;
	printf("收到请求[%s] IP地址的消息\n", msg->szMAC);
	
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

//收到请求下载数据块的消息(转发）
void CMsgHandler::ProcMsgGetBlocks(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream)
{
	MSGDef::TMSG_GETBLOCKS2 *msg = (MSGDef::TMSG_GETBLOCKS2 *)data;
	Peer_Info *peer = m_lstOnlineUser.GetAPeer(msg->szDestMAC);
	if (NULL != peer)
	{
		printf("收到转发请求下载数据消息，查找目标客户端成功！\r\n");
		SendData(msg, sizeof(MSGDef::TMSG_GETBLOCKS2), stream, peer->arrAddr[peer->nAddrNum - 1].IPAddr);
		return;
	}

	printf("收到转发请求下载数据消息，查找目标客户端失败！\r\n");
	//system("pause");
}

//收到文件下载数据(转发）
void CMsgHandler::ProcMsgFileBlockData(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream)
{
	MSGDef::TMSG_FILEBLOCKDATA2 *msg = (MSGDef::TMSG_FILEBLOCKDATA2 *)data;

	printf("收到下载数据块，将转发给ip:%s！\r\n", msg->srcIPAddr);
	//system("pause");
	SendData(msg, sizeof(MSGDef::TMSG_FILEBLOCKDATA2), stream, msg->srcIPAddr);
}