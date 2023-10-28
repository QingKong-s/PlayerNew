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

LRESULT CDlgNewBookmark::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CDlgNewBookmark*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		p = (CDlgNewBookmark*)lParam;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		p->OnInitDialog(hWnd);
	}
	return FALSE;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_DESTROY:
		return HANDLE_WM_DESTROY(hWnd, wParam, lParam, p->OnDestroy);

	case WM_DPICHANGED:
		return HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, p->OnDpiChanged);

	case WM_COMMAND:
	{
		if (lParam && HIWORD(wParam) == BN_CLICKED)
			switch (LOWORD(wParam))
			{
			case IDC_BT_OK:
				p->OnCmdOk();
				return 0;
			case IDC_BT_CANCEL:
				p->OnCmdCancel();
				return 0;
			}
	}
	break;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void CDlgNewBookmark::OnInitDialog(HWND hWnd)
{
	UpdateDpiInit(eck::GetDpi(hWnd));
	m_hFont = eck::EzFont(L"微软雅黑", 9);
	RECT rc;
	GetClientRect(hWnd, &rc);

	const int xInput = m_Ds.iMargin + m_Ds.iGap + m_Ds.cxLabel;
	const int yLine2 = m_Ds.iMargin + m_Ds.iGap + m_Ds.cyComm;
	const int yBT = yLine2 + m_Ds.iGap * 2 + m_Ds.cyComm;
	m_LAName.Create(L"名称：", WS_VISIBLE, 0, m_Ds.iMargin, m_Ds.iMargin, m_Ds.cxLabel, m_Ds.cyComm, hWnd, IDC_LA_TIP1);
	m_EDName.Create(L"新书签", WS_VISIBLE, WS_EX_CLIENTEDGE, xInput, m_Ds.iMargin, m_Ds.cxInput, m_Ds.cyComm, hWnd, IDC_ED_NAME);

	m_LAColor.Create(L"颜色：", WS_VISIBLE, 0, m_Ds.iMargin, yLine2, m_Ds.cxLabel, m_Ds.cyComm, hWnd, IDC_LA_TIP2);
	m_CLPColor.Create(L"新书签", WS_VISIBLE, 0, xInput, yLine2, m_Ds.cxInput, m_Ds.cyComm, hWnd, IDC_CLP_CLR);
	m_CLPColor.SetItemHeight(m_Ds.cyComm);

	const int xBTCancel = rc.right - m_Ds.iMargin - m_Ds.cxBT;
	m_BTCancel.Create(L"取消", WS_VISIBLE, 0, xBTCancel, yBT, m_Ds.cxBT, m_Ds.cyComm, hWnd, IDC_BT_CANCEL);
	m_BTOk.Create(L"确定", WS_VISIBLE | BS_DEFPUSHBUTTON, 0, 
		xBTCancel - m_Ds.iGap - m_Ds.cxBT, yBT, m_Ds.cxBT, m_Ds.cyComm, hWnd, IDC_BT_OK);
	eck::SetFontForWndAndCtrl(hWnd, m_hFont);
}

void CDlgNewBookmark::OnDestroy(HWND hWnd)
{
	DeleteObject(m_hFont);
}

void CDlgNewBookmark::OnDpiChanged(HWND hWnd, int xDpi, int yDpi, RECT* pRect)
{
	UpdateDpi(xDpi);
	SetWindowPos(hWnd, NULL,
		pRect->left,
		pRect->top,
		pRect->right - pRect->left,
		pRect->bottom - pRect->top,
		SWP_NOZORDER | SWP_NOACTIVATE);
}

void CDlgNewBookmark::OnCmdOk()
{
	m_pParam->rsName = m_EDName.GetText();
	if (!m_pParam->rsName.Size())
	{
		Utils::MsgBox(L"名称为空", NULL, L"错误", 1, (HICON)TD_ERROR_ICON, m_hWnd);
		return;
	}
	m_pParam->cr = m_CLPColor.GetColor();
	m_bRet = TRUE;
	PostQuitMessage(0);
}

void CDlgNewBookmark::OnCmdCancel()
{
	PostQuitMessage(0);
}

INT_PTR CDlgNewBookmark::DlgBox(HWND hParent, void* pData)
{
	const HWND hOwner = PreModal(hParent);
	BOOL bNeedEnableOwner;
	if (hOwner && hOwner != GetDesktopWindow() && IsWindowEnabled(hOwner))
	{
		bNeedEnableOwner = TRUE;
		EnableWindow(hOwner, FALSE);
	}
	else
		bNeedEnableOwner = FALSE;

	m_pParam = (PARAM*)pData;
	constexpr PCWSTR c_szTile[]{ L"读入列表",L"保存列表" };
	RECT rc;
	if (hParent)
		GetWindowRect(hParent, &rc);
	else
		GetWindowRect(GetDesktopWindow(), &rc);

	const int iDpi = eck::GetDpi(hParent);
	const int cx = eck::DpiScale(352, iDpi);
	const int cy = eck::DpiScale(170, iDpi);

	m_hWnd = CreateWindowExW(0, eck::WCN_DLG, L"添加书签", WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
		rc.left + (rc.right - rc.left - cx) / 2, rc.top + (rc.bottom - rc.top - cy) / 2, cx, cy,
		hOwner, NULL, eck::g_hInstance, pData);
	eck::SetWindowProc(m_hWnd, WndProc);
	SendMsg(WM_INITDIALOG, 0, (LPARAM)this);

	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessageW(m_hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	if (bNeedEnableOwner)
		EnableWindow(hOwner, TRUE);
	if (hParent)
		SetActiveWindow(hParent);
	PostModal();
	Destroy();

	return m_bRet;
}
