#include "CLVPlay.h"
#include "CWndList.h"
#include "CWndMain.h"

static void PaintLVItem(eck::CListView& LV, CWndList& WndList, PLAYLISTUNIT& Item,
	NMLVCUSTOMDRAW* pnmlvcd, HTHEME hTheme, int idx, int idxLVItem)
{
	const auto hDC = pnmlvcd->nmcd.hdc;
	if (idx == App->GetPlayer().GetCurrFile())// 标记现行播放项
	{
		eck::CEzCDC cdc(pnmlvcd->nmcd.hdr.hwndFrom, 1, 1);
		SetDCBrushColor(cdc.GetDC(), eck::ARGBToColorref(App->GetMainWnd()->GetDwmColorArgb()));
		constexpr RECT rcTemp{ 0,0,1,1 };
		FillRect(cdc.GetDC(), &rcTemp, GetStockBrush(DC_BRUSH));

		AlphaBlend(hDC, pnmlvcd->nmcd.rc.left, pnmlvcd->nmcd.rc.top,
			pnmlvcd->nmcd.rc.right - pnmlvcd->nmcd.rc.left,
			pnmlvcd->nmcd.rc.bottom - pnmlvcd->nmcd.rc.top,
			cdc.GetDC(), 0, 0, 1, 1, { AC_SRC_OVER,0,110,0 });
	}
	else if (idxLVItem % 2)// 交替行色
		FillRect(hDC, &pnmlvcd->nmcd.rc, GetSysColorBrush(COLOR_3DFACE));

	int iState;
	if (LV.GetItemState(idxLVItem, LVIS_SELECTED) == LVIS_SELECTED)// 选中
	{
		if (pnmlvcd->nmcd.uItemState & CDIS_HOT)
			iState = LISS_HOTSELECTED;
		else
			iState = LISS_SELECTED;
	}
	else if (pnmlvcd->nmcd.uItemState & CDIS_HOT)
		iState = LISS_HOT;
	else
		iState = 0;
	if (iState)// 画表项框
		DrawThemeBackground(hTheme, hDC, LVP_LISTITEM, iState, &pnmlvcd->nmcd.rc, NULL);

	if (Item.s.bIgnore)
		SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
	else
		SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));

	RECT rc = pnmlvcd->nmcd.rc;
	rc.left += WndList.GetDs().cxLVTextSpace;
	rc.right = rc.left + LV.GetColumnWidth(0) - WndList.GetDs().cxLVTextSpace;
	DrawTextW(hDC, Item.rsName.Data(), Item.rsName.Size(), &rc,
		DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);
	rc.left = rc.right;
	rc.right = pnmlvcd->nmcd.rc.right;
	WCHAR szTime[eck::c_cchI32ToStrBufNoRadix2 * 2 + 10];
	const int cchTime = swprintf(szTime, L"%02d:%02d", 
		int(Item.s.uSecTime / 60), int(Item.s.uSecTime % 60));
	DrawTextW(hDC, szTime, cchTime, &rc,
		DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);

	if (idx == App->GetPlayer().GetLaterPlaying())// 稍后播放
		FrameRect(hDC, &pnmlvcd->nmcd.rc, GetSysColorBrush(COLOR_HIGHLIGHT));
}

LRESULT CLVSearch::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
	{
		if (wParam == VK_RETURN)// 按回车键播放曲目
		{
			const int idx = GetCurrSel();
			if (idx < 0)
				break;
			m_WndList.PlayListItem(App->GetPlayer().AtSearchingIndex(idx));
		}
		else if (wParam == 0x41)// A键按下
			if (GetKeyState(VK_CONTROL) & 0x80000000)// Ctrl + A 全选
				SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
	}
	break;
	case WM_THEMECHANGED:
	{
		CloseThemeData(m_hTheme);
		m_hTheme = OpenThemeData(hWnd, L"ListView");
	}
	break;
	case WM_DESTROY:
		CloseThemeData(m_hTheme);
		break;
	case WM_CREATE:
		m_hTheme = OpenThemeData(hWnd, L"ListView");
		break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

