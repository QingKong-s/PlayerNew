#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CButton.h"
#include "eck\CTrackBar.h"
#include "eck\CStatic.h"
#include "eck\CListBox.h"
#include "eck\GraphicsHelper.h"

class CUvBlock final :public eck::CStatic
{
private:
	eck::CEzCDC m_DC{};

	int m_cxClient = 0,
		m_cyClient = 0;

	int m_cxRightOld = 0,
		m_cxLeftOld = 0;

	struct
	{
		int y;		// 高度
		BOOL b;		// 时间是否已置零
		UINT uTime;	// 时间
	}
	m_Time[2]{};

	enum
	{
		IDT_TIMER = 101,
		TE_TIMER = 60
	};

	void PaintContent(HDC hDC);
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	PNInline void Pause() const
	{
		KillTimer(HWnd, IDT_TIMER);
	}

	PNInline void Resume() const
	{
		SetTimer(HWnd, IDT_TIMER, TE_TIMER, NULL);
	}

	void Init()
	{
		RECT rc;
		GetClientRect(HWnd, &rc);
		m_cxClient = rc.right;
		m_cyClient = rc.bottom;
		m_DC.Create(HWnd, m_cxClient, m_cyClient);
	}
};

class CDlgFxCommon final :public eck::CDialog
{
private:
	eck::CPushButton m_BTReset{};

	eck::CTrackBar m_TBTempo{};
	eck::CTrackBar m_TBSpeed{};
	eck::CTrackBar m_TBPan{};
	eck::CTrackBar m_TBVol{};
	eck::CTrackBar m_TBPitch{};

	eck::CStatic m_STTempo{};
	eck::CStatic m_STSpeed{};
	eck::CStatic m_STPan{};
	eck::CStatic m_STVol{};
	eck::CStatic m_STPitch{};
	CUvBlock m_STVu{};

	eck::CPushButton m_BTVol{};
	
	eck::CListBox m_LBFxPriority{};

	HICON m_hiTempo = NULL;
	HICON m_hiSpeed = NULL;
	HICON m_hiPan = NULL;
	HICON m_hiVol = NULL;
	HICON m_hiPitch = NULL;
	HICON m_hiSilent = NULL;

	void UpdateCtrl();

	void UpdateDpi(int iDpi);
public:
	PNInline HWND CreateDlg(HWND hParent, void* pParam) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_FXCOMM), hParent);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;
};