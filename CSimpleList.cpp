#include "CSimpleList.h"

void CSimpleList::ScrollProc(int iPos, int iPrevPos, LPARAM lParam)
{
	auto p = (CSimpleList*)lParam;
	DXGI_PRESENT_PARAMETERS dpp;
	POINT ptOffset = { 0,iPrevPos - iPos };
	if(ptOffset.y == 0)
		return;
	dpp.pScrollOffset = &ptOffset;

	RECT rcClient{ 0,0,p->m_cxClient,p->m_cyClient };
	if(ptOffset.y > 0)
		rcClient.top = ptOffset.y;
	else
		rcClient.bottom += ptOffset.y;
	dpp.pScrollRect = &rcClient;

	RECT rcDirty;
	dpp.pDirtyRects = &rcDirty;
	dpp.DirtyRectsCount = 1;

	D2D1_RECT_F rcItem;
	p->m_idxTop = iPos / p->m_cyItem;
	p->m_cyInvisibleTop = iPos - p->m_idxTop * p->m_cyItem;

	auto pDC = p->m_pDC;
	pDC->BeginDraw();
	if (ptOffset.y < 0)// 向上滚动
	{
		rcDirty = { 0,p->m_cyClient + ptOffset.y,p->m_cxClient,p->m_cyClient };

		int idxBottom = (p->m_ScrollView.GetPos() + p->m_cyClient) / p->m_cyItem;
		if(idxBottom >= p->m_cItems)
			idxBottom = p->m_cItems - 1;
		for (int i = idxBottom; i >= 0; --i)
		{
			p->RedrawItem(i, rcItem);
			if (rcItem.bottom <= rcDirty.top)
				break;
		}
	}
	else
	{
		rcDirty = { 0,0,p->m_cxClient,ptOffset.y };

		for (int i = p->m_idxTop; i < p->m_cItems; ++i)
		{
			p->RedrawItem(i, rcItem);
			if (rcItem.top >= rcDirty.bottom)
				break;
		}
	}
	pDC->EndDraw();
	p->m_pSwapChain->Present1(0, 0, &dpp);
}

LRESULT CSimpleList::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CSimpleList*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_MOUSEMOVE:
		return HANDLE_WM_MOUSEMOVE(hWnd, wParam, lParam, p->OnMouseMove);
	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, p->OnSize);
	case WM_MOUSELEAVE:
		return HANDLE_WM_MOUSELEAVE(hWnd, wParam, lParam, p->OnMouseLeave);
	case WM_MOUSEWHEEL:
		return HANDLE_WM_MOUSEWHEEL(hWnd, wParam, lParam, p->OnMouseWheel);
	case WM_NCCREATE:
		p = (CSimpleList*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		break;
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, p->OnCreate);
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

BOOL CSimpleList::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	rc.right = std::max(rc.right, 8l);
	rc.bottom = std::max(rc.bottom, 8l);
	DXGI_SWAP_CHAIN_DESC1 DxgiSwapChainDesc
	{
		(UINT)rc.right,
		(UINT)rc.bottom,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		FALSE,
		{1, 0},
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		2,
		DXGI_SCALING_NONE,
		DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
		DXGI_ALPHA_MODE_UNSPECIFIED,
		0
	};

	HRESULT hr;
	if (FAILED(hr = App->m_pDxgiFactory->CreateSwapChainForHwnd(
		App->m_pDxgiDevice,
		hWnd,
		&DxgiSwapChainDesc,
		NULL,
		NULL,
		&m_pSwapChain)))// 创建交换链
	{
		assert(FALSE);
	}

	if (FAILED(hr = App->m_pD2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pDC)))// 创建设备上下文
	{
		assert(FALSE);
	}

	IDXGISurface1* pSurface;
	if (FAILED(hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface))))// 取缓冲区
	{
		assert(FALSE);
	}

	D2D1_BITMAP_PROPERTIES1 D2dBmpProp
	{
		{DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
		96,
		96,
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		NULL
	};

	if (FAILED(hr = m_pDC->CreateBitmapFromDxgiSurface(pSurface, &D2dBmpProp, &m_pBmpBK)))// 创建位图自DXGI表面
	{
		assert(FALSE);
	}

	pSurface->Release();

	m_pDC->SetTarget(m_pBmpBK);

	m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBrText);
	m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGray), &m_pBrHot);
	m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBrBk);

	App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL, DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 26.f, L"zh-cn", &m_pTextFormat);

	m_ScrollView.SetHWND(hWnd);
	m_ScrollView.SetTimerID(1001);

	return TRUE;
}

