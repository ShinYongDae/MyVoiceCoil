
// MyVoiceCoilDlg.h : 헤더 파일
//

#pragma once
#include "SimpleSmac.h"

// CMyVoiceCoilDlg 대화 상자
class CMyVoiceCoilDlg : public CDialog
{
	BOOL m_bThreadAlive, m_bThreadStateEnd;
	std::thread t1, t2;
	void ThreadStart();
	void ThreadStop();

	CSimpleSmac* m_pSmac[2];
	CString m_sRcv[2];

	CString m_sCmd[2];
	BOOL m_bRepeat[2];

	void DispSerialStatus();

// 생성입니다.
public:
	CMyVoiceCoilDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	~CMyVoiceCoilDlg();
	static void ProcThrd(const LPVOID lpContext);
	static void ProcThrd2(const LPVOID lpContext);

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MYVOICECOIL_DIALOG };
#endif

public:


protected:
	void ThreadEnd();
	BOOL ProcDlg();
	BOOL ProcDlg2();
	BOOL ThreadIsAlive();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheckConnect();
	afx_msg void OnBnClickedBtnSend();
	afx_msg LRESULT wmSmacReceived(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnBnClickedBtnHommingL();
	afx_msg void OnBnClickedCheckRepeatL();
	afx_msg void OnBnClickedCheckRepeatR();
	afx_msg void OnBnClickedBtnHommingR();
	afx_msg void OnBnClickedBtnClear2();
	afx_msg void OnBnClickedCheckConnect2();
	afx_msg void OnBnClickedBtnSend2();
};
