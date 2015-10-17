#pragma once

#include "acl_cpp/lib_acl.hpp"
#include "ServerEX.h"

class CAutoLogin :
	public acl::thread
{
public:
	CAutoLogin();
	~CAutoLogin();

	void *run();

private:

};
