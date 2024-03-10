#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CButton.h"
#include "eck\CTrackBar.h"
#include "eck\CComboBox.h"


class CDlgFxFlanger final :public eck::CDialog
{
private:
	eck::CCheckButton m_CBEnable{};
	eck::CPushButton m_BTReset{};

	eck::CTrackBar m_TBWetDryMix{};
	eck::CTrackBar m_TBDepth{};
	eck::CTrackBar m_TBFeedback{};
	eck::CTrackBar m_TBFreq{};
	eck::CCheckButton m_RBPhaseSine{};
	eck::CCheckButton m_RBPhaseTriangle{};
	eck::CTrackBar m_TBDelay{};
	eck::CComboBox m_CBBPhase{};

	void UpdateCtrl();
public:
	PNInline HWND CreateDlg(HWND hParent, void* pParam) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_FLANGER), hParent);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;
};