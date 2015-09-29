/////////////////////////////////////
//�ֿ����������ࣺ��������񷽷�����������
/////////////////////////////////////

#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CDownloader;

class CReqSender :
	public acl::thread
{
public:
	CReqSender();
	~CReqSender();

	//��ʼ��
	bool Init(const char *toaddr, acl::socket_stream &sock, acl::string &md5, CDownloader *pNotify);

	//����һ�����ݿ���Ÿ�����������,�ǿ�ʱʧ�ܷ���false
	bool PushTask(std::vector<DWORD> &blockNums);

	virtual void *run();

	//ֹͣ
	void Stop();

private:
	//ƴװҪ�����������ݿ�����ݰ�
	bool MakeRequestHeader(MSGDef::TMSG_GETBLOCKS &msg);
	bool MakeRequestHeader2(MSGDef::TMSG_GETBLOCKS2 &msg);

	//�������� ��ָ���Ƿ���Ҫ������ת��
	bool SendRequest(void *data, size_t size, bool bTransmit);

	//ֱ��P2P
	int P2PNoTrasmit();
	int P2PWithTransmit();

	acl::locker m_lockBlockNum;
	std::vector<DWORD>  m_vBlockNums;        //��Ҫ���ص����ݿ����
	acl::string m_macAddr;                   //Ŀ��IP��ַ
	acl::string m_fileMD5;                   //��������󶨵��ļ�MD5���
	bool m_bExit;

	CDownloader *m_pNotify;                 //CDownloader����ָ��
	//acl::socket_stream *m_sock;              //�ȹ�����ͬһ��SOCKET����ͨ�����Զ�����SOCKET������������������������������
};

