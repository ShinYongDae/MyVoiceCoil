
// MyVoiceCoil.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CMyVoiceCoilApp:
// �� Ŭ������ ������ ���ؼ��� MyVoiceCoil.cpp�� �����Ͻʽÿ�.
//

class CMyVoiceCoilApp : public CWinApp
{
public:
	CMyVoiceCoilApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CMyVoiceCoilApp theApp;