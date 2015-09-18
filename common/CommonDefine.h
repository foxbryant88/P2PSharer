
#ifndef __COMMON_DEFINE_H__
#define __COMMON_DEFINE_H__

#include "PeerList.h"

#define MAX_TRY_NUMBER		2

#define SERVER_PORT			9999
#define MAX_PACKET_SIZE		1300

// ������Ϣ��ʶ
enum eMSG
{
	eINVALID_MSG = 100,
	eMSG_USERLOGIN,                // �û���½
	eMSG_USERLOGINACK,			   // ����ȷ���û���½����Ϣ
	eMSG_P2PCONNECT,			   // ���û���������һ���û��������ʹ���Ϣ
	eMSG_P2PCONNECTACK,            // ȷ�ϴ򶴳ɹ�����Ϣ
	eMSG_P2PDATA,				   // ��������
	eMSG_P2PDATAACK,               // ȷ���յ�����
	//eMSG_REQFILE,                  // Э��������ļ�
	//eMSG_REQFILEACK,               // ͬ���ļ�����
	eMSG_GETBLOCKS,                // �������������ݵ���Ϣ
	eMSG_GETBLOCKSACK,             // ͬ�����������
	eMSG_USERLOGOUT,			   // ֪ͨserver�û��˳�
	eMSG_USERACTIVEQUERY,		   // ��ѯ�û��Ƿ񻹴���
	eMSG_GETUSERCLIENTIP,          // ��ȡָ��MAC�û���IP��ַ
	eMSG_GETUSERCLIENTIPACK,       // �յ�����MAC�����صĶ�ӦIP��ַ
	eMsg_FILEBLOCKDATA,            // �ļ����ع����е����ݿ�
};

// Flag���壨��ͬʱ�������̨�ͻ��˽������ʸ���Ŀ��IP��
#define FORMAT_FLAG_P2PCONN "P2P_CONNECT_%s"                  // ��״̬��� �������Է���ַ
#define FORMAT_FLAG_P2PDATA "P2P_DATA_%s"                     // ��������״̬��� �������Է���ַ
#define FORMAT_FLAG_REQFILE "REQ_FILE_%s_%d"                  // ����Э�������ļ��ı�� �������Է���ַ���ļ�ID
#define FORMAT_FLAG_GETBLOCKS "GET_BLOCKS_%s_%d"              // �����������ؿ�ı�� �������Է���ַ���ļ�ID
#define FORMAT_FLAG_GETCLIENTIP "GET_CLIENT_IP_OF_MAC_%s"     // ��ȡָ��MAC��IP��ַ
#define FORMAT_FLAG_LOGIN       "LOGIN_SERVER_%s"             // ���͵�¼������

#define EACH_BLOCK_SIZE  1200                                 //�ļ���Ƭ�Ĵ�С
#define MAX_REQUEST_BLOCKS_COUNT  (EACH_BLOCK_SIZE / sizeof(DWORD))

struct BLOCK_DATA_INFO
{
	char md5[32];                          // �ļ���MD5ֵ
	DWORD dwBlockNumber;                   // ���ݿ����
	int datalen;                           // ���ݳ���
	char data[EACH_BLOCK_SIZE];

	BLOCK_DATA_INFO(){ memset(md5, 0, 32); dwBlockNumber = 0; datalen = 0; memset(data, 0, EACH_BLOCK_SIZE); };
};

class MSGDef										// ������Ϣ�Ľṹ��
{
public:
#pragma pack(1)										// ʹ�ṹ������ݰ���1�ֽ�������,ʡ�ռ�

	// ��Ϣͷ
	struct TMSG_HEADER
	{
		char    cMsgID;								// ��Ϣ��ʶ

		TMSG_HEADER(char MsgID = eINVALID_MSG)
			: cMsgID(MsgID)
		{
		}
	};

	// �û���½
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

	// ����ȷ���û���½����Ϣ
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

	// ���û���������һ���û��������ʹ���Ϣ
	struct TMSG_P2PCONNECT
		: TMSG_HEADER
	{
		Peer_Info	PeerInfo;
		char szMAC[MAX_MACADDR_LEN];           //���ӵ�Ŀ���ַ

		TMSG_P2PCONNECT(const Peer_Info& rPeerInfo)
			: TMSG_HEADER(eMSG_P2PCONNECT)
		{
			memset(szMAC, 0, MAX_MACADDR_LEN);
			PeerInfo = rPeerInfo;
		}
	};


	// ���յ��ڵ�Ĵ���Ϣ����������������P2Pͨ�ŵ�ַ
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

	// һ��client����һ��client��������
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
	
	// client�յ���һ��client���͵�����֮���ȷ��
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

	//// client�յ���һ��client���͵����������ļ���Ϣ
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
	//// client�յ���һ��client���͵����������ļ���Ϣ
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

	// client�յ���һ��client���͵��������ؿ���Ϣ
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

	// client�յ���һ��client���͵��������ؿ���Ϣ
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

	// �ļ���������е����ݿ�
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


	// ֪ͨserver�û��˳�
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

	// ��ѯ�û��Ƿ񻹴���
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

	// ����ͻ���IP
	struct TMSG_GETUSERCLIENTIP
		: TMSG_HEADER
	{
		Peer_Info PeerInfo;
		char       szMAC[MAX_MACADDR_LEN];       // Ŀ��ͻ���MAC��ַ

		TMSG_GETUSERCLIENTIP(const Peer_Info& rPeerInfo = Peer_Info())
			: TMSG_HEADER(eMSG_GETUSERCLIENTIP)
		{
			memset(szMAC, 0, MAX_MACADDR_LEN);
			PeerInfo = rPeerInfo;
		}
	};


	// ����ͻ���IP�õ��Ļظ�
	struct TMSG_GETUSERCLIENTIPACK
		: TMSG_HEADER
	{
		Peer_Info PeerInfo;
		char       szMAC[MAX_MACADDR_LEN];       // Ŀ��ͻ���MAC��ַ

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

//��ȡ����MAC��ַ
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