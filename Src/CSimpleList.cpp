#include "CSimpleList.h"

void CSimpleList::ScrollProc(int iPos, int iPrevPos, LPARAM lParam)
{
	auto p = (CSimpleList*)lParam;

	p->CalcTopItem();

	DXGI_PRESENT_PARAMETERS dpp;
	POINT ptOffset{ 0,iPrevPos - iPos };
	if (ptOffset.y == 0)
		return;
	dpp.pScrollOffset = &ptOffset;

	RECT rcClient{ 0,0,p->m_cxClient,p->m_cyClient };
	if (ptOffset.y > 0)
	{
		if (ptOffset.y >= p->m_cyClient)// 滚动距离超过页面大小，直接重画整个页面
		{
			p->Redraw();
			return;
		}
		else
			rcClient.top = ptOffset.y;
	}
	else
	{
		if (-ptOffset.y >= p->m_cyClient)// 滚动距离超过页面大小，直接重画整个页面
		{
			p->Redraw();
			return;
		}
		else
			rcClient.bottom += ptOffset.y;
	}

	dpp.pScrollRect = &rcClient;

	RECT rcDirty;
	dpp.pDirtyRects = &rcDirty;
	dpp.DirtyRectsCount = 1;

	D2D1_RECT_F rcItem;

	auto pDC = p->m_pDC;
	pDC->BeginDraw();
	if (ptOffset.y < 0)// 滚动条滑块向下
	{
		rcDirty = { 0,p->m_cyClient + ptOffset.y,p->m_cxClient,p->m_cyClient };

		if (p->m_bGroup)
		{
			auto it = std::lower_bound(p->m_Group.begin(), p->m_Group.end(), p->m_cyClient + ptOffset.y, [](const GROUPITEM& x, int iPos)
				{
					return x.y < iPos;
				});

			--it;
			int idxTopNew = (int)std::distance(p->m_Group.begin(), it);
			for (int i = idxTopNew; i < (int)p->m_Group.size(); ++i)
			{
				p->DrawGroup(i, rcItem);
				if (rcItem.top >= rcDirty.bottom)
					break;
				EckCounter(p->m_Group[i].Item.size(), j)
				{
					p->DrawGroupItem(i, j, rcItem);
					if (rcItem.top > rcDirty.bottom)
						break;
				}
			}
		}
		else
		{
			int idxBottom = (p->m_ScrollView.GetPos() + p->m_cyClient) / p->m_cyItem;
			if (idxBottom >= p->m_cItems)
				idxBottom = p->m_cItems - 1;
			for (int i = idxBottom; i >= 0; --i)
			{
				p->RedrawItem(i, rcItem);
				if (rcItem.bottom <= rcDirty.top)
					break;
			}
		}
	}
	else// 滚动条滑块向上
	{
		rcDirty = { 0,0,p->m_cxClient,ptOffset.y };

		if (p->m_bGroup)
		{
			for (int i = p->m_idxTop; i < (int)p->m_Group.size(); ++i)
			{
				p->DrawGroup(i, rcItem);
				if (rcItem.top > rcDirty.bottom)
					break;
				EckCounter(p->m_Group[i].Item.size(), j)
				{
					p->DrawGroupItem(i, j, rcItem);
					if (rcItem.top >= rcDirty.bottom)
						break;
				}
			}
		}
		else
		{
			for (int i = p->m_idxTop; i < p->m_cItems; ++i)
			{
				p->RedrawItem(i, rcItem);
				if (rcItem.top >= rcDirty.bottom)
					break;
			}
		}
	}

	pDC->EndDraw();
	p->m_pSwapChain->Present1(1, 0, &dpp);
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
		DXGI_ALPHA_MODE_IGNORE,
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
		{DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_IGNORE},
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
	m_pDC->CreateSolidColorBrush(D2D1::ColorF(0x66CCFF, 0.6f), &m_pBrGroup);

	App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL, DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, eck::DpiScaleF(12.f, eck::GetDpi(hWnd)), L"zh-cn", &m_pTextFormat);

	m_ScrollView.SetHWND(hWnd);

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
	int idxHot;
	if (m_bGroup)
	{
		SLHITTEST slht;
		idxHot = HitTest({ x,y }, &slht);
		if (idxHot != m_idxHot || m_idxHotItemSGroup != slht.idxGroup)
		{
			std::swap(idxHot, m_idxHot);
			std::swap(slht.idxGroup, m_idxHotItemSGroup);
			Redraw(slht.idxGroup, { idxHot });
			Redraw(m_idxHotItemSGroup, { m_idxHot });
		}
	}
	else
	{
		idxHot = HitTest({ x,y });
		if (idxHot != m_idxHot)
		{
			std::swap(idxHot, m_idxHot);
			Redraw({ idxHot,m_idxHot });
		}
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
	if (m_bGroup)
	{
		int idxGroup = -1;
		std::swap(idx, m_idxHot);
		std::swap(idxGroup, m_idxHotItemSGroup);
		Redraw(idxGroup, { idx });
	}
	else
	{
		std::swap(idx, m_idxHot);
		if (idx >= 0)
			Redraw({ idx });
	}
}

void CSimpleList::OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT fwKeys)
{
	m_ScrollView.OnMouseWheel2(-zDelta / WHEEL_DELTA, ScrollProc, (LPARAM)this);
}

