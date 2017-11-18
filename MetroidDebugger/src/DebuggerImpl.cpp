#include <iostream>
#include <tchar.h>
#include <Windows.h>
#include <string>
#include <map>
#include "Defines.h"
#include "DebuggerImpl.h"

DebuggerImpl::DebuggerImpl(LPCTSTR debugeePath, HWND dialogHandle)
	: DebuggeePath(debugeePath), DialogHandle(dialogHandle)
{
}

void DebuggerImpl::DebuggerThreadProc()
{
	// Create debuggee process
	PROCESS_INFORMATION processInfo;
	{
		STARTUPINFO startupInfo;

		ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		ZeroMemory(&processInfo, sizeof(processInfo));

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
			&processInfo);
	}

	// Debugger loop
	{
		// debug event data from the OS
		DEBUG_EVENT debugEvent = {};
		// message displayed to the user
		std::string eventMessage = {};
		// used by `ContinueDebugEvent()` in case of exception
		DWORD continueStatus = DBG_CONTINUE;
		// DLL info cache, used to report about unloaded DLLs
		std::map<LPVOID, std::string> DLLNameMap;

		bool continueDebugging = true;
		while (continueDebugging)
		{
			WaitForDebugEvent(&debugEvent, INFINITE);

			switch (debugEvent.dwDebugEventCode)
			{
			case CREATE_PROCESS_DEBUG_EVENT:
				OutputDebugString(L"CREATE_PROCESS_DEBUG_EVENT\n");
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

			SendMessage(DialogHandle, DEBUG_EVENT_MESSAGE, (WPARAM)&eventMessage, debugEvent.dwDebugEventCode);
			ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, continueStatus);
			continueStatus = DBG_CONTINUE;
		}
	}
}

