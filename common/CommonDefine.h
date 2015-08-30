
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
	eMSG_REQFILE,                  // Э��������ļ�
	eMSG_REQFILEACK,               // ͬ���ļ�����
	eMSG_GETBLOCKS,                // �������������ݵ���Ϣ
	eMSG_GETBLOCKSACK,             // ͬ�����������
	eMSG_USERLOGOUT,			   // ֪ͨserver�û��˳�
	eMSG_USERACTIVEQUERY,		   // ��ѯ�û��Ƿ񻹴���
};

// Flag���壨��ͬʱ�������̨�ͻ��˽������ʸ���Ŀ��IP��
#define FORMAT_FLAG_P2PCONN "P2P_CONNECT_%s"           // ��״̬��� �������Է���ַ
#define FORMAT_FLAG_P2PDATA "P2P_DATA_%s"              // ��������״̬��� �������Է���ַ
#define FORMAT_FLAG_REQFILE "REQ_FILE_%s_%d"           // ����Э�������ļ��ı�� �������Է���ַ���ļ�ID
#define FORMAT_FLAG_GETBLOCKS "GET_BLOCKS_%s_%d"       // �����������ؿ�ı�� �������Է���ַ���ļ�ID


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
		char ConnToAddr[MAX_ADDR_LENGTH];           //���ӵ�Ŀ���ַ

		TMSG_P2PCONNECT(const Peer_Info& rPeerInfo)
			: TMSG_HEADER(eMSG_P2PCONNECT)
		{
			memset(ConnToAddr, 0, MAX_ADDR_LENGTH);
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

	// client�յ���һ��client���͵����������ļ���Ϣ
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

	// client�յ���һ��client���͵����������ļ���Ϣ
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

	// client�յ���һ��client���͵��������ؿ���Ϣ
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
#endif // __COMMON_DEFINE_H__