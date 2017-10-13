#include "MetroidDebugger.h"
#include "../MetroidDebuggerDlg.h"

CMetroidDebugger MetroidDebuggerApp;

BOOL CMetroidDebugger::InitInstance()
{
	CWinApp::InitInstance();

	CMetroidDebuggerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR response = dlg.DoModal();

	if (response == IDOK)
	{
	}
	else if (response == IDCANCEL)
	{
	}

	return FALSE;
}

