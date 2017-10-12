#pragma once
#include <afxcmn.h>

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
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_cDebugEvents;
	afx_msg void OnBnClicked_StartDebugging();
	void DebuggerThreadProc();
	LRESULT OnDebugEventMessage(WPARAM wParam, LPARAM lParam);

private:
	// TODO init to 0
	int TotalEventsCount;
	int ThreadCount;
	int DLLCount;
	/*
	Name of the process to debug.
	*/
	CString DebugProcessName;
	CString GetFileNameFromHandle(HANDLE hFile);
public:
	afx_msg void OnLvnItemchangedDebugEvents(NMHDR *pNMHDR, LRESULT *pResult);
};
