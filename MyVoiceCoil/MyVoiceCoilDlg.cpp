
// MyVoiceCoilDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "MyVoiceCoil.h"
#include "MyVoiceCoilDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyVoiceCoilDlg 대화 상자



CMyVoiceCoilDlg::CMyVoiceCoilDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_MYVOICECOIL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pSmac[0] = NULL;
	m_sRcv[0] = _T("");
	m_sCmd[0] = _T("");
	m_bRepeat[0] = FALSE;

	m_pSmac[1] = NULL;
	m_sRcv[1] = _T("");
	m_sCmd[1] = _T("");
	m_bRepeat[1] = FALSE;

	ThreadStart();

}

CMyVoiceCoilDlg::~CMyVoiceCoilDlg()
{
	ThreadStop();
	Sleep(30);
	t1.join();
	t2.join();

	if (m_pSmac[0])
	{
		delete m_pSmac[0];
		m_pSmac[0] = NULL;
	}
	if (m_pSmac[1])
	{
		delete m_pSmac[1];
		m_pSmac[1] = NULL;
	}

}

void CMyVoiceCoilDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMyVoiceCoilDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHECK_CONNECT, &CMyVoiceCoilDlg::OnBnClickedCheckConnect)
	ON_BN_CLICKED(IDC_BTN_SEND, &CMyVoiceCoilDlg::OnBnClickedBtnSend)
	ON_MESSAGE(WM_SMAC_RECEIVED, wmSmacReceived)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CMyVoiceCoilDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_HOMMING_L, &CMyVoiceCoilDlg::OnBnClickedBtnHommingL)
	ON_BN_CLICKED(IDC_CHECK_REPEAT_L, &CMyVoiceCoilDlg::OnBnClickedCheckRepeatL)
	ON_BN_CLICKED(IDC_CHECK_REPEAT_R, &CMyVoiceCoilDlg::OnBnClickedCheckRepeatR)
	ON_BN_CLICKED(IDC_BTN_HOMMING_R, &CMyVoiceCoilDlg::OnBnClickedBtnHommingR)
	ON_BN_CLICKED(IDC_BTN_CLEAR2, &CMyVoiceCoilDlg::OnBnClickedBtnClear2)
	ON_BN_CLICKED(IDC_CHECK_CONNECT2, &CMyVoiceCoilDlg::OnBnClickedCheckConnect2)
	ON_BN_CLICKED(IDC_BTN_SEND2, &CMyVoiceCoilDlg::OnBnClickedBtnSend2)
	ON_BN_CLICKED(IDC_BTN_ESCAPE_L, &CMyVoiceCoilDlg::OnBnClickedBtnEscapeL)
	ON_BN_CLICKED(IDC_BTN_ESCAPE_R, &CMyVoiceCoilDlg::OnBnClickedBtnEscapeR)
END_MESSAGE_MAP()


// CMyVoiceCoilDlg 메시지 처리기

BOOL CMyVoiceCoilDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	GetDlgItem(IDC_EDIT_COM_PORT)->SetWindowText(_T("4"));
	GetDlgItem(IDC_EDIT_BAUD_RATE)->SetWindowText(_T("38400"));
	GetDlgItem(IDC_EDIT_DATA_SIZE)->SetWindowText(_T("8"));

	GetDlgItem(IDC_EDIT_COM_PORT2)->SetWindowText(_T("6"));
	GetDlgItem(IDC_EDIT_BAUD_RATE2)->SetWindowText(_T("38400"));
	GetDlgItem(IDC_EDIT_DATA_SIZE2)->SetWindowText(_T("8"));

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMyVoiceCoilDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMyVoiceCoilDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMyVoiceCoilDlg::ProcThrd(const LPVOID lpContext)
{
	CMyVoiceCoilDlg* pDlg = reinterpret_cast<CMyVoiceCoilDlg*>(lpContext);

	while (pDlg->ThreadIsAlive())
	{
		if (!pDlg->ProcDlg())
			break;
	}

	pDlg->ThreadEnd();
}

BOOL CMyVoiceCoilDlg::ProcDlg()
{
	CString sEscape;	// Escape(27)
	sEscape.Format(_T("%c"), 27);

	if (!m_sCmd[0].IsEmpty())
	{
		if (m_pSmac[0])
		{
			m_pSmac[0]->Send(m_sCmd[0]);
			if (m_sCmd[0] == sEscape)
			{
				m_sCmd[0] = _T("");
			}
		}
		if (!m_bRepeat[0])
			m_sCmd[0] = _T("");
	}

	//if (!m_sCmd[1].IsEmpty())
	//{
	//	if (m_pSmac[1])
	//	{
	//		m_pSmac[1]->Send(m_sCmd[1]);
	//	}
	//	if (!m_bRepeat[1])
	//		m_sCmd[1] = _T("");
	//}
	Sleep(30);
	return TRUE;
}

