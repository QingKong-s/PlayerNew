#include <format>

#include "CWndMain.h"


constexpr PCWSTR c_pszWndClassMain = L"PlayerNew.WndClass.Main";

void CWndMain::UpdateDpi(int iDpi)
{
	m_iDpi = iDpi;
	eck::UpdateDpiSize(m_Ds, iDpi);
}

LRESULT CWndMain::WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CWndMain*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_NCCREATE:
		p = (CWndMain*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		break;
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, p->OnCreate);
	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, p->OnSize);
	case WM_DPICHANGED:
		return HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, p->OnDpiChanged);
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void CWndMain::OnSize(HWND hWnd, UINT uState, int cx, int cy)
{
	HDWP hDwp = BeginDeferWindowPos(3);
	hDwp = DeferWindowPos(hDwp, m_Bk.GetHWND(), NULL,
		0,
		0,
		m_xSeparateBar,
		cy, SWP_NOZORDER | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_List.GetHWND(), NULL,
		m_xSeparateBar + m_Ds.cxSeparateBar,
		0,
		cx - m_Ds.cxSeparateBar - m_xSeparateBar,
		cy, SWP_NOZORDER | SWP_NOACTIVATE);
	EndDeferWindowPos(hDwp);
}

BOOL CWndMain::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
	UpdateDpi(eck::GetDpi(hWnd));
	m_Bk.Create(NULL, WS_CHILD, 0, 0, 0, 0, 0, hWnd, IDC_BK);
	m_List.Create(L"列表", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 0, 0, 0, hWnd, IDC_LIST);

	RECT rc;
	GetClientRect(hWnd, &rc);

	m_xSeparateBar = rc.right * 2 / 3;
	OnSize(hWnd, 0, rc.right, rc.bottom);

	ShowWindow(m_Bk.GetHWND(), SW_SHOWNOACTIVATE);
	ShowWindow(m_List.GetHWND(), SW_SHOWNOACTIVATE);
	return TRUE;
	m_vStr.resize(100000);
	EckCounter(m_vStr.size(), i)
	{
		m_vStr[i] = std::format(L"第{}行", i).c_str();
	}
	m_Sl.SetItemHeight(40);
	m_Sl.SetDispInfoProc([this](SLGETDISPINFO* p)
		{
			p->pItemInfo->pszText= m_vStr[p->pItemInfo->idxItem];
			p->pItemInfo->cchText = m_vStr[p->pItemInfo->idxItem].Size();
			return 0;
		});
	m_Sl.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 100, 100, 500, 700, hWnd, 0);
	m_Sl.SetItemCount((int)m_vStr.size());
	return TRUE;
}

void CWndMain::OnDpiChanged(HWND hWnd, int xDpi, int yDpi, RECT* pRect)
{
	UpdateDpi(xDpi);
}

ATOM CWndMain::RegisterWndClass()
{
	WNDCLASSEX wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc_Main;
	wcex.hInstance = App->GetHInstance();
	wcex.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.lpszClassName = c_pszWndClassMain;
	wcex.cbWndExtra = sizeof(void*);
	return RegisterClassExW(&wcex);
}

HWND CWndMain::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	m_hWnd = CreateWindowExW(dwExStyle, c_pszWndClassMain, pszText, dwStyle,
		x, y, cx, cy, NULL, NULL, App->GetHInstance(), this);
	return m_hWnd;
}