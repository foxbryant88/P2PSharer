#include "stdafx.h"
#include "FlagMgr.h"


CFlagMgr::CFlagMgr()
{
}


CFlagMgr::~CFlagMgr()
{
}

//根据格式字符串及后缀返回标记
acl::string CFlagMgr::GetFlag(const char *formatstr, const char *suffix)
{
	acl::string flag;
	flag.format(formatstr, suffix);

	return flag;
}

//循环检查标记是否为1 
//成功返回true 否则false
bool CFlagMgr::WaitFlagIs1(const acl::string &flag)
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

//循环检查标记是否为1 
//成功返回true 否则false
bool CFlagMgr::WaitFlagIs2(const acl::string &flag)
{
#define WAIT_RETRY 100

	for (int i = 0; i < WAIT_RETRY; i++)
	{
		if (m_mapFlags[flag] == 2)
		{
			return true;
		}

		Sleep(20);
	}

	return false;
}

//检查标记是否为1 
//成功返回true 否则false
bool CFlagMgr::CheckFlag(const acl::string &flag)
{
	return m_mapFlags[flag] == 1;
}

//设置标记为指定值
void CFlagMgr::SetFlag(acl::string &flag, byte val)
{
	m_lockFlag.lock();
	m_mapFlags[flag] = val;
	m_lockFlag.unlock();
}

//设置标记为指定值
void CFlagMgr::SetFlag(const char *formatstr, const char *suffix, byte val)
{
	acl::string flag;
	flag.format(formatstr, suffix);

	m_lockFlag.lock();
	m_mapFlags[flag] = val;
	m_lockFlag.unlock();
}

//移除标记
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

//移除标记
void CFlagMgr::RMFlag(const char *formatstr, const char *suffix)
{
	acl::string flag;
	flag.format(formatstr, suffix);
	
	RMFlag(flag);
}