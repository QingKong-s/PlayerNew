#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CButton.h"
#include "eck\CTrackBar.h"


class CDlgFxGargle final :public eck::CDialog
{
private:
	eck::CCheckButton m_CBEnable{};
	eck::CPushButton m_BTReset{};

	eck::CTrackBar m_TBRate{};
	eck::CCheckButton m_RBPhaseSine{};
	eck::CCheckButton m_RBPhaseQuad{};

	void UpdateCtrl();
public:
	PNInline HWND CreateDlg(HWND hParent, void* pParam) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_GARGLE), hParent);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;
};