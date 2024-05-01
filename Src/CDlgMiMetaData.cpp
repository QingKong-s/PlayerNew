#include "CDlgMiMetaData.h"

void CDlgMiMetaData::UpdateDpi(int iDpi)
{
	if (m_hFont)
	{
		auto hFontNew = eck::ReCreateFontForDpiChanged(m_hFont, iDpi, m_iDpi);
		eck::SetFontForWndAndCtrl(HWnd, hFontNew);
		std::swap(m_hFont, hFontNew);
		DeleteObject(hFontNew);
	}
	else
		m_hFont = eck::CreateDefFont(iDpi);
	m_iDpi = iDpi;
	eck::UpdateDpiSize(m_Ds, m_iDpi);
}

BOOL CDlgMiMetaData::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	UpdateDpi(eck::GetDpi(hDlg));
	m_TL.Create(NULL, WS_CHILD | WS_VISIBLE, 0,
		0, 0, 0, 0, hDlg, 0);
	eck::SetFontForWndAndCtrl(hDlg, m_hFont);
	return 0;
}

LRESULT CDlgMiMetaData::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		int cx, cy;
		ECK_GET_SIZE_LPARAM(cx, cy, lParam);
		SetWindowPos(m_TL.HWnd, NULL,
			m_Ds.iMargin, m_Ds.iMargin, cx - m_Ds.iMargin, cy - m_Ds.iMargin,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}