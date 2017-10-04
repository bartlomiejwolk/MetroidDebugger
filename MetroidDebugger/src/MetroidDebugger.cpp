#include "MetroidDebugger.h"

CMetroidDebugger MetroidDebuggerApp;

BOOL CMetroidDebugger::InitInstance()
{
	CWinApp::InitInstance();

	AfxMessageBox(_T("Hello World"));
	return FALSE;
}

