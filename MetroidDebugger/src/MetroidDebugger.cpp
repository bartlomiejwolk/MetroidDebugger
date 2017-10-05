#include "MetroidDebugger.h"
#include "../MetroidDebuggerDlg.h"

CMetroidDebugger MetroidDebuggerApp;

BOOL CMetroidDebugger::InitInstance()
{
	CWinApp::InitInstance();

	//AfxMessageBox(_T("Hello World"));

	CMetroidDebuggerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR response = dlg.DoModal();

	if (response == IDOK)
	{
		OutputDebugStringW(L"response is IDOK");
	}
	else if (response == IDCANCEL)
	{

	}

	return FALSE;
}

