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

//初始化本地UDP并记录服务端地址
bool ServerEX::Init(const char* addr)
{
	addr_ = addr;
	acl::acl_cpp_init();

	if (!m_sockstream.bind_udp("127.0.0.1:0"))
	{
		logEx.fatal1("绑定本地UDP端口失败！%d", acl::last_error());
		return false;
	}

	logEx.msg1("绑定本地UDP端口:%s", m_sockstream.get_local(true));
 
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
			g_log.error1("读取数据失败：%s", m_sockstream.get_peer(true));
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
		//m_errmsg.format("收到来自：%s的数据：%s", m_sockstream.get_peer(true), buf);
		//MessageBox(NULL, m_errmsg, "client", MB_OK);
		//m_errmsg = "fds";
	}

	return NULL;
}

//发送登录消息
bool ServerEX::SendMsg_UserLogin()
{
	if (!m_sockstream.set_peer(addr_))
	{
		MessageBox(NULL, "设置连接服务器失败", "client", MB_OK);
		return false;
	}

	MSGDef::TMSG_USERLOGIN tMsgUserLogin(m_peerInfo);
	memcpy(tMsgUserLogin.PeerInfo.P2PAddr, "test", 5);
	memcpy(tMsgUserLogin.PeerInfo.IPAddr, "test1", 6);

	if (m_sockstream.write(&tMsgUserLogin, sizeof(tMsgUserLogin), true) == -1)
	{
		m_errmsg.format("向%s发送上线消息失败,err:%d", m_sockstream.get_peer(true), acl::last_error());
		MessageBox(NULL, m_errmsg, "client", MB_OK);
		return false;
	}

	return true;
}

//登录确认消息
void ServerEX::ProcMsgUserLoginAck(MSGDef::TMSG_HEADER *data)
{
	MSGDef::TMSG_USERLOGINACK *msg = (MSGDef::TMSG_USERLOGINACK*)data;

	m_peerInfo = msg->PeerInfo;
	m_errmsg.format("登录成功，外网IP：%s", m_peerInfo.IPAddr);
	g_log.msg1(m_errmsg);
	MessageBox(NULL, m_errmsg, "client", MB_OK);
}

//发送请求连接其它客户端命令
bool ServerEX::SendCMD_CONN(acl::string &target)
{
// 	acl::string data = "CMD_CONN";
// 	DAT_HDR req_hdr;
// 	req_hdr.len = htonl(target.length());                      //消息头中指定目标客户端地址的长度
// 	ACL_SAFE_STRNCPY(req_hdr.cmd, data, sizeof(req_hdr.cmd));
// 
// 	if (!WriteDataToServer(&req_hdr, sizeof(req_hdr)))
// 	{
// 		MessageBox(NULL, "请求P2P连接时，发送消息头失败!", "error", MB_OK);
// 		logEx.error1("请求P2P连接时，发送消息头失败!");
// 		return false;
// 	}
// 
// 	if (!WriteDataToServer((void *)target.c_str(), target.length()))
// 	{
// 		MessageBox(NULL, "请求P2P连接时，发送目标地址失败!", "error", MB_OK);
// 		logEx.error1("请求P2P连接时，发送目标地址失败!");
// 		return false;
// 	}

	return true;
}


//向服务端写入信息
bool ServerEX::WriteDataToServer(const void* data, size_t size)
{
	if (!m_sockstream.alive())
	{		
		if (!m_sockstream.set_peer(addr_))
		{
			m_errmsg.format("流对象未打开，设置服务器地址失败!");
			MessageBox(NULL, m_errmsg.c_str(), "client", MB_OK);
			return false;
		}
	}
	acl::string buf = "hello, world";
	size = buf.length();
	if (m_sockstream.write(buf, true) == -1)
	{
		m_errmsg.format("发送数据失败：%d!", acl::last_error());
		MessageBox(NULL, m_errmsg, "client", MB_OK);
		return false;
	}

	return true;
}