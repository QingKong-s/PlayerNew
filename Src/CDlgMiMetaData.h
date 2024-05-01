#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CTreeList.h"

class CDlgMiMetaData :public eck::CDialogNew
{
private:
	eck::CTreeList m_TL{};
	HFONT m_hFont{};
	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(iMargin, c_DlgMargin)
		;
	ECK_DS_END_VAR(m_Ds);

	void UpdateDpi(int iDpi);
public:
	HWND CreateDlg(HWND hParent, void* pData = NULL) override
	{
		return IntCreateModelessDlg(0, eck::WCN_DLG, L"元数据", WS_CHILD,
			0, 0, 0, 0, hParent, NULL, App->GetHInstance(), NULL);
	}

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};