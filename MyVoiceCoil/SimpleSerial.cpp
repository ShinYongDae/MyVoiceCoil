// SimpleSerial.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MyVoiceCoil.h"
#include "SimpleSerial.h"


// CSimpleSerial

IMPLEMENT_DYNAMIC(CSimpleSerial, CWnd)

CSimpleSerial::CSimpleSerial(CWnd* pParent, BYTE ComPort, DWORD BaudRate, BYTE ByteSize, BYTE Parity, BYTE StopBits)
{
	m_pParent = pParent;
	if (pParent)
		m_hParent = pParent->GetSafeHwnd();

	m_bPortReady = FALSE;
	m_pReadBuffer = new char[BUFSIZE];
	ZeroMemory(m_pReadBuffer, sizeof(m_pReadBuffer));

	CString sPort;
	sPort.Format(_T("COM%d"), ComPort);

	m_hComm = CreateFile(	sPort,//(LPCWSTR)cPort,							// name of the write
							GENERIC_READ | GENERIC_WRITE,					// open for reading or writing 
							FILE_SHARE_READ | FILE_SHARE_WRITE,				// do not share (FILE_SHARE_READ)
							NULL,											// default security
							OPEN_ALWAYS,									// create new file only OPEN_EXISTING
							FILE_ATTRIBUTE_NORMAL,							// normal file FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED
							NULL	);										// no attr. template
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(_T("CreateFile() Error"));
		return;
	}

	SetCommMask(m_hComm, EV_BREAK | EV_ERR | EV_RXCHAR | EV_TXEMPTY);					// EV_RXCHAR event 설정
	SetupComm(m_hComm, BUFSIZE, BUFSIZE);												// InQueue, OutQueue 크기 설정
	PurgeComm(m_hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);	// 포트 비우기
	//SetCommunicationTimeouts();
	SetCommunicationTimeouts(0xFFFFFFFF, 0, 1000, 0, 1000);

	m_dcb.DCBlength = sizeof(DCB);
	if ((m_bPortReady = GetCommState(m_hComm, &m_dcb)) == 0)
	{
		AfxMessageBox(_T("GetCommState Error"));
		CloseHandle(m_hComm);
		return;
	}

	m_dcb.BaudRate = BaudRate;
	m_dcb.ByteSize = ByteSize;
	m_dcb.Parity = Parity;							// ( 0 ~ 4 ) : None, Odd, Even, Mark, Space
	m_dcb.StopBits = StopBits;						// ONESTOPBIT;
	m_dcb.fBinary = true;
	m_dcb.fDsrSensitivity = false;
	m_dcb.fParity = false;							// Enable Parity Check
	m_dcb.fOutX = false;
	m_dcb.fInX = false;
	m_dcb.fNull = false;
	m_dcb.fAbortOnError = true;
	m_dcb.fOutxCtsFlow = false;
	m_dcb.fOutxDsrFlow = false;
	m_dcb.fDtrControl = DTR_CONTROL_DISABLE;
	m_dcb.fDsrSensitivity = false;
	m_dcb.fRtsControl = RTS_CONTROL_DISABLE;
	m_dcb.fOutxCtsFlow = false;
	m_dcb.fOutxCtsFlow = false;

	m_bPortReady = SetCommState(m_hComm, &m_dcb);
	if (m_bPortReady == 0)
	{
		AfxMessageBox(_T("SetCommState Error"));
		CloseHandle(m_hComm);
		return;
	}

	ThreadStart();
}

