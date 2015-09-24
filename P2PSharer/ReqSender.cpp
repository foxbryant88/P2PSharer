#include "stdafx.h"
#include "ReqSender.h"


CReqSender::CReqSender()
{
	m_bExit = false;
}


CReqSender::~CReqSender()
{
	Stop();
}

//��ʼ��
bool CReqSender::Init(const char *toaddr, acl::socket_stream &sock, acl::string &md5)
{
	m_macAddr = toaddr;
	//m_sock = &sock;
	m_fileMD5 = md5;

	return true;
}

//����һ�����ݿ���Ÿ�����������,�ǿ�ʱʧ�ܷ���false
bool CReqSender::PushTask(std::vector<DWORD> &blockNums)
{
	if (!m_vBlockNums.empty() || blockNums.size() > MAX_REQUEST_BLOCKS_COUNT)
	{
		return false;
	}

	m_lockBlockNum.lock();
	m_vBlockNums.swap(blockNums);
	m_lockBlockNum.unlock();

	return true;
}

//ƴװҪ�����������ݿ�����ݰ�
bool CReqSender::MakeRequestHeader(MSGDef::TMSG_GETBLOCKS &msg)
{
	if (m_vBlockNums.size() == 0)
	{
		return false;
	}

	m_lockBlockNum.lock();
	for (int i = 0; i < m_vBlockNums.size(); i++)
	{
		msg.FileBlock.block[i] = m_vBlockNums[i];
	}

	m_vBlockNums.clear();
	m_lockBlockNum.unlock();

	return true;
}

void *CReqSender::run()
{
	MSGDef::TMSG_GETBLOCKS tMsg;
	memcpy(tMsg.FileBlock.md5, m_fileMD5.c_str(), 33);

	while (!m_bExit)
	{
		if (!MakeRequestHeader(tMsg))
		{
			Sleep(20);
			continue;
		}

		if (!g_serEx.SendMsg_P2PData_BaseMAC(&tMsg, sizeof(tMsg), m_macAddr))
		{
			Sleep(10);
			continue;
		}

		ShowMsg("CReqSender��������ʧ��");

		//����ϴμ�¼
		memset(&tMsg.FileBlock.block[0], 0, MAX_REQUEST_BLOCKS_COUNT * sizeof(DWORD));
	}

	delete this;
	return NULL;
}

//ֹͣ
void CReqSender::Stop()
{
	m_bExit = true;
}