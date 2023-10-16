#include "CWndMain.h"
#include <format>

constexpr PCWSTR c_pszWndClassMain = L"PlayerNew.WndClass.Main";

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

	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

BOOL CWndMain::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
	return TRUE;
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