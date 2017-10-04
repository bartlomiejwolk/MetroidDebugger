#include "MetroidDebugger.h"

MetroidDebugger MetroidDebuggerApp;

BOOL MetroidDebugger::InitInstance()
{
	CWinApp::InitInstance();

	AfxMessageBox(_T("Hello World"));
	return FALSE;
}

