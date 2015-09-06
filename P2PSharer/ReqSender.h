/////////////////////////////////////
//分块请求下载类：控制向服务方发送下载请求
/////////////////////////////////////

#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CReqSender :
	public acl::thread
{
public:
	CReqSender();
	~CReqSender();

	//初始化
	bool Init(const char *toaddr, acl::socket_stream &sock);

	//分配一批数据块序号给本对象下载,非空时失败返回false
	bool PushTask(std::vector<DWORD> &blockNums);

	void *run();

	//停止
	void Stop();

private:
	//拼装要请求下载数据块的数据包
	bool MakeRequestHeader(MSGDef::TMSG_GETBLOCKS &msg);

	acl::locker m_lockBlockNum;
	std::vector<DWORD>  m_vBlockNums;        //需要下载的数据块序号
	acl::string m_addr;                      //目标IP地址
	bool m_bExit;

	acl::socket_stream *m_sock;              //先共享用同一个SOCKET，调通后再试独立的SOCKET？？？？？？？？？？？？？？？
};

