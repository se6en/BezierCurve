
// BezierCurveDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "BezierCurve.h"
#include "BezierCurveDlg.h"
#include "afxdialogex.h"
#include "SpeedCurveCtrl.h"
//#include "BezierCurveControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBezierCurveDlg dialog



CBezierCurveDlg::CBezierCurveDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BEZIERCURVE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pBezierCurveControl = nullptr;
}

void CBezierCurveDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_BUTTON_RESET, m_btnReset);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_BUTTON_ADD, m_btnAdd);

	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBezierCurveDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_RESET, OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnBnClickedButtonAdd)
END_MESSAGE_MAP()


// CBezierCurveDlg message handlers

BOOL CBezierCurveDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	if (m_pBezierCurveControl == nullptr)
	{
		m_pBezierCurveControl = new CSpeedCurveCtrl()/*new CBezierCurveControl()*/;

		m_pBezierCurveControl->Create(_T("BezierCurve"), NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_WND_BEZIER_CURVE);
	}

	MoveWindow(CRect(0, 0, 1000, 600));
	CenterWindow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBezierCurveDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBezierCurveDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBezierCurveDlg::OnSize(UINT nType, int cx, int cy)
{
	if (m_pBezierCurveControl == nullptr || m_pBezierCurveControl->GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rcClient;
	GetClientRect(rcClient);

	rcClient.DeflateRect(20, 20);

	CRect rcButton;
	rcButton.left = rcClient.left;
	rcButton.right = rcButton.left + 60;
	rcButton.bottom = rcClient.bottom;
	rcButton.top = rcButton.bottom - 30;
	m_btnReset.MoveWindow(rcButton);

	rcButton.right = rcClient.right;
	rcButton.left = rcButton.right - 60;
	m_btnRemove.MoveWindow(rcButton);

	rcButton.right = rcButton.left - 20;
	rcButton.left = rcButton.right - 60;
	m_btnAdd.MoveWindow(rcButton);

	CRect rcCurveWindow;
	rcCurveWindow = rcClient;
	rcCurveWindow.bottom = rcButton.top - 20;
	m_pBezierCurveControl->MoveWindow(rcCurveWindow);
}

void CBezierCurveDlg::OnBnClickedButtonReset()
{
}

void CBezierCurveDlg::OnBnClickedButtonAdd()
{
}

void CBezierCurveDlg::OnBnClickedButtonRemove()
{
}
