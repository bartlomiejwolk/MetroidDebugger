// MetroidDebuggerDlg.cpp : implementation file
//

#include <afxdlgs.h>
#include "stdafx.h"
#include "src/MetroidDebugger.h"
#include "MetroidDebuggerDlg.h"
#include "afxdialogex.h"
#include "resource.h"


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


// CMetroidDebuggerDlg message handlers


void CMetroidDebuggerDlg::OnBnClicked_StartDebugging()
{
	// terminate thread if is debugging

	// Open file dialog
	CFileDialog fileDialog(true, L"EXE", 0, 6, L"Executables|*.exe||");
	if (fileDialog.DoModal() == IDCANCEL)
	{
		return;
	}
}
