#pragma once
#include <afxcmn.h>

class CMetroidDebuggerDlg : public CDialog
{
	DECLARE_DYNAMIC(CMetroidDebuggerDlg)

public:
	/*
	*/
	CListCtrl m_cDebugEvents;

	CMetroidDebuggerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMetroidDebuggerDlg();

	/*
	*/
	afx_msg void OnBnClicked_StartDebugging();
	
	/*
	*/
	void DebuggerThreadProc();

	/*
	*/
	CString GetDebugStringFromDebugEvent(
		const DEBUG_EVENT &debugEvent, 
		const PROCESS_INFORMATION &processInfo) const;

	/*
	*/
	LRESULT OnDebugEventMessage(WPARAM wParam, LPARAM lParam);
	
	/*
	*/
	afx_msg void OnLvnItemchangedDebugEvents(NMHDR *pNMHDR, LRESULT *pResult);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_METROIDDEBUGGER_DIALOG };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	// TODO init to 0
	int TotalEventsCount;
	int ThreadCount;
	int DLLCount;
	int OutputDebugStringCount;
	int ExceptionCount;

	/*
	Full path to the executable to debug.
	*/
	CString DebugProcessName;

	/*
	*/
	CString GetFileNameFromHandle(HANDLE hFile);
};
