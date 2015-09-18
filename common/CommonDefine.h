
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
	//eMSG_REQFILE,                  // 协商请求的文件
	//eMSG_REQFILEACK,               // 同意文件请求
	eMSG_GETBLOCKS,                // 向服务方请求块数据的信息
	eMSG_GETBLOCKSACK,             // 同意块数据请求
	eMSG_USERLOGOUT,			   // 通知server用户退出
	eMSG_USERACTIVEQUERY,		   // 查询用户是否还存在
	eMSG_GETUSERCLIENTIP,          // 获取指定MAC用户的IP地址
	eMSG_GETUSERCLIENTIPACK,       // 收到请求MAC所返回的对应IP地址
	eMsg_FILEBLOCKDATA,            // 文件下载过程中的数据块
};

// Flag定义（因同时可能与多台客户端交互，故附带目标IP）
#define FORMAT_FLAG_P2PCONN "P2P_CONNECT_%s"                  // 打洞状态标记 参数：对方地址
#define FORMAT_FLAG_P2PDATA "P2P_DATA_%s"                     // 发送数据状态标记 参数：对方地址
#define FORMAT_FLAG_REQFILE "REQ_FILE_%s_%d"                  // 发送协商下载文件的标记 参数：对方地址、文件ID
#define FORMAT_FLAG_GETBLOCKS "GET_BLOCKS_%s_%d"              // 发送请求下载块的标记 参数：对方地址、文件ID
#define FORMAT_FLAG_GETCLIENTIP "GET_CLIENT_IP_OF_MAC_%s"     // 获取指定MAC的IP地址
#define FORMAT_FLAG_LOGIN       "LOGIN_SERVER_%s"             // 发送登录请求标记

#define EACH_BLOCK_SIZE  1200                                 //文件分片的大小
#define MAX_REQUEST_BLOCKS_COUNT  (EACH_BLOCK_SIZE / sizeof(DWORD))

struct BLOCK_DATA_INFO
{
	char md5[32];                          // 文件的MD5值
	DWORD dwBlockNumber;                   // 数据块序号
	int datalen;                           // 数据长度
	char data[EACH_BLOCK_SIZE];

