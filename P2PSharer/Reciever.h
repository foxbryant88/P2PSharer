#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CReciever :
	public acl::thread
{
public:
	CReciever();
	~CReciever();

	//初始化文件信息
	bool Init(acl::ofstream &files, char *md5, DWORD filesize);

	//缓存数据,参数内存由调用者申请，本模块处理完毕后释放
	void CacheData(const char *data);

	void *run();

	//停止，缓存文件的已处理信息
	void Stop();

private:
	//从缓存取数据
	const char *GetData();

// 	//处理数据
// 	void DataProcess()
	std::vector<const char *> m_vdata;
	acl::locker m_lockvdata;

	acl::ofstream *m_fstream;
	char m_md5[32];
	DWORD m_dwFileSize;

	bool m_bStop;
};

