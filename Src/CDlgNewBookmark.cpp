#include "CDlgNewBookmark.h"
#include "Utils.h"

void CDlgNewBookmark::UpdateDpiInit(int iDpi)
{
	m_iDpi = iDpi;
	eck::UpdateDpiSize(m_Ds, iDpi);
}

void CDlgNewBookmark::UpdateDpi(int iDpi)
{
	const int iDpiOld = m_iDpi;
	UpdateDpiInit(iDpi);

	const HFONT hOldFont = m_hFont;
	m_hFont = eck::ReCreateFontForDpiChanged(m_hFont, iDpi, iDpiOld);
	eck::SetFontForWndAndCtrl(m_hWnd, m_hFont);
	DeleteObject(hOldFont);

	RECT rc;
	GetClientRect(m_hWnd, &rc);

	m_CLPColor.SetItemHeight(m_Ds.cyComm);

	HDWP hDwp = BeginDeferWindowPos(10);
	const int xInput = m_Ds.iMargin + m_Ds.iGap + m_Ds.cxLabel;
	const int yLine2 = m_Ds.iMargin + m_Ds.iGap + m_Ds.cyComm;
	const int yBT = yLine2 + m_Ds.iGap * 2;
	DeferWindowPos(hDwp, m_LAName.GetHWND(), NULL,
		m_Ds.iMargin,
		m_Ds.iMargin,
		m_Ds.cxLabel,
		m_Ds.cyComm,
		SWP_NOZORDER | SWP_NOACTIVATE);
	DeferWindowPos(hDwp, m_EDName.GetHWND(), NULL,
		xInput,
		m_Ds.iMargin,
		m_Ds.cxInput,
		m_Ds.cyComm,
		SWP_NOZORDER | SWP_NOACTIVATE);
	DeferWindowPos(hDwp, m_LAColor.GetHWND(), NULL,
		m_Ds.iMargin,
		yLine2,
		m_Ds.cxLabel,
		m_Ds.cyComm,
		SWP_NOZORDER | SWP_NOACTIVATE);
	DeferWindowPos(hDwp, m_CLPColor.GetHWND(), NULL,
		xInput,
		yLine2,
		m_Ds.cxInput,
		m_Ds.cyComm, SWP_NOZORDER | SWP_NOACTIVATE);
	const int xBTCancel = rc.right - m_Ds.iMargin - m_Ds.cxBT;
	DeferWindowPos(hDwp, m_BTCancel.GetHWND(), NULL,
		xBTCancel,
		yBT,
		m_Ds.cxBT,
		m_Ds.cyComm,
		SWP_NOZORDER | SWP_NOACTIVATE);
	DeferWindowPos(hDwp, m_BTOk.GetHWND(), NULL,
		xBTCancel - m_Ds.iGap - m_Ds.cxBT,
		yBT,
		m_Ds.cxBT,
		m_Ds.cyComm,
		SWP_NOZORDER | SWP_NOACTIVATE);
	EndDeferWindowPos(hDwp);
}

BOOL CDlgNewBookmark::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	UpdateDpiInit(eck::GetDpi(hDlg));
	m_hFont = eck::EzFont(L"微软雅黑", 9);
	RECT rc;
	GetClientRect(hDlg, &rc);

	const int xInput = m_Ds.iMargin + m_Ds.iGap + m_Ds.cxLabel;
	const int yLine2 = m_Ds.iMargin + m_Ds.iGap + m_Ds.cyComm;
	const int yBT = yLine2 + m_Ds.iGap * 2 + m_Ds.cyComm;
	m_LAName.Create(L"名称：", WS_VISIBLE, 0, m_Ds.iMargin, m_Ds.iMargin, m_Ds.cxLabel, m_Ds.cyComm, hDlg, IDC_LA_TIP1);
	m_EDName.Create(L"新书签", WS_GROUP | WS_TABSTOP | WS_VISIBLE, WS_EX_CLIENTEDGE, xInput, m_Ds.iMargin, m_Ds.cxInput, m_Ds.cyComm, hDlg, IDC_ED_NAME);

	m_LAColor.Create(L"颜色：", WS_VISIBLE, 0, m_Ds.iMargin, yLine2, m_Ds.cxLabel, m_Ds.cyComm, hDlg, IDC_LA_TIP2);
	m_CLPColor.Create(L"新书签", WS_TABSTOP | WS_VISIBLE, 0, xInput, yLine2, m_Ds.cxInput, m_Ds.cyComm, hDlg, IDC_CLP_CLR);
	m_CLPColor.SetItemHeight(m_Ds.cyComm);

	const int xBTCancel = rc.right - m_Ds.iMargin - m_Ds.cxBT;
	m_BTOk.Create(L"确定", WS_GROUP | WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 0,
		xBTCancel - m_Ds.iGap - m_Ds.cxBT, yBT, m_Ds.cxBT, m_Ds.cyComm, hDlg, IDOK);
	m_BTCancel.Create(L"取消", WS_TABSTOP | WS_VISIBLE, 0, xBTCancel, yBT, m_Ds.cxBT, m_Ds.cyComm, hDlg, IDCANCEL);

	eck::SetFontForWndAndCtrl(hDlg, m_hFont);
	SetFocus(m_BTOk.HWnd);
	return FALSE;
}

void CDlgNewBookmark::OnOk(HWND hCtrl)
{
	m_pParam->rsName = m_EDName.GetText();
	if (!m_pParam->rsName.Size())
	{
		Utils::MsgBox(L"名称为空", NULL, L"错误", 1, (HICON)TD_ERROR_ICON, m_hWnd);
		return;
	}
	m_pParam->cr = m_CLPColor.GetColor();
	EndDlg(TRUE);
}

LRESULT CDlgNewBookmark::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		DeleteObject(m_hFont);
		break;

	case WM_DPICHANGED:
	{
		const auto prc = (RECT*)lParam;
		UpdateDpi(LOWORD(wParam));
		SetWindowPos(hWnd, NULL,
			prc->left,
			prc->top,
			prc->right - prc->left,
			prc->bottom - prc->top,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	return 0;
	}

	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

INT_PTR CDlgNewBookmark::DlgBox(HWND hParent, void* pData)
{
	m_pParam = (PARAM*)pData;
	const int iDpi = eck::GetDpi(hParent);
	const int cx = eck::DpiScale(352, iDpi);
	const int cy = eck::DpiScale(170, iDpi);

	return IntCreateModalDlg(0, eck::WCN_DLG, L"添加书签", WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
		0, 0, cx, cy, hParent, NULL, eck::g_hInstance, NULL, eck::DLGNCF_CENTERPARENT);
}
