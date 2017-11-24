#pragma once
#include <wtypes.h>

class DebuggerImpl
{
public:
	DebuggerImpl(LPCTSTR debuggeePath, HWND dialogHandle);

	/*
	Creates process to debug, listenes and handles debug events, controls
	execution of the debugee. Method executed on a separate thread.
	*/
	void DebuggerThreadProc();

private:
	/*
	Full path to the executable to debug
	*/
	LPCTSTR DebuggeePath;
	
	HWND DialogHandle;

	/*
	True if the very first breakpoint sent by the OS on application start was hit.
	*/
	bool OsBreakpointHit;

	/*
	*/
	std::wstring GetDebugStringFromDebugEvent(
		const DEBUG_EVENT& debugEvent,
		const PROCESS_INFORMATION& processInfo) const;

	/*
	*/
	std::wstring WStringFormat(const wchar_t* fmt_str, ...);

	/*
	*/
	std::wstring GetFileNameFromHandle(HANDLE hFile);

	/*
	*/
	DWORD GetStartAddress(HANDLE hProcess, HANDLE hThread);
};