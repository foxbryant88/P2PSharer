#pragma once
#include "stdafx.h"
#include <iostream>
#include "lib_acl.h"
#include "acl_cpp\lib_acl.hpp"
#include "CommonDefine.h"

class ServerEX: public acl::thread
{
public:
	ServerEX();
	~ServerEX();

	//初始化本地UDP并记录服务端地址
	bool Init(const char* server_addr);

	//接收数据的线程函数
	void* run();

	//发送登录消息
	bool SendMsg_UserLogin();

	//发送请求连接其它客户端命令
	bool SendCMD_CONN(acl::string &target);

	//发送其它。。。


private:
	//登录确认消息
	void ProcMsgUserLoginAck(MSGDef::TMSG_HEADER *data);

	//收到请求P2P连接（打洞）的消息
	void ProcMsgP2PConnect(MSGDef::TMSG_HEADER *data);

	//收到确认打洞成功的消息
	void ProcMsgP2PConnectAck(MSGDef::TMSG_HEADER *data);

	//服务方收到下载请求后，向客户发送数据
	void ProcMsgP2PData(MSGDef::TMSG_HEADER *data);

	//客户端收到数据后确认
	void ProcMsgP2PDataAck(MSGDef::TMSG_HEADER *data);

	//服务方收到协商请求下载的文件
	void ProcMsgReqFile(MSGDef::TMSG_HEADER *data);

	//协商请求下载的文件成功
	void ProcMsgReqFileAck(MSGDef::TMSG_HEADER *data);

	//服务方收到客户方请求哪些块数据
	void ProcMsgGetBlocks(MSGDef::TMSG_HEADER *data);

	//服务方收到客户方请求哪些块数据
	void ProcMsgGetBlocksAck(MSGDef::TMSG_HEADER *data);

	//收到查询是否存活消息
	void ProcMsgUserActiveQuery(MSGDef::TMSG_HEADER *data);


	acl::string addr_;              //服务端地址
	Peer_Info m_peerInfo;           //本机信息
	acl::string m_errmsg;           //错误信息
	acl::socket_stream m_sockstream;
	bool m_bExit;

	//向服务端写入信息
	bool WriteDataToServer(const void* data, size_t size);
};

