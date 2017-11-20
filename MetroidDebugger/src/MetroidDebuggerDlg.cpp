// MetroidDebuggerDlg.cpp : implementation file
//

#include <afxdlgs.h>
#include <afxstr.h>
#include <concrt.h>
#include <map>
#include "stdafx.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "Psapi.h"
#include "Defines.h"
#include "MetroidDebuggerDlg.h"
#include "AboutDlg.h"
#include "DebuggerImpl.h"

IMPLEMENT_DYNAMIC(CMetroidDebuggerDlg, CDialog)

// TODO Consider creating a namespace
DWORD WINAPI DebuggerThread(void* param)
{
	CMetroidDebuggerDlg* thisDlg = static_cast<CMetroidDebuggerDlg*>(param);
	HWND thisDlgHandle = thisDlg->m_hWnd;

	// convert CString to pointer to string
	wchar_t* debuggeePath = thisDlg->DebuggeePath.GetBuffer(thisDlg->DebuggeePath.GetLength());
	
	/* Execute debugger loop */
	DebuggerImpl debugger(debuggeePath, thisDlgHandle);
	debugger.DebuggerThreadProc();
	
	return 0;
}

CMetroidDebuggerDlg::CMetroidDebuggerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_METROIDDEBUGGER_DIALOG, pParent)
{
	ResetDebuggerData();
	IsDebugging = false;
	
	Icon = AfxGetApp()->LoadIcon(IDI_MAINFRAME);
}

CMetroidDebuggerDlg::~CMetroidDebuggerDlg()
{
}

void CMetroidDebuggerDlg::OnBnClicked_StartDebugging()
{
	// "Stop debugging" button pressed
	if (IsDebugging)
	{
		TerminateThread(DebugThread, 0xDEAD);
		// TODO terminate also the debugee
		
		DebugEvents.InsertItem(TotalEventsCount, _T("*** Debugging terminated by the user ***"));
		DebugEvents.EnsureVisible(TotalEventsCount, false);

		IsDebugging = false;
		SetDebuggingModeUI();

		return;
	}

	ResetDebuggerData();
	
	/* Get path of the executable to debug */
	CFileDialog fileDialog(true, L"EXE", NULL, 6, L"Executables|*.exe||");
	if (fileDialog.DoModal() == IDCANCEL)
	{
		return;
	}
	DebuggeePath = fileDialog.GetPathName();

	// Create debug thread
	{
		DebugThread = CreateThread(0, 0, DebuggerThread, this, 0, 0);
		if (DebugThread == NULL)
		{
			AfxMessageBox(_T("Failed to start debugging!"));
			return;
		}
	}

	IsDebugging = true;
	SetDebuggingModeUI();
}

LRESULT CMetroidDebuggerDlg::OnDebugEventMessage(WPARAM wParam, LPARAM lParam)
{
	/* Get event message from wParam */
	std::wstring* messageStr ((std::wstring*)wParam);
	CString message ((*messageStr).c_str());
	
	switch (lParam)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"Process started: " + message);
		break;
	case CREATE_THREAD_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, message);
		ThreadCount++;
		break;
	case EXIT_THREAD_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, message);
		ThreadCount--;
		break;
	case EXIT_PROCESS_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"Process exited with code: " + message);
		IsDebugging = false;
		SetDebuggingModeUI();
		break;
	case LOAD_DLL_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"DLL loaded: " + message);
		DLLCount++;
		break;
	case UNLOAD_DLL_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"DLL Unloaded: " + message);
		break;
	case OUTPUT_DEBUG_STRING_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"Debug Message: " + message);
		OutputDebugStringCount++;
		break;
	case EXCEPTION_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"Debug Exception: " + message);
		ExceptionCount++;
		break;
	}

	TotalEventsCount++;
	DebugEvents.EnsureVisible(TotalEventsCount, false);
	CString label;

	label.Format(L"Total Debugging events: %d", TotalEventsCount);
	TotalEventsControl.SetWindowText(label);

	label.Format(L"Threads: %d", ThreadCount);
	ThreadCountControl.SetWindowText(label);

	label.Format(L"DLLs: %d", DLLCount);
	DLLCountControl.SetWindowText(label);

	label.Format(L"Exceptions: %d", ExceptionCount);
	ExceptionCountControl.SetWindowText(label);

	label.Format(L"OutputDebugs: %d", OutputDebugStringCount);
	OutputDebugControl.SetWindowText(label);

	return NULL;
}

void CMetroidDebuggerDlg::OnLvnItemchangedDebugEvents(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

BOOL CMetroidDebuggerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* sysMenu = GetSystemMenu(FALSE);
	if (sysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			sysMenu->AppendMenu(MF_SEPARATOR);
			sysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(Icon, TRUE);
	SetIcon(Icon, FALSE);

	DebugEvents.InsertColumn(0, L"Debug Event", LVCFMT_LEFT, 640);
	DebugEvents.SetExtendedStyle(DebugEvents.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	SetDebuggingModeUI();

	return TRUE;
}

void CMetroidDebuggerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEBUG_EVENTS, DebugEvents);
	DDX_Control(pDX, IDC_TOTAL_EVENTS, TotalEventsControl);
	DDX_Control(pDX, IDC_THREAD_COUNT, ThreadCountControl);
	DDX_Control(pDX, IDC_DLL_COUNT, DLLCountControl);
	DDX_Control(pDX, IDC_DEBUG_COUNT, OutputDebugControl);
	DDX_Control(pDX, IDC_EXCEPTION_COUNT, ExceptionCountControl);
}

void CMetroidDebuggerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}


BEGIN_MESSAGE_MAP(CMetroidDebuggerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START_DEBUG, &CMetroidDebuggerDlg::OnBnClicked_StartDebugging)
	ON_MESSAGE(DEBUG_EVENT_MESSAGE, OnDebugEventMessage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_DEBUG_EVENTS, &CMetroidDebuggerDlg::OnLvnItemchangedDebugEvents)
END_MESSAGE_MAP()

void CMetroidDebuggerDlg::SetDebuggingModeUI()
{
	ThreadCountControl.EnableWindow(IsDebugging);
	DLLCountControl.EnableWindow(IsDebugging);
	TotalEventsControl.EnableWindow(IsDebugging);
	OutputDebugControl.EnableWindow(IsDebugging);
	ExceptionCountControl.EnableWindow(IsDebugging);

	CWnd* startButton = GetDlgItem(IDC_START_DEBUG);
	if (IsDebugging)
	{
		startButton->SetWindowText(_T("&Stop Debugging"));
	}
	else
	{
		startButton->SetWindowText(_T("&Start Debugging"));
	}
}

void CMetroidDebuggerDlg::ResetDebuggerData()
{
	TotalEventsCount = 0;
	ThreadCount = 1;
	DLLCount = 0;
	OutputDebugStringCount = 0;
	ExceptionCount = 0;
}
