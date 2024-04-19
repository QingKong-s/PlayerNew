#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CButton.h"

class CDlgOptVisual :public eck::CDialog
{
private:
	eck::CCheckButton m_CBScrollOverflowText{};
public:
	PNInline HWND CreateDlg(HWND hParent, void* pData = NULL) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_OPTVISUAL), hParent);
	}
};