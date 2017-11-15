#include <iostream>
#include <tchar.h>
#include <Windows.h>
#include "DebuggerImpl.h"

void DebuggerImpl::DebuggerThreadProc()
{
	OutputDebugString(_T("DebuggerThreadProc()\n"));
}

