#include "stdafx.h"
#include "ServerEX.h"


static int __timeout = 10;

acl::log g_cli_exlog;

ServerEX::ServerEX()
{
	g_cli_exlog.open("serEx.log", "ServerEx");

	m_bExit = false;

	m_objFlagMgr = new CFlagMgr;
	
}


ServerEX::~ServerEX()
{
	m_bExit = true;
	
}

//初始化本地UDP并记录服务端地址
bool ServerEX::Init(const char* addr, const char *name)
{
	if (NULL == m_objFlagMgr)
	{
		g_cli_exlog.fatal1("标记对象不能为空！");
		return false;
	}

	server_addr_ = addr;
	acl::acl_cpp_init();

	if (!m_sockstream.bind_udp("0.0.0.0:0"))
	{
		g_cli_exlog.fatal1("绑定本地UDP端口失败！%d", acl::last_error());
		return false;
	}

	g_cli_exlog.msg1("绑定本地UDP端口:%s", m_sockstream.get_local(true));
// 	memcpy(m_peerInfo.szMAC, GetMacAddr().c_str(), MAX_MACADDR_LEN);
	memcpy(m_peerInfo.szMAC, name, MAX_MACADDR_LEN);

	//获取所有网卡IP地址
	acl::string tmpForPort = m_sockstream.get_local(true);
	char *portInfo = tmpForPort.find(":");
	GetLocalIPs(m_peerInfo, portInfo);
	m_sockstream.set_rw_timeout(0);

	return true;
}

//获取本机所有网卡IP
void ServerEX::GetLocalIPs(Peer_Info &peerInfo, acl::string portInfo)
{
	// 得到本机的IP地址
	char szHost[256];
	::gethostname(szHost, 256);

	// 得到本机所有适配器的IP地址和端口号,这些就是私有地址/端口号
	char *pIP;
	hostent* pHost = ::gethostbyname(szHost);
	for (int i = 0; i < MAX_ADDNUM - 1; ++i)
	{
		if (NULL == (pIP = pHost->h_addr_list[i]))
		{
			break;
		}
		struct in_addr tmpIp;
		memcpy(&tmpIp, pIP, pHost->h_length);
		_snprintf_s(peerInfo.arrAddr[i].IPAddr, MAX_ADDR_LENGTH, "%s%s", inet_ntoa(tmpIp), portInfo.c_str());

		++peerInfo.nAddrNum;
	}
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
			g_cli_exlog.error1("读取数据失败：%s", m_sockstream.get_peer(true));
			continue;
		}

		msg = (MSGDef::TMSG_HEADER*)buf;
		switch (msg->cMsgID)
		{
		case eMSG_USERLOGINACK:
			ProcMsgUserLoginAck(msg);
			break;
		case eMSG_P2PCONNECT:
			ProcMsgP2PConnect(msg, &m_sockstream);
			break;
		case eMSG_P2PCONNECTACK:
			ProcMsgP2PConnectAck(msg, &m_sockstream);
			break;
		case eMSG_P2PDATA:
			ProcMsgP2PData(msg);
			break;
		case eMSG_USERACTIVEQUERY:
			ProcMsgUserActiveQuery(msg, &m_sockstream);
			break;
		case eMSG_GETUSERCLIENTIPACK:
			ProcMsgGetUserClientAck(msg);
			break;
		default:
			g_cli_exlog.error1("错误的消息类型：%d", msg->cMsgID);
			break;
		}
	}

	return NULL;
}

//向指定地址发送数据
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

		g_cli_exlog.error1("向[%s]发送数据失败,err:%d", addr, acl::last_error());
	}
	else
    	g_cli_exlog.error1("设置远程地址[%s]失败,err:%d", addr, acl::last_error());

	m_lockSockStream.unlock();

	return false;
}


//发送登录消息
bool ServerEX::SendMsg_UserLogin()
{
	MSGDef::TMSG_USERLOGIN tMsgUserLogin(m_peerInfo);

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
					g_cli_exlog.msg1("登录成功！");
					m_objFlagMgr->RMFlag(loginFlag);

					cout << "登录成功！外网IP：" << m_peerInfo.arrAddr[m_peerInfo.nAddrNum - 1].IPAddr << endl;

					return true;
				}
				Sleep(100);
			}
		}

		Sleep(1000);
	}

	m_errmsg.format("向%s发送登录消息失败,err:%d", m_sockstream.get_peer(true), acl::last_error());
	//MessageBox(NULL, m_errmsg, "client", MB_OK);
	cout << m_errmsg.c_str() << endl;
	return false;
}


