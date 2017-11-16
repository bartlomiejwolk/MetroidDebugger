#include <iostream>
#include <tchar.h>
#include <Windows.h>
#include "DebuggerImpl.h"

//void DebuggerImpl::SetDebugProcessName(LPCTSTR debugProcessName)
//{
//	DebugProcessName = debugProcessName;
//}

DebuggerImpl::DebuggerImpl(LPCTSTR debugeePath)
	: DebuggeePath(debugeePath)
{
}

void DebuggerImpl::DebuggerThreadProc()
{
	// Create process to debug
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

}

