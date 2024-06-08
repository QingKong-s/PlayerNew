#include "CDlgMusicInfo.h"

BOOL CDlgMusicInfo::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	UpdateDpi(eck::GetDpi(hDlg));

	m_Tab.AttachNew(GetDlgItem(hDlg, IDC_TAB));
	m_Tab.InsertItem(L"元数据");
	m_Tab.InsertItem(L"封面");
	m_Tab.InsertItem(L"其他");
	m_Tab.SetItemSize(m_Ds.cxTab, m_Ds.cyTab);

	RECT rc;
	GetClientRect(m_Tab.HWnd, &rc);
	RECT rcTabItem;
	m_Tab.GetItemRect(0, &rcTabItem);

	EckCounter(ARRAYSIZE(m_pDlg), i)
	{
		const auto pDlg = m_pDlg[i];
		pDlg->CreateDlg(NULL);
		pDlg->SetStyle(WS_CHILD | WS_CLIPCHILDREN);
		pDlg->SendMsg(PNWM_MIDLG_INIT, 0, (LPARAM)this);
		eck::SetFontForWndAndCtrl(pDlg->HWnd, GetFont());
		SetParent(pDlg->HWnd, m_Tab.HWnd);
		SetWindowPos(pDlg->HWnd, NULL,
			1,
			rcTabItem.bottom+1,
			rc.right - rc.left-2,
			rc.bottom - rcTabItem.bottom-3, SWP_NOZORDER | SWP_NOACTIVATE);
		pDlg->SendMsg(PNWM_MIDLG_UPDATEIDX, m_idxCurr, 0);
	}
	m_Tab.SetCurSel(App->GetOptionsMgr().MiPage);
	m_pDlg[App->GetOptionsMgr().MiPage]->Show(SW_SHOWNOACTIVATE);
	return TRUE;
}

LRESULT CDlgMusicInfo::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		const auto pnmhdr = (NMHDR*)lParam;
		if (pnmhdr->code == TCN_SELCHANGE && pnmhdr->hwndFrom == m_Tab.HWnd)
		{
			const int idxCurr = m_Tab.GetCurSel();
			App->GetOptionsMgr().MiPage = idxCurr;
			int i = 0;
			m_pDlg[idxCurr]->Show(SW_SHOWNOACTIVATE);
			for (; i < idxCurr; ++i)
				m_pDlg[i]->Show(SW_HIDE);
			for (++i; i < ARRAYSIZE(m_pDlg); ++i)
				m_pDlg[i]->Show(SW_HIDE);
			return 0;
		}
	}
	break;
	case WM_COMMAND:
	{
		if (HIWORD(wParam) == BN_CLICKED)
			switch (LOWORD(wParam))
			{
			case IDC_BT_PREV:
			{
				const int cItem = App->GetPlayer().GetList().GetCount();
				if (cItem)
				{
					--m_idxCurr;
					if (m_idxCurr < 0)
						m_idxCurr = cItem - 1;
				}
				else
					m_idxCurr = -1;
				for (auto e : m_pDlg)
					e->SendMsg(PNWM_MIDLG_UPDATEIDX, m_idxCurr, 0);
			}
			return 0;
			case IDC_BT_NEXT:
			{
				const int cItem = App->GetPlayer().GetList().GetCount();
				if (cItem)
				{
					++m_idxCurr;
					if (m_idxCurr >= cItem)
						m_idxCurr = 0;
				}
				else
					m_idxCurr = -1;
				for (auto e : m_pDlg)
					e->SendMsg(PNWM_MIDLG_UPDATEIDX, m_idxCurr, 0);
			}
			return 0;
			}
	}
	break;
	case PNWM_MIDLG_GETIDX:
		return m_idxCurr;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}