//登录确认消息
void ServerEX::ProcMsgUserLoginAck(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_USERLOGINACK *msg = (MSGDef::TMSG_USERLOGINACK*)data;

	m_peerInfo = msg->PeerInfo;

	//设置登录成功标记
	m_objFlagMgr->SetFlag(m_objFlagMgr->GetFlag(FORMAT_FLAG_LOGIN, server_addr_), 1);

	m_errmsg.format("登录成功，外网IP：%s", m_peerInfo.arrAddr[m_peerInfo.nAddrNum - 1].IPAddr);
	cout << m_errmsg.c_str() << endl;
	//MessageBox(NULL, m_errmsg, "client", MB_OK);
}

//请求服务端转发P2P打洞请求
//还需考虑目标客户端重新上线的情况（端口变化）!!!!!
bool ServerEX::SendMsg_P2PConnect(const char *mac)
{
	Peer_Info *peer = m_lstUser.GetAPeer(mac);
	if (NULL == peer)
	{
		if (!SendMsg_GetIPofMAC(mac))
		{
			cout << "获取IP地址失败，用户名：" << mac << endl;
			return false;
		}

		peer = m_lstUser.GetAPeer(mac);
		cout << "获取地址成功，用户名：" << mac << endl;
	}

	m_errmsg.format("准备发起打洞消息，目标：%s", peer->arrAddr[peer->nAddrNum - 1].IPAddr);
	cout << m_errmsg.c_str() << endl;


	MSGDef::TMSG_P2PCONNECT tMsgConnect(m_peerInfo);
	memcpy(tMsgConnect.szMAC, mac, strlen(mac));

	//初始化发送标记
	acl::string flag = m_objFlagMgr->GetFlag(FORMAT_FLAG_P2PCONN, peer->arrAddr[peer->nAddrNum - 1].IPAddr);
	m_objFlagMgr->SetFlag(flag, 0);

	//先向目标发送打洞
	for (int i = 0; i < 1; i++)
	{
		for (int i = 0; i < peer->nAddrNum; ++i)
		{
			SendData(&tMsgConnect, sizeof(tMsgConnect), &m_sockstream, peer->arrAddr[i].IPAddr);
		}
	}

	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(&tMsgConnect, sizeof(tMsgConnect), &m_sockstream, server_addr_) && m_objFlagMgr->WaitFlag(flag))
		{
			m_errmsg.format("向服务端[%s]发送P2P打洞转发消息成功", server_addr_);
			m_objFlagMgr->RMFlag(flag);
			cout << m_errmsg.c_str() << endl;

			return true;
		}

		Sleep(1000);
	}

	m_errmsg.format("向服务端[%s]发送P2P打洞转发消息失败,err:%d", m_sockstream.get_peer(true), acl::last_error());
	cout << m_errmsg.c_str() << endl;
	return false;
}

//请求获取指定MAC的IP地址，以便进行打洞通信等操作
bool ServerEX::SendMsg_GetIPofMAC(const char *mac)
{
	if (NULL != m_lstUser.GetAPeer(mac))
	{
		return true;
	}

	MSGDef::TMSG_GETUSERCLIENTIP tMsgGetClientIP(m_peerInfo);
			
	memcpy(tMsgGetClientIP.szMAC, mac, strlen(mac));

	//初始化发送标记
	acl::string flag = m_objFlagMgr->GetFlag(FORMAT_FLAG_GETCLIENTIP, mac);
	m_objFlagMgr->SetFlag(flag, 0);	

	for (int iRetry = 0; iRetry < MAX_TRY_NUMBER; iRetry++)
	{
		if (SendData(&tMsgGetClientIP, sizeof(tMsgGetClientIP), &m_sockstream, server_addr_) && m_objFlagMgr->WaitFlag(flag))
		{
			g_cli_exlog.msg1("向服务端[%s]发送请求[%s]的IP地址成功", server_addr_, mac);
			m_objFlagMgr->RMFlag(flag);


 			m_errmsg.format("获取[%s]的客户端信息成功！", mac);
			cout << m_errmsg.c_str() << endl;

			return true;
		}

		Sleep(1000);
	}

	m_errmsg.format("向服务端[%s]发送请求[%s]的IP地址失败", server_addr_.c_str(), mac);
	cout << m_errmsg.c_str() << endl;
	return false;

}

