#pragma once
#include "stdafx.h"
#include <iostream>
#include "lib_acl.h"
#include "acl_cpp\lib_acl.hpp"
#include "CommonDefine.h"
#include "PeerList.h"
#include "FlagMgr.h"
#include "FileReciever.h"

class ServerEX: public acl::thread
{
public:
	ServerEX();
	~ServerEX();

	//��ʼ������UDP����¼����˵�ַ
	bool Init(const char* server_addr);

	//�������ݵ��̺߳���
	void* run();

	//����socket�������ã�����
	acl::socket_stream &GetSockStream();

	//���͵�¼��Ϣ
	bool SendMsg_UserLogin();

	//��������ת��P2P������
	bool SendMsg_P2PConnect(const char *mac);

	//�����ȡָ��MAC��IP��ַ���Ա���д�ͨ�ŵȲ���
	bool SendMsg_GetIPofMAC(const char *mac);

	//��������������

	//����P2P���ݣ�������
	bool SendMsg_P2PData(const char *data, const char *toaddr);

private:
	//��¼ȷ����Ϣ
	void ProcMsgUserLoginAck(MSGDef::TMSG_HEADER *data);

	//�յ�����P2P���ӣ��򶴣�����Ϣ
	void ProcMsgP2PConnect(MSGDef::TMSG_HEADER *data);

	//�յ�ȷ�ϴ򶴳ɹ�����Ϣ
	void ProcMsgP2PConnectAck(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream);

	//�����յ������������ͻ���������
	void ProcMsgP2PData(MSGDef::TMSG_HEADER *data);

	//�ͻ����յ����ݺ�ȷ��
	void ProcMsgP2PDataAck(MSGDef::TMSG_HEADER *data);

	////�����յ�Э���������ص��ļ�
	//void ProcMsgReqFile(MSGDef::TMSG_HEADER *data);

	////Э���������ص��ļ��ɹ�
	//void ProcMsgReqFileAck(MSGDef::TMSG_HEADER *data);

	//�����յ��ͻ���������Щ������
	void ProcMsgGetBlocks(MSGDef::TMSG_HEADER *data);

	//�����յ��ͻ���������Щ������
	void ProcMsgGetBlocksAck(MSGDef::TMSG_HEADER *data);

	//�յ���ѯ�Ƿ�����Ϣ
	void ProcMsgUserActiveQuery(MSGDef::TMSG_HEADER *data, acl::socket_stream *stream);

	//�յ�������ָ��MAC��IP
	void ProcMsgGetUserClientAck(MSGDef::TMSG_HEADER *data);

	//�յ��ļ���������
	void ProcMsgFileBlockData(MSGDef::TMSG_HEADER *data);

	//��ָ����ַ��������
	bool SendData(void *data, size_t size, acl::socket_stream *stream, const char *addr);


	acl::string server_addr_;              //����˵�ַ
	Peer_Info m_peerInfo;                  //������Ϣ
	acl::string m_errmsg;                  //������Ϣ

	acl::locker m_lockSockStream;
	acl::socket_stream m_sockstream;
	bool m_bExit;

	acl::locker m_lockListUser;              //�ͻ����б���
	PeerList m_lstUser;                      //���뱾�����ӵĿͻ����б�

	CFlagMgr *m_objFlagMgr;
	CFileReciever *m_objReciever;                //�ļ����ն���
};

