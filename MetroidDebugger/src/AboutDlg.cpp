// AboutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MetroidDebugger.h"
#include "AboutDlg.h"
#include "afxdialogex.h"
#include "../resource.h"

// CAboutDlg dialog

IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)

CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_ABOUTBOX, pParent)
{

}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAboutDlg message handlers
