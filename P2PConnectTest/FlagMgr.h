/////////////////////////////////
//各种数据发布标记，通过map实现
///////////////////////////////////

#pragma once

class CFlagMgr
{
public:
	CFlagMgr();
	~CFlagMgr();

	//根据格式字符串及后缀返回标记
	acl::string GetFlag(const char *formatstr, const char *suffix);

	//循环检查标记是否为1
	bool WaitFlag(const acl::string &flag);

	//设置标记为指定值
	void SetFlag(acl::string &flag, byte val);
	void SetFlag(const char *formatstr, const char *suffix, byte val);

	//移除标记
	void RMFlag(acl::string &flag);
	void RMFlag(const char *formatstr, const char *suffix);

private:
	acl::locker m_lockFlag;                  //发送标记锁

	std::map<acl::string, byte> m_mapFlags;  //用于表明发送数据是否成功的各种标记
										     //key:操作标记的名称 value：1表示已确认 0表示未确认

};

