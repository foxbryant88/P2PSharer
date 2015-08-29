
#ifndef __COMMON_DEFINE_H__
#define __COMMON_DEFINE_H__

#include "PeerList.h"

#define MAX_TRY_NUMBER		2

#define SERVER_PORT			9999
#define MAX_PACKET_SIZE		1300

// 各种消息标识
enum eMSG
{
	eINVALID_MSG = 100,
	eMSG_USERLOGIN,                // 用户登陆
	eMSG_USERLOGINACK,			   // 发送确认用户登陆的信息
	eMSG_P2PCONNECT,			   // 有用户请求让另一个用户向它发送打洞消息
	eMSG_P2PCONNECTACK,            // 确认打洞成功的消息
	eMSG_P2PDATA,				   // 发送数据
	eMSG_P2PDATAACK,               // 确认收到数据
	eMSG_REQFILE,                  // 协商请求的文件
	eMSG_REQFILEACK,               // 同意文件请求
	eMSG_GETBLOCKS,                // 向服务方请求块数据的信息
	eMSG_GETBLOCKSACK,             // 同意块数据请求
	eMSG_USERLOGOUT,			   // 通知server用户退出
	eMSG_USERACTIVEQUERY,		   // 查询用户是否还存在
};

// Flag定义（因同时可能与多台客户端交互，故附带目标IP）
#define FORMAT_FLAG_P2PCONN "P2P_CONNECT_%s"           // 打洞状态标记 参数：对方地址
#define FORMAT_FLAG_P2PDATA "P2P_DATA_%s"              // 发送数据状态标记 参数：对方地址
#define FORMAT_FLAG_REQFILE "REQ_FILE_%s_%d"           // 发送协商下载文件的标记 参数：对方地址、文件ID
#define FORMAT_FLAG_GETBLOCKS "GET_BLOCKS_%s_%d"       // 发送请求下载块的标记 参数：对方地址、文件ID


class MSGDef										// 定义消息的结构体
{
public:
#pragma pack(1)										// 使结构体的数据按照1字节来对齐,省空间

	// 消息头
	struct TMSG_HEADER
	{
		char    cMsgID;								// 消息标识

		TMSG_HEADER(char MsgID = eINVALID_MSG)
			: cMsgID(MsgID)
		{
		}
	};

	// 用户登陆
	struct TMSG_USERLOGIN
		: TMSG_HEADER
	{
		Peer_Info PeerInfo;

		TMSG_USERLOGIN(const Peer_Info &rPeerinfo)
			: TMSG_HEADER(eMSG_USERLOGIN)
		{
			PeerInfo = rPeerinfo;
		}
	};

	// 发送确认用户登陆的信息
	struct TMSG_USERLOGINACK
		: TMSG_HEADER
	{
		Peer_Info PeerInfo;

		TMSG_USERLOGINACK(const Peer_Info& rPeerInfo)
			: TMSG_HEADER(eMSG_USERLOGINACK)
		{
			PeerInfo = rPeerInfo;
		}
	};

	// 有用户请求让另一个用户向它发送打洞消息
	struct TMSG_P2PCONNECT
		: TMSG_HEADER
	{
		Peer_Info	PeerInfo;
		char ConnToAddr[ADDR_SIZE];           //连接的目标地址

		TMSG_P2PCONNECT(const Peer_Info& rPeerInfo)
			: TMSG_HEADER(eMSG_P2PCONNECT)
		{
			memset(ConnToAddr, 0, ADDR_SIZE);
			PeerInfo = rPeerInfo;
		}
	};


	// 接收到节点的打洞消息，在这里设置它的P2P通信地址
	struct TMSG_P2PCONNECTACK
		: TMSG_HEADER
	{
		Peer_Info PeerInfo;

		TMSG_P2PCONNECTACK(const Peer_Info& rPeerInfo)
			: TMSG_HEADER(eMSG_P2PCONNECTACK)
		{
			PeerInfo = rPeerInfo;
		}
	};

	// 一个client向另一个client发送数据
	struct TMSG_P2PDATA
		: TMSG_HEADER
	{
		Peer_Info	PeerInfo;
		char		szMsg[MAX_PACKET_SIZE - sizeof(TMSG_HEADER)-sizeof(Peer_Info)];

		TMSG_P2PDATA(const Peer_Info& rPeerInfo)
			: TMSG_HEADER(eMSG_P2PDATA)
		{
			PeerInfo = rPeerInfo;
			memset(szMsg, 0, MAX_PACKET_SIZE - sizeof(TMSG_HEADER)-sizeof(PeerInfo));
		}
	};
	
	// client收到另一个client发送的数据之后的确认
	struct TMSG_P2PDATAACK
		: TMSG_HEADER
	{
		Peer_Info PeerInfo;

		TMSG_P2PDATAACK(const Peer_Info& rPeerInfo)
			: TMSG_HEADER(eMSG_P2PDATAACK)
		{
			PeerInfo = rPeerInfo;
		}
	};

	// client收到另一个client发送的请求下载文件消息
	struct TMSG_REQFILE
		: TMSG_HEADER
	{
		File_Info FileInfo;
		TMSG_REQFILE(const File_Info &rFileInfo)
			: TMSG_HEADER(eMSG_REQFILE)
		{
			FileInfo = rFileInfo;
		}
	};

	// client收到另一个client发送的请求下载文件消息
	struct TMSG_REQFILEACK
		: TMSG_HEADER
	{
		File_Info FileInfo;
		TMSG_REQFILEACK(const File_Info &rFileInfo)
			: TMSG_HEADER(eMSG_REQFILEACK)
		{
			FileInfo = rFileInfo;
		}
	};

	// client收到另一个client发送的请求下载块消息
	struct TMSG_GETBLOCKS
		: TMSG_HEADER
	{
		File_BLOCKS FileBlock;
		TMSG_GETBLOCKS(const File_BLOCKS &rFileBlock)
			: TMSG_HEADER(eMSG_GETBLOCKS)
		{
			FileBlock = rFileBlock;
		}
	};

	// client收到另一个client发送的请求下载块消息
	struct TMSG_GETBLOCKSACK
		: TMSG_HEADER
	{
		File_BLOCKS FileBlock;
		TMSG_GETBLOCKSACK(const File_BLOCKS &rFileBlock)
			: TMSG_HEADER(eMSG_GETBLOCKSACK)
		{
			FileBlock = rFileBlock;
		}
	};


	// 通知server用户退出
	struct TMSG_USERLOGOUT
		: TMSG_HEADER
	{
		Peer_Info PeerInfo;
		TMSG_USERLOGOUT(const Peer_Info& rPeerInfo)
			: TMSG_HEADER(eMSG_USERLOGOUT)
		{
			PeerInfo = rPeerInfo;
		}
	};

	// 查询用户是否还存在
	struct TMSG_USERACTIVEQUERY
		: TMSG_HEADER
	{
		Peer_Info PeerInfo;

		TMSG_USERACTIVEQUERY(const Peer_Info& rPeerInfo = Peer_Info())
			: TMSG_HEADER(eMSG_USERACTIVEQUERY)
		{
			PeerInfo = rPeerInfo;
		}
	};

#pragma pack()
};

#endif // __COMMON_DEFINE_H__