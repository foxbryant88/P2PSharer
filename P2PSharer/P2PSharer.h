
// P2PSharer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CP2PSharerApp: 
// �йش����ʵ�֣������ P2PSharer.cpp
//

class CP2PSharerApp : public CWinApp
{
public:
	CP2PSharerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CP2PSharerApp theApp;