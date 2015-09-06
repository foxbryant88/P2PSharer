/////////////////////////////////////
//文件发送类，负责向请求方发送文件分块数据
/////////////////////////////////////

#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"

class CFileSender
{
public:
	CFileSender();
	~CFileSender();
};