LRESULT CLVSearch::OnNotifyMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		if (((NMHDR*)lParam)->code == NM_CUSTOMDRAW)
		{
			const auto pnmlvcd = (NMLVCUSTOMDRAW*)lParam;
			bProcessed = TRUE;
			switch (pnmlvcd->nmcd.dwDrawStage)
			{
			case CDDS_PREPAINT:
				return CDRF_NOTIFYITEMDRAW;
			case CDDS_ITEMPREPAINT:
			{
				const HDC hDC = pnmlvcd->nmcd.hdc;
				const int idx = (int)pnmlvcd->nmcd.dwItemSpec;
				const int idxReal = App->GetPlayer().AtSearchingIndex(idx);
				auto& Item = App->GetPlayer().AtSearching(idx);

				PaintLVItem(*this, m_WndList, Item, pnmlvcd, m_hTheme, idxReal, idx);

				const auto& rsKeyWord = m_WndList.GetCurrSearchKeyword();
				int pos = 0;
				SIZE sizeKeyword;
				GetTextExtentPoint32W(hDC, rsKeyWord.Data(), rsKeyWord.Size(), &sizeKeyword);
				RECT rcBK{ 0,pnmlvcd->nmcd.rc.top,GetColumnWidth(0),pnmlvcd->nmcd.rc.bottom };
				RECT rc, rcColumn1;
				GetHeaderCtrl().GetItemRect(0, &rcColumn1);
				rcColumn1.top = pnmlvcd->nmcd.rc.top;
				rcColumn1.bottom = pnmlvcd->nmcd.rc.bottom;

				IntersectClipRect(hDC, rcColumn1.left, rcColumn1.top, rcColumn1.right, rcColumn1.bottom);
				while ((pos = eck::FindStrI(Item.rsName.Data(), rsKeyWord.Data(), pos)) != eck::INVALID_STR_POS)
				{
					if (pos)
					{
						rc = pnmlvcd->nmcd.rc;
						DrawTextW(hDC, Item.rsName.Data(), pos, &rc,
							DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_CALCRECT);
						rcBK.left = rc.right + m_WndList.GetDs().cxLVTextSpace;
					}
					else
						rcBK.left = m_WndList.GetDs().cxLVTextSpace;
					rcBK.right = rcBK.left + sizeKeyword.cx;
					if (rcBK.left < rcColumn1.right)
					{
						const int iOldMode = SetBkMode(hDC, OPAQUE);
						const auto crOld = SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
						const auto crOldText = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
						DrawTextW(hDC, Item.rsName.Data() + pos, rsKeyWord.Size(), &rcBK,
							DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP);
						SetBkMode(hDC, iOldMode);
						SetBkColor(hDC, crOld);
						SetTextColor(hDC, crOldText);
					}
					++pos;
				}
				SelectClipRgn(hDC, NULL);
			}
			return CDRF_SKIPDEFAULT;
			}
			return CDRF_SKIPDEFAULT;
		}
	}
	break;
	}
	return __super::OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
}

LRESULT CLVPlay::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case LVM_DELETECOLUMN:
	case LVM_INSERTCOLUMNW:
	case LVM_SETBKCOLOR:
	case LVM_SETBKIMAGEW:
	case LVM_SETCOLUMNW:
	case LVM_SETCOLUMNORDERARRAY:
	case LVM_SETCOLUMNWIDTH:
	case LVM_SETEXTENDEDLISTVIEWSTYLE:
	case LVM_SETOUTLINECOLOR:
	case LVM_SETTEXTBKCOLOR:
	case LVM_SETTEXTCOLOR:
	case LVM_SETVIEW:
		m_LVSearch.SendMsg(uMsg, wParam, lParam);
		break;
	case LVM_SETIMAGELIST:
	{
		HIMAGELIST hIml = ImageList_Duplicate((HIMAGELIST)lParam);
		m_LVSearch.SendMsg(uMsg, wParam, (LPARAM)hIml);
	}
	break;
	case WM_KEYDOWN:
	{
		if (wParam == VK_RETURN)// 按回车键播放曲目
		{
			const int idx = GetCurrSel();
			if (idx < 0)
				break;
			m_WndList.PlayListItem(idx);
		}
		else if (wParam == 0x41)// A键按下
			if (GetKeyState(VK_CONTROL) & 0x80000000)// Ctrl + A 全选
				SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
	}
	break;
	case WM_THEMECHANGED:
	{
		CloseThemeData(m_hTheme);
		m_hTheme = OpenThemeData(hWnd, L"ListView");
	}
	break;
	case WM_DESTROY:
		CloseThemeData(m_hTheme);
		break;
	case WM_CREATE:
		m_hTheme = OpenThemeData(hWnd, L"ListView");
		break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

LRESULT CLVPlay::OnNotifyMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		if (((NMHDR*)lParam)->code == NM_CUSTOMDRAW)
		{
			const auto pnmlvcd = (NMLVCUSTOMDRAW*)lParam;
			bProcessed = TRUE;
			switch (pnmlvcd->nmcd.dwDrawStage)
			{
			case CDDS_PREPAINT:
				return CDRF_NOTIFYITEMDRAW;
			case CDDS_ITEMPREPAINT:
			{
				auto& Item = App->GetPlayer().GetList().At((int)pnmlvcd->nmcd.dwItemSpec);
				const HDC hDC = pnmlvcd->nmcd.hdc;
				const int idx = (int)pnmlvcd->nmcd.dwItemSpec;

				PaintLVItem(*this, m_WndList, Item, pnmlvcd, m_hTheme, idx, idx);

				if (Item.s.bBookmark)
				{
					const auto& bm = App->GetPlayer().GetList().AtBookmark(idx);
					HGDIOBJ hOldPen = SelectObject(hDC, CreatePen(PS_SOLID, 1, bm.crColor));
					MoveToEx(hDC, pnmlvcd->nmcd.rc.left, pnmlvcd->nmcd.rc.top, NULL);
					LineTo(hDC, pnmlvcd->nmcd.rc.right, pnmlvcd->nmcd.rc.top);
					DeleteObject(SelectObject(hDC, hOldPen));
				}
			}
			return CDRF_SKIPDEFAULT;
			}
			return CDRF_SKIPDEFAULT;
		}
	}
	break;
	}
	return __super::OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
}