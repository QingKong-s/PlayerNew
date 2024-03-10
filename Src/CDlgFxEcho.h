#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CButton.h"
#include "eck\CTrackBar.h"


class CDlgFxEcho final :public eck::CDialog
{
private:
	eck::CCheckButton m_CBEnable{};
	eck::CPushButton m_BTReset{};

	eck::CTrackBar m_TBWetDryMix{};
	eck::CTrackBar m_TBFeedback{};
	eck::CTrackBar m_TBLDelay{};
	eck::CTrackBar m_TBRDelay{};
	eck::CCheckButton m_CBPanDelay{};

	void UpdateCtrl();
public:
	PNInline HWND CreateDlg(HWND hParent, void* pParam) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_ECHO), hParent);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;
};