void CMyVoiceCoilDlg::ProcThrd2(const LPVOID lpContext)
{
	CMyVoiceCoilDlg* pDlg = reinterpret_cast<CMyVoiceCoilDlg*>(lpContext);

	while (pDlg->ThreadIsAlive())
	{
		if (!pDlg->ProcDlg2())
			break;
	}

	pDlg->ThreadEnd();
}

BOOL CMyVoiceCoilDlg::ProcDlg2()
{
	CString sEscape;	// Escape(27)
	sEscape.Format(_T("%c"), 27);

	//if (!m_sCmd[0].IsEmpty())
	//{
	//	if (m_pSmac[0])
	//	{
	//		m_pSmac[0]->Send(m_sCmd[0]);
	//	}
	//	if (!m_bRepeat[0])
	//		m_sCmd[0] = _T("");
	//}

	if (!m_sCmd[1].IsEmpty())
	{
		if (m_pSmac[1])
		{
			m_pSmac[1]->Send(m_sCmd[1]);
			if (m_sCmd[1] == sEscape)
			{
				m_sCmd[1] = _T("");
			}
		}
		if (!m_bRepeat[1])
			m_sCmd[1] = _T("");
	}
	Sleep(30);
	return TRUE;
}

void CMyVoiceCoilDlg::ThreadStart()
{
	m_bThreadStateEnd = FALSE;
	m_bThreadAlive = TRUE;
	t1 = std::thread(ProcThrd, this);
	t2 = std::thread(ProcThrd2, this);
}

void CMyVoiceCoilDlg::ThreadStop()
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

void CMyVoiceCoilDlg::ThreadEnd()
{
	m_bThreadStateEnd = TRUE;
}

BOOL CMyVoiceCoilDlg::ThreadIsAlive()
{
	return m_bThreadAlive;
}


LRESULT CMyVoiceCoilDlg::wmSmacReceived(WPARAM wParam, LPARAM lParam)
{
	CString sReceived = (LPCTSTR)lParam;
	int nId = (int)wParam;
	m_sRcv[nId] += sReceived;

	int nPos;
	nPos = m_sRcv[nId].Find(_T("OK"));
	if (nPos > -1) 
	{
		if(nId == 0)
			GetDlgItem(IDC_MESSAGE_LIST)->SetWindowText(m_sRcv[nId]);
		else if(nId == 1)
			GetDlgItem(IDC_MESSAGE_LIST2)->SetWindowText(m_sRcv[nId]);
	}

	nPos = m_sRcv[nId].Find(_T("ESC"));
	if (nPos > -1)
	{
		if (nId == 0)
		{
			GetDlgItem(IDC_MESSAGE_LIST)->SetWindowText(m_sRcv[nId]);
			m_bRepeat[0] = FALSE;
			((CButton*)GetDlgItem(IDC_CHECK_REPEAT_L))->SetCheck(FALSE);

		}
		else if (nId == 1)
		{
			GetDlgItem(IDC_MESSAGE_LIST2)->SetWindowText(m_sRcv[nId]);
			m_bRepeat[1] = FALSE;
			((CButton*)GetDlgItem(IDC_CHECK_REPEAT_R))->SetCheck(FALSE);
		}
	}

	return (LRESULT)1;
}



void CMyVoiceCoilDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	switch (nIDEvent)
	{
	case 0:
		KillTimer(0);
		DispSerialStatus();
		SetTimer(0, 300, NULL);
		break;
	default:
		break;
	}

	CDialog::OnTimer(nIDEvent);
}

void CMyVoiceCoilDlg::DispSerialStatus()
{
	if (m_pSmac[0])
	{
		BOOL bOn = ((CButton*)GetDlgItem(IDC_CHECK_CONNECT))->GetCheck();
		if (m_pSmac[0]->IsConnected())
		{
			if (!bOn)
				((CButton*)GetDlgItem(IDC_CHECK_CONNECT))->SetCheck(TRUE);
		}
		else
		{
			if (bOn)
				((CButton*)GetDlgItem(IDC_CHECK_CONNECT))->SetCheck(FALSE);
		}
	}

	if (m_pSmac[1])
	{
		BOOL bOn = ((CButton*)GetDlgItem(IDC_CHECK_CONNECT2))->GetCheck();
		if (m_pSmac[1]->IsConnected())
		{
			if (!bOn)
				((CButton*)GetDlgItem(IDC_CHECK_CONNECT2))->SetCheck(TRUE);
		}
		else
		{
			if (bOn)
				((CButton*)GetDlgItem(IDC_CHECK_CONNECT2))->SetCheck(FALSE);
		}
	}
}


