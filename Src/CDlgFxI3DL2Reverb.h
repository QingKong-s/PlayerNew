#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CButton.h"
#include "eck\CTrackBar.h"


class CDlgFxI3DL2Reverb final :public eck::CDialog
{
private:
	eck::CCheckButton m_CBEnable{};
	eck::CPushButton m_BTReset{};

	eck::CTrackBar m_TBRoom{};
	eck::CTrackBar m_TBRoomHF{};
	eck::CTrackBar m_TBRoomRollOffFactor{};
	eck::CTrackBar m_TBDecayTime{};
	eck::CTrackBar m_TBDecayHFRatio{};
	eck::CTrackBar m_TBReflections{};

	eck::CTrackBar m_TBReflectionsDelay{};
	eck::CTrackBar m_TBReverb{};
	eck::CTrackBar m_TBReverbDelay{};
	eck::CTrackBar m_TBDiffusion{};
	eck::CTrackBar m_TBDensity{};
	eck::CTrackBar m_TBHFReference{};


	void UpdateCtrl();
public:
	PNInline HWND CreateDlg(HWND hParent, void* pParam) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_I3DL2REVERB), hParent);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;
};