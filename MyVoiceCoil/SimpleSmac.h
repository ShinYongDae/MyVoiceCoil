#pragma once
#include <thread>

#include "SimpleSerial.h"


#define WM_SMAC_RECEIVED	(WM_USER + 0x0400)	// 0x7FFF (Range of WM_USER is 0x400 ~ 0x7FFF)
#define DELAY_TIME	1000

typedef struct tagInfo
{
	int nComPort, nBaudRate, nDataSize;
	tagInfo()
	{
		nComPort = 4;
		nBaudRate = 38400;
		nDataSize = 8;
	}
}stInfo;

// CSimpleSmac

class CSimpleSmac : public CWnd
{
	DECLARE_DYNAMIC(CSimpleSmac)

	CWnd* m_pParent;
	HWND m_hParent;

	BOOL CreateWndForm(DWORD dwStyle);

	BOOL m_bThreadAlive, m_bThreadStateEnd;
	std::thread t1;
	void ThreadStart();
	void ThreadStop();

	CSimpleSerial *m_pSerial;
	CString m_sRcv;
	stInfo m_stInfo;
	int m_nId;

	BOOL WaitCmd(CString sCmd);

public:
	CSimpleSmac(int nId, CWnd* pParent = NULL);
	virtual ~CSimpleSmac();
	static void ProcThrd(const LPVOID lpContext);

public:
	void Init(int nComPort, int nBaudRate, int nDataSize);
	BOOL IsConnected();
	BOOL Send(CString sSend);
	void Clear();
	void MoveHome();

protected:
	void ThreadEnd();
	BOOL ProcSmac();
	BOOL ThreadIsAlive();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT wmSerialReceived(WPARAM wParam, LPARAM lParam);
};


