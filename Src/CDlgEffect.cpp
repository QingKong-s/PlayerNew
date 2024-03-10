#include "CDlgEffect.h"

BOOL CDlgEffect::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_Tab.Attach(GetDlgItem(hDlg, IDC_TAB_EFFECT));
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
		L"移相",
		L"自动哇音"
	};
	static_assert(ARRAYSIZE(c_pszTab) == ARRAYSIZE(m_pDlg));

	RECT rc;
	GetClientRect(m_Tab.HWnd, &rc);
	RECT rcTabItem;

	EckCounter(ARRAYSIZE(c_pszTab), i)
	{
		m_Tab.InsertItem(c_pszTab[i]);
		if (i == 0)
			m_Tab.GetItemRect(0, &rcTabItem);
		if (m_pDlg[i])
		{
			const auto pDlg = m_pDlg[i];
			pDlg->CreateDlg(NULL);
			pDlg->SetStyle(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
			SetParent(pDlg->HWnd, m_Tab.HWnd);
			SetWindowPos(pDlg->HWnd, NULL,
				-1,
				rcTabItem.bottom,
				rc.right - rc.left,
				rc.bottom - rcTabItem.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
		}
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
			for (; i < idxCurr; ++i)
				if(m_pDlg[i])
				m_pDlg[i]->Show(SW_HIDE);
			if (m_pDlg[i])
			m_pDlg[i]->Show(SW_SHOWNOACTIVATE);
			for (++i; i < ARRAYSIZE(m_pDlg); ++i)
				if (m_pDlg[i])
				m_pDlg[i]->Show(SW_HIDE);
			return 0;
		}
	}
	break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}