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

	//���յ�����Ϣ����
	void CacheMsgData(const RECIEVE_DATA &rdata);

	//��Ϣ�����߳�
	void* run();

	//ά�������б�
	void MaintainUserlist();

private:
	//�ӻ���ȡ����
	bool GetMsgData(RECIEVE_DATA &rdata);

	//�ͻ��˵�¼��Ϣ
	void ProcUserLoginMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock);

	//�ͻ�������ת��P2P������Ϣ
	void ProcP2PConnectMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock);

	//�ͻ���ע����¼��Ϣ
	void ProcLogoutMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock);

	//�ͻ���ע����¼��Ϣ
	void ProcActiveMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream &sock);

	acl::locker m_lockMsgData; 
	acl::locker m_lockUserList;
	std::vector<RECIEVE_DATA> m_vMsgData;     //�յ��������Ȼ���
	PeerList m_lstOnlineUser;                 //���߿ͻ����б�
	acl::socket_stream m_SockStream;
	bool m_bExit;
	acl::string m_errmsg;
};
