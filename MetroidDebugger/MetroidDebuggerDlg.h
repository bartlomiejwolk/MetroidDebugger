#pragma once
#include <afxcmn.h>

class CMetroidDebuggerDlg : public CDialog
{
	DECLARE_DYNAMIC(CMetroidDebuggerDlg)

public:
	/*
	Contains debug messages displayed to the user through List Control
	*/
	CListCtrl m_cDebugEvents;

	CMetroidDebuggerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMetroidDebuggerDlg();

	/*
	*/
	afx_msg void OnBnClicked_StartDebugging();
	
	/*
	Main debugger loop. Listenes and handles debug events.
	Controls execution of the debugee. Method executed on a separate thread.
	*/
	void DebuggerThreadProc();

	/*
	Handles custom DEBUG_EVENT_MESSAGE (sent by DebuggerThreadProc()).
	Updates `m_cDebugEvents`.
	*/
	LRESULT OnDebugEventMessage(WPARAM wParam, LPARAM lParam);
	
	/*
	*/
	afx_msg void OnLvnItemchangedDebugEvents(NMHDR* pNMHDR, LRESULT* pResult);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_METROIDDEBUGGER_DIALOG };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	/*
	Full path to the executable to debug
	*/
	CString DebugProcessName;

	int TotalEventsCount;
	int ThreadCount;
	int DLLCount;
	int OutputDebugStringCount;
	int ExceptionCount;

	// TODO rename
	CStatic ThreadCountControl;
	CStatic DLLCountControl;
	CStatic TotalEventsControl;
	CStatic OutputDebugControl;
	CStatic ExceptionCountControl;

	/*
	*/
	CString GetFileNameFromHandle(HANDLE hFile);

	/*
	*/
	CString GetDebugStringFromDebugEvent(
		const DEBUG_EVENT& debugEvent, 
		const PROCESS_INFORMATION& processInfo) const;

	/*
	Used by `DebuggerThreadProc()`. Takes OUT arguments.
	*/
	void ProcessExceptionDebugEvent(
		const DEBUG_EVENT& debugEvent, 
		CString& eventMessage, 
		DWORD& continueStatus) const;

};
