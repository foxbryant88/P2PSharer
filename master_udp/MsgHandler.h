#pragma once
#include <vector>
#include "acl_cpp/lib_acl.hpp"
#include "CommonDefine.h"
#include "PeerList.h"

struct RECIEVE_DATA
{
	acl::string peerAddr;
	acl::string data;
};

class CMsgHandler :
	public acl::thread
{
public:
	CMsgHandler();
	~CMsgHandler();

	//将收到的消息缓存
	void CacheMsgData(const RECIEVE_DATA &rdata);

	//消息处理线程
	void* run();

	//维护在线列表
	void MaintainUserlist();

private:
	//从缓存取数据
	bool GetMsgData(RECIEVE_DATA &rdata);

	//客户端登录消息
	void ProcUserLoginMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock);

	//客户端请求转发P2P连接消息
	void ProcP2PConnectMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock);

	//客户端注销登录消息
	void ProcLogoutMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock);

	//客户端注销登录消息
	void ProcActiveMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock);

	acl::locker m_lockMsgData; 
	acl::locker m_lockUserList;
	std::vector<RECIEVE_DATA> m_vMsgData;     //收到的数据先缓存
	PeerList m_lstOnlineUser;                 //在线客户端列表
	acl::socket_stream m_SockStream;
	bool m_bExit;
	acl::string m_errmsg;
};

