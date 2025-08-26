// SimpleSmac.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MyVoiceCoil.h"
#include "SimpleSmac.h"


// CSimpleSmac

IMPLEMENT_DYNAMIC(CSimpleSmac, CWnd)

CSimpleSmac::CSimpleSmac(int nId, CWnd* pParent/*=NULL*/)
{
	m_pParent = pParent;
	if (pParent)
		m_hParent = pParent->GetSafeHwnd();
	m_nId = nId;

	CreateWndForm(WS_CHILD | WS_OVERLAPPED);
	ThreadStart();

	m_pSerial = NULL;
	m_sRcv = _T("");

}

CSimpleSmac::~CSimpleSmac()
{
	ThreadStop();
	Sleep(30);
	t1.join();

	if (m_pSerial)
	{
		delete m_pSerial;
		m_pSerial = NULL;
	}
}


BEGIN_MESSAGE_MAP(CSimpleSmac, CWnd)
	ON_WM_TIMER()
	ON_MESSAGE(WM_SERIAL_RECEIVED, wmSerialReceived)
END_MESSAGE_MAP()



// CSimpleSmac 메시지 처리기입니다.
BOOL CSimpleSmac::CreateWndForm(DWORD dwStyle)
{
	if (!Create(NULL, _T("SimpleSmac"), dwStyle, CRect(0, 0, 0, 0), m_pParent, (UINT)this))
	{
		AfxMessageBox(_T("CSimpleSmac::Create() Failed!!!"));
		return FALSE;
	}

	return TRUE;
}


void CSimpleSmac::ProcThrd(const LPVOID lpContext)
{
	CSimpleSmac* pSimpleSmac = reinterpret_cast<CSimpleSmac*>(lpContext);

	while (pSimpleSmac->ThreadIsAlive())
	{
		if (!pSimpleSmac->ProcSmac())
			break;
	}

	pSimpleSmac->ThreadEnd();
}

BOOL CSimpleSmac::ProcSmac()
{
	Sleep(100);
	return TRUE;
}

void CSimpleSmac::ThreadStart()
{
	m_bThreadStateEnd = FALSE;
	m_bThreadAlive = TRUE;
	t1 = std::thread(ProcThrd, this);
}

void CSimpleSmac::ThreadStop()
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

void CSimpleSmac::ThreadEnd()
{
	m_bThreadStateEnd = TRUE;
}

BOOL CSimpleSmac::ThreadIsAlive()
{
	return m_bThreadAlive;
}

void CSimpleSmac::Init(int nComPort, int nBaudRate, int nDataSize)
{
	m_stInfo.nComPort = nComPort;
	m_stInfo.nBaudRate = nBaudRate;
	m_stInfo.nDataSize = nDataSize;

	if (!m_pSerial)
		m_pSerial = new CSimpleSerial(this, (BYTE)nComPort, (DWORD)nBaudRate, (BYTE)nDataSize);
}

BOOL CSimpleSmac::IsConnected()
{
	if (m_pSerial)
		return m_pSerial->IsConnected();
	return FALSE;
}

void CSimpleSmac::Clear()
{
	m_sRcv = _T("");
}

BOOL CSimpleSmac::Send(CString sSend)
{
	if (m_pSerial)
		return m_pSerial->Send(sSend);

	return FALSE;
}

/*
Wait Pos : Position(AR14), Vel(AR30), Acc(AR32)
Punching : FinalPos(AR16), FinalVel(AR20), FinalAcc(AR22), Torque(AR40), TorqueOffset(AR48)
*/
LRESULT CSimpleSmac::wmSerialReceived(WPARAM wParam, LPARAM lParam)
{
	CString sReceived = (LPCTSTR)lParam;
	m_sRcv += sReceived;
	CString sSend;
	CString sEscape;	// Escape(27)
	sEscape.Format(_T("%c"), 27);

	BOOL bCmd = FALSE;
	int nPos = -1;
	nPos = m_sRcv.Find(_T("MS10"));		// SERVO ON/OFF
	if (nPos > -1)
		bCmd = TRUE;
	nPos = m_sRcv.Find(_T("MS20"));		// Get State
	if (nPos > -1)
		bCmd = TRUE;
	//nPos = m_sRcv.Find(_T("MS30"));	// Move to position (AR10) : (Speed : AR20, Accelerate : AR22)
	//if (nPos > -1)
	//	bCmd = TRUE;
	nPos = m_sRcv.Find(_T("MS40"));		// Move Shift Position (AR12) : (Speed : AR20, Acc : AR22)
	if (nPos > -1)
		bCmd = TRUE;
	//nPos = m_sRcv.Find(_T("MS50"));	// Move Final Position (AR14) : (Speed : AR20, Acc : AR22)
	//if (nPos > -1)
	//	bCmd = TRUE;
	nPos = m_sRcv.Find(_T("MS60"));		// Punching (Pos : AR14, Speed : AR20, Acc : AR22), ((Pos : AR16, Speed : AR20, Acc : AR22)), (Torque : AR40), (Pos : AR14, Speed : AR20, Acc : AR22)
	if (nPos > -1)
		bCmd = TRUE;
	nPos = m_sRcv.Find(_T("MS70"));		// Measure Surface
	if (nPos > -1)
		bCmd = TRUE;
	nPos = m_sRcv.Find(_T("MS220"));	// Homing
	if (nPos > -1)
		bCmd = TRUE;

	nPos = m_sRcv.Find(sEscape);		// Escape
	if (nPos > -1)
	{
		sReceived = _T("ESC");
	}

	if(bCmd)
	{
		m_sRcv = _T("");
		sSend.Format(_T("(%d)OK"), m_nId);
		Send(sSend);
	}
	else
		::SendMessage(m_hParent, WM_SMAC_RECEIVED, (WPARAM)m_nId, (LPARAM)(LPCTSTR)sReceived);


	return (LRESULT)1;
}


void CSimpleSmac::MoveHome()
{
	//	double dPos;
	CString sSend;
	sSend.Format(_T("MS220")); // Homming
	Clear();
	Send(sSend);
	Sleep(30);
	Send(_T("\r\n"));
	Sleep(100);
	if (!WaitCmd(_T("OK")))
	{
		AfxMessageBox(_T("Fail Homming."));
	}
}

BOOL CSimpleSmac::WaitCmd(CString sCmd)
{

	DWORD CurTimer, StartTimer;
	MSG message;

	StartTimer = GetTickCount();
	CurTimer = GetTickCount();

	while (DELAY_TIME > int(CurTimer - StartTimer))
	{
		int nPos = m_sRcv.Find(sCmd);
		if (nPos >= 0)
			return TRUE;
		else
			Sleep(30);

		CurTimer = GetTickCount();
		//if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		//{
		//	if (!IsDialogMessage(&message))
		//	{
		//		TranslateMessage(&message);
		//		DispatchMessage(&message);
		//	}
		//}
	}

	if (DELAY_TIME <= int(CurTimer - StartTimer))
		return FALSE;

	return TRUE;
}


void CSimpleSmac::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CWnd::OnTimer(nIDEvent);
}
