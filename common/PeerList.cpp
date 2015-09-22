#include "PeerList.h"

Peer_Info::Peer_Info()
	: dwActiveTime(0)
	, nAddrNum(0)
{
	for (int i = 0; i < MAX_ADDNUM; i++)
	{
		memset(arrAddr[i].IPAddr, 0, MAX_ADDR_LENGTH);
	}

	memset(P2PAddr, 0, MAX_ADDR_LENGTH);
	memset(szMAC, 0, MAX_MACADDR_LEN);
}

Peer_Info Peer_Info::operator=(const Peer_Info& rPeerinfo)
{
	if (&rPeerinfo == this)
		return *this;

	for (int i = 0; i < MAX_ADDNUM; ++i)
	{
		memcpy(arrAddr[i].IPAddr, rPeerinfo.arrAddr[i].IPAddr, MAX_ADDR_LENGTH);
	}

	memcpy(P2PAddr, rPeerinfo.P2PAddr, MAX_ADDR_LENGTH);
	memcpy(szMAC, rPeerinfo.szMAC, MAX_MACADDR_LEN);

	dwActiveTime = rPeerinfo.dwActiveTime;
	nAddrNum = rPeerinfo.nAddrNum;

	return *this;
}

File_Info::File_Info()
{
	//nFileID = -1;
	memset(md5, 0, sizeof(md5));
}

File_Info File_Info::operator=(const File_Info& file)
{
	if (this == &file)
	{
		return *this;
	}

//	nFileID = file.nFileID;
	memcpy(md5, file.md5, sizeof(md5));

	return *this;
}

PeerList::PeerList()
{

}

PeerList::~PeerList()
{
	DeleteAllPeer();
}

bool PeerList::AddPeer(const Peer_Info& rPeerInfo)
{
	m_lockListUser.lock();
	m_PeerInfoList.push_back(rPeerInfo);
	m_lockListUser.unlock();

	return true;
}

bool PeerList::DeleteAllPeer()
{
	m_PeerInfoList.clear();

	return true;
}

bool PeerList::DeleteAPeer(const char* macAddr)
{
	PeerInfoListIter Iter1, Iter2;

	m_lockListUser.lock();
	for (Iter1 = m_PeerInfoList.begin(), Iter2 = m_PeerInfoList.end();
		 Iter1 != Iter2;
		 ++Iter1)
	{
		if (!strcmp(Iter1->szMAC, macAddr))
		{
			m_PeerInfoList.erase(Iter1);
			m_lockListUser.unlock();
			return true;
		}
	}

	m_lockListUser.unlock();
	return false;
}

Peer_Info* PeerList::GetAPeer(const char* macAddr)
{
	PeerInfoListIter Iter1, Iter2;

	m_lockListUser.lock();

	for (Iter1 = m_PeerInfoList.begin(), Iter2 = m_PeerInfoList.end();
		Iter1 != Iter2;
		++Iter1)
	{
		if (!strcmp(Iter1->szMAC, macAddr))
		{
			m_lockListUser.unlock();
			return &(*Iter1);
		}
	}

	m_lockListUser.unlock();
	return NULL;
}

Peer_Info*	PeerList::GetAPeerBasedOnIP(const char* ip)
{
	PeerInfoListIter Iter1, Iter2;

	m_lockListUser.lock();

	for (Iter1 = m_PeerInfoList.begin(), Iter2 = m_PeerInfoList.end();
		Iter1 != Iter2;
		++Iter1)
	{
		for (int i = 0; i < Iter1->nAddrNum; i++)
		{
			if (!strcmp(Iter1->arrAddr[i].IPAddr, ip))
			{
				m_lockListUser.unlock();
				return &(*Iter1);
			}
		}
	}

	m_lockListUser.unlock();
	return NULL;

}

int PeerList::GetCurrentSize()
{
	return (int)m_PeerInfoList.size();
}

Peer_Info* PeerList::operator[](int nIndex)
{
	if (nIndex < 0 || nIndex >= GetCurrentSize())
	{
		return NULL;
	}
	else
	{
		PeerInfoListIter Iter1;
		int i;
		for (i = 0, Iter1 = m_PeerInfoList.begin();
			i < nIndex;
			++Iter1, ++i)
		{
			;
		}
		return &(*Iter1);
	}
}