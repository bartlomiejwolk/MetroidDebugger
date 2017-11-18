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

#define BUFSIZE 512

IMPLEMENT_DYNAMIC(CMetroidDebuggerDlg, CDialog)

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
		/* Create copy of the path */
		//size_t pathLen = DebuggeePath.GetLength();
		/*wchar_t* pathHeap = new wchar_t[pathLen + 1];
		wcscpy_s(pathHeap, pathLen + 1, path.GetBuffer(pathLen));*/

		/* Create thread*/
		DebugThread = CreateThread(0, 0, DebuggerThread, this, 0, 0);
		if (DebugThread == NULL)
		{
			//delete(pathHeap);
			AfxMessageBox(_T("Failed to start debugging!"));
			return;
		}
	}

	IsDebugging = true;
	SetDebuggingModeUI();
}

static DWORD GetStartAddress(HANDLE hProcess, HANDLE hThread)
{
	// TODO implement
	//(LPVOID)debugEvent.u.CreateProcessInfo.lpStartAddress;
	return 0;
}

void CMetroidDebuggerDlg::DebuggerThreadProc()
{
	PROCESS_INFORMATION processInfo;

	// Debugger loop
	{
		// debug event data from the OS
		DEBUG_EVENT debugEvent = {};
		// message displayed to the user
		CString eventMessage;
		// used by `ContinueDebugEvent()` in case of exception
		DWORD continueStatus = DBG_CONTINUE;
		// DLL info cache, used to report about unloaded DLLs
		std::map<LPVOID, CString> DLLNameMap;

		bool continueDebugging = true;
		while (continueDebugging)
		{
			WaitForDebugEvent(&debugEvent, INFINITE);

			switch (debugEvent.dwDebugEventCode)
			{
			case CREATE_PROCESS_DEBUG_EVENT:
				{
					eventMessage = GetFileNameFromHandle(debugEvent.u.CreateProcessInfo.hFile);

					// insert break point instruction at program start
					{
						DWORD dwStartAddress = GetStartAddress(processInfo.hProcess, processInfo.hThread);
						BYTE cInstruction;
						DWORD dwReadBytes;
						ReadProcessMemory(processInfo.hProcess, (void*)dwStartAddress, &cInstruction, 1, &dwReadBytes);
						BYTE originalInstruction = cInstruction;
						cInstruction = 0xCC;
						WriteProcessMemory(processInfo.hProcess, (void*)dwStartAddress, &cInstruction, 1, &dwReadBytes);
						FlushInstructionCache(processInfo.hProcess, (void*)dwStartAddress, 1);
					}
				}
				break;

			case CREATE_THREAD_DEBUG_EVENT:
				eventMessage.Format(
					L"Thread 0x%x (Id. %d) created at: 0x%x",
					debugEvent.u.CreateThread.hThread,
					debugEvent.dwThreadId,
					debugEvent.u.CreateThread.lpStartAddress);
				break;

			case EXIT_THREAD_DEBUG_EVENT:
				eventMessage.Format(
					L"Thread %d exited with code: %d",
					debugEvent.dwThreadId,
					debugEvent.u.ExitThread.dwExitCode);
				break;

			case EXIT_PROCESS_DEBUG_EVENT:
				eventMessage.Format(L"0x%x", debugEvent.u.ExitProcess.dwExitCode);
				continueDebugging = false;
				IsDebugging = false;
				break;

			case LOAD_DLL_DEBUG_EVENT:
				eventMessage = GetFileNameFromHandle(debugEvent.u.LoadDll.hFile);
				DLLNameMap.insert(std::make_pair(debugEvent.u.LoadDll.lpBaseOfDll, eventMessage));
				eventMessage.AppendFormat(L" %x", debugEvent.u.LoadDll.lpBaseOfDll);
				break;

			case UNLOAD_DLL_DEBUG_EVENT:
				eventMessage.Format(L"%s", DLLNameMap[debugEvent.u.UnloadDll.lpBaseOfDll]);
				break;

			case OUTPUT_DEBUG_STRING_EVENT:
				eventMessage = GetDebugStringFromDebugEvent(debugEvent, processInfo);
				break;

			case EXCEPTION_DEBUG_EVENT:
				const EXCEPTION_DEBUG_INFO& exceptionInfo = debugEvent.u.Exception;
				switch (exceptionInfo.ExceptionRecord.ExceptionCode)
				{
				case STATUS_BREAKPOINT:
					{
					// breakpoint that was set by the debugger
						if (OsBreakpointHit)
						{
							CONTEXT lcContext;
							lcContext.ContextFlags = CONTEXT_ALL;
							GetThreadContext(processInfo.hThread, &lcContext);
						}
						// First brakpoint sent by OS
						else
						{
							OsBreakpointHit = true;
						}
					}
					eventMessage = "Break point";
					break;

				default:
					if (exceptionInfo.dwFirstChance == 1)
					{
						eventMessage.Format(
							L"First chance exception at %x, exception code: 0x%08x",
							exceptionInfo.ExceptionRecord.ExceptionAddress,
							exceptionInfo.ExceptionRecord.ExceptionCode);
					}
					continueStatus = DBG_EXCEPTION_NOT_HANDLED;
				}
				break;
			}

			SendMessage(DEBUG_EVENT_MESSAGE, (WPARAM)&eventMessage, debugEvent.dwDebugEventCode);
			ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, continueStatus);
			continueStatus = DBG_CONTINUE;
		}
	}
}

