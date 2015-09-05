

#ifndef __PEER_LIST_H__
#define __PEER_LIST_H__

#include <map>
#include <list>
// #include <Windows.h>
#include "lib_acl.h"
#include "acl_cpp\lib_acl.hpp"

#define	MAX_MACADDR_LEN		18
#define MAX_ADDNUM			5
#define MAX_ADDR_LENGTH           22


//struct Addr_Info
//{
//	unsigned short	usPort;							// 端口号
//	acl::string		dwIP;							// IP地址
//	
//	Addr_Info operator = (const Addr_Info& rAddrInfo)
//	{
//		usPort = rAddrInfo.usPort;
//		dwIP   = rAddrInfo.dwIP;
//
//		return *this;
//	}
//
//	bool operator == (const Addr_Info& rAddrInfo)
//	{
//		if (usPort == rAddrInfo.usPort && dwIP == rAddrInfo.dwIP)
//			return true;
//
//		return false;
//	}
//};

struct Peer_Info
{
	//Addr_Info  IPAddr[MAX_ADDNUM];        // 本机所有适配器的IP地址和端口号,
	//acl::string  IPAddr[MAX_ADDNUM];        // 本机所有适配器的IP地址和端口号,
	//                                      // 数组的第nAddrNum + 1个元素是本次
	//                                      // 通讯server端分配的IP地址和端口号
	//acl::string    IPAddr;
	char IPAddr[MAX_ADDR_LENGTH];
	char IPAddr_Local[MAX_ADDR_LENGTH];
 	char       szMAC[MAX_MACADDR_LEN];       // MAC地址作为用户名
	int        dwActiveTime;              // 活跃时间
	int        nAddrNum;                  // 适配器数目
	//acl::string  P2PAddr;                 // P2P连接地址
	char P2PAddr[MAX_ADDR_LENGTH];  //暂未用
	Peer_Info(); 

	Peer_Info operator=(const Peer_Info& rPeerinfo);
};

struct File_Info
{
	//unsigned short nFileID;                // 文件ID，从1开始 传输数据时以此标示
	char md5[32];                          // 文件的MD5值

	File_Info();
	File_Info operator=(const File_Info& file);
};

struct File_BLOCKS
{
	unsigned short nFileID;
	DWORD block[295];       //确保包长度不超过1200
};

//与本机相连的所有客户端列表
class PeerList
{
public:
	PeerList();
	~PeerList();

	bool		AddPeer(const Peer_Info& rPeerInfo);
	bool		DeleteAPeer(const char* macAddr);
	bool		DeleteAllPeer();
	Peer_Info*	GetAPeer(const char* macAddr);   //参数：P2P连接地址
	int			GetCurrentSize();
	Peer_Info*	operator[](int nIndex);

private:
 	typedef std::list<Peer_Info> PeerInfoList;
	typedef PeerInfoList::iterator PeerInfoListIter;
	PeerInfoList	m_PeerInfoList;
};

#endif // __PEER_LIST_H__
