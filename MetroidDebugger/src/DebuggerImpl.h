#pragma once
#include <wtypes.h>

class DebuggerImpl
{
public:
	DebuggerImpl(LPCTSTR debuggeePath, HWND dialogHandle);
	void DebuggerThreadProc();

private:
	/*
	Full path to the executable to debug
	*/
	LPCTSTR DebuggeePath;
	
	HWND DialogHandle;
};