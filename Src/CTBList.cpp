#include "CTBList.h"

void CTBList::UpdateIcon(int iDpi)
{
    IWICBitmap* pBmp;
    eck::UpdateDpiSize(m_Ds, iDpi);
    EckCounter(ARRAYSIZE(m_hi), i)
    {
        pBmp = App->ScaleImageForButton(c_idxIcon[i], iDpi);
        DestroyIcon(m_hi[i]);
        m_hi[i] = eck::CreateHICON(pBmp);
        pBmp->Release();
    }
}

LRESULT CTBList::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SETFONT:
        m_hFont = (HFONT)wParam;
        break;
    case WM_CREATE:
        UpdateIcon(eck::GetDpi(hWnd));
        if (eck::ShouldAppUseDarkMode())
            m_hTheme = OpenThemeData(NULL, L"ItemsView::ListView");
        else
            m_hTheme = OpenThemeData(hWnd, L"ToolBar");
        break;
    case WM_THEMECHANGED:
    {
        const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
        CloseThemeData(m_hTheme);
        if (eck::ShouldAppUseDarkMode())
            m_hTheme = OpenThemeData(NULL, L"ItemsView::ListView");
        else
            m_hTheme = OpenThemeData(hWnd, L"ToolBar");
        SetButtonSize(m_Ds.cxToolBtn, m_Ds.cyTool);
        return lResult;
    }
    break;
    case WM_DPICHANGED_BEFOREPARENT:
        UpdateIcon(eck::GetDpi(hWnd));
        break;
    case WM_DESTROY:
        CloseThemeData(m_hTheme);
        for (auto& e : m_hi)
        {
            DestroyIcon(e);
            e = NULL;
        }
        break;
    }
    return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

LRESULT CTBList::OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed)
{
    switch (uMsg)
    {
    case WM_NOTIFY:
    {
        if (((NMHDR*)lParam)->code == NM_CUSTOMDRAW)
        {
            bProcessed = TRUE;
            const auto pnmtbcd = (NMTBCUSTOMDRAW*)lParam;
            switch (pnmtbcd->nmcd.dwDrawStage)
            {
            case CDDS_PREPAINT:
            {
                SetDCBrushColor(pnmtbcd->nmcd.hdc, eck::GetThreadCtx()->crDefBkg);
                FillRect(pnmtbcd->nmcd.hdc, &pnmtbcd->nmcd.rc, GetStockBrush(DC_BRUSH));
            }
            return CDRF_NOTIFYITEMDRAW;
            case CDDS_ITEMPREPAINT:
            {
                const auto* const pth = eck::GetThreadCtx();
                const int idx = (int)pnmtbcd->nmcd.dwItemSpec - TBCID_BEGIN;
                if (idx < 0 || idx >= ARRAYSIZE(c_idxIcon))
                    return CDRF_DODEFAULT;

                const HDC hDC = pnmtbcd->nmcd.hdc;
                const RECT& rc = pnmtbcd->nmcd.rc;

                int iState = 0;
                if (eck::IsBitSet(pnmtbcd->nmcd.uItemState, CDIS_SELECTED) || m_uBTFlags[idx])
                    if (eck::IsBitSet(pnmtbcd->nmcd.uItemState, CDIS_HOT))
                        iState = LISS_HOTSELECTED;// == TS_HOTCHECKED;
                    else
                        iState = LISS_SELECTED;// == TS_PRESSED;
                else
                    if (eck::IsBitSet(pnmtbcd->nmcd.uItemState, CDIS_HOT))
                        iState = LISS_HOT;// == TS_HOT;
                if (iState)
                    DrawThemeBackground(m_hTheme, hDC,
                        eck::ShouldAppUseDarkMode() ? LVP_LISTITEM : TP_BUTTON,
                        iState, &rc, NULL);

                constexpr static PCWSTR c_pszText[]
                {
                    L"定位",
                    L"添加",
                    L"读取",
                    L"保存",
                    L"清空",
                    L"管理"
                };

                const int y = rc.top + (rc.bottom - rc.top - m_Ds.cyIcon) / 2;
                const int x = rc.left + (y - rc.top);
                DrawIconEx(hDC, x, y, m_hi[idx], m_Ds.cxIcon, m_Ds.cyIcon, 0, NULL, DI_NORMAL);
                RECT rcText{ rc };
                rcText.left = x + m_Ds.cxIcon;
                SelectObject(hDC, m_hFont);
                SetBkMode(hDC, TRANSPARENT);
                SetTextColor(hDC, pth->crDefText);
                DrawTextW(hDC, c_pszText[idx], 2, &rcText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);
            }
            return CDRF_SKIPDEFAULT;
            }
            return CDRF_SKIPDEFAULT;
        }
	}
	break;

	case WM_COMMAND:
	{
		const int idx = LOWORD(wParam) - TBCID_BEGIN;
		EckAssert(idx >= 0 && idx < ARRAYSIZE(c_idxIcon));
		if (idx == 1 || idx == 5)
		{
			bProcessed = TRUE;
            m_uBTFlags[idx] = 1;
			const auto lResult = DefNotifyMsg(hParent, uMsg, wParam, lParam);
			m_uBTFlags[idx] = 0;
			Redraw();
			return lResult;
		}
	}
	break;
	}

	return __super::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
}