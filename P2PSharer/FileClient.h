/////////////////////////////////////
//文件接收类，负责对接收到的文件分块重新拼装
/////////////////////////////////////
#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CFileClient :
	public acl::thread
{
public:
	CFileClient();
	~CFileClient();

	//初始化文件信息
	bool Init(acl::ofstream &files, char *md5, DWORD filesize);

	//缓存数据,参数内存由调用者申请，本模块处理完毕后释放
	void CacheData(void *data);

	void *run();

	//停止，缓存文件的已处理信息
	void Stop();

private:
	//从缓存取数据
	void *GetData();

// 	//处理数据
// 	void DataProcess()
	std::vector<void *> m_vdata;
	acl::locker m_lockvdata;

	acl::ofstream *m_fstream;
	char m_md5[33];
	DWORD m_dwFileSize;

	bool m_bStop;
};

