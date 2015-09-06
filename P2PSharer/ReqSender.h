/////////////////////////////////////
//�ֿ����������ࣺ��������񷽷�����������
/////////////////////////////////////

#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CReqSender :
	public acl::thread
{
public:
	CReqSender();
	~CReqSender();

	//��ʼ��
	bool Init(const char *toaddr, acl::socket_stream &sock);

	//����һ�����ݿ���Ÿ�����������,�ǿ�ʱʧ�ܷ���false
	bool PushTask(std::vector<DWORD> &blockNums);

	void *run();

	//ֹͣ
	void Stop();

private:
	//ƴװҪ�����������ݿ�����ݰ�
	bool MakeRequestHeader(MSGDef::TMSG_GETBLOCKS &msg);

	acl::locker m_lockBlockNum;
	std::vector<DWORD>  m_vBlockNums;        //��Ҫ���ص����ݿ����
	acl::string m_addr;                      //Ŀ��IP��ַ
	bool m_bExit;

	acl::socket_stream *m_sock;              //�ȹ�����ͬһ��SOCKET����ͨ�����Զ�����SOCKET������������������������������
};