//请求获取指定MAC的IP地址并返回
bool ServerEX::SendMsg_GetIPofMAC(const char *mac, acl::string &ip)
{
	if (SendMsg_GetIPofMAC(mac))
	{
		Peer_Info *peer = m_lstUser.GetAPeer(mac);
		ip = peer->arrAddr[peer->nAddrNum - 1].IPAddr;

		return true;
	}

	return false;
}

//收到确认打洞成功的消息
void ServerEX::ProcMsgP2PConnectAck(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream)
{
	m_errmsg.format("----------收到%s确认打洞成功的消息", stream->get_peer(true));
	cout << m_errmsg.c_str() << endl;

	MSGDef::TMSG_P2PCONNECTACK *msg = (MSGDef::TMSG_P2PCONNECTACK *)data;
	memcpy(msg->PeerInfo.P2PAddr, stream->get_peer(true), MAX_ADDR_LENGTH);

	//仅在P2P地址变更时更新
	if (0 != strcmp(m_lstUser.GetAPeer(msg->PeerInfo.szMAC)->P2PAddr, msg->PeerInfo.P2PAddr))
	{
		//删除旧的
		m_lstUser.DeleteAPeer(msg->PeerInfo.szMAC);

		//打洞成功的客户端存入列表
		m_lstUser.AddPeer(msg->PeerInfo);
	}

	//设置打洞成功标记
	m_objFlagMgr->SetFlag(m_objFlagMgr->GetFlag(FORMAT_FLAG_P2PCONN, msg->PeerInfo.arrAddr[msg->PeerInfo.nAddrNum - 1].IPAddr), 1);
	cout << "收到打洞确认消息" << endl;

}

//收到请求P2P连接（打洞）的消息
void ServerEX::ProcMsgP2PConnect(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream)
{
	MSGDef::TMSG_P2PCONNECT *msg = (MSGDef::TMSG_P2PCONNECT*)data;
	m_errmsg.format("收到打洞请求，源：%s 准备回复！\r\n", stream->get_peer(true));
	cout << m_errmsg.c_str() << endl;

	MSGDef::TMSG_P2PCONNECTACK tMsgP2PConnectAck(m_peerInfo);
	if (!strcmp(stream->get_peer(true), server_addr_))
	{
		//服务器转发过来的，则向源请求方的所有IP回复
		for (int i = 0; i < msg->PeerInfo.nAddrNum; ++i)
		{
			acl::string ip = msg->PeerInfo.arrAddr[i].IPAddr;
			if (SendData(&tMsgP2PConnectAck, sizeof(tMsgP2PConnectAck), &m_sockstream, ip))
			{
				m_errmsg.format("向[%s]回复确认成功", ip);
			}
		}
	}
	else
	{
	    //客户端发送过来的，则直接回复
		Peer_Info *peer = m_lstUser.GetAPeer(msg->PeerInfo.szMAC);
		if (NULL != peer && strlen(peer->P2PAddr) <= 1)
		{
			memcpy(peer->P2PAddr, stream->get_peer(true), MAX_ADDR_LENGTH);
		}

		SendData(&tMsgP2PConnectAck, sizeof(tMsgP2PConnectAck), &m_sockstream, stream->get_peer(true));
	}
}


//发送P2P数据，仅测试
bool ServerEX::SendMsg_P2PData(const char *data, const char *tomac)
{
	for (int iRetry = 0; iRetry < 10; iRetry++)
	{
		Peer_Info *peer = m_lstUser.GetAPeer(tomac);
		if (peer != NULL && strlen(peer->P2PAddr) > 1)
		{
			//尝试发送数据
			MSGDef::TMSG_P2PDATA tMsgP2PData(m_peerInfo);
			memcpy(tMsgP2PData.szMsg, data, strlen(data));
			if (SendData(&tMsgP2PData, sizeof(tMsgP2PData), &m_sockstream, peer->P2PAddr))
			{
				//g_cli_exlog.msg1("向%s发送数据成功！", peer->IPAddr);
				cout << "发送数据成功，目标：" << peer->P2PAddr << endl;
				return true;
			}


			cout << "连接已不可用，尝试重新打洞！" << endl;
			//ShowMsg("连接已不可用，尝试重新打洞！");

			memcpy(peer->P2PAddr, 0, MAX_ADDR_LENGTH);
		}


		//如果未连接或发送失败则开始打洞
		SendMsg_P2PConnect(tomac);
		Sleep(300);
	}
}

