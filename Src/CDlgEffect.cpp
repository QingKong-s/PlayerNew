#include "CDlgEffect.h"

void CDlgEffect::UpdateDpi(int iDpi)
{
	eck::UpdateDpiSize(m_Ds, iDpi);

	auto pBmp = App->ScaleImageForButton(IIDX_Tick, iDpi);
	DestroyIcon(m_hiTick);
	m_hiTick = eck::CreateHICON(pBmp);

	pBmp = App->ScaleImageForButton(IIDX_Circle, iDpi);
	DestroyIcon(m_hiReset);
	m_hiReset = eck::CreateHICON(pBmp);

	pBmp = App->ScaleImageForButton(IIDX_TickRed, iDpi);
	DestroyIcon(m_hiTick2);
	m_hiTick2 = eck::CreateHICON(pBmp);
}

BOOL CDlgEffect::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_Tab.AttachNew(GetDlgItem(hDlg, IDC_TAB_EFFECT));
	UpdateDpi(eck::GetDpi(hDlg));

	constexpr static PCWSTR c_pszTab[]
	{
		L"常规",
		L"均衡器",
		L"合唱",
		L"压缩",
		L"失真",
		L"回声",
		L"镶边",
		L"漱口",
		L"3D混响",
		L"混响",
		L"环绕",
		//L"移相",
		//L"自动哇音"
	};
	static_assert(ARRAYSIZE(c_pszTab) == ARRAYSIZE(m_pDlg));

	RECT rc;
	GetClientRect(m_Tab.HWnd, &rc);
	RECT rcTabItem;

	m_Tab.SetItemSize(m_Ds.cxTab, m_Ds.cyTab);

	EckCounter(ARRAYSIZE(c_pszTab), i)
	{
		m_Tab.InsertItem(c_pszTab[i]);
		if (i == 0)
			m_Tab.GetItemRect(0, &rcTabItem);

		const auto pDlg = m_pDlg[i];
		pDlg->CreateDlg(NULL);
		pDlg->SetStyle(WS_CHILD | WS_CLIPCHILDREN);
		SetParent(pDlg->HWnd, m_Tab.HWnd);
		SetWindowPos(pDlg->HWnd, NULL,
			-1,
			rcTabItem.bottom,
			rc.right - rc.left,
			rc.bottom - rcTabItem.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
		pDlg->SetBkColor(GetSysColor(COLOR_WINDOW));

		auto pBT = dynamic_cast<eck::CButton*>
			(eck::CWndFromHWND(GetDlgItem(pDlg->HWnd, IDC_BT_RESET)));
		pBT->SetImage(m_hiReset, IMAGE_ICON);

		pBT = dynamic_cast<eck::CButton*>
			(eck::CWndFromHWND(GetDlgItem(pDlg->HWnd, IDC_CB_ENABLE)));
		if (pBT)
			pBT->SetImage(m_hiTick, IMAGE_ICON);
	}
	Show(SW_SHOW);

	return FALSE;
}

LRESULT CDlgEffect::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		const auto pnmhdr = (NMHDR*)lParam;
		if (pnmhdr->code == TCN_SELCHANGE && pnmhdr->hwndFrom == m_Tab.HWnd)
		{
			const int idxCurr = m_Tab.GetCurSel();
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

	case WM_DPICHANGED:
		UpdateDpi(LOWORD(wParam));
		break;

	case WM_DESTROY:
		DestroyIcon(m_hiTick);
		DestroyIcon(m_hiReset);
		DestroyIcon(m_hiTick2);
		break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}