	BLOCK_DATA_INFO(){ memset(md5, 0, 32); dwBlockNumber = 0; datalen = 0; memset(data, 0, EACH_BLOCK_SIZE); };
};

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
		char szMAC[MAX_MACADDR_LEN];           //连接的目标地址

		TMSG_P2PCONNECT(const Peer_Info& rPeerInfo)
			: TMSG_HEADER(eMSG_P2PCONNECT)
		{
			memset(szMAC, 0, MAX_MACADDR_LEN);
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

	//// client收到另一个client发送的请求下载文件消息
	//struct TMSG_REQFILE
	//	: TMSG_HEADER
	//{
	//	File_Info FileInfo;
	//	TMSG_REQFILE(const File_Info &rFileInfo)
	//		: TMSG_HEADER(eMSG_REQFILE)
	//	{
	//		FileInfo = rFileInfo;
	//	}
	//};
	// 
	//// client收到另一个client发送的请求下载文件消息
	//struct TMSG_REQFILEACK
	//	: TMSG_HEADER
	//{
	//	File_Info FileInfo;
	//	TMSG_REQFILEACK(const File_Info &rFileInfo)
	//		: TMSG_HEADER(eMSG_REQFILEACK)
	//	{
	//		FileInfo = rFileInfo;
	//	}
	//};

	// client收到另一个client发送的请求下载块消息
	struct TMSG_GETBLOCKS
		: TMSG_HEADER
	{
		File_BLOCKS FileBlock;
		//TMSG_GETBLOCKS(const File_BLOCKS &rFileBlock)
		//	: TMSG_HEADER(eMSG_GETBLOCKS)
		//{
		//	FileBlock = rFileBlock;
		//}
		TMSG_GETBLOCKS()
			: TMSG_HEADER(eMSG_GETBLOCKS)
		{
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

	// 文件传输过程中的数据块
	struct TMSG_FILEBLOCKDATA
		: TMSG_HEADER
	{
		BLOCK_DATA_INFO info;

		TMSG_FILEBLOCKDATA() : TMSG_HEADER(eMsg_FILEBLOCKDATA){};
		TMSG_FILEBLOCKDATA(const BLOCK_DATA_INFO &rblockInfo)
			: TMSG_HEADER(eMsg_FILEBLOCKDATA)
		{
			info = rblockInfo;
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

	// 请求客户端IP
	struct TMSG_GETUSERCLIENTIP
		: TMSG_HEADER
	{
		Peer_Info PeerInfo;
		char       szMAC[MAX_MACADDR_LEN];       // 目标客户端MAC地址

		TMSG_GETUSERCLIENTIP(const Peer_Info& rPeerInfo = Peer_Info())
			: TMSG_HEADER(eMSG_GETUSERCLIENTIP)
		{
			memset(szMAC, 0, MAX_MACADDR_LEN);
			PeerInfo = rPeerInfo;
		}
	};


	// 请求客户端IP得到的回复
	struct TMSG_GETUSERCLIENTIPACK
		: TMSG_HEADER
	{
		Peer_Info PeerInfo;
		char       szMAC[MAX_MACADDR_LEN];       // 目标客户端MAC地址

		TMSG_GETUSERCLIENTIPACK(const Peer_Info& rPeerInfo = Peer_Info())
			: TMSG_HEADER(eMSG_GETUSERCLIENTIPACK)
		{
			PeerInfo = rPeerInfo;
		}
	};

#pragma pack()
};

static void ShowMsg(acl::string msg)
{
	MessageBox(NULL, msg, "OK", MB_OK);
}

static void ShowError(acl::string msg)
{
	MessageBox(NULL, msg, "Error", MB_OK);
}

//获取本机MAC地址
static acl::string GetMacAddr()
{
	char *file = "d:\\config.dat";
	char buf[30] = { 0 };
	GetPrivateProfileString("LocalInfo", "mac", "", buf, 30, file);

	acl::string pseudoMac(buf);
	if (pseudoMac == "")
	{
		srand((unsigned int)time(NULL));
		pseudoMac.format("%02X-%02X-%02X-%02X-%02X-%02X", rand() % 100, rand() % 100, rand() % 100, rand() % 100, rand() % 100, rand() % 100);
		WritePrivateProfileString("LocalInfo", "mac", pseudoMac, file);
	}

	return pseudoMac;
}

static acl::string IntToString(int val)
{
	acl::string res;
	res << val;

	return res;
}

//static char* Unicode2Utf8(const char* unicode)
//{
//	int len;
//	len = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)unicode, -1, NULL, 0, NULL, NULL);
//	char *szUtf8 = (char*)malloc(len + 1);
//	memset(szUtf8, 0, len + 1);
//	WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)unicode, -1, szUtf8, len, NULL, NULL);
//	return szUtf8;
//}
//
//static char* Ansi2Unicode(const char* str)
//{
//	int dwUnicodeLen = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
//	if (!dwUnicodeLen)
//	{
//		return strdup(str);
//	}
//	size_t num = dwUnicodeLen*sizeof(wchar_t);
//	wchar_t *pwText = (wchar_t*)malloc(num);
//	memset(pwText, 0, num);
//	MultiByteToWideChar(CP_ACP, 0, str, -1, pwText, dwUnicodeLen);
//	return (char*)pwText;
//}
//
//static char* ConvertAnsiToUtf8(const char* str)
//{
//	char* unicode = Ansi2Unicode(str);
//	char* utf8 = Unicode2Utf8(unicode);
//	free(unicode);
//	return utf8;
//}
#endif // __COMMON_DEFINE_H__