bool ServerEX::SendMsg_P2PData(void *data, size_t size, const char *toMac)
{
	for (int iRetry = 0; iRetry < 3; iRetry++)
	{
		Peer_Info *peer = m_lstUser.GetAPeer(toMac);
		if (peer != NULL  && strlen(peer->P2PAddr) > 1)
		{
			//尝试发送数据
			for (int i = 0; i < 3; i++)
			{
				if (SendData(data, size, &m_sockstream, peer->P2PAddr))
				{
					g_cli_exlog.msg1("向%s  ip:%s发送数据成功！", toMac, peer->P2PAddr);
					return true;
				}

				Sleep(1000);
			}

			ShowMsg("连接已不可用，尝试重新打洞！");
		}

		//如果未连接或发送失败则开始打洞
		if (SendMsg_P2PConnect(toMac))
			continue;;

		Sleep(500);
	}

	return false;
}

//服务方收到下载请求后，向客户发送数据
void ServerEX::ProcMsgP2PData(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_P2PDATA *msg = (MSGDef::TMSG_P2PDATA *)data;
	cout << "++++++++++ 收到数据：" << msg->szMsg << endl;
}

//收到查询是否存活消息
void ServerEX::ProcMsgUserActiveQuery(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream)
{
	MSGDef::TMSG_USERACTIVEQUERY tMsgUserActive(m_peerInfo);
	//ShowMsg("收到存活检测消息!");

	if (!SendData(&tMsgUserActive, sizeof(tMsgUserActive), stream, server_addr_))
	{
		ShowError("回复存活消息失败！");
	}
}

acl::socket_stream &ServerEX::GetSockStream()
{
	return m_sockstream;
}

//收到所请求指定MAC的IP
void ServerEX::ProcMsgGetUserClientAck(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_GETUSERCLIENTIPACK *msg = (MSGDef::TMSG_GETUSERCLIENTIPACK *)data;

	//加入用户列表
	m_lstUser.AddPeer(msg->PeerInfo);

	//设置请求成功标记
	acl::string flag = m_objFlagMgr->GetFlag(FORMAT_FLAG_GETCLIENTIP, msg->PeerInfo.szMAC);
	m_objFlagMgr->SetFlag(flag, 1);

	m_errmsg.format("收到%s的客户端信息！", msg->PeerInfo.szMAC);
	cout << m_errmsg.c_str() << endl;
}
/*
//收到文件下载数据
void ServerEX::ProcMsgFileBlockData(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_FILEBLOCKDATA *msg = (MSGDef::TMSG_FILEBLOCKDATA *)data;
	ShowMsg("收到下载数据块，即将写入文件");

	std::map<acl::string, CDownloader *>::iterator itTemp = g_mapFileDownloader.find(msg->info.md5);
	if (itTemp != g_mapFileDownloader.end())
	{
		itTemp->second->Recieve(msg);
	}
}

//收到请求下载数据块的消息
void ServerEX::ProcMsgGetBlocks(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream)
{
	MSGDef::TMSG_GETBLOCKS *msg = (MSGDef::TMSG_GETBLOCKS *)data;
	static void *buf = new char[EACH_BLOCK_SIZE];

	ShowMsg("收到GetBlocks下载数据分块消息");

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
				ShowMsg("读取分块成功，即将开始传输！");

				MSGDef::TMSG_FILEBLOCKDATA tdata;
				memcpy(tdata.info.md5, msg->FileBlock.md5, 32);
				tdata.info.dwBlockNumber = dwPos;
				tdata.info.datalen = len;
				memcpy(tdata.info.data, buf, EACH_BLOCK_SIZE);

				SendData(&tdata, sizeof(tdata), stream, stream->get_peer());
			}
			else
			{
				ShowMsg("读取分块失败！");
			}
		}
	}
	else
	{
		acl::string fullPath;
		fullPath.url_decode(g_resourceMgr->GetFileFullPath(msg->FileBlock.md5));
		
		m_errmsg.format("文件下载路径为：%s", fullPath.c_str());
		ShowMsg(m_errmsg);

		CFileServer *pFileServer = new CFileServer;
		if (!pFileServer->Init(fullPath, EACH_BLOCK_SIZE))
		{
			m_errmsg.format("初始化下载文件[%s]失败!", fullPath.c_str());
			g_clientlog.error1(m_errmsg);

			ShowError(m_errmsg);
			return;
		}

		g_mapFileServer[msg->FileBlock.md5] = pFileServer;
		goto start;
	}
}
*/