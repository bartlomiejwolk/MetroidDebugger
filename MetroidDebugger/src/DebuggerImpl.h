#pragma once
#include <wtypes.h>

class DebuggerImpl
{
public:
	DebuggerImpl(LPCTSTR debuggeePath, HWND dialogHandle);

	/*
	Creates process to debug, listenes and handles debug events, controls
	execution of the debugee.
	*/
	void DebuggerThreadProc();

private:
	/*
	Handle to the application main window.
	*/
	HWND MainDialogHandle;

	/*
	Full path to the executable to debug
	*/
	LPCTSTR DebuggeePath;

	/*
	Process info returned by OS CreateProcess() function.
	*/
	PROCESS_INFORMATION DebuggeeProcessInfo;

	/*
	Last debug event info returned by OS function `WaitForDebugEvent()`.
	*/
	DEBUG_EVENT DebugEvent;

	/*
	Message to be displayed to the user on debug event.
	*/
	std::wstring EventMessage;

	/*
	True if the very first breakpoint sent by the OS on application start was hit.
	*/
	bool OsBreakpointHit;

	/*
	Controls debugger loop. The debugger thread will end when this is set to false.
	*/
	bool ContinueDebugging;

	/*
	Use by `ContinueDebugEvent()` in the debugger loop to control debuggee.
	*/
	DWORD ContinueStatus;

	/*
	Cache used to store loaded DLL names. Used to report unloaded DLLs.
	LPVOID - pointer to DLL base
	wstring - DLL name
	*/
	std::map<LPVOID, std::wstring> DLLNameMap;

private:
	void CreateDebuggeeProcess();

	/*
	*/
	std::wstring GetDebugStringFromDebugEvent(
		const DEBUG_EVENT& debugEvent,
		const PROCESS_INFORMATION& processInfo) const;

	/*
	*/
	std::wstring GetFileNameFromHandle(HANDLE hFile);
	
	/*
	*/
	std::wstring WStringFormat(const wchar_t* fmt_str, ...);

	/*
	Start address of the debuggee main func. taken from `CREATE_PROCESS_DEBUG_INFO`
	struct.
	*/
	LPTHREAD_START_ROUTINE DebuggeeStartAddress;

	void HandleCreateProcessDebugEvent();
	void HandleCreateThreadDebugEvent();
	void HandleExitThreadDebugEvent();
	void HandleExitProcessDebugEvent();
	void HandleLoadDllDebugEvent();
	void HandleUnloadDllDebugEvent();
	void HandleOutputDebugStringEvent();
	void HandleExceptionDebugEvent();

	void HandleStatusBreakpointExceptionCode();
	void InsertBreakpointInstruction();
	void HandleOtherExceptionCode(const EXCEPTION_DEBUG_INFO &exceptionInfo);
};