void CMyVoiceCoilDlg::OnBnClickedCheckConnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString sComPort, sBaudRate, sDataSize;
	BOOL bOn = ((CButton*)GetDlgItem(IDC_CHECK_CONNECT))->GetCheck();

	if (bOn)
	{
		GetDlgItem(IDC_EDIT_COM_PORT)->GetWindowText(sComPort);
		GetDlgItem(IDC_EDIT_BAUD_RATE)->GetWindowText(sBaudRate);
		GetDlgItem(IDC_EDIT_DATA_SIZE)->GetWindowText(sDataSize);

		if (!m_pSmac[0])
			m_pSmac[0] = new CSimpleSmac(0, this);

		if (m_pSmac[0])
			m_pSmac[0]->Init((BYTE)_tstoi(sComPort), (DWORD)_tstoi(sBaudRate), (BYTE)_tstoi(sDataSize));
	}
	else
	{
		if (m_pSmac[0])
		{
			delete m_pSmac[0];
			m_pSmac[0] = NULL;
		}
	}
}


void CMyVoiceCoilDlg::OnBnClickedBtnSend()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString sMsg;
	GetDlgItem(IDC_TXT_MESSAGE)->GetWindowText(sMsg);
	if (!sMsg.IsEmpty())
	{
		if (m_pSmac[0])
		{
			m_pSmac[0]->Send(sMsg);
		}
	}
}


void CMyVoiceCoilDlg::OnBnClickedBtnClear()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_sRcv[0] = _T("");
	GetDlgItem(IDC_MESSAGE_LIST)->SetWindowText(m_sRcv[0]);
	if (m_pSmac[0])
		m_pSmac[0]->Clear();
}



void CMyVoiceCoilDlg::OnBnClickedCheckRepeatL()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BOOL bOn = ((CButton*)GetDlgItem(IDC_CHECK_REPEAT_L))->GetCheck();
	m_bRepeat[0] = bOn;
}

void CMyVoiceCoilDlg::OnBnClickedBtnHommingL()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString sHomming = _T("MS220");	// Homming
	m_sCmd[0] = sHomming;
}

void CMyVoiceCoilDlg::OnBnClickedBtnEscapeL()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString sEscape;	// Escape(27)
	sEscape.Format(_T("%c"), 27);
	m_sCmd[0] = sEscape;
}


void CMyVoiceCoilDlg::OnBnClickedCheckConnect2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString sComPort, sBaudRate, sDataSize;
	BOOL bOn = ((CButton*)GetDlgItem(IDC_CHECK_CONNECT2))->GetCheck();

	if (bOn)
	{
		GetDlgItem(IDC_EDIT_COM_PORT2)->GetWindowText(sComPort);
		GetDlgItem(IDC_EDIT_BAUD_RATE2)->GetWindowText(sBaudRate);
		GetDlgItem(IDC_EDIT_DATA_SIZE2)->GetWindowText(sDataSize);

		if (!m_pSmac[1])
			m_pSmac[1] = new CSimpleSmac(1, this);

		if (m_pSmac[1])
			m_pSmac[1]->Init((BYTE)_tstoi(sComPort), (DWORD)_tstoi(sBaudRate), (BYTE)_tstoi(sDataSize));
	}
	else
	{
		if (m_pSmac[1])
		{
			delete m_pSmac[1];
			m_pSmac[1] = NULL;
		}
	}
}


void CMyVoiceCoilDlg::OnBnClickedBtnSend2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString sMsg;
	GetDlgItem(IDC_TXT_MESSAGE2)->GetWindowText(sMsg);
	if (!sMsg.IsEmpty())
	{
		if (m_pSmac[1])
		{
			m_pSmac[1]->Send(sMsg);
		}
	}
}


void CMyVoiceCoilDlg::OnBnClickedBtnClear2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_sRcv[1] = _T("");
	GetDlgItem(IDC_MESSAGE_LIST2)->SetWindowText(m_sRcv[1]);
	if (m_pSmac[1])
		m_pSmac[1]->Clear();
}


void CMyVoiceCoilDlg::OnBnClickedCheckRepeatR()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BOOL bOn = ((CButton*)GetDlgItem(IDC_CHECK_REPEAT_R))->GetCheck();
	m_bRepeat[1] = bOn;
}


void CMyVoiceCoilDlg::OnBnClickedBtnHommingR()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString sHomming = _T("MS220");	// Homming
	m_sCmd[1] = sHomming;
}



void CMyVoiceCoilDlg::OnBnClickedBtnEscapeR()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString sEscape;	// Escape(27)
	sEscape.Format(_T("%c"), 27);
	m_sCmd[1] = sEscape;
}
