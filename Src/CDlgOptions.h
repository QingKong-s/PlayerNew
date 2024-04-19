#pragma once
#include "CDlgOptApp.h"
#include "CDlgOptVisual.h"
#include "CDlgOptPlay.h"
#include "CDlgOptLrc.h"
#include "CDlgOptHotKey.h"

#include "eck\CListBox.h"
#include "eck\CScrollBar.h"

class CLBOptions :public eck::CListBox
{
private:
	HFONT m_hFont{};

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cyItem, 32)
		ECK_DS_ENTRY(cxMargin, 10)
		;
	ECK_DS_END_VAR(m_Ds);
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_CREATE:
		case WM_DPICHANGED_BEFOREPARENT:
		{
			m_iDpi = eck::GetDpi(hWnd);
			eck::UpdateDpiSize(m_Ds, m_iDpi);
		}
		break;

		case WM_SETFONT:
			m_hFont = (HFONT)wParam;
			break;
		}
		return __super::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		switch (uMsg)
		{
		case WM_MEASUREITEM:
		{
			bProcessed = TRUE;
			const auto pmis = (MEASUREITEMSTRUCT*)lParam;
			pmis->itemHeight = m_Ds.cyItem;
		}
		return TRUE;

		case WM_DRAWITEM:
		{
			bProcessed = TRUE;
			const auto pdis = (DRAWITEMSTRUCT*)lParam;

			if (pdis->itemID != -1)
			{
				COLORREF crOld;
				// 画背景
				if (pdis->itemState & ODS_SELECTED)
				{
					FillRect(pdis->hDC, &pdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
					crOld = SetTextColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
				}
				else
				{
					FillRect(pdis->hDC, &pdis->rcItem, GetSysColorBrush(COLOR_WINDOW));
					crOld = SetTextColor(pdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
				}
				// 画文本
				int iBKModeOld = SetBkMode(pdis->hDC, TRANSPARENT);
				HGDIOBJ hFontOld = SelectObject(pdis->hDC, m_hFont);

				RECT rcText = pdis->rcItem;
				rcText.left += m_Ds.cxMargin;// 左边空一点
				if (pdis->itemData)
					DrawTextW(pdis->hDC, (PCWSTR)pdis->itemData, -1, &rcText,
						DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP);
				// 还原
				SelectObject(pdis->hDC, hFontOld);
				SetBkMode(pdis->hDC, iBKModeOld);
				SetTextColor(pdis->hDC, crOld);
			}
		}
		return TRUE;
		}
		return __super::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
	}

	void AttachNew(HWND hWnd) override
	{
		__super::AttachNew(hWnd);

		m_iDpi = eck::GetDpi(hWnd);
		eck::UpdateDpiSize(m_Ds, m_iDpi);

		m_hFont = GetFont();
		SetItemHeight(0, m_Ds.cyItem);
	}
};

class CDlgOptions :public eck::CDialog
{
private:
	struct METRICS
	{
		int cy;
		int iPos;
		BOOL bShowScroll;
	};
	CDlgOptApp		m_DlgApp{};
	CDlgOptVisual	m_DlgVisual{};
	CDlgOptPlay		m_DlgPlay{};
	CDlgOptLrc		m_DlgLrc{};
	CDlgOptHotKey	m_DlgHotKey{};

	eck::CDialog* m_pDlg[5]
	{
		&m_DlgApp,
		&m_DlgVisual,
		&m_DlgPlay,
		&m_DlgLrc,
		&m_DlgHotKey
	};

	METRICS m_Metrics[ARRAYSIZE(m_pDlg)]{};

	CLBOptions m_LB{};
	eck::CScrollBar m_SB{};

	RECT m_rcView{};

	HFONT m_hFontBig{};

	void OnMouseWheel(int iDelta);

	void Apply();
public:
	PNInline INT_PTR DlgBox(HWND hParent, void* pData = NULL) override
	{
		return IntCreateModalDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_OPTIONS), hParent);
	}

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;

	BOOL PreTranslateMessage(const MSG& Msg) override;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void OnOk(HWND hCtrl) override;

	void SwitchPage(int idx);
};