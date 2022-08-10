
// BezierCurveDlg.h : header file
//

#pragma once

//class CBezierCurveControl;
// CBezierCurveDlg dialog

class CSpeedCurveCtrl;

class CBezierCurveDlg : public CDialogEx
{
// Construction
public:
	CBezierCurveDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BEZIERCURVE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedButtonReset();

	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemove();

private:
	CSpeedCurveCtrl* m_pBezierCurveControl;

	CButton m_btnReset;
	CButton m_btnAdd;
	CButton m_btnRemove;
};
