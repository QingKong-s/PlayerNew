#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"

class CDlgOptHotKey :public eck::CDialog
{
private:

public:
	PNInline HWND CreateDlg(HWND hParent, void* pData = NULL) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_OPTHOTKEY), hParent);
	}
};