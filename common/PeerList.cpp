#include "PeerList.h"

Peer_Info::Peer_Info()
	: dwActiveTime(0)
	, nAddrNum(0)
{
	memset(IPAddr, 0, sizeof(IPAddr));
	memset(P2PAddr, 0, sizeof(P2PAddr));
}

Peer_Info Peer_Info::operator=(const Peer_Info& rPeerinfo)
{
	if (&rPeerinfo == this)
		return *this;

	memcpy(IPAddr, rPeerinfo.IPAddr, sizeof(IPAddr));
	memcpy(P2PAddr, rPeerinfo.P2PAddr, sizeof(P2PAddr));
	dwActiveTime = rPeerinfo.dwActiveTime;
	nAddrNum = rPeerinfo.nAddrNum;
	//strcpy(szUserName, rPeerinfo.szUserName);
// 	for (int i = 0; i < nAddrNum; ++i)
// 	{
// 		IPAddr[i] = rPeerinfo.IPAddr[i];
// 	}

	return *this;
}

File_Info::File_Info()
{
	nFileID = -1;
	memset(md5, 0, sizeof(md5));
}

File_Info File_Info::operator=(const File_Info& file)
{
	if (this == &file)
	{
		return *this;
	}

	nFileID = file.nFileID;
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
	m_PeerInfoList.push_back(rPeerInfo);

	return true;
}

bool PeerList::DeleteAllPeer()
{
	m_PeerInfoList.clear();

	return true;
}

bool PeerList::DeleteAPeer(const char* addrInfo)
{
	PeerInfoListIter Iter1, Iter2;

	for (Iter1 = m_PeerInfoList.begin(), Iter2 = m_PeerInfoList.end();
		 Iter1 != Iter2;
		 ++Iter1)
	{
		if (!_stricmp(Iter1->P2PAddr, addrInfo))
		{
			m_PeerInfoList.erase(Iter1);
			return true;
		}
	}

	return false;
}

Peer_Info* PeerList::GetAPeer(const char* addrInfo)
{
	PeerInfoListIter Iter1, Iter2;

	for (Iter1 = m_PeerInfoList.begin(), Iter2 = m_PeerInfoList.end();
		Iter1 != Iter2;
		++Iter1)
	{
		if (!_stricmp(Iter1->P2PAddr, addrInfo))
		{
			return &(*Iter1);
		}
	}

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