#pragma once

#include <afxwin.h>

class CMetroidDebugger : public CWinApp
{
public:
	BOOL InitInstance() override;
};

extern CMetroidDebugger MetroidDebuggerApp;