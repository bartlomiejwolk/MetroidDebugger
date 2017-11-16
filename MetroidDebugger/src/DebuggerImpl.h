#pragma once
#include <wtypes.h>

class DebuggerImpl
{
public:
	DebuggerImpl(LPCTSTR debuggeePath);
	//void SetDebugProcessName(LPCTSTR debugProcessName);
	void DebuggerThreadProc();

private:
	/*
	Full path to the executable to debug
	*/
	LPCTSTR DebuggeePath;

};