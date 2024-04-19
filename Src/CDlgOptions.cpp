#include "CDlgOptions.h"
#include "CWndMain.h"

void CDlgOptions::OnMouseWheel(int iDelta)
{
	const int idxCurr = m_LB.GetCurrSel();
	if (!m_Metrics[idxCurr].bShowScroll)
		return;

	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_PAGE;
	m_SB.GetInfo(&si);
	const int iOldPos = si.nPos;
	si.nPos += (si.nPage / 3 * iDelta);

	si.fMask = SIF_POS;
	m_SB.SetInfo(&si);
	m_SB.GetInfo(&si);
	m_Metrics[idxCurr].iPos = si.nPos;
	ScrollWindow(m_pDlg[idxCurr]->HWnd, 0, iOldPos - si.nPos, NULL, NULL);
}

void CDlgOptions::Apply()
{
	for (auto pDlg : m_pDlg)
		pDlg->SendMsg(PNWM_OPTDLG_APPLY, 0, 0);
	App->GetMainWnd()->SettingChanged();
}

BOOL CDlgOptions::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_hFontBig = eck::EzFont(L"微软雅黑", 12);

	m_LB.AttachNew(GetDlgItem(hDlg, IDC_LB_OPTIONS));
	m_SB.AttachNew(GetDlgItem(hDlg, IDC_SB_SETTINGS));

	m_LB.SetFont(m_hFontBig);

	m_LB.AddString(L"程序");
	m_LB.AddString(L"显示");
	m_LB.AddString(L"播放");
	m_LB.AddString(L"歌词");
	m_LB.AddString(L"快捷键");

	RECT rcLB;
	GetWindowRect(m_LB.HWnd, &rcLB);
	eck::ScreenToClient(hDlg, &rcLB);

	const int cxSB = m_SB.Width;

	const RECT rcPos// 左顶宽高
	{
		rcLB.right + rcLB.left,
		rcLB.top,
		ClientWidth - (rcLB.right + rcLB.left) - cxSB,
		rcLB.bottom - rcLB.top,
	};

	HDWP hDwp = BeginDeferWindowPos(ARRAYSIZE(m_pDlg) + 1);
	for (int i{}; auto pDlg : m_pDlg)
	{
		pDlg->CreateDlg(NULL);
		pDlg->SetStyle(WS_CHILD);
		pDlg->ModifyStyle(0, WS_EX_DLGMODALFRAME, GWL_EXSTYLE);
		SetParent(pDlg->HWnd, hDlg);
		hDwp = DeferWindowPos(hDwp, pDlg->HWnd, NULL,
			rcPos.left, rcPos.top, rcPos.right, rcPos.bottom,
			SWP_NOZORDER | SWP_NOACTIVATE);
		m_Metrics[i].cy = pDlg->Height;
		m_Metrics[i].bShowScroll = (m_Metrics[i].cy > rcPos.bottom);
		++i;
	}
	hDwp = DeferWindowPos(hDwp, m_SB.HWnd, NULL,
		ClientWidth - cxSB, rcPos.top, cxSB, rcPos.bottom,
		SWP_NOZORDER | SWP_NOACTIVATE);
	EndDeferWindowPos(hDwp);

	m_rcView = rcPos;
	m_rcView.right += (m_rcView.left + cxSB);
	m_rcView.bottom += m_rcView.top;

	SwitchPage(0);
	return FALSE;
}

BOOL CDlgOptions::PreTranslateMessage(const MSG& Msg)
{
	switch (Msg.message)
	{
	case WM_MOUSEWHEEL:
	{
		POINT pt{ Msg.pt };
		ScreenToClient(HWnd, &pt);
		if (!eck::PtInRect(m_rcView, pt))
			break;
		OnMouseWheel(-GET_WHEEL_DELTA_WPARAM(Msg.wParam) / WHEEL_DELTA);
	}
	return TRUE;
	}
	return __super::PreTranslateMessage(Msg);
}

LRESULT CDlgOptions::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_VSCROLL:
	{
		const int idxCurr = m_LB.GetCurrSel();
		if (!m_Metrics[idxCurr].bShowScroll)
			break;

		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		m_SB.GetInfo(&si);
		const int iOldPos = si.nPos;
		switch (LOWORD(wParam))
		{
		case SB_LEFT:
			si.nPos = 0;
			break;
		case SB_RIGHT:
			si.nPos = si.nMax;
			break;
		case SB_LINELEFT:
			si.nPos--;
			break;
		case SB_LINERIGHT:
			si.nPos++;
			break;
		case SB_PAGELEFT:
			si.nPos -= si.nPage;
			break;
		case SB_PAGERIGHT:
			si.nPos += si.nPage;
			break;
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		}

		si.fMask = SIF_POS;
		m_SB.SetInfo(&si);
		m_SB.GetInfo(&si);
		m_Metrics[idxCurr].iPos = si.nPos;
		ScrollWindow(m_pDlg[idxCurr]->HWnd, 0, iOldPos - si.nPos, NULL, NULL);
	}
	break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_LB_OPTIONS:
		{
			if (HIWORD(wParam) == LBN_SELCHANGE)
				SwitchPage(m_LB.GetCurrSel());
		}
		return TRUE;

		case IDC_BT_APPLY:
			Apply();
			break;
		}
	}
	break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

void CDlgOptions::OnOk(HWND hCtrl)
{
	Apply();
	EndDlg(0);
}

void CDlgOptions::SwitchPage(int idx)
{
	m_LB.SetCurrSel(idx);
	m_pDlg[idx]->Show(SW_SHOWNOACTIVATE);
	int i = 0;
	for (; i < idx; ++i)
		m_pDlg[i]->Show(SW_HIDE);
	for (++i; i < ARRAYSIZE(m_pDlg); ++i)
		m_pDlg[i]->Show(SW_HIDE);
	if (m_Metrics[idx].bShowScroll)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		si.nMin = 0;
		si.nMax = m_Metrics[idx].cy;
		si.nPos = m_Metrics[idx].iPos;
		si.nPage = m_rcView.bottom - m_rcView.top;
		m_SB.SetInfo(&si);
		m_SB.Show(SW_SHOWNOACTIVATE);
	}
	else
		m_SB.Show(SW_HIDE);
}