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

	void CreateDebuggeeProcess();

private:
	/*
	Full path to the executable to debug
	*/
	LPCTSTR DebuggeePath;
	
	// TODO rename to MainDialogHandle
	HWND DialogHandle;

	/*
	True if the very first breakpoint sent by the OS on application start was hit.
	*/
	bool OsBreakpointHit;

	/*
	Message to be displayed to the user on debug event.
	*/
	std::wstring EventMessage = {};

	/*
	Last debug event info returned by OS function `WaitForDebugEvent()`.
	*/
	DEBUG_EVENT DebugEvent = {};

	/*
	Controls debugger loop. The debugger thread will end when this is set to false.
	*/
	bool ContinueDebugging = true;

	/*
	Cache used to store loaded DLL names. Used to report unloaded DLLs.
	LPVOID - pointer to DLL base
	wstring - DLL name
	*/
	std::map<LPVOID, std::wstring> DLLNameMap;

	/*
	Process info returned by OS CreateProcess() function.
	*/
	// TODO rename to DebuggeeProcessInfo
	PROCESS_INFORMATION ProcessInfo = {};
	
	/*
	Use by `ContinueDebugEvent()` in the debugger loop to control debuggee.
	*/
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