CSimpleSerial::~CSimpleSerial()
{
	if (m_bPortReady)
	{
		m_bPortReady = FALSE;
		SetCommMask(m_hComm, 0);
		PurgeComm(m_hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
		CloseHandle(m_hComm);
	}

	ThreadStop();
	Sleep(30);
	t1.join();

	delete m_pReadBuffer;
}


BEGIN_MESSAGE_MAP(CSimpleSerial, CWnd)
END_MESSAGE_MAP()


// CSimpleSerial 메시지 처리기입니다.

BOOL CSimpleSerial::SetCommunicationTimeouts(DWORD ReadIntervalTimeout, DWORD ReadTotalTimeoutMultiplier, DWORD ReadTotalTimeoutConstant, DWORD WriteTotalTimeoutMultiplier, DWORD WriteTotalTimeoutConstant)
{
	if ((m_bPortReady = GetCommTimeouts(m_hComm, &m_CommTimeouts)) == 0)
	{
		AfxMessageBox(_T("GetCommTimeouts function failed"));
		return FALSE;
	}

	m_CommTimeouts.ReadIntervalTimeout = ReadIntervalTimeout;
	m_CommTimeouts.ReadTotalTimeoutConstant = ReadTotalTimeoutConstant;
	m_CommTimeouts.ReadTotalTimeoutMultiplier = ReadTotalTimeoutMultiplier;
	m_CommTimeouts.WriteTotalTimeoutConstant = WriteTotalTimeoutConstant;
	m_CommTimeouts.WriteTotalTimeoutMultiplier = WriteTotalTimeoutMultiplier;

	m_bPortReady = SetCommTimeouts(m_hComm, &m_CommTimeouts);

	if (m_bPortReady == 0)
	{
		//MessageBox(L"SetCommTimeouts function failed",L"Com Port Error",MB_OK+MB_ICONERROR); 
		AfxMessageBox(_T("SetCommTimeouts function failed"));
		CloseHandle(m_hComm);
		return FALSE;
	}

	return TRUE;
}

BOOL CSimpleSerial::Send(CString sSend)
{
	int nLen = sSend.GetLength() + 1; // for '\0'
	char* cSend = new char[nLen];
	StringToChar(sSend, cSend);

	DWORD dwBytesWritten = 0;	// Sended Byte Size
	//WriteFile(open file handle, start of data to write, number of bytes to write, number of bytes that were written, no overlapped structure)
	if (WriteFile(m_hComm, cSend, strlen(cSend), &dwBytesWritten, NULL) == 0)
	{
		delete[] cSend;
		return FALSE; // 데이터 내보내기 실패
	}
	delete[] cSend;
	return TRUE;
}

void CSimpleSerial::ThreadStart()
{
	m_bThreadStateEnd = FALSE;
	m_bThreadAlive = TRUE;
	t1 = std::thread(ProcThrd, this);
}

void CSimpleSerial::ProcThrd(const LPVOID lpContext)
{
	CSimpleSerial* pSimpleSerial = reinterpret_cast<CSimpleSerial*>(lpContext);

	while (pSimpleSerial->ThreadIsAlive())
	{
		if (!pSimpleSerial->ProcReceive())
			break;
	}

	pSimpleSerial->ThreadEnd();
}

BOOL CSimpleSerial::ProcReceive()
{
	ClearReadBuffer();

	DWORD	dwErrorFlags;
	COMSTAT	ComStat;
	DWORD	dwBytesTransferred = 0;
	ClearCommError(m_hComm, &dwErrorFlags, &ComStat);
	DWORD dwLength = min((DWORD)BUFSIZE, ComStat.cbInQue); // 읽은 데이터 크기

	if (dwLength > 0)
	{
		char buffer[BUFSIZE] = { 0, };
		if (ReadFile(m_hComm, buffer, dwLength, &dwBytesTransferred, 0))
		{
			memcpy(m_pReadBuffer, buffer, dwBytesTransferred);
			m_pReadBuffer[dwBytesTransferred] = _T('\0');
			CString sMsg = CharToString(m_pReadBuffer);
			::SendMessage(m_hParent, WM_SERIAL_RECEIVED, (WPARAM)0, (LPARAM)(LPCTSTR)sMsg);
		}
		else
		{
			// Error
			dwLength = 0;
			ClearCommError(m_hComm, &dwErrorFlags, &ComStat);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CSimpleSerial::ThreadIsAlive()
{
	return m_bThreadAlive;
}

void CSimpleSerial::ThreadStop()
{
	m_bThreadAlive = FALSE;
	//MSG message;
	//const DWORD dwTimeOut = 1000 * 60 * 3; // 3 Minute
	//DWORD dwStartTick = GetTickCount();
	//Sleep(30);
	//while (!m_bThreadStateEnd)
	//{
	//	if (GetTickCount() >= (dwStartTick + dwTimeOut))
	//	{
	//		AfxMessageBox(_T("WaitUntilThreadEnd() Time Out!!!", NULL, MB_OK | MB_ICONSTOP));
	//		return;
	//	}
	//	if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
	//	{
	//		::TranslateMessage(&message);
	//		::DispatchMessage(&message);
	//	}
	//	Sleep(30);
	//}
}

void CSimpleSerial::ThreadEnd()
{
	m_bThreadStateEnd = TRUE;
}


void CSimpleSerial::ClearReadBuffer()
{
	ZeroMemory(m_pReadBuffer, sizeof(m_pReadBuffer));
}

BOOL CSimpleSerial::IsConnected()
{
	return (!m_bThreadStateEnd);
}

void CSimpleSerial::StringToChar(CString str, char* szStr)  // char* returned must be deleted... 
{
	int nLen = str.GetLength();
	strcpy(szStr, CT2A(str));
	szStr[nLen] = _T('\0');
}

void CSimpleSerial::StringToTChar(CString str, TCHAR* tszStr) // TCHAR* returned must be deleted... 
{
	int nLen = str.GetLength() + 1;
	memset(tszStr, 0x00, nLen * sizeof(TCHAR));
	_tcscpy(tszStr, str);
}

CString CSimpleSerial::CharToString(char *szStr)
{
	CString strRet;

	int nLen = strlen(szStr) + sizeof(char);
	wchar_t *tszTemp = NULL;
	tszTemp = new WCHAR[nLen];

	MultiByteToWideChar(CP_ACP, 0, szStr, -1, tszTemp, nLen * sizeof(WCHAR));

	strRet.Format(_T("%s"), (CString)tszTemp);
	if (tszTemp)
	{
		delete[] tszTemp;
		tszTemp = NULL;
	}
	return strRet;
}
