/////////////////////////////////////
//文件提供者类，负责提供源文件分块数据
/////////////////////////////////////

#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CFileServer
{
public:
	CFileServer();
	~CFileServer();

	//初始化，打开文件流
	bool Init(const char *fullpath, int blocksize);

	//获取指定分块的数据 length传入时表示想要获取的字节数
	//返回时表示实际获取字节数
	bool GetBlockData(DWORD dwPos, void *buf, int &len);

	//结束
	void Stop();


private:
	int m_nBlockSize;             //每一分块大小
	acl::ifstream m_fstream;
};

