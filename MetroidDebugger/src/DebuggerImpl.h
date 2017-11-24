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

	// TODO rename
	std::wstring EventMessage = {};

	DEBUG_EVENT DebugEvent = {};

	bool ContinueDebugging = true;

	std::map<LPVOID, std::wstring> DLLNameMap;

	PROCESS_INFORMATION ProcessInfo = {};
	
	DWORD ContinueStatus = DBG_CONTINUE;

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
	
	void HandleCreateProcessDebugEvent();
	void HandleCreateThreadDebugEvent();
	void HandleExitThreadDebugEvent();
	void HandleExitProcessDebugEvent();
	void HandleLoadDllDebugEvent();
	void HandleUnloadDllDebugEvent();
	void HandleOutputDebugStringEvent();
	void HandleExceptionDebugEvent();
};