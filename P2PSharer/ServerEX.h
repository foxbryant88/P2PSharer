#pragma once
#include "stdafx.h"
#include <iostream>
#include "lib_acl.h"
#include "acl_cpp\lib_acl.hpp"
#include "CommonDefine.h"

class ServerEX: public acl::thread
{
public:
	ServerEX();
	~ServerEX();

	//��ʼ������UDP����¼����˵�ַ
	bool Init(const char* server_addr);

	//�������ݵ��̺߳���
	void* run();

	//���͵�¼��Ϣ
	bool SendMsg_UserLogin();

	//�����������������ͻ�������
	bool SendCMD_CONN(acl::string &target);

	//��������������


private:
	//��¼ȷ����Ϣ
	void ProcMsgUserLoginAck(MSGDef::TMSG_HEADER *data);

	//�յ�����P2P���ӣ��򶴣�����Ϣ
	void ProcMsgP2PConnect(MSGDef::TMSG_HEADER *data);

	//�յ�ȷ�ϴ򶴳ɹ�����Ϣ
	void ProcMsgP2PConnectAck(MSGDef::TMSG_HEADER *data);

	//�����յ������������ͻ���������
	void ProcMsgP2PData(MSGDef::TMSG_HEADER *data);

	//�ͻ����յ����ݺ�ȷ��
	void ProcMsgP2PDataAck(MSGDef::TMSG_HEADER *data);

	//�����յ�Э���������ص��ļ�
	void ProcMsgReqFile(MSGDef::TMSG_HEADER *data);

	//Э���������ص��ļ��ɹ�
	void ProcMsgReqFileAck(MSGDef::TMSG_HEADER *data);

	//�����յ��ͻ���������Щ������
	void ProcMsgGetBlocks(MSGDef::TMSG_HEADER *data);

	//�����յ��ͻ���������Щ������
	void ProcMsgGetBlocksAck(MSGDef::TMSG_HEADER *data);

	//�յ���ѯ�Ƿ�����Ϣ
	void ProcMsgUserActiveQuery(MSGDef::TMSG_HEADER *data);


	acl::string addr_;              //����˵�ַ
	Peer_Info m_peerInfo;           //������Ϣ
	acl::string m_errmsg;           //������Ϣ
	acl::socket_stream m_sockstream;
	bool m_bExit;

	//������д����Ϣ
	bool WriteDataToServer(const void* data, size_t size);
};

