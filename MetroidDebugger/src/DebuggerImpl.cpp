#include <iostream>
#include <tchar.h>
#include <Windows.h>
#include <string>
#include <map>
#include <Psapi.h>
#include <memory>
#include <iosfwd>
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr
#include <string.h>
#include <utility>
#include "Defines.h"
#include "DebuggerImpl.h"

#define BUFSIZE 512

DebuggerImpl::DebuggerImpl(LPCTSTR debugeePath, HWND dialogHandle) : 
	DebuggeePath(debugeePath), 
	MainDialogHandle(dialogHandle), 
	DebuggeeProcessInfo{},
	DebugEvent{},
	EventMessage{},
	OsBreakpointHit(false),
	ContinueDebugging(true),
	ContinueStatus(DBG_CONTINUE),
	DLLNameMap{},
	DebuggeeStartAddress{}
{
}

void DebuggerImpl::DebuggerThreadProc()
{
	CreateDebuggeeProcess();

	// Debugger loop
	while (ContinueDebugging)
	{
		WaitForDebugEvent(&DebugEvent, INFINITE);

		switch (DebugEvent.dwDebugEventCode)
		{
		case CREATE_PROCESS_DEBUG_EVENT:
			HandleCreateProcessDebugEvent();
			break;

		case CREATE_THREAD_DEBUG_EVENT:
			HandleCreateThreadDebugEvent();
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			HandleExitThreadDebugEvent();
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			HandleExitProcessDebugEvent();
			break;

		case LOAD_DLL_DEBUG_EVENT:
			HandleLoadDllDebugEvent();
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			HandleUnloadDllDebugEvent();
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			HandleOutputDebugStringEvent();
			break;

		case EXCEPTION_DEBUG_EVENT:
			HandleExceptionDebugEvent();
			break;
		}

		// notify application main window
		SendMessage(MainDialogHandle, DEBUG_EVENT_MESSAGE, (WPARAM)&EventMessage, DebugEvent.dwDebugEventCode);

		ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, ContinueStatus);
		ContinueStatus = DBG_CONTINUE;
	}
}

void DebuggerImpl::CreateDebuggeeProcess()
{
	STARTUPINFO startupInfo;

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	ZeroMemory(&DebuggeeProcessInfo, sizeof(DebuggeeProcessInfo));

	CreateProcess(
		DebuggeePath,
		NULL,
		NULL,
		NULL,
		false,
		DEBUG_ONLY_THIS_PROCESS,
		NULL,
		NULL,
		&startupInfo,
		&DebuggeeProcessInfo);
}

std::wstring DebuggerImpl::GetDebugStringFromDebugEvent(
	const DEBUG_EVENT &debugEvent,
	const PROCESS_INFORMATION &processInfo) const
{
	const OUTPUT_DEBUG_STRING_INFO& debugStringInfo = debugEvent.u.DebugString;

	/* Read debug message */
	LPVOID* lpvMsg = new LPVOID[debugStringInfo.nDebugStringLength];
	ZeroMemory(lpvMsg, debugStringInfo.nDebugStringLength);
	ReadProcessMemory(
		processInfo.hProcess,
		debugStringInfo.lpDebugStringData,
		lpvMsg,
		debugStringInfo.nDebugStringLength,
		NULL);

	/* Convert debug message to Unicode. */
	std::wstring debugString;
	if (debugStringInfo.fUnicode)
	{
		wchar_t* wcPtr = reinterpret_cast<wchar_t*>(lpvMsg);
		debugString = std::wstring(wcPtr);
	}
	else
	{
		char* cPtr = reinterpret_cast<char*>(lpvMsg);
		
		int wcharsNum = MultiByteToWideChar(CP_UTF8, 0, cPtr, -1, NULL, 0);
		wchar_t* buff = new wchar_t[wcharsNum];
		MultiByteToWideChar(CP_UTF8, 0, cPtr, -1, buff, wcharsNum);

		debugString = std::wstring(buff);
		delete[] buff;
	}
	delete[] lpvMsg;

	return debugString;
}

