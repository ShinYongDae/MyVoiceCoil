#pragma once
#include <thread>

#define WM_SERIAL_RECEIVED	(WM_USER + 0x73FF)	// 0x7FFF (Range of WM_USER is 0x400 ~ 0x7FFF)
#define BUFSIZE 4096

// CSimpleSerial

class CSimpleSerial : public CWnd
{
	DECLARE_DYNAMIC(CSimpleSerial)

	HANDLE			m_hComm;
	DCB				m_dcb;
	BOOL			m_bPortReady;
	COMMTIMEOUTS	m_CommTimeouts;

	CWnd* m_pParent;
	HWND m_hParent;
	BOOL m_bThreadAlive, m_bThreadStateEnd;
	std::thread t1;

	void StringToChar(CString str, char* szStr);
	void StringToTChar(CString str, TCHAR* tszStr);
	CString CharToString(char *szStr);
	void ThreadStart();
	void ThreadStop();
	void ClearReadBuffer();

public:
	CSimpleSerial(CWnd* pParent, BYTE ComPort, DWORD BaudRate, BYTE ByteSize = 8, BYTE Parity = 0, BYTE StopBits = ONESTOPBIT);
	virtual ~CSimpleSerial();

	char* m_pReadBuffer;
	SOCKET clientSocket;

	BOOL SetCommunicationTimeouts(DWORD ReadIntervalTimeout=0, DWORD ReadTotalTimeoutMultiplier=0, DWORD ReadTotalTimeoutConstant=0, DWORD WriteTotalTimeoutMultiplier=0, DWORD WriteTotalTimeoutConstant=0);

	static void ProcThrd(const LPVOID lpContext);
	BOOL Send(CString sSend);
	BOOL IsConnected();

protected:
	void ThreadEnd();
	BOOL ProcReceive();
	BOOL ThreadIsAlive();

protected:
	DECLARE_MESSAGE_MAP()
};


