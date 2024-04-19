#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"

class CDlgOptPlay :public eck::CDialog
{
private:

public:
	PNInline HWND CreateDlg(HWND hParent, void* pData = NULL) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_OPTPLAYING), hParent);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};