// This function is optimized!
std::wstring DebuggerImpl::GetFileNameFromHandle(HANDLE hFile)
{
	BOOL bSuccess = FALSE;
	TCHAR pszFilename[MAX_PATH + 1];
	HANDLE hFileMap;

	std::wstring strFilename;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

	if (dwFileSizeLo == 0 && dwFileSizeHi == 0)
	{
		return strFilename;
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
									strFilename = WStringFormat(L"%s%s", szDrive, pszFilename + uNameLen);

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

/*
Source: https://stackoverflow.com/a/8098080/2964286
*/
std::wstring DebuggerImpl::WStringFormat(const wchar_t* fmt_str, ...)
{
	int final_n, n = wcslen(fmt_str) * 2; // make it even
	std::unique_ptr<wchar_t[]> formatted;
	va_list ap;
	while (1)
	{
		formatted.reset(new wchar_t[n]);
		wcscpy_s(&formatted[0], n, fmt_str);
		va_start(ap, fmt_str);
		final_n = vswprintf(&formatted[0], n, fmt_str, ap);
		va_end(ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	return std::wstring(formatted.get());
}

void DebuggerImpl::HandleCreateProcessDebugEvent()
{
	EventMessage = GetFileNameFromHandle(DebugEvent.u.CreateProcessInfo.hFile);
	DebuggeeStartAddress = DebugEvent.u.CreateProcessInfo.lpStartAddress;
}

void DebuggerImpl::HandleCreateThreadDebugEvent()
{
	EventMessage = WStringFormat(
		L"Thread 0x%x (Id. %d) created at: 0x%x",
		DebugEvent.u.CreateThread.hThread,
		DebugEvent.dwThreadId,
		DebugEvent.u.CreateThread.lpStartAddress);
}

void DebuggerImpl::HandleExitThreadDebugEvent()
{
	EventMessage = WStringFormat(
		L"Thread %d exited with code: %d",
		DebugEvent.dwThreadId,
		DebugEvent.u.ExitThread.dwExitCode);
}

void DebuggerImpl::HandleExitProcessDebugEvent()
{
	EventMessage = WStringFormat(L"0x%x", DebugEvent.u.ExitProcess.dwExitCode);
	ContinueDebugging = false;
}

void DebuggerImpl::HandleLoadDllDebugEvent()
{
	EventMessage = GetFileNameFromHandle(DebugEvent.u.LoadDll.hFile); "";
	DLLNameMap.insert(std::make_pair(DebugEvent.u.LoadDll.lpBaseOfDll, EventMessage));
	EventMessage += WStringFormat(L" %x", DebugEvent.u.LoadDll.lpBaseOfDll);
}

void DebuggerImpl::HandleUnloadDllDebugEvent()
{
	std::wstring DLLName = DLLNameMap[DebugEvent.u.UnloadDll.lpBaseOfDll];
	EventMessage = WStringFormat(L"%ls", DLLName.c_str());
}

void DebuggerImpl::HandleOutputDebugStringEvent()
{
	EventMessage = GetDebugStringFromDebugEvent(DebugEvent, DebuggeeProcessInfo);
}

void DebuggerImpl::HandleExceptionDebugEvent()
{
	const EXCEPTION_DEBUG_INFO& exceptionInfo = DebugEvent.u.Exception;
	switch (exceptionInfo.ExceptionRecord.ExceptionCode)
	{
	case STATUS_BREAKPOINT:
		HandleStatusBreakpointExceptionCode();
		break;
	case STATUS_SINGLE_STEP:
		HandleSingleStepExceptionCode();
		break;
	default:
		HandleOtherExceptionCode(exceptionInfo);
	}
}

// TODO rename to HandleBreakpointExceptionCode()
void DebuggerImpl::HandleStatusBreakpointExceptionCode()
{
	// breakpoint that was set by the debugger
	if (OsBreakpointHit)
	{
		/* Restore previous instruction */
		DWORD dwWriteSize;
		WriteProcessMemory(
			DebuggeeProcessInfo.hProcess,
			(void*)DebuggeeStartAddress,
			&OriginalInstruction,
			1,
			&dwWriteSize);

		FlushInstructionCache(DebuggeeProcessInfo.hProcess, (void*)DebuggeeStartAddress, 1);

		/* Move instruction pointer back one byte */
		CONTEXT lcContext;
		lcContext.ContextFlags = CONTEXT_ALL;
		GetThreadContext(DebuggeeProcessInfo.hThread, &lcContext);
		lcContext.Eip--;
		SetThreadContext(DebuggeeProcessInfo.hThread, &lcContext);
	
		EventMessage = L"Debugger break point";
	}
	// first brakpoint sent by the OS
	else
	{
		OsBreakpointHit = true;
		InsertBreakpointInstruction();
		EventMessage = L"OS break point";
	}
}

void DebuggerImpl::HandleSingleStepExceptionCode()
{

}

void DebuggerImpl::InsertBreakpointInstruction()
{
	BYTE cInstruction{};
	DWORD dwReadBytes{};

	ReadProcessMemory(
		DebuggeeProcessInfo.hProcess,
		(void*)DebuggeeStartAddress,
		&cInstruction,
		1,
		&dwReadBytes);

	OriginalInstruction = cInstruction;
	cInstruction = 0xCC;

	WriteProcessMemory(
		DebuggeeProcessInfo.hProcess,
		(void*)DebuggeeStartAddress,
		&cInstruction,
		1,
		&dwReadBytes);

	FlushInstructionCache(DebuggeeProcessInfo.hProcess, (void*)DebuggeeStartAddress, 1);
}

void DebuggerImpl::HandleOtherExceptionCode(const EXCEPTION_DEBUG_INFO &exceptionInfo)
{
	if (exceptionInfo.dwFirstChance == 1)
	{
		EventMessage = WStringFormat(
			L"First chance exception at %x, exception code: 0x%08x",
			exceptionInfo.ExceptionRecord.ExceptionAddress,
			exceptionInfo.ExceptionRecord.ExceptionCode);
	}
	ContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
}
