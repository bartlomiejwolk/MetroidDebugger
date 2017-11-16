#pragma once
#include <afxcmn.h>
#include <map>
#include "DebuggerImpl.h"

class CMetroidDebuggerDlg : public CDialog
{
	DECLARE_DYNAMIC(CMetroidDebuggerDlg)

public:
	/*
	Contains debug messages displayed to the user through List Control
	*/
	CListCtrl DebugEvents;

	CMetroidDebuggerDlg(CWnd* pParent = NULL);   // standard constructor

	/*
	Resets data collected by the debugger during the debugging process.
	*/
	void ResetDebuggerData();

	virtual ~CMetroidDebuggerDlg();

	/*
	Start debugging button handler.
	*/
	afx_msg void OnBnClicked_StartDebugging();
	
	/*
	Creates process to debug, listenes and handles debug events, controls 
	execution of the debugee. Method executed on a separate thread.
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
	HICON Icon;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	/*
	*/
	HANDLE DebugThread;

	/*
	True as long as the debug process runs.
	*/
	bool IsDebugging;

	/*
	True if the very first breakpoint sent by the OS on application start was hit.
	*/
	bool OsBreakpointHit;

	/* Helper variables used to store debugger data. Used to populate text controls. */
	int TotalEventsCount;
	int ThreadCount;
	int DLLCount;
	int OutputDebugStringCount;
	int ExceptionCount;

	/* Text controls used to display debugger info to the user. */
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
	Updates UI properties.
	*/
	void SetDebuggingModeUI();
};
