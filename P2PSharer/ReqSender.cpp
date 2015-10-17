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

//初始化
bool CReqSender::Init(const char *toaddr, acl::socket_stream &sock, acl::string &md5, CDownloader *pNotify)
{
	m_macAddr = toaddr;
	//m_sock = &sock;
	m_fileMD5 = md5;

	m_pNotify = pNotify;
	return true;
}

//分配一批数据块序号给本对象下载,非空时失败返回false
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

//拼装要请求下载数据块的数据包
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

//发送请求 可指定是否需要服务器转发
bool CReqSender::SendRequest(void *data, size_t size, bool bTransmit)
{
	if (!bTransmit)
		return g_serEx.SendMsg_P2PData_BaseMAC(data, size, m_macAddr);
	else
		return g_serEx.SendMsgToServer(data, size);
}

//直接P2P
//返回0:正常结束 1:直接P2P发送失败
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

		//清空上次记录
		memset(&tMsg.FileBlock.block[0], 0, MAX_REQUEST_BLOCKS_COUNT * sizeof(DWORD));
		Sleep(300);
		iRetry = 0;
	}

	return 0;
}

//返回0:正常结束 1:服务器转发失败
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

		//清空上次记录
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
		if (ShowErrorWithChoose("双方NAT类型不支持P2P传输，是否通过服务器转发？") == IDYES)
		{
			if (P2PWithTransmit() == 0)
				goto cleanObj;

			ShowError("请求服务器转发数据失败,将终止下载！");
		}

		//通知downloader移除该请求对象
		m_pNotify->RemoveFailConnSender(this);
	}

cleanObj:
	delete this;
	return NULL;
}

//停止
void CReqSender::Stop()
{
	m_bExit = true;
}