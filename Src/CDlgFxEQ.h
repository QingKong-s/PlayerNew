#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CButton.h"
#include "eck\CTrackBar.h"
#include "eck\CComboBox.h"


class CDlgFxEQ final :public eck::CDialog
{
private:
	eck::CCheckButton m_CBEnable{};
	eck::CPushButton m_BTReset{};

	eck::CTrackBar m_TBBandWidth{};
	eck::CTrackBar m_TB[10]{};
	eck::CComboBox m_CBBPresetEQ{};

	void UpdateCtrl();
public:
	PNInline HWND CreateDlg(HWND hParent, void* pParam) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_EQ), hParent);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;
};