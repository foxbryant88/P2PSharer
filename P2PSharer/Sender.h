/////////////////////////////////////
//CSender用于控制向服务方发送下载请求
/////////////////////////////////////

#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CSender :
	public acl::thread
{
public:
	CSender();
	~CSender();

	//初始化
	bool Init(const char *toaddr);

	//分配一批数据块序号给本对象下载
	bool PushTask(std::vector<DWORD> &blockNums);

	void *run();

private:

};