LRESULT CMetroidDebuggerDlg::OnDebugEventMessage(WPARAM wParam, LPARAM lParam)
{
	CString* message = (CString*)wParam;
	switch (lParam)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"Process started: " + *message);
		break;
	case CREATE_THREAD_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, *message);
		ThreadCount++;
		break;
	case EXIT_THREAD_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, *message);
		ThreadCount--;
		break;
	case EXIT_PROCESS_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"Process exited with code: " + *message);
		SetDebuggingModeUI();
		break;
	case LOAD_DLL_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"DLL loaded: " + *message);
		DLLCount++;
		break;
	case UNLOAD_DLL_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"DLL Unloaded: " + *message);
		break;
	case OUTPUT_DEBUG_STRING_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"Debug Message: " + *message);
		OutputDebugStringCount++;
		break;
	case EXCEPTION_DEBUG_EVENT:
		DebugEvents.InsertItem(TotalEventsCount, L"Debug Exception: " + *message);
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

// This function is optimized!
CString CMetroidDebuggerDlg::GetFileNameFromHandle(HANDLE hFile)
{
	BOOL bSuccess = FALSE;
	TCHAR pszFilename[MAX_PATH + 1];
	HANDLE hFileMap;

	CString strFilename;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

	if (dwFileSizeLo == 0 && dwFileSizeHi == 0)
	{
		return CString();
	}

	// Create a file mapping object.
	hFileMap = CreateFileMapping(hFile,
		NULL,
		PAGE_READONLY,
		0,
		1,
		NULL);

	if (hFileMap)
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem)
		{
			if (GetMappedFileName(GetCurrentProcess(),
				pMem,
				pszFilename,
				MAX_PATH))
			{

				// Translate path with device name to drive letters.
				TCHAR szTemp[BUFSIZE];
				szTemp[0] = '\0';

				if (GetLogicalDriveStrings(BUFSIZE - 1, szTemp))
				{
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;

					do
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDevice(szDrive, szName, MAX_PATH))
						{
							size_t uNameLen = _tcslen(szName);

							if (uNameLen < MAX_PATH)
							{
								bFound = _tcsnicmp(pszFilename, szName,
									uNameLen) == 0;

								if (bFound)
								{
									strFilename.Format(L"%s%s", szDrive, pszFilename + uNameLen);
								}
							}
						}

						// Go to the next NULL character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}
			bSuccess = TRUE;
			UnmapViewOfFile(pMem);
		}

		CloseHandle(hFileMap);
	}

	return(strFilename);
}


CString CMetroidDebuggerDlg::GetDebugStringFromDebugEvent(
	const DEBUG_EVENT &debugEvent, 
	const PROCESS_INFORMATION &processInfo) const
{
	const OUTPUT_DEBUG_STRING_INFO& debugStringInfo = debugEvent.u.DebugString;

	WCHAR* msg = new WCHAR[debugStringInfo.nDebugStringLength];
	ZeroMemory(msg, debugStringInfo.nDebugStringLength);

	ReadProcessMemory(
		processInfo.hProcess,
		debugStringInfo.lpDebugStringData,
		msg,
		debugStringInfo.nDebugStringLength,
		NULL);

	CString debugString;
	if (debugStringInfo.fUnicode)
	{
		debugString = msg;
	}
	else
	{
		debugString = (LPSTR)msg;
	}
	delete[] msg;		

	return debugString;
}

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
