#include "CWndList.h"

constexpr PCWSTR c_szDefListName = L"当前无播放列表";

LRESULT CWndList::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CWndList*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, p->OnSize);
	case WM_NCCREATE:
		p = (CWndList*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		break;
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, p->OnCreate);
	}
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

BOOL CWndList::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
	m_LAListName.Create(c_szDefListName, WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_LA_LIST_NAME);

	m_EDSearch.Create(NULL, WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, 0, hWnd, IDC_ED_SEARCH);
	m_EDSearch.SetFrameType(1);

	m_BTSearch.Create(L"搜索", WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_BT_SEARCH);

	m_BTTool[TBBTI_LOCATE].Create(L"定位", WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_BT_LOCATE);
	m_BTTool[TBBTI_ADD].Create(L"添加", WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_BT_ADD);
	m_BTTool[TBBTI_LOADLIST].Create(L"载入", WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_BT_LOADLIST);
	m_BTTool[TBBTI_SAVELIST].Create(L"保存", WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_BT_SAVELIST);
	m_BTTool[TBBTI_MANAGE].Create(L"管理", WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_BT_MANAGE);

	m_LVList.Create(NULL, WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA, 0, 0, 0, 0, 0, hWnd, IDC_LV_LIST);
	constexpr DWORD dwLVExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	m_LVList.SetLVExtendStyle(dwLVExStyle);
	m_LVList.InsertColumn(L"名称", 0, 200);
	m_LVList.InsertColumn(L"时长", 1, 100);
	return 0;
}

void CWndList::OnSize(HWND hWnd, UINT state, int cx, int cy)
{
}

ATOM CWndList::RegisterWndClass()
{
	WNDCLASSEX wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = eck::g_hInstance;
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.lpszClassName = WCN_LIST;
	wcex.cbWndExtra = sizeof(void*);
	return RegisterClassExW(&wcex);
}

HWND CWndList::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle, int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	m_hWnd = CreateWindowExW(dwExStyle, WCN_LIST, pszText, dwStyle,
		x, y, cx, cy, hParent, NULL, eck::g_hInstance, this);
	return m_hWnd;
}
