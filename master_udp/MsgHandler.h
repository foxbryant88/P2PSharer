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

	//��ʼ��
	void Init(acl::socket_stream *Sock);

	//���յ�����Ϣ����
	void CacheMsgData(const RECIEVE_DATA &rdata);

	//��Ϣ�����߳�
	void* run();

	//ά�������б�
	void MaintainUserlist();

	//��Ϣ����
	void DealMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);


private:
	//�ӻ���ȡ����
	bool GetMsgData(RECIEVE_DATA &rdata);

	//��ָ����ַ��������
	bool SendData(void *data, size_t size, acl::socket_stream *stream, const char *addr);

 	//�ͻ��˵�¼��Ϣ
 	void ProcUserLoginMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);

	//�ͻ�������ת��P2P����Ϣ
	void ProcP2PConnectMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);

	//�ͻ���ע����¼��Ϣ
	void ProcLogoutMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);

	//�յ��ͻ���ȷ�ϴ����Ϣ
	void ProcActiveMsg(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);

	//�յ�����ָ���ͻ���IP����Ϣ
	void ProcGetUserClientIP(MSGDef::TMSG_HEADER *pMsgHeader, acl::socket_stream *sock);

	//�յ������������ݿ����Ϣ(ת����
	void ProcMsgGetBlocks(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream);

	//�յ��ļ���������(ת����
	void ProcMsgFileBlockData(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream);

	acl::locker m_lockMsgData; 
	acl::locker m_lockUserList;
	acl::locker m_lockSocket;

	std::vector<RECIEVE_DATA> m_vMsgData;     //�յ��������Ȼ���
	PeerList m_lstOnlineUser;                 //���߿ͻ����б�
	acl::socket_stream m_SockStream;
	acl::socket_stream *m_pSock;
	bool m_bExit;
	acl::string m_errmsg;
};

