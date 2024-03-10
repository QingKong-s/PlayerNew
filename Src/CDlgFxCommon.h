#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CButton.h"
#include "eck\CTrackBar.h"
#include "eck\CStatic.h"
#include "eck\CListBox.h"


class CDlgFxCommon final :public eck::CDialog
{
private:
	eck::CPushButton m_BTReset{};

	eck::CTrackBar m_TBTempo{};
	eck::CTrackBar m_TBSpeed{};
	eck::CTrackBar m_TBPan{};
	eck::CTrackBar m_TBVol{};

	eck::CStatic m_STTempo{};
	eck::CStatic m_STSpeed{};
	eck::CStatic m_STPan{};
	eck::CStatic m_STVol{};
	eck::CStatic m_STVu{};

	eck::CPushButton m_BTVol{};
	
	eck::CListBox m_LBFxPriority{};

	void UpdateCtrl();
public:
	PNInline HWND CreateDlg(HWND hParent, void* pParam) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_FXCOMM), hParent);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;
};