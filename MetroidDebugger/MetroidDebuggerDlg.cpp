// MetroidDebuggerDlg.cpp : implementation file
//

#include <afxdlgs.h>
#include "stdafx.h"
#include "src/MetroidDebugger.h"
#include "MetroidDebuggerDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include <afxstr.h>
#include <concrt.h>


// CMetroidDebuggerDlg dialog

IMPLEMENT_DYNAMIC(CMetroidDebuggerDlg, CDialog)

CMetroidDebuggerDlg::CMetroidDebuggerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG1, pParent)
{

}

CMetroidDebuggerDlg::~CMetroidDebuggerDlg()
{
}

void CMetroidDebuggerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMetroidDebuggerDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CMetroidDebuggerDlg::OnBnClicked_StartDebugging)
END_MESSAGE_MAP()

DWORD WINAPI DebuggerThread(void* param)
{
	CMetroidDebuggerDlg* thisDlg = static_cast<CMetroidDebuggerDlg*>(param);
	thisDlg->DebuggerThreadProc();
	
	return 0;
}

// CMetroidDebuggerDlg message handlers


void CMetroidDebuggerDlg::OnBnClicked_StartDebugging()
{
	// Terminate thread if is debugging

	// Get executable to debug
	CFileDialog fileDialog(true, L"EXE", 0, 6, L"Executables|*.exe||");
	if (fileDialog.DoModal() == IDCANCEL)
	{
		return;
	}

	// Create debug thread
	CString debugProcessName = fileDialog.GetPathName();
	HANDLE debugThread = CreateThread(0, 0, DebuggerThread, this, 0, 0);
	if (debugThread == NULL)
	{
		AfxMessageBox(_T("Failed to start debugging!"));
	}
}

void CMetroidDebuggerDlg::DebuggerThreadProc()
{

}