void CSimpleList::OnSize(HWND hWnd, UINT uState, int cx, int cy)
{
	m_cxClient = cx;
	m_cyClient = cy;

	m_pDC->SetTarget(NULL);// 移除引用
	m_pBmpBK->Release();

	HRESULT hr;
	if (FAILED(hr = m_pSwapChain->ResizeBuffers(0, cx, cy, DXGI_FORMAT_UNKNOWN, 0)))
	{
		assert(FALSE);
	}

	IDXGISurface1* pSurface;
	hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface));
	if (!pSurface)
	{
		assert(FALSE);
	}

	D2D1_BITMAP_PROPERTIES1 D2dBmpProp
	{
		{DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
		96,
		96,
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		NULL
	};

	if (FAILED(hr = m_pDC->CreateBitmapFromDxgiSurface(pSurface, &D2dBmpProp, &m_pBmpBK)))
	{
		assert(FALSE);
	}

	pSurface->Release();

	m_pDC->SetTarget(m_pBmpBK);

	m_ScrollView.SetPage(m_cyClient);

	Redraw();
}

void CSimpleList::OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags)
{
	int idxHot = HitTest({ x,y });
	if (idxHot != m_idxHot)
	{
		std::swap(idxHot, m_idxHot);
		Redraw({ idxHot,m_idxHot });
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = hWnd;
	TrackMouseEvent(&tme);
}

void CSimpleList::OnMouseLeave(HWND hWnd)
{
	int idx = -1;
	std::swap(idx, m_idxHot);
	if (idx >= 0)
		Redraw({ idx });
}

void CSimpleList::OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT fwKeys)
{
	//int iOldPos = m_ScrollView.GetPos();
	//m_ScrollView.OnMouseWheel(zDelta / WHEEL_DELTA);

	m_ScrollView.OnMouseWheel2(zDelta / WHEEL_DELTA, ScrollProc, (LPARAM)this);
}

ATOM CSimpleList::RegisterWndClass()
{
	WNDCLASSEX wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = eck::g_hInstance;
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.lpszClassName = WCN_SIMPLELIST;
	wcex.cbWndExtra = sizeof(void*);
	return RegisterClassExW(&wcex);
}

HWND CSimpleList::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle, int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	m_hWnd = CreateWindowExW(dwExStyle, WCN_SIMPLELIST, pszText, dwStyle,
		x, y, cx, cy, hParent, NULL, eck::g_hInstance, this);
	return m_hWnd;
}

void CSimpleList::Redraw()
{
	m_pDC->BeginDraw();

	D2D1_RECT_F rcItem{};

	for (int i = m_idxTop; i < m_cItems; ++i)
	{
		RedrawItem(i, rcItem);
		if (rcItem.top > m_cyClient)
			break;
	}

	m_pDC->EndDraw();
	m_pSwapChain->Present(0, 0);
}

void CSimpleList::Redraw(std::initializer_list<int> idxItem)
{
	m_pDC->BeginDraw();

	std::vector<RECT> UpdateRects{};
	UpdateRects.reserve(idxItem.size());

	D2D1_RECT_F rcItem{};
	int idxBottom = (m_ScrollView.GetPos() + m_cyClient) / m_cyItem;
	for (int idx : idxItem)
	{
		if (idx < m_idxTop || idx > idxBottom)
			continue;

		RedrawItem(idx, rcItem);

		if (rcItem.bottom > m_cyClient)
			rcItem.bottom = (float)m_cyClient;
		if (rcItem.top < 0.f)
			rcItem.top = 0.f;

		UpdateRects.push_back({ (long)rcItem.left,(long)rcItem.top,(long)rcItem.right,(long)rcItem.bottom });
	}

	m_pDC->EndDraw();

	DXGI_PRESENT_PARAMETERS dpp{};
	dpp.DirtyRectsCount = (UINT)UpdateRects.size();
	dpp.pDirtyRects = UpdateRects.data();
	m_pSwapChain->Present1(0, 0, &dpp);
}

int CSimpleList::HitTest(POINT pt)
{
	if (pt.x < 0 || pt.x >= m_cxClient || pt.y < 0 || pt.y >= m_cyClient)
		return -1;
	return (pt.y + m_ScrollView.GetPos()) / m_cyItem;
}
