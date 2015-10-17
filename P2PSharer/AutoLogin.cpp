#include "stdafx.h"
#include "AutoLogin.h"

CAutoLogin::CAutoLogin()
{
}


CAutoLogin::~CAutoLogin()
{
}

void *CAutoLogin::run()
{
	while (true)
	{
		g_serEx.DoLogin();
		Sleep(5000);
	}
}