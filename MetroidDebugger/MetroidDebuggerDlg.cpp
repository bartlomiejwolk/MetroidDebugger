// MetroidDebuggerDlg.cpp : implementation file
//

#include <afxdlgs.h>
#include <afxstr.h>
#include <concrt.h>
#include "stdafx.h"
#include "afxdialogex.h"
#include "resource.h"
#include "src/MetroidDebugger.h"
#include "MetroidDebuggerDlg.h"

// For thread messaging
#define DEBUG_EVENT_MESSAGE		WM_APP + 0x100

IMPLEMENT_DYNAMIC(CMetroidDebuggerDlg, CDialog)

CMetroidDebuggerDlg::CMetroidDebuggerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_METROIDDEBUGGER_DIALOG, pParent)
{

}

CMetroidDebuggerDlg::~CMetroidDebuggerDlg()
{
}

void CMetroidDebuggerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEBUG_EVENTS, m_cDebugEvents);
}


BEGIN_MESSAGE_MAP(CMetroidDebuggerDlg, CDialog)
	ON_BN_CLICKED(IDC_START_DEBUG, &CMetroidDebuggerDlg::OnBnClicked_StartDebugging)
	ON_MESSAGE(DEBUG_EVENT_MESSAGE, OnDebugEventMessage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_DEBUG_EVENTS, &CMetroidDebuggerDlg::OnLvnItemchangedDebugEvents)
END_MESSAGE_MAP()

DWORD WINAPI DebuggerThread(void* param)
{
	CMetroidDebuggerDlg* thisDlg = static_cast<CMetroidDebuggerDlg*>(param);
	thisDlg->DebuggerThreadProc();
	
	return 0;
}

// CMetroidDebuggerDlg message handlers


void CMetroidDebuggerDlg::OnBnClicked_StartDebugging()
{
	// TODO Terminate thread if is debugging

	// Reset fields
	TotalEventsCount = 0;

	// Get executable to debug
	CFileDialog fileDialog(true, L"EXE", NULL, 6, L"Executables|*.exe||");
	if (fileDialog.DoModal() == IDCANCEL)
	{
		return;
	}

	// Create debug thread
	DebugProcessName = fileDialog.GetPathName();
	HANDLE debugThread = CreateThread(0, 0, DebuggerThread, this, 0, 0);
	if (debugThread == NULL)
	{
		AfxMessageBox(_T("Failed to start debugging!"));
	}
}

void CMetroidDebuggerDlg::DebuggerThreadProc()
{
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	ZeroMemory(&processInfo, sizeof(processInfo));

	CreateProcess(
		DebugProcessName,
		NULL,
		NULL,
		NULL,
		false,
		DEBUG_ONLY_THIS_PROCESS,
		NULL,
		NULL,
		&startupInfo,
		&processInfo);

	DEBUG_EVENT debugEvent;
	CString eventMessage;
	DWORD continueStatus = DBG_CONTINUE;
	
	bool continueDebugging = true;
	while (continueDebugging)
	{
		if (!WaitForDebugEvent(&debugEvent, INFINITE))
		{
			return;
		}

		switch (debugEvent.dwDebugEventCode)
		{
			case CREATE_PROCESS_DEBUG_EVENT:
				eventMessage = GetFileNameFromHandle(debugEvent.u.CreateProcessInfo.hFile);
				break;
			
			case CREATE_THREAD_DEBUG_EVENT:
				break;
			
			case EXIT_THREAD_DEBUG_EVENT:
				break;
		
			case EXIT_PROCESS_DEBUG_EVENT:
				break;
		
			case LOAD_DLL_DEBUG_EVENT:
				break;
		
			case UNLOAD_DLL_DEBUG_EVENT:
				break;
		
			case OUTPUT_DEBUG_STRING_EVENT:
				break;
		
			case EXCEPTION_DEBUG_EVENT:
				break;
		}

		SendMessage(DEBUG_EVENT_MESSAGE, (WPARAM)&eventMessage, debugEvent.dwDebugEventCode);
		ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, continueStatus);
		continueStatus = DBG_CONTINUE;
	}
}

LRESULT CMetroidDebuggerDlg::OnDebugEventMessage(WPARAM wParam, LPARAM lParam)
{
	CString* pMessage = (CString*)wParam;
	switch (lParam)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
		//OutputDebugStringW(_T("???"));
		m_cDebugEvents.InsertItem(TotalEventsCount, L"Process started: " + *pMessage);
		break;
	}

	TotalEventsCount++;

	return NULL;
}

CString CMetroidDebuggerDlg::GetFileNameFromHandle(HANDLE hFile)
{
	return CString();
}


void CMetroidDebuggerDlg::OnLvnItemchangedDebugEvents(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
