

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
//	unsigned short	usPort;							// �˿ں�
//	acl::string		dwIP;							// IP��ַ
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
	//Addr_Info  IPAddr[MAX_ADDNUM];        // ����������������IP��ַ�Ͷ˿ں�,
	//acl::string  IPAddr[MAX_ADDNUM];        // ����������������IP��ַ�Ͷ˿ں�,
	//                                      // ����ĵ�nAddrNum + 1��Ԫ���Ǳ���
	//                                      // ͨѶserver�˷����IP��ַ�Ͷ˿ں�
	//acl::string    IPAddr;
	char IPAddr[MAX_ADDR_LENGTH];
	char IPAddr_Local[MAX_ADDR_LENGTH];
 	char       szMAC[MAX_MACADDR_LEN];       // MAC��ַ��Ϊ�û���
	int        dwActiveTime;              // ��Ծʱ��
	int        nAddrNum;                  // ��������Ŀ
	//acl::string  P2PAddr;                 // P2P���ӵ�ַ
	char P2PAddr[MAX_ADDR_LENGTH];  //��δ��
	Peer_Info(); 

	Peer_Info operator=(const Peer_Info& rPeerinfo);
};

struct File_Info
{
	//unsigned short nFileID;                // �ļ�ID����1��ʼ ��������ʱ�Դ˱�ʾ
	char md5[32];                          // �ļ���MD5ֵ

	File_Info();
	File_Info operator=(const File_Info& file);
};

struct File_BLOCKS
{
	unsigned short nFileID;
	DWORD block[295];       //ȷ�������Ȳ�����1200
};

//�뱾�����������пͻ����б�
class PeerList
{
public:
	PeerList();
	~PeerList();

	bool		AddPeer(const Peer_Info& rPeerInfo);
	bool		DeleteAPeer(const char* macAddr);
	bool		DeleteAllPeer();
	Peer_Info*	GetAPeer(const char* macAddr);   //������P2P���ӵ�ַ
	int			GetCurrentSize();
	Peer_Info*	operator[](int nIndex);

private:
 	typedef std::list<Peer_Info> PeerInfoList;
	typedef PeerInfoList::iterator PeerInfoListIter;
	PeerInfoList	m_PeerInfoList;
};

#endif // __PEER_LIST_H__
