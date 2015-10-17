

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
#define MAX_REQUEST_BLOCKS_COUNT  290

struct Addr_Info
{
	char IPAddr[MAX_ADDR_LENGTH];

	Addr_Info(){ memset(IPAddr, 0, MAX_ADDR_LENGTH); }
};

struct Peer_Info
{
	Addr_Info  arrAddr[MAX_ADDNUM];        // 本机所有适配器的IP地址和端口号,
	//                                      // 数组的第nAddrNum + 1个元素是本次
	//                                      // 通讯server端分配的IP地址和端口号
 	char       szMAC[MAX_MACADDR_LEN];       // MAC地址作为用户名
	int        dwActiveTime;              // 活跃时间
	int        nAddrNum;                  // 适配器数目
	char       P2PAddr[MAX_ADDR_LENGTH];
	Peer_Info();

	Peer_Info operator=(const Peer_Info& rPeerinfo);
};

struct File_Info
{
	//unsigned short nFileID;                // 文件ID，从1开始 传输数据时以此标示
	char md5[33];                          // 文件的MD5值

	File_Info();
	File_Info operator=(const File_Info& file);
};

struct File_BLOCKS
{
	//unsigned short nFileID;
	char md5[33];                          // 文件的MD5值
	DWORD block[MAX_REQUEST_BLOCKS_COUNT];       //确保包长度不超过1200
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
	Peer_Info*	GetAPeerBasedOnIP(const char* ip);
	int			GetCurrentSize();
	Peer_Info*	operator[](int nIndex);

private:
 	typedef std::list<Peer_Info> PeerInfoList;
	typedef PeerInfoList::iterator PeerInfoListIter;
	PeerInfoList	m_PeerInfoList;

	acl::locker m_lockListUser;              //客户端列表锁

};

#endif // __PEER_LIST_H__