void CSimpleList::CalcTopItem()
{
	if (m_bGroup)
	{
		if (!m_Group.size())
		{
			m_idxTop = 0;
			return;
		}
		auto it = std::lower_bound(m_Group.begin(), m_Group.end(), m_ScrollView.GetPos(), [](const GROUPITEM& x, int iPos)
			{
				return x.y < iPos;
			});

		if (it == m_Group.end())
			EckDbgBreak();

		if (it == m_Group.begin())
		{
			m_idxTop = 0;
			return;
		}
		--it;
		m_idxTop = (int)std::distance(m_Group.begin(), it);
	}
	else
	{
		m_idxTop = m_ScrollView.GetPos() / m_cyItem;
		m_cyInvisibleTop = m_ScrollView.GetPos() - m_idxTop * m_cyItem;
	}
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
	D2D1_RECT_F rcItem{};

	if (m_bGroup)
	{
		if (!m_Group.size())
			return;

		m_pDC->BeginDraw();
		m_pDC->Clear(D2D1::ColorF(D2D1::ColorF::White));

		for (int i = m_idxTop; i < (int)m_Group.size(); ++i)
		{
			DrawGroup(i, rcItem);

			EckCounter(m_Group[i].Item.size(), j)
			{
				DrawGroupItem(i, j, rcItem);
			}
		}
	}
	else
	{
		for (int i = m_idxTop; i < m_cItems; ++i)
		{
			RedrawItem(i, rcItem);
			if (rcItem.top > m_cyClient)
				break;
		}
	}

	m_pDC->EndDraw();
	m_pSwapChain->Present(1, 0);
}

void CSimpleList::Redraw(std::initializer_list<int> idxItem)
{
	std::vector<RECT> UpdateRects{};
	UpdateRects.reserve(idxItem.size());

	D2D1_RECT_F rcItem{};

	if (m_bGroup)
	{
		return;
	}
	else
	{
		m_pDC->BeginDraw();
		int idxBottom = (m_ScrollView.GetPos() + m_cyClient) / m_cyItem;
		for (int idx : idxItem)
		{
			if (idx < m_idxTop || idx > idxBottom)
				continue;

			if (RedrawItem(idx, rcItem))
			{
				if (rcItem.bottom > m_cyClient)
					rcItem.bottom = (float)m_cyClient;
				if (rcItem.top < 0.f)
					rcItem.top = 0.f;

				UpdateRects.push_back({ (long)rcItem.left,(long)rcItem.top,(long)rcItem.right,(long)rcItem.bottom });
			}
		}
	}

	m_pDC->EndDraw();

	DXGI_PRESENT_PARAMETERS dpp{};
	dpp.DirtyRectsCount = (UINT)UpdateRects.size();
	dpp.pDirtyRects = UpdateRects.data();
	m_pSwapChain->Present1(1, 0, &dpp);
}

void CSimpleList::Redraw(int idxGroup, std::initializer_list<int> idxItem)
{
	if (!m_bGroup || idxGroup < 0)
		return;
	std::vector<RECT> UpdateRects{};
	UpdateRects.reserve(idxItem.size());

	D2D1_RECT_F rcItem{};

	m_pDC->BeginDraw();
	
	for (int idx : idxItem)
	{
		if (idx < 0)
			continue;
		if (DrawGroupItem(idxGroup, idx, rcItem))
		{
			if (rcItem.bottom > m_cyClient)
				rcItem.bottom = (float)m_cyClient;
			if (rcItem.top < 0.f)
				rcItem.top = 0.f;

			UpdateRects.push_back({ (long)rcItem.left,(long)rcItem.top,(long)rcItem.right,(long)rcItem.bottom });
		}
	}

	m_pDC->EndDraw();
	if (!UpdateRects.size())
		return;
	DXGI_PRESENT_PARAMETERS dpp{};
	dpp.DirtyRectsCount = (UINT)UpdateRects.size();
	dpp.pDirtyRects = UpdateRects.data();
	m_pSwapChain->Present1(1, 0, &dpp);
}

int CSimpleList::HitTest(POINT pt, SLHITTEST* pslht)
{
	if (pt.x < 0 || pt.x >= m_cxClient || pt.y < 0 || pt.y >= m_cyClient)
		return -1;
	if (m_bGroup)
	{
		auto it = std::lower_bound(m_Group.begin(), m_Group.end(), pt.y + m_ScrollView.GetPos(), [](const GROUPITEM& x, int iPos)
			{
				return x.y < iPos;
			});
		
		pslht->idxGroup = -1;
		if (it == m_Group.end() || it == m_Group.begin())
			return -1;

		--it;
		const int idxGroup = std::distance(m_Group.begin(), it);
		if (pt.y > it->y && pt.y < it->y + m_cyGroupHeader)
		{
			pslht->idxGroup = idxGroup;
			return -1;
		}

		pslht->idxGroup = idxGroup;
		const int y = pt.y + m_ScrollView.GetPos() - it->y - m_cyGroupHeader;
		if (y >= 0)
		{
			int idx = y / m_cyItem;
			if (idx < (int)it->Item.size())
				return idx;
		}
		return -1;
	}
	else
	{
		const int idx = (pt.y + m_ScrollView.GetPos()) / m_cyItem;
		if (idx >= (int)m_Item.size())
			return -1;
		else
			return idx;
	}
}

void CSimpleList::ReCalc(int idxBegin)
{
	int y = 0;
	for (int i = idxBegin; i < m_Group.size(); ++i)
	{
		auto& Group = m_Group[i];
		Group.y = y;
		y += m_cyGroupHeader;
		for (auto& Item : Group.Item)
		{
			Item.y = y;
			y += m_cyItem;
		}
		
		if (Group.Item.size() < c_iMinLinePerGroup)
		{
			y += ((c_iMinLinePerGroup - Group.Item.size()) * m_cyItem);
		}
	}
	m_ScrollView.SetMax(y);
}