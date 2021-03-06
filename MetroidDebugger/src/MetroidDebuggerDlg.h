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

	// TODO Make private. Create getter.
	CString DebuggeePath;

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
	Updates UI properties according to debugger state.
	*/
	void SetDebuggingModeUI();
};
