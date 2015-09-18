#include "stdafx.h"
#include "FlagMgr.h"


CFlagMgr::CFlagMgr()
{
}


CFlagMgr::~CFlagMgr()
{
}

//���ݸ�ʽ�ַ�������׺���ر��
acl::string CFlagMgr::GetFlag(const char *formatstr, const char *suffix)
{
	acl::string flag;
	flag.format(formatstr, suffix);

	return flag;
}

//ѭ��������Ƿ�Ϊ1 
//�ɹ�����true ����false
bool CFlagMgr::WaitFlag(const acl::string &flag)
{
#define WAIT_RETRY 100

	for (int i = 0; i < WAIT_RETRY; i++)
	{
		if (m_mapFlags[flag] == 1)
		{
			return true;
		}

		Sleep(20);
	}

	return false;
}

//���ñ��Ϊָ��ֵ
void CFlagMgr::SetFlag(acl::string &flag, byte val)
{
	m_lockFlag.lock();
	m_mapFlags[flag] = val;
	m_lockFlag.unlock();
}

//���ñ��Ϊָ��ֵ
void CFlagMgr::SetFlag(const char *formatstr, const char *suffix, byte val)
{
	acl::string flag;
	flag.format(formatstr, suffix);

	m_lockFlag.lock();
	m_mapFlags[flag] = val;
	m_lockFlag.unlock();
}

//�Ƴ����
void CFlagMgr::RMFlag(acl::string &flag)
{
	std::map<acl::string, byte>::iterator itFlag = m_mapFlags.find(flag);
	if (itFlag != m_mapFlags.end())
	{
		m_lockFlag.lock();
		m_mapFlags.erase(itFlag);
		m_lockFlag.unlock();
	}
}

//�Ƴ����
void CFlagMgr::RMFlag(const char *formatstr, const char *suffix)
{
	acl::string flag;
	flag.format(formatstr, suffix);
	
	RMFlag(flag);
}