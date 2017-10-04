#pragma once


// CMetroidDebuggerDlg dialog

class CMetroidDebuggerDlg : public CDialog
{
	DECLARE_DYNAMIC(CMetroidDebuggerDlg)

public:
	CMetroidDebuggerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMetroidDebuggerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
