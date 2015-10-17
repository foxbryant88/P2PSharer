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
bool CReqSender::Init(const char *toaddr, acl::socket_stream &sock, acl::string &md5, CDownloader *pNotify)
{
	m_macAddr = toaddr;
	//m_sock = &sock;
	m_fileMD5 = md5;

	m_pNotify = pNotify;
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
bool CReqSender::MakeRequestHeader2(MSGDef::TMSG_GETBLOCKS2 &msg)
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

//�������� ��ָ���Ƿ���Ҫ������ת��
bool CReqSender::SendRequest(void *data, size_t size, bool bTransmit)
{
	if (!bTransmit)
		return g_serEx.SendMsg_P2PData_BaseMAC(data, size, m_macAddr);
	else
		return g_serEx.SendMsgToServer(data, size);
}

//ֱ��P2P
//����0:�������� 1:ֱ��P2P����ʧ��
int CReqSender::P2PNoTrasmit()
{
	MSGDef::TMSG_GETBLOCKS tMsg;
	memcpy(tMsg.FileBlock.md5, m_fileMD5.c_str(), 33);

	int iRetry = 0;

	while (!m_bExit)
	{
		if (!MakeRequestHeader(tMsg))
		{
			Sleep(20);
			continue;
		}

		if (!SendRequest(&tMsg, sizeof(tMsg), false))
		{
			if (iRetry++ < MAX_SEND_DATA_RETRY)
			{
				Sleep(1000);
				continue;
			}
			else
			{
				return 1;
			}
		}

		//����ϴμ�¼
		memset(&tMsg.FileBlock.block[0], 0, MAX_REQUEST_BLOCKS_COUNT * sizeof(DWORD));
		Sleep(300);
		iRetry = 0;
	}

	return 0;
}

//����0:�������� 1:������ת��ʧ��
int CReqSender::P2PWithTransmit()
{
	MSGDef::TMSG_GETBLOCKS2 tMsg;
	memcpy(tMsg.FileBlock.md5, m_fileMD5.c_str(), 33);
	memcpy(tMsg.szDestMAC, m_macAddr.c_str(), MAX_MACADDR_LEN);
	memcpy(tMsg.srcIPAddr, g_serEx.GetNatIP(), MAX_ADDR_LENGTH);

	int iRetry = 0;

	while (!m_bExit)
	{
		if (!MakeRequestHeader2(tMsg))
		{
			Sleep(20);
			continue;
		}

		if (!SendRequest(&tMsg, sizeof(tMsg), true))
		{
			if (iRetry++ < MAX_SEND_DATA_RETRY)
			{
				Sleep(1000);
				continue;
			}
			else
			{
				return 1;
			}
		}

		//����ϴμ�¼
		memset(&tMsg.FileBlock.block[0], 0, MAX_REQUEST_BLOCKS_COUNT * sizeof(DWORD));
		iRetry = 0;
		Sleep(300);
	}

	return 0;
}

void *CReqSender::run()
{
	int ret = P2PNoTrasmit();
	if (ret == 1)
	{
		if (ShowErrorWithChoose("˫��NAT���Ͳ�֧��P2P���䣬�Ƿ�ͨ��������ת����") == IDYES)
		{
			if (P2PWithTransmit() == 0)
				goto cleanObj;

			ShowError("���������ת������ʧ��,����ֹ���أ�");
		}

		//֪ͨdownloader�Ƴ����������
		m_pNotify->RemoveFailConnSender(this);
	}

cleanObj:
	delete this;
	return NULL;
}

//ֹͣ
void CReqSender::Stop()
{
	m_bExit = true;
}