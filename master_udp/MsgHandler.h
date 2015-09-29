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

	//初始化
	void Init(acl::socket_stream *Sock);

	//将收到的消息缓存
	void CacheMsgData(const RECIEVE_DATA &rdata);

	//消息处理线程
	void* run();

	//维护在线列表
	void MaintainUserlist();

	//消息处理
	void DealMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);


private:
	//从缓存取数据
	bool GetMsgData(RECIEVE_DATA &rdata);

	//向指定地址发送数据
	bool SendData(void *data, size_t size, acl::socket_stream *stream, const char *addr);

 	//客户端登录消息
 	void ProcUserLoginMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);

	//客户端请求转发P2P打洞消息
	void ProcP2PConnectMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);

	//客户端注销登录消息
	void ProcLogoutMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);

	//收到客户端确认存活消息
	void ProcActiveMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);

	//收到请求指定客户端IP的消息
	void ProcGetUserClientIP(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);

	//收到请求下载数据块的消息(转发）
	void ProcMsgGetBlocks(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream);

	//收到文件下载数据(转发）
	void ProcMsgFileBlockData(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream);

	acl::locker m_lockMsgData; 
	acl::locker m_lockUserList;
	acl::locker m_lockSocket;

	std::vector<RECIEVE_DATA> m_vMsgData;     //收到的数据先缓存
	PeerList m_lstOnlineUser;                 //在线客户端列表
	acl::socket_stream m_SockStream;
	acl::socket_stream *m_pSock;
	bool m_bExit;
	acl::string m_errmsg;
};

