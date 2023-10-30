#include "CWndBK.h"
constexpr static D2D_COLOR_F c_D2DClrCyanDeeper{ 0.f,0.3764f,0.7529f,1.f };
constexpr D2D1_COLOR_F c_D2DCrUIProgBarTempMark{ 0.0f,0.502f,1.0f,1.0f };
#define MYCLR_BTHOT				0xE6E8B1
#define MYCLR_BTPUSHED			0xE6E843
#define MYCLR_BTCHECKED			0xE6E88C

#define BTMBKBTNCOUNT			10

constexpr PCWSTR c_szBtmTip[]
{
    L"上一曲",
    L"播放/暂停",
    L"停止",
    L"下一曲",
    L"歌词",
    L"循环方式",
    L"播放设置",
    L"显示/隐藏播放列表",
    L"设置",
    L"关于",
    L"循环方式：整体循环",
    L"循环方式：随机播放",
    L"循环方式：单曲播放",
    L"循环方式：单曲循环",
    L"循环方式：整体播放"
};

LRESULT CWndBK::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CWndBK*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_TIMER:
	{
		switch (wParam)
		{
		case IDT_PGS:
		{
			App->GetPlayer().Tick();
			p->m_vDirtyRect.clear();
			p->m_pDC->BeginDraw();
			for (auto pElem : p->m_vElemsWantTimer)
			{
				if (eck::IsBitSet(pElem->m_uFlags, UIEF_ONLYPAINTONTIMER))
				{
					pElem->Redraw();
					p->m_vDirtyRect.emplace_back(pElem->m_rcInWnd);
				}
				else
					pElem->OnTimer(wParam);
			}
			p->m_pDC->EndDraw();
			DXGI_PRESENT_PARAMETERS dpp{};
			dpp.DirtyRectsCount = (UINT)p->m_vDirtyRect.size();
			dpp.pDirtyRects = p->m_vDirtyRect.data();
			p->m_pSwapChain->Present1(1, 0, &dpp);
		}
		return 0;

		default:
		{
            for (auto pElem : p->m_vElemsWantTimer)
            {
                pElem->OnTimer(wParam);
            }
        }
        return 0;
        }
    }
    return 0;

    case WM_NCHITTEST:
    case WM_SETCURSOR:
    case WM_NOTIFY:
    case WM_MOUSEACTIVATE:
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    case WM_LBUTTONDOWN:
        SetFocus(hWnd);
        break;

	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, p->OnSize);
	case WM_NCCREATE:
		p = (CWndBK*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		break;
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, p->OnCreate);
	}

	for (auto pElem : p->m_vElems)
	{
        if (!eck::IsBitSet(pElem->m_uFlags, UIEF_NOEVENT))
            if (pElem->OnEvent(uMsg, wParam, lParam))
                break;
	}

	switch (uMsg)
    {
    case WM_MOUSEMOVE:
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = hWnd;
        tme.dwFlags = TME_LEAVE | TME_HOVER;
        tme.dwHoverTime = 200;
        TrackMouseEvent(&tme);
    }
    return 0;
    }

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

BOOL CWndBK::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
    m_iDpi = eck::GetDpi(hWnd);
    eck::UpdateDpiSize(m_Ds, m_iDpi);
    eck::UpdateDpiSizeF(m_DsF, m_iDpi);
	RECT rc;
	GetClientRect(hWnd, &rc);
	rc.right = std::max(rc.right, 8L);
	rc.bottom = std::max(rc.bottom, 8L);
	DXGI_SWAP_CHAIN_DESC1 DxgiSwapChainDesc
	{
		(UINT)rc.right,
		(UINT)rc.bottom,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		FALSE,
		{1, 0},
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		2,
		DXGI_SCALING_STRETCH,
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
		{DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE},
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

    D2D_SIZE_U D2DSizeU = { 8,8 };
    D2dBmpProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
    if (FAILED(hr = m_pDC->CreateBitmap(D2DSizeU, NULL, 0, D2dBmpProp, &m_pBmpBKStatic)))// 创建一幅内存位图
    {
		EckDbgPrint(L"ID2D1DeviceContext::CreateBitmap Error");
    }

    m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBrWhite);
    m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.5f), &m_pBrWhite2);

	m_pDC->SetTarget(m_pBmpBK);

    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Prev], &m_pBmpIcon[ICIDX_Prev]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Triangle], &m_pBmpIcon[ICIDX_Play]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Block], &m_pBmpIcon[ICIDX_Stop]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Next], &m_pBmpIcon[ICIDX_Next]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Lrc], &m_pBmpIcon[ICIDX_Lrc]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Circle], &m_pBmpIcon[ICIDX_RMAllLoop]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_ArrowRight3], &m_pBmpIcon[ICIDX_RMAll]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_ArrowCross], &m_pBmpIcon[ICIDX_RMRadom]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_CircleOne], &m_pBmpIcon[ICIDX_RMSingleLoop]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_ArrowRight], &m_pBmpIcon[ICIDX_RMSingle]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_PlayOpt], &m_pBmpIcon[ICIDX_PlayOpt]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_PlayList], &m_pBmpIcon[ICIDX_PlayList]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Gear], &m_pBmpIcon[ICIDX_Options]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Info], &m_pBmpIcon[ICIDX_About]);
    m_pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Pause], &m_pBmpIcon[ICIDX_Pause]);
    return TRUE;
}

void CWndBK::OnSize(HWND hWnd, UINT state, int cx, int cy)
{
    m_cxClient = cx;
    m_cyClient = cy;
    m_rcfClient = { 0.f,0.f,(float)cx,(float)cy };

    m_pDC->SetTarget(NULL);// 移除引用
    m_pBmpBK->Release();

    HRESULT hr;
    if (FAILED(hr = m_pSwapChain->ResizeBuffers(0, std::max(cx, 8), std::max(cy, 8), DXGI_FORMAT_UNKNOWN, 0)))
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

	m_pBmpBKStatic->Release();
	D2D_SIZE_U D2DSizeU = { (UINT32)cx, (UINT32)cy };
	D2dBmpProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
	if (FAILED(hr = m_pDC->CreateBitmap(D2DSizeU, NULL, 0, D2dBmpProp, &m_pBmpBKStatic)))
	{
		EckDbgPrint(L"ID2D1DeviceContext::CreateBitmap Error");
	}
	m_pDC->SetTarget(m_pBmpBK);
}

void CWndBK::GenElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    for (auto pElem : m_vElems)
        pElem->OnElemEvent(uEvent, wParam, lParam);
}

CWndBK::CWndBK()
{
}

CWndBK::~CWndBK()
{
}

ATOM CWndBK::RegisterWndClass()
{
	WNDCLASSEX wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = App->GetHInstance();
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.lpszClassName = WCN_MAINBK;
	wcex.cbWndExtra = sizeof(void*);
	return RegisterClassExW(&wcex);
}

HWND CWndBK::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle, int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	m_hWnd = CreateWindowExW(dwExStyle, WCN_MAINBK, pszText, dwStyle,
		x, y, cx, cy, hParent, eck::i32ToP<HMENU>(nID), App->GetHInstance(), this);
	return m_hWnd;
}

void CWndBK::AddElem(CUIElem* pElem)
{
    pElem->m_pBK = this;
    m_vElems.emplace_back(pElem);
    m_vAllElems.emplace_back(pElem);
    m_vDirtyRect.reserve(m_vElems.size());
    if (eck::IsBitSet(pElem->m_uFlags, UIEF_WANTTIMEREVENT))
        m_vElemsWantTimer.emplace_back(pElem);
}

void CWndBK::OnPlayingControl(PLAYINGCTRLTYPE uType)
{
    switch (uType)
    {
    case PCT_PLAY:
    {
        SAFE_RELEASE(m_pBmpAlbum);
        m_pDC->CreateBitmapFromWicBitmap(App->GetPlayer().GetWicBmpCover(), &m_pBmpAlbum);
        auto Size = m_pBmpAlbum->GetSize();
        m_cxAlbum = (int)Size.width;
        m_cyAlbum = (int)Size.height;
        UpdateStaticBmp();
        SetTimer(m_hWnd, IDT_PGS, 40, NULL);
    }
    break;
    case PCT_STOP:
        KillTimer(m_hWnd, IDT_PGS);
        break;
    }

    GenElemEvent(UIEE_ONPLAYINGCTRL, uType, 0);
    Redraw();
}

void CWndBK::Redraw()
{
    m_pDC->BeginDraw();
    m_pDC->SetTarget(m_pBmpBK);
    m_pDC->DrawBitmap(m_pBmpBKStatic);
	for (auto pElem : m_vElems)
    {
		if (!eck::IsBitSet(pElem->m_uFlags, UIEF_STATIC))
            pElem->Redraw();
    }
    m_pDC->EndDraw();
    m_pSwapChain->Present(0, 0);
}

void CWndBK::UpdateStaticBmp()
{
    if (m_cxClient <= 0 || m_cyClient <= 0)
        return;
    IWICBitmap* pWICBitmapOrg = App->GetPlayer().GetWicBmpCover();// 原始WIC位图

    ////////////////////绘制静态位图
    m_pDC->BeginDraw();
    m_pDC->SetTarget(m_pBmpBKStatic);

    UINT cx0, cy0;
    float cxRgn/*截取区域宽*/, cyRgn/*截取区域高*/, cx/*缩放后图片宽*/, cy/*缩放后图片高*/;
    D2D_RECT_F D2DRectF;

    ID2D1SolidColorBrush* pBrush;
    if (pWICBitmapOrg)
    {
        /*
        情况一，客户区宽为最大边
        cxClient   cyClient
        -------- = --------
         cxPic      cyRgn
        情况二，客户区高为最大边
        cyClient   cxClient
        -------- = --------
         cyPic      cxRgn
        */
        ////////////////////处理缩放与截取（无论怎么改变窗口大小，用来模糊的封面图都要居中充满整个窗口）
        D2D_POINT_2F D2DPtF;
        pWICBitmapOrg->GetSize(&cx0, &cy0);
        cyRgn = (float)m_cyClient / (float)m_cxClient * (float)cx0;
        if (cyRgn <= cy0)// 情况一
        {
            cx = (float)m_cxClient;
            cy = cx * cy0 / cx0;
            D2DPtF = { 0,(float)(m_cyClient - cy) / 2 };
        }
        else// 情况二
        {
            cy = (float)m_cyClient;
            cx = cx0 * cy / cy0;
            D2DPtF = { (float)(m_cxClient - cx) / 2,0 };
        }
        ////////////缩放
        IWICBitmapScaler* pWICBitmapScaler;// WIC位图缩放器
        App->m_pWicFactory->CreateBitmapScaler(&pWICBitmapScaler);
        pWICBitmapScaler->Initialize(pWICBitmapOrg, (UINT)cx, (UINT)cy, WICBitmapInterpolationModeCubic);// 缩放
        IWICBitmap* pWICBmpScaled;// 缩放后的WIC位图
        App->m_pWicFactory->CreateBitmapFromSource(pWICBitmapScaler, WICBitmapNoCache, &pWICBmpScaled);
        pWICBitmapScaler->Release();// 释放WIC位图缩放器
        ID2D1Bitmap1* pD2DBmpScaled;// 缩放后的D2D位图
        m_pDC->CreateBitmapFromWicBitmap(pWICBmpScaled, &pD2DBmpScaled);// 转换为D2D位图
        pWICBmpScaled->Release();// 释放缩放后的WIC位图

        ////////////模糊 
        ID2D1Image* pD2DBmpBlurred = NULL;
        ID2D1Effect* pD2DEffect;
        m_pDC->CreateEffect(CLSID_D2D1GaussianBlur, &pD2DEffect);
        if (pD2DEffect)
        {
            pD2DEffect->SetInput(0, pD2DBmpScaled);
            pD2DEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 50.0f);// 标准偏差
            pD2DEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);// 硬边缘
            pD2DEffect->GetOutput(&pD2DBmpBlurred);
            pD2DEffect->Release();
        }
        pD2DBmpScaled->Release();
        ////////////画模糊背景
        if (pD2DBmpBlurred)
        {
            m_pDC->DrawImage(pD2DBmpBlurred, &D2DPtF);
            pD2DBmpBlurred->Release();
        }
        ////////////画半透明遮罩
        m_pDC->FillRectangle(&m_rcfClient, m_pBrWhite2);
    }
    else// 没有图就全刷白吧
        m_pDC->FillRectangle(&m_rcfClient, m_pBrWhite);

    ////////////////////画静态元素
	for (auto pElem : m_vElems)
    {
        if (pElem->m_uFlags & UIEF_STATIC)
            pElem->Redraw();
    }

    m_pDC->EndDraw();
    m_pDC->SetTarget(m_pBmpBK);
}





LRESULT CUIElem::DefElemEventProc(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
	switch (uEvent)
	{
	case UIEE_SETRECT:
	{
		auto prc = (RECT*)wParam;
		//---------相对位置
		m_rcRelative = *prc;
		//---------绝对位置
		m_rc = *prc;
		if (m_pParent)
			OffsetRect(&m_rc, m_pParent->m_rc.left, m_pParent->m_rc.top);
		// 取D2D矩形
		m_rcF = eck::MakeD2DRcF(m_rc);
		// 计算尺寸
		m_cx = m_rc.right - m_rc.left;
		m_cy = m_rc.bottom - m_rc.top;
		m_cxHalf = m_cx / 2;
		m_cyHalf = m_cy / 2;
		// 计算窗口范围内的矩形
		m_rcInWnd = m_rc;
		if (m_rcInWnd.left < 0)
			m_rcInWnd.left = 0;
		if (m_rcInWnd.top < 0)
			m_rcInWnd.top = 0;
		if (m_rcInWnd.right > m_pBK->m_cxClient)
			m_rcInWnd.right = m_pBK->m_cxClient;
		if (m_rcInWnd.bottom > m_pBK->m_cyClient)
			m_rcInWnd.bottom = m_pBK->m_cyClient;
		//---------更新子窗口的绝对位置
		RECT rc;
		for (auto pElem : m_vChildren)
		{
			pElem->GetElemRect(&rc);
			pElem->SetElemRect(&rc);
		}

		if (!m_pParent)
			m_pBK->m_vDirtyRect.emplace_back(m_rcInWnd);
	}
	return 0;
	}
	return 0;
}

CUIElem::~CUIElem()
{
}

void CUIElem::SetParent(CUIElem* pParent)
{
    if (m_pParent == pParent)
        return;
    if (m_pParent)
    {
        auto it = std::find(m_pParent->m_vChildren.begin(), m_pParent->m_vChildren.end(), this);
        if (it != m_pParent->m_vChildren.end())
            m_pParent->m_vChildren.erase(it);
    }
    else
    {
        auto it = std::find(m_pBK->m_vElems.begin(), m_pBK->m_vElems.end(), this);
        if (it != m_pBK->m_vElems.end())
            m_pBK->m_vElems.erase(it);
    }
    m_pParent = pParent;
    if (m_pParent)
        m_pParent->m_vChildren.emplace_back(this);
    else
        m_pBK->m_vElems.emplace_back(this);
    RECT rc;
    GetElemRect(&rc);
    SetElemRect(&rc);
}

CUIAlbum::CUIAlbum()
{
    m_uType = UIET_ALBUM;
    m_uFlags = UIEF_STATIC;
}

void CUIAlbum::Redraw()
{
    //auto pWICBmp = m_CurrSongInfo.mi.pWICBitmap;
    if (!m_pBK->m_pBmpAlbum)
        return;

    auto pDC = m_pBK->m_pDC;

    int cxRgn = m_rc.right - m_rc.left,
        cyRgn = m_rc.bottom - m_rc.top;

    int iSize;
    int cx0 = m_pBK->m_cxAlbum, cy0 = m_pBK->m_cyAlbum;
    D2D1_RECT_F rcF;
    
    // 还得是祖传代码好用
    if ((float)m_cx / (float)m_cy > (float)cx0 / (float)cy0)// y对齐
    {
        iSize = cx0 * m_cy / cy0;
        rcF.left = m_rcF.left + (m_cx - iSize) / 2;
        rcF.right = rcF.left + iSize;
        rcF.top = m_rcF.top;
        rcF.bottom = rcF.top + m_cy;
    }
    else// x对齐
    {
        iSize = m_cx * cy0 / cx0;
        rcF.left = m_rcF.left;
        rcF.right = rcF.left + (FLOAT)m_cx;
        rcF.top = m_rcF.top + (m_cy - iSize) / 2;
        rcF.bottom = rcF.top + (FLOAT)iSize;
    }
    ////////////画封面图
    pDC->DrawBitmap(m_pBK->m_pBmpAlbum, &rcF);

    BkDbg_DrawElemFrame();
}


CUIAlbumRotating::CUIAlbumRotating()
{
    m_uType = UIET_ALBUMROTATING;
    m_uFlags = UIEF_WANTTIMEREVENT | UIEF_NOEVENT;
}

CUIAlbumRotating::~CUIAlbumRotating()
{
    m_pBrAlbum->Release();
    m_pBrUV->Release();
    m_pBrUV2->Release();
}

void CUIAlbumRotating::UpdateAlbumBrush()
{
    auto pBmp = m_pBK->m_pBmpAlbum;
    if (pBmp)
    {
        if (m_pBrAlbum)
            m_pBrAlbum->Release();

        float fRadius, cx, cy;
        const int iAlbumLevel = m_pBK->m_Ds.sizeAlbumLevel;
        cx = m_rcF.right - m_rcF.left - iAlbumLevel * 2;
        cy = m_rcF.bottom - m_rcF.top - iAlbumLevel * 2;
        fRadius = std::min(cx / 2.f, cy / 2.f);
        float xStart = m_rcF.left + iAlbumLevel + cx / 2.f - fRadius, yStart = m_rcF.top + iAlbumLevel + cy / 2.f - fRadius;
        float fSize = fRadius * 2;
        float fScaleFactor;
        const int cxImg = m_pBK->m_cxAlbum, cyImg = m_pBK->m_cyAlbum;
        if (cxImg > cyImg)// 宽度较大
        {
            fScaleFactor = fSize / cyImg;
            xStart -= ((cxImg - cyImg) / 2.f * fSize / cyImg);
        }
        else// 高度较大
        {
            fScaleFactor = fSize / cxImg;
            yStart -= ((cyImg - cxImg) / 2.f * fSize / cxImg);
        }

		const D2D1_BRUSH_PROPERTIES BrushProp
		{
			1.f,
			D2D1::Matrix3x2F::Translation(xStart, yStart) *
				D2D1::Matrix3x2F::Scale(fScaleFactor, fScaleFactor, D2D1::Point2F(xStart, yStart))
		};
		m_pBK->m_pDC->CreateBitmapBrush(pBmp, NULL, &BrushProp, &m_pBrAlbum);
	}
}

void CUIAlbumRotating::Redraw()
{
    auto pDC = m_pBK->m_pDC;
    pDC->PushAxisAlignedClip(&m_rcF, D2D1_ANTIALIAS_MODE_ALIASED);
    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);// 刷背景

    DWORD dwLevel = -1;
    const int iAlbumLevel = m_pBK->m_Ds.sizeAlbumLevel;
    if (App->GetPlayer().IsFileActive())
    {
        dwLevel = App->GetPlayer().GetBass().GetLevel();
        /////////////////////////////画封面边缘
        float fRadius;
        fRadius = std::min(m_cx / 2.f, m_cy / 2.f);
        D2D1_ELLIPSE Ellipse
        {
            { m_rcF.left + m_cx / 2.f,m_rcF.top + m_cy / 2.f },
            fRadius,
            fRadius
        };

        pDC->FillEllipse(&Ellipse, m_pBrUV);// 画外圈
        float fOffset = 0.f;
        fRadius -= iAlbumLevel;
        if (dwLevel != -1)
        {
            fOffset = ((float)(LOWORD(dwLevel) + HIWORD(dwLevel)) / 2.f) / 32768.f * iAlbumLevel;
            fRadius += fOffset;
            Ellipse.radiusX = Ellipse.radiusY = fRadius;
            pDC->FillEllipse(&Ellipse, m_pBrUV2);// 画电平指示
        }
        /////////////////////////////画封面
        pDC->SetTransform(D2D1::Matrix3x2F::Rotation(m_fAngle, Ellipse.point));// 置旋转变换

        fRadius = fRadius - fOffset;
        Ellipse.radiusX = Ellipse.radiusY = fRadius;
        pDC->FillEllipse(&Ellipse, m_pBrAlbum);

        pDC->SetTransform(D2D1::Matrix3x2F::Identity());// 还原空变换
    }
    else
    {
        D2D1_ELLIPSE D2DEllipse;
        D2DEllipse.point = { m_rcF.left + m_cx / 2.f,m_rcF.top + m_cy / 2.f };
        float fRadius;
        fRadius = std::min(m_cx / 2.f, m_cy / 2.f);
        D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;

        pDC->FillEllipse(&D2DEllipse, m_pBrUV);// 画外圈

        fRadius -= iAlbumLevel;
        D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;
        pDC->FillEllipse(&D2DEllipse, m_pBrUV2);
    }

    pDC->PopAxisAlignedClip();
    BkDbg_DrawElemFrame();
}

void CUIAlbumRotating::OnTimer(UINT uTimerID)
{
    if (uTimerID == CWndBK::IDT_PGS)
    {
        m_fAngle += 0.5f;
        if (m_fAngle >= 360.f)
            m_fAngle = 0;
        Redraw();
        m_pBK->m_vDirtyRect.emplace_back(m_rcInWnd);
    }
}

LRESULT CUIAlbumRotating::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    auto lResult = DefElemEventProc(uEvent, wParam, lParam);
    switch (uEvent)
    {
    case UIEE_ONPLAYINGCTRL:
    {
        if (wParam == PCT_PLAY)
        {
            m_fAngle = 0.f;
            UpdateAlbumBrush();
            Redraw(TRUE);
        }
    }
    break;
    }
    return lResult;
}

BOOL CUIAlbumRotating::InitElem()
{
    SAFE_RELEASE(m_pBrUV);
    SAFE_RELEASE(m_pBrUV2);
    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBrUV2);
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.6f), &m_pBrUV);
    return TRUE;
}


void CUIWaves::GetWavesData()
{
    std::thread Thread([this]()
        {
            CBass Bass;
            Bass.Open(App->GetPlayer().GetCurrFileName().Data(), BASS_STREAM_DECODE, BASS_MUSIC_DECODE | BASS_MUSIC_PRESCAN);
            if (!Bass.GetHStream())
            {
                EckDbgPrint(CBass::GetErrorMsg(CBass::GetError()));
                m_ThreadState = ThreadState::Error;
                return;
            }
            ULONGLONG ullLength = (ULONGLONG)(Bass.GetLength() * 1000.);

            size_t cBars = ullLength / 20ull;
            m_vWavesData.resize(cBars);

            EckCounter(cBars, i)
            {
                m_vWavesData[i] = Bass.GetLevel();
                if (WaitForSingleObject(m_hEvent, 0) == WAIT_OBJECT_0)
                {
                    m_vWavesData.clear();
                    m_ThreadState = ThreadState::Stopped;
                    return;
                }
            }

            m_ThreadState = ThreadState::Stopped;
        });
    DuplicateHandle(GetCurrentProcess(), Thread.native_handle(), GetCurrentProcess(), &m_hThread, GENERIC_ALL, FALSE, 0);
    Thread.detach();
}

CUIWaves::CUIWaves()
{
    m_uType = UIET_WAVES;
    m_uFlags = UIEF_NOEVENT | UIEF_WANTTIMEREVENT | UIEF_ONLYPAINTONTIMER;
    m_hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
}

BOOL CUIWaves::InitElem()
{
    SAFE_RELEASE(m_pBrLine);
    SAFE_RELEASE(m_pBrCenterMark);
    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrLine);
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pBrCenterMark);
    App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		m_pBK->Dpi(18.f), L"zh-cn", &m_pTfTip);
    m_pTfTip->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_pTfTip->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);// 水平垂直都居中
    return TRUE;
}

LRESULT CUIWaves::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
	auto lResult = DefElemEventProc(uEvent, wParam, lParam);
	switch (uEvent)
	{
	case UIEE_ONPLAYINGCTRL:
	{
		switch (wParam)
		{
		case PCT_PLAY:
			m_ThreadState = ThreadState::Running;
			CloseHandle(m_hThread);
            ResetEvent(m_hEvent);
			GetWavesData();
			break;
		case PCT_STOP:
			if (m_hThread)
			{
				SetEvent(m_hEvent);
				WaitForSingleObject(m_hThread, INFINITE);
				CloseHandle(m_hThread);
				m_hThread = NULL;
			}
			break;
		}
	}
	break;
	}
	return lResult;
}

CUIWaves::~CUIWaves()
{
    m_pBrLine->Release();
    m_pBrCenterMark->Release();
    m_pTfTip->Release();
    CloseHandle(m_hThread);
    CloseHandle(m_hEvent);
}

void CUIWaves::Redraw()
{
    auto pDC = m_pBK->m_pDC;
    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);// 刷背景

    PCWSTR pszText = NULL;

	if (m_hThread && WaitForSingleObject(m_hThread, 0) != WAIT_OBJECT_0)// 正在加载
        pszText = L"正在加载...";
    else if (!App->GetPlayer().GetBass().GetHStream())// 已停止
        pszText = L"未播放";
    else if (m_ThreadState == ThreadState::Error)// 出错
        pszText = L"错误！";

    if (pszText)// 应当显示提示
    {
        pDC->DrawTextW(pszText, (int)wcslen(pszText), m_pTfTip, &m_rcF, m_pBrLine);
		BkDbg_DrawElemFrame();
		return;
	}

    const int idxCurr = (int)(App->GetPlayer().GetPos() / 20ull);// 算数组索引    20ms一单位
    const int cBars = (int)m_vWavesData.size();
	if (idxCurr > cBars - 1)
		return;

	int i = idxCurr;
	int x = m_rc.left + m_cxHalf,
		y = m_rc.top + m_cyHalf;
	D2D1_POINT_2F PtF1, PtF2;

	pDC->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	pDC->PushAxisAlignedClip(&m_rcF, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	// 上面是右声道，下面是左声道
	while (TRUE)// 向右画
	{
		PtF1 = { (float)x, (float)(y - HIWORD(m_vWavesData[i]) * m_cyHalf / 32768) };
		PtF2 = { (float)x, (float)(y + LOWORD(m_vWavesData[i]) * m_cyHalf / 32768) };
		pDC->DrawLine(PtF1, PtF2, m_pBrLine, (FLOAT)m_cxLine);
		x += m_cxLine;
		i++;
		if (i > cBars - 1 || x >= m_rc.right)
			break;
	}
	i = idxCurr;
	x = m_rc.left + m_cxHalf;
	while (TRUE)// 向左画
	{
		PtF1 = { (FLOAT)x, (FLOAT)(y - HIWORD(m_vWavesData[i]) * m_cyHalf / 32768) };
		PtF2 = { (FLOAT)x, (FLOAT)(y + LOWORD(m_vWavesData[i]) * m_cyHalf / 32768) };
		pDC->DrawLine(PtF1, PtF2, m_pBrLine, (FLOAT)m_cxLine);
		x -= m_cxLine;
		i--;
		if (i < 0 || x < m_rc.left)
			break;
	}
	x = m_rc.left + m_cxHalf;

	PtF1 = { (FLOAT)x, m_rcF.top };
	PtF2 = { (FLOAT)x, m_rcF.bottom };
    pDC->DrawLine(PtF1, PtF2, m_pBrCenterMark, (FLOAT)m_pBK->m_Ds.cxWaveLine);

	pDC->PopAxisAlignedClip();
	pDC->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	BkDbg_DrawElemFrame();
}


BOOL CUISpe::InitElem()
{
    SAFE_RELEASE(m_pBrBar);
    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrBar);
    return TRUE;
}

void CUISpe::Redraw()
{
    auto pDC = m_pBK->m_pDC;

    float fData[128];
	if (App->GetPlayer().GetBass().GetData(fData, BASS_DATA_FFT256) == -1)
    {
        ZeroMemory(m_piOldHeight, m_cbPerUnit);
        ZeroMemory(m_piHeight, m_cbPerUnit);
        ZeroMemory(fData, sizeof(fData));
    }

    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);// 刷背景
    for (int i = 0; i < m_iCount; i++)
    {
        ++m_piTime[i];
		m_piHeight[i] = abs((int)(fData[i] * m_cy * 2.f));
        //////////////////频谱条
        if (m_piHeight[i] > m_cy)// 超高
            m_piHeight[i] = m_cy;// 回来
#define SPESTEP_BAR					7
#define SPESTEP_MAX					11
        if (m_piHeight[i] > m_piOldHeight[i])// 当前的大于先前的
            m_piOldHeight[i] = m_piHeight[i];// 顶上去
        else
            m_piOldHeight[i] -= SPESTEP_BAR;// 如果不大于就继续落

        if (m_piOldHeight[i] < 3)// 太低了
            m_piOldHeight[i] = 3;// 回来

        //////////////////峰值
        if (m_piTime[i] > 10)// 时间已到
            m_piOldMaxPos[i] -= SPESTEP_MAX;// 下落

        if (m_piOldHeight[i] > m_piOldMaxPos[i])// 频谱条大于峰值
        {
            m_piOldMaxPos[i] = m_piOldHeight[i];// 峰值顶上去，重置时间
            m_piTime[i] = 0;
        }

        if (m_piOldMaxPos[i] < 3)// 太低了
            m_piOldMaxPos[i] = 3;// 回来
        //////////////////绘制
        //////////频谱条
        D2D_RECT_F D2DRectF;
        D2DRectF.left = (FLOAT)(m_rcF.left + (m_cxBar + m_cxGap) * i);
        D2DRectF.top = (FLOAT)(m_rcF.top + m_cy - m_piOldHeight[i]);
        D2DRectF.right = D2DRectF.left + m_cxBar;
        D2DRectF.bottom = (FLOAT)(m_rcF.top + m_cy);
        if (D2DRectF.right > m_rcF.right)
            break;
        pDC->FillRectangle(&D2DRectF, m_pBrBar);

        //////////峰值指示
        D2DRectF.top = (FLOAT)(m_rcF.top + m_cy - m_piOldMaxPos[i]);
        D2DRectF.bottom = D2DRectF.top + 3;
        pDC->FillRectangle(&D2DRectF, m_pBrBar);
    }

    BkDbg_DrawElemFrame();
}

void CUISpe::SetCount(int i)
{
    if (i > 0)
    {
        // c * cxBar + (c - 1) * cxDiv = cx
        // 解得：cxBar = (cx - (c - 1) * cxDiv) / c
        m_iCount = i;
        m_cxBar = (m_cx - (i - 1) * m_cxGap) / i;
        if (m_cxBar <= 0)
            goto Fail;
        m_cbPerUnit = sizeof(int) * i;
        m_vBuf.resize(i * 4);
        m_piOldHeight = m_vBuf.data();
        m_piHeight = m_vBuf.data() + i;
        m_piOldMaxPos = m_vBuf.data() + i * 2;
        m_piTime = m_vBuf.data() + i * 3;
        return;
    }
Fail:
    m_iCount = 0;
    m_cxBar = 0.f;
    m_cbPerUnit = 0;
    m_vBuf.clear();
    m_piOldHeight = m_piHeight = m_piOldMaxPos = m_piTime = NULL;
}

CUISpe::CUISpe()
{
    m_uType = UIET_SPE;
    m_uFlags = UIEF_NOEVENT | UIEF_WANTTIMEREVENT | UIEF_ONLYPAINTONTIMER;
}

CUISpe::~CUISpe()
{
    SAFE_RELEASE(m_pBrBar);
}

LRESULT CUISpe::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    switch (uEvent)
    {
    case UIEE_SETRECT:
    {
        auto lResult = DefElemEventProc(uEvent, wParam, lParam);
        if (m_iCount)
            m_cxBar = (m_cx - (m_iCount - 1) * m_cxGap) / m_iCount;
        return lResult;
    }
    break;
    }

    return DefElemEventProc(uEvent, wParam, lParam);
}


BOOL CUISpe2::InitElem()
{
    SAFE_RELEASE(m_pBrLine);
    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrLine);
    return TRUE;
}

CUISpe2::CUISpe2()
{
    m_uType = UIET_SPE2;
    m_uFlags = UIEF_NOEVENT | UIEF_WANTTIMEREVENT | UIEF_ONLYPAINTONTIMER;
}

CUISpe2::~CUISpe2()
{
    SAFE_RELEASE(m_pBrLine);
}

void CUISpe2::Redraw()
{
    auto pDC = m_pBK->m_pDC;
    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);// 刷背景

	App->GetPlayer().GetBass().GetData(m_pfBuf, (DWORD)m_cbBuf);// 取频谱数据

    D2D1_POINT_2F PtF1, PtF2;
    int k, l;
    for (int j = 0; j < 2; ++j)
    {
        for (int i = 0; i < m_cSample - 1; ++i)
        {
            k = (int)((1 - m_pfBuf[i * 2 + j]) * m_cy / 2);// 直接从Bass示例里狠狠地抄
            if (k < 0)
                k = 0;
            else if (k > m_cy)
                k = m_cy;
            PtF1 = { m_rcF.left + i * m_cxStep,m_rcF.top + k };

            l = (int)((1 - m_pfBuf[(i + 1) * 2 + j]) * m_cy / 2);
            if (l < 0)
                l = 0;
            else if (l > m_cy)
                l = m_cy;
            PtF2 = { m_rcF.left + (i + 1) * m_cxStep,m_rcF.top + l };
            pDC->DrawLine(PtF1, PtF2, m_pBrLine, (float)m_pBK->m_Ds.cxSepLine);
        }
    }

    BkDbg_DrawElemFrame();
}

void CUISpe2::SetSampleCount(int i)
{
    delete[] m_pfBuf;
    if (i > 1)
    {
        m_cSample = i;
        m_cbBuf = 2 * i * sizeof(float);
        m_pfBuf = new float[m_cbBuf];
        m_cxStep = (float)m_cx / (float)(i - 1);
    }
    else
    {
        m_cSample = 0;
        m_cbBuf = 0;
        m_pfBuf = NULL;
        m_cxStep = 0.f;
        EckDbgPrint(L"样本数不正确");
        EckDbgBreak();
    }
}

void CUISpe2::OnTimer(UINT uTimerID)
{
    if (uTimerID == CWndBK::IDT_PGS)
    {
        Redraw();
    }
}

LRESULT CUISpe2::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    auto lResult = DefElemEventProc(uEvent, wParam, lParam);
    switch (uEvent)
    {
    case UIEE_SETRECT:
    {
        if (m_cSample > 1)
            m_cxStep = (float)m_cx / (float)(m_cSample - 1);
    }
    break;
    }
    return lResult;
}


CUIProgBar::CUIProgBar()
{
    m_uType = UIET_PROGBAR;
    m_uFlags = UIEF_WANTTIMEREVENT;
}

CUIProgBar::~CUIProgBar()
{
    SAFE_RELEASE(m_pBrNormal);
    SAFE_RELEASE(m_pBrBK);
    SAFE_RELEASE(m_pBrTempMark);
}

void CUIProgBar::Redraw()
{
    auto pDC = m_pBK->m_pDC;

    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);
    D2D_RECT_F rcF;
    ID2D1SolidColorBrush* pBrush;

    ULONGLONG ull;
    if (m_bShowTempMark)
    {
        ull = m_ullTempPos;
        pBrush = m_pBrTempMark;
    }
    else
    {
        ull = m_ullPos;
        pBrush = m_pBrNormal;
    }

    rcF.left = m_rcF.left;
    rcF.top = m_rcF.top + (FLOAT)((m_cy - m_pBK->m_Ds.cyProgBarTrack) / 2);
    rcF.bottom = rcF.top + (FLOAT)m_pBK->m_Ds.cyProgBarTrack;
    if (m_ullMax)
    {
        rcF.right = rcF.left + ull * m_cx / m_ullMax;
        pDC->FillRectangle(&rcF, pBrush);

        rcF.left = rcF.right;
        rcF.right = m_rcF.right;
        pDC->FillRectangle(&rcF, m_pBrBK);
    }
    else
    {
        rcF.right = m_rcF.right;
        pDC->FillRectangle(&rcF, m_pBrBK);
    }
    BkDbg_DrawElemFrame();
}

BOOL CUIProgBar::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        POINT pt = GET_PT_LPARAM(lParam);
        if (PtInRect(&m_rc, pt))
        {
            m_bLBtnDown = TRUE;
            SetCapture(m_pBK->m_hWnd);
            EnableTempPosMark(TRUE);
            ULONGLONG ull;
            if (HitTest(pt, &ull))
            {
                SetTempPosMark(ull);
                Redraw(TRUE);
            }
        }
    }
    return 0;

    case WM_LBUTTONUP:
    {
        if (m_bLBtnDown)
        {
            POINT pt = GET_PT_LPARAM(lParam);
            m_bLBtnDown = FALSE;
            ReleaseCapture();
            EnableTempPosMark(FALSE);

            ULONGLONG ull;
            if (HitTest(pt, &ull))
            {
                m_ullPos = ull;
                App->GetPlayer().GetBass().SetPosition(ull / 1000.);
                Redraw(TRUE);
            }
        }
    }
    return 0;

    case WM_MOUSEMOVE:
    {
        if (m_bShowTempMark)
        {
            POINT pt = GET_PT_LPARAM(lParam);
            ULONGLONG ull;
            if (HitTest(pt, &ull))
            {
                SetTempPosMark(ull);
                Redraw(TRUE);
            }
        }
    }
    return 0;

    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)// Esc撤销进度调节
        {
            if (m_bLBtnDown)
            {
                m_bLBtnDown = FALSE;
                ReleaseCapture();
                EnableTempPosMark(FALSE);
                Redraw(TRUE);
            }
        }
    }
    return 0;
    }

    return 0;
}

BOOL CUIProgBar::InitElem()
{
    SAFE_RELEASE(m_pBrNormal);
    SAFE_RELEASE(m_pBrBK);
    SAFE_RELEASE(m_pBrTempMark);
    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrNormal);
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &m_pBrBK);
    pDC->CreateSolidColorBrush(c_D2DCrUIProgBarTempMark, &m_pBrTempMark);
    return TRUE;
}

int CUIProgBar::HitTest(POINT pt, ULONGLONG* pullPos)
{
    *pullPos = 0ull;
    if (m_cx <= 0)
        return FALSE;

    int iOffset = pt.x - m_rc.left;
    if (iOffset < 0)
        iOffset = 0;
    else if (iOffset > m_cx)
        iOffset = m_cx;

    *pullPos = m_ullMax * (ULONGLONG)iOffset / (ULONGLONG)m_cx;
    return TRUE;
}

void CUIProgBar::OnTimer(UINT uTimerID)
{
    if (uTimerID == CWndBK::IDT_PGS)
    {
        m_ullPos = App->GetPlayer().GetPos();
        Redraw();
        m_pBK->m_vDirtyRect.emplace_back(m_rcInWnd);
    }
}

LRESULT CUIProgBar::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    auto lResult = DefElemEventProc(uEvent, wParam, lParam);
    switch (uEvent)
    {
    case UIEE_ONPLAYINGCTRL:
    {
        SetMax(App->GetPlayer().GetLength());
        SetPos(0ull);
    }
    break;
    }
    return lResult;
}


CUIToolBar::CUIToolBar()
{
    m_uType = UIET_TOOLBAR;
    m_uFlags = UIEF_WANTTIMEREVENT;
}

CUIToolBar::~CUIToolBar()
{
    DestroyWindow(m_hToolTip);

    SAFE_RELEASE(m_pBrText);
    SAFE_RELEASE(m_pBrBtnHot);
    SAFE_RELEASE(m_pBrBtnPushed);
    SAFE_RELEASE(m_pBrBtnChecked);
}

BOOL CUIToolBar::InitElem()
{
    SAFE_RELEASE(m_pBrText);
    SAFE_RELEASE(m_pBrBtnHot);
    SAFE_RELEASE(m_pBrBtnPushed);
    SAFE_RELEASE(m_pBrBtnChecked);
    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBrText);
    pDC->CreateSolidColorBrush(D2D1::ColorF(eck::ReverseColorref(MYCLR_BTHOT)), &m_pBrBtnHot);
    pDC->CreateSolidColorBrush(D2D1::ColorF(eck::ReverseColorref(MYCLR_BTPUSHED)), &m_pBrBtnPushed);
    pDC->CreateSolidColorBrush(D2D1::ColorF(eck::ReverseColorref(MYCLR_BTCHECKED)), &m_pBrBtnChecked);
    App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        9, L"zh-cn", &m_pTfTime);
    m_pTfTime->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    m_pTfTime->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    HWND hWndBK = m_pBK->m_hWnd;
    m_hToolTip = CreateWindowExW(0, TOOLTIPS_CLASSW, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP,
        0, 0, 0, 0, hWndBK, NULL, NULL, NULL);// 创建工具提示

    m_ti = { sizeof(TTTOOLINFOW),TTF_TRACK | TTF_IDISHWND | TTF_ABSOLUTE,hWndBK,(UINT_PTR)hWndBK,{0},App->GetHInstance(),NULL,0 };
    SendMessageW(m_hToolTip, TTM_ADDTOOLW, 0, (LPARAM)&m_ti);

    m_rcFTimeLabel.left = m_rcF.left;
    m_rcFTimeLabel.top = m_rcF.top;
    m_rcFTimeLabel.right = m_rcFTimeLabel.left + (float)m_pBK->m_Ds.cxTime;
    m_rcFTimeLabel.bottom = m_rcF.bottom;
    m_rcTimeLabel = eck::MakeRect(m_rcFTimeLabel);

    m_cchTime = lstrlenW(m_szTime);
    return TRUE;
}

void CUIToolBar::Redraw()
{
    auto pDC = m_pBK->m_pDC;
//    auto pGdiInteropRT = m_BK.m_pGdiInteropRT;
//    HDC hDC;
//
//    pDC->DrawBitmap(m_BK.m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);
//
//    g_pDWTFNormal->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
//    g_pDWTFNormal->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
//    pDC->DrawTextW(m_szTime, m_cchTime, g_pDWTFNormal, &m_rcFTimeLabel, m_pBrText, D2D1_DRAW_TEXT_OPTIONS_CLIP);
//
//    int iIconOffest = (GC.cyBT - GC.iIconSize) / 2;
//    int x = (int)m_rcFTimeLabel.right, y = m_rc.top + iIconOffest;
//    RECT rc = { 0,0,0,GC.cyBT };
//    D2D_RECT_F rcF;
//    UITOOLBARBTNINDEX i = TBBI_INVALID;
//    ID2D1SolidColorBrush* pBrush;
//    if (m_idxHot != -1 || m_idxPush != -1)
//    {
//        if (m_idxPush != -1)
//        {
//            i = m_idxPush;
//            pBrush = m_pBrBtnPushed;
//        }
//        else
//        {
//            i = m_idxHot;
//            pBrush = m_pBrBtnHot;
//            if (m_uBtnsCheckState & (1 << i))
//                goto SkipDrawHot;
//        }
//
//        rcF.left = x + (FLOAT)(GC.cyBT * i);
//        rcF.top = m_rcF.top;
//        rcF.right = rcF.left + GC.cyBT;
//        rcF.bottom = m_rcF.bottom;
//        pDC->FillRectangle(&rcF, pBrush);
//    }
//
//SkipDrawHot:
//    if (i != m_idxPush)
//        for (int i = 0; i < TBBI_COUNT; ++i)
//        {
//            if (m_uBtnsCheckState & (1 << i))
//            {
//                rcF.left = x + (FLOAT)(GC.cyBT * i);
//                rcF.top = m_rcF.top;
//                rcF.right = rcF.left + GC.cyBT;
//                rcF.bottom = m_rcF.bottom;
//                pDC->FillRectangle(&rcF, m_pBrBtnChecked);
//            }
//        }
//
//    pGdiInteropRT->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
//
//    HICON hi[] =
//    {
//        GR.hiLast,
//        (g_bPlayIcon ? GR.hiPlay : GR.hiPause),
//        GR.hiStop,
//        GR.hiNext,
//        GR.hiLrc,
//        NULL,
//        GR.hiPlaySetting,
//        GR.hiPlayList,
//        GR.hiSettings,
//        GR.hiInfo
//    };
//
//    switch (m_iRepeatMode)
//    {
//    case REPEATMODE_TOTALLOOP:  hi[TBBI_REPEATMODE] = GR.hiArrowCircle;      break;
//    case REPEATMODE_RADOM:      hi[TBBI_REPEATMODE] = GR.hiArrowCross;       break;
//    case REPEATMODE_SINGLE:     hi[TBBI_REPEATMODE] = GR.hiArrowRight;       break;
//    case REPEATMODE_SINGLELOOP: hi[TBBI_REPEATMODE] = GR.hiArrowCircleOne;   break;
//    case REPEATMODE_TOTAL:      hi[TBBI_REPEATMODE] = GR.hiArrowRightThree;  break;
//    default:                    hi[TBBI_REPEATMODE] = GR.hiArrowCircle;      break;
//    }
//
//    for (HICON hIcon : hi)
//    {
//        DrawIconEx(hDC, x + iIconOffest, y, hIcon, 0, 0, 0, NULL, DI_NORMAL);// 6 循环方式
//        x += GC.cyBT;
//    }
//
//    pGdiInteropRT->ReleaseDC(&m_rcInWnd);
    BkDbg_DrawElemFrame();
}

void CUIToolBar::RedrawTimeInfo()
{
    auto pDC = m_pBK->m_pDC;

    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcFTimeLabel, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcFTimeLabel);

    pDC->DrawTextW(m_szTime, m_cchTime, m_pTfTime, &m_rcFTimeLabel, m_pBrText, D2D1_DRAW_TEXT_OPTIONS_CLIP);

    BkDbg_DrawElemFrame();
}

BOOL CUIToolBar::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        POINT pt = GET_PT_LPARAM(lParam);
        m_idxPush = HitTest(pt);
        if (m_idxPush >= 0)
        {
            m_bLBtnDown = TRUE;
            SetCapture(m_pBK->m_hWnd);
            Redraw(TRUE);
        }
    }
    return 0;

    case WM_LBUTTONUP:
    {
        POINT pt = GET_PT_LPARAM(lParam);
        if (m_bLBtnDown)
        {
            m_bLBtnDown = FALSE;
            ReleaseCapture();
        }

        if (PtInRect(&m_rc, pt) || m_idxPush != -1)
        {
            if (m_idxPush == TBBI_INVALID)
                return 0;
            UITOOLBARBTNINDEX i = m_idxPush;
            m_idxPush = TBBI_INVALID;
            m_idxLastHot = TBBI_INVALID;
            m_bLBtnDown = FALSE;

            Redraw(TRUE);

            if (i != HitTest(pt))
                return 0;

            m_ti.lpszText = NULL;
            SendMessageW(m_hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&m_ti);
            SendMessageW(m_hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&m_ti);
            DoCmd(i);
        }
    }
    return 0;

    case WM_MOUSEMOVE:
    {
        POINT pt = GET_PT_LPARAM(lParam);
        if (PtInRect(&m_rc, pt))
        {
            m_bInToolBar = TRUE;
            if (!m_bLBtnDown)
            {
                m_idxHot = HitTest(pt);
                if (m_idxLastHot != m_idxHot)
                {
                    m_ti.lpszText = NULL;
                    SendMessageW(m_hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&m_ti);
                    SendMessageW(m_hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&m_ti);
                    m_idxLastHot = m_idxHot;
                    Redraw(TRUE);
                }
            }
        }
        else if (m_bInToolBar)
        {
            m_bInToolBar = FALSE;
            m_idxLastHot = m_idxHot = TBBI_INVALID;
            Redraw(TRUE);
            SendMessageW(m_hToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
        }
    }
    return 0;

    case WM_MOUSELEAVE:
    {
        if (m_idxHot != TBBI_INVALID)
        {
            m_ti.lpszText = NULL;
            SendMessageW(m_hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&m_ti);
            SendMessageW(m_hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&m_ti);
            m_idxLastHover = m_idxLastHot = m_idxHot = TBBI_INVALID;
            Redraw(TRUE);
            SendMessageW(m_hToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
        }
    }
    return 0;

    case WM_MOUSEHOVER:
    {
        if (m_idxHot != -1 && m_idxPush == -1 && m_idxLastHover != m_idxHot)
        {
            POINT pt = GET_PT_LPARAM(lParam);
            ClientToScreen(m_pBK->m_hWnd, &pt);
            m_idxLastHover = m_idxHot;
            m_ti.lpszText = NULL;

            if (m_idxHot == 5)
                m_ti.lpszText = (LPWSTR)c_szBtmTip[BTMBKBTNCOUNT + (int)COptionsMgr::GetInst().iRepeatMode];
            else
                m_ti.lpszText = (LPWSTR)c_szBtmTip[m_idxHot];

            SendMessageW(m_hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&m_ti);
            SendMessageW(m_hToolTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ti);
            SendMessageW(m_hToolTip, TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));
        }
    }
    return 0;
    }
    return 0;
}

UITOOLBARBTNINDEX CUIToolBar::HitTest(POINT pt)
{
    if (!PtInRect(&m_rc, pt))
        return TBBI_INVALID;

    int x = m_rc.left + m_pBK->m_Ds.cxTime;
    for (int i = 0; i < BTMBKBTNCOUNT; i++)
    {
        if (pt.x > x + i * m_cy && pt.x < x + (i + 1) * m_cy)
            return (UITOOLBARBTNINDEX)i;
    }

    return TBBI_INVALID;
}

void CUIToolBar::DoCmd(UITOOLBARBTNINDEX i)
{
    //switch (i)
    //{
    //case TBBI_PREV:// 上一曲
    //{
    //    if (g_iCurrFileIndex == -1)
    //        return;
    //    Playing_PlayNext(TRUE);
    //}
    //return;

    //case TBBI_PLAY:// 播放/暂停
    //{
    //    if (g_iCurrFileIndex == -1)
    //        if (g_ItemData->iCount > 0)
    //            Playing_PlayFile(0);
    //        else
    //            return;
    //    Playing_PlayOrPause();
    //}
    //return;

    //case TBBI_STOP:// 停止
    //{
    //    if (g_iCurrFileIndex == -1)
    //        return;
    //    Playing_Stop();
    //}
    //return;

    //case TBBI_NEXT:// 下一曲
    //{
    //    if (g_iCurrFileIndex == -1)
    //        return;
    //    Playing_PlayNext();
    //}
    //return;

    //case TBBI_LRC:// 歌词
    //{
    //    LrcWnd_Show();
    //    if (IsWindow(g_hLrcWnd))
    //        m_uBtnsCheckState |= (1 << TBBI_LRC);
    //    else
    //        m_uBtnsCheckState &= ~(1 << TBBI_LRC);
    //}
    //return;

    //case TBBI_REPEATMODE:// 循环方式
    //{
    //    m_iRepeatMode++;
    //    if (m_iRepeatMode > 4)
    //        m_iRepeatMode %= 5;
    //    Redraw(TRUE);
    //}
    //return;

    //case TBBI_PLAYINGOPT:// 播放设置
    //{
    //    static HWND hDlgEffect = NULL;
    //    if (IsWindow(hDlgEffect) && GetWindowThreadProcessId(hDlgEffect, NULL) == GetCurrentThreadId())
    //    {
    //        SetForegroundWindow(hDlgEffect);
    //    }
    //    else
    //    {
    //        hDlgEffect = CreateDialogParamW(g_hInst, MAKEINTRESOURCEW(IDD_EFFECT), m_pBK->m_hWnd, DlgProc_Effect, 0);
    //        ShowWindow(hDlgEffect, SW_SHOW);
    //    }
    //}
    //return;

    //case TBBI_PLAYLIST:// 播放列表
    //{
    //    HMENU hMenu = CreatePopupMenu();
    //    AppendMenuW(hMenu, g_bListSeped ? MF_CHECKED : 0, IDMI_PL_SEPARATE, L"将列表从主窗口拆离");
    //    AppendMenuW(hMenu, g_bListHidden ? 0 : MF_CHECKED, IDMI_PL_SHOW, L"显示播放列表");

    //    int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD,
    //        m_rc.left + (int)m_rcFTimeLabel.right + GC.cyBT * TBBI_PLAYLIST, m_rc.top + (int)m_rcFTimeLabel.bottom, 0, m_BK.m_hWnd, NULL);
    //    DestroyMenu(hMenu);
    //    switch (iRet)
    //    {
    //    case IDMI_PL_SEPARATE:
    //        UI_SeparateListWnd(!g_bListSeped);
    //        break;

    //    case IDMI_PL_SHOW:
    //        UI_ShowList(g_bListHidden);
    //        break;
    //    }
    //}
    //return;

    //case TBBI_OPTIONS:// 设置
    //    DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_OPTIONS), g_hMainWnd, DlgProc_Options, 0);
    //    return;

    //case TBBI_ABOUT: // 关于
    //    DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_ABOUT), g_hMainWnd, DlgProc_About, 0);
    //    return;
    //}
}

void CUIToolBar::OnTimer(UINT uTimerID)
{
    if (uTimerID == CWndBK::IDT_PGS)
    {
        const int iMin = (int)App->GetPlayer().GetPosF() / 60,
            iMin2 = (int)(App->GetPlayer().GetLength() / 1000 / 60);

        m_cchTime = swprintf_s(m_szTime, L"%02d:%02d/%02d:%02d",
            iMin,
            (int)App->GetPlayer().GetPosF() - iMin * 60,
            iMin2,
            (int)(App->GetPlayer().GetLength() / 1000) - iMin2 * 60);
        RedrawTimeInfo();
    }
}

LRESULT CUIToolBar::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    switch (uEvent)
    {
    case UIEE_SETRECT:
    {
        LRESULT lResult = DefElemEventProc(uEvent, wParam, lParam);
        m_rcFTimeLabel.left = m_rcF.left;
        m_rcFTimeLabel.top = m_rcF.top;
        m_rcFTimeLabel.right = m_rcFTimeLabel.left + (float)m_pBK->m_Ds.cxTime;
        m_rcFTimeLabel.bottom = m_rcF.bottom;

        m_rcTimeLabel = eck::MakeRect(m_rcFTimeLabel);
        m_pBK->m_vDirtyRect.push_back(m_rcTimeLabel);
        return lResult;
    }
    }

    return DefElemEventProc(uEvent, wParam, lParam);
}


void CUIInfo::UpdateTextFormat()
{
    SAFE_RELEASE(m_pTfTitle);
    SAFE_RELEASE(m_pTfTip);
    App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		m_pBK->Dpi(18.f), L"zh-cn", &m_pTfTitle);
    App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        m_pBK->Dpi(12.f), L"zh-cn", &m_pTfTip);

    DWRITE_TRIMMING DWTrimming
    {
        DWRITE_TRIMMING_GRANULARITY_CHARACTER,// 按字符裁剪
        0,
        0
    };
    IDWriteInlineObject* pDWInlineObj;// 省略号裁剪内联对象
    App->m_pDwFactory->CreateEllipsisTrimmingSign(m_pTfTitle, &pDWInlineObj);// 创建省略号裁剪
    if (!pDWInlineObj)
        return;
    ///////////置文本格式
    m_pTfTitle->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_pTfTitle->SetTrimming(&DWTrimming, pDWInlineObj);// 置溢出裁剪
    m_pTfTitle->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 不换行
    m_pTfTip->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_pTfTip->SetTrimming(&DWTrimming, pDWInlineObj);// 置溢出裁剪
    m_pTfTip->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 不换行
    pDWInlineObj->Release();
}

CUIInfo::CUIInfo()
{
    m_uType = UIET_INFO;
    m_uFlags = UIEF_STATIC;
}

void CUIInfo::Redraw()
{
	auto pDC = m_pBK->m_pDC;
	////////////////////画顶部提示信息
	PCWSTR psz;
	if (App->GetPlayer().IsFileActive())
	{
		const auto& Item = App->GetPlayer().GetList().At(App->GetPlayer().GetCurrFile());
		psz = Item.rsName.Data();
	}
	else
		psz = L"未播放";
    ///////////画大标题
    D2D1_RECT_F rcF{ m_rcF.left,m_rcF.top,m_rcF.right,m_rcF.top + (FLOAT)m_pBK->m_Ds.cyTopTitle };
    pDC->DrawTextW(psz, lstrlenW(psz), m_pTfTitle, &rcF, m_pBrBigTip);
    ///////////画其他信息
    constexpr PCWSTR pszTip[4]
    {
        L"标题：",
        L"艺术家：",
        L"专辑：",
        L"备注："
    };

    const auto& MusicInfo = App->GetPlayer().GetMusicInfo();
    const PCWSTR pszTip2[4]
    {
        MusicInfo.rsTitle.Data(),
        MusicInfo.rsArtist.Data(),
        MusicInfo.rsAlbum.Data(),
        MusicInfo.rsComment.Data()
    };
    const int cchTip2[]
    {
        MusicInfo.rsTitle.Size(),
        MusicInfo.rsArtist.Size(),
        MusicInfo.rsAlbum.Size(),
        MusicInfo.rsComment.Size()
    };

    const int cxTopTip = m_pBK->m_Ds.cxTopTip;
    const int cyTopTip = m_pBK->m_Ds.cyTopTip;
    rcF.right = rcF.left + cxTopTip;
    rcF.top += (FLOAT)(cyTopTip + m_pBK->m_Ds.sizeTopTipGap);
    rcF.bottom = rcF.top + cyTopTip;

    int cxElem = m_rc.right - m_rc.left;
    for (int i = 0; i < sizeof(pszTip) / sizeof(PCWSTR); ++i)
    {
        pDC->DrawTextW(pszTip[i], (UINT32)wcslen(pszTip[i]), m_pTfTip, &rcF, m_pBrSmallTip);
        if (pszTip2[i])
        {
            rcF.left += (FLOAT)cxTopTip;
            rcF.right = m_rcF.right;
            pDC->DrawTextW(pszTip2[i], cchTip2[i], m_pTfTip, &rcF, m_pBrSmallTip);
            rcF.left = m_rcF.left;
            rcF.right = rcF.left + cxTopTip;
        }
        rcF.top += cyTopTip;
        rcF.bottom = rcF.top + cyTopTip;
    }
    BkDbg_DrawElemFrame();
}

BOOL CUIInfo::InitElem()
{
    SAFE_RELEASE(m_pBrBigTip);
    SAFE_RELEASE(m_pBrSmallTip);
    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrBigTip);
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBrSmallTip);
    UpdateTextFormat();
    return TRUE;
}

CUIInfo::~CUIInfo()
{
    SAFE_RELEASE(m_pBrBigTip);
    SAFE_RELEASE(m_pBrSmallTip);
    SAFE_RELEASE(m_pTfTitle); 
    SAFE_RELEASE(m_pTfTip);
}


CUIButton::CUIButton()
{
    m_uType = UIET_BUTTON;
    m_uFlags = UIEF_NONE;
}

CUIButton::~CUIButton()
{
    m_pBrHot->Release();
    m_pBrPressed->Release();
}

void CUIButton::Redraw()
{
    auto pDC = m_pBK->m_pDC;
    if (!eck::IsBitSet(m_uStyle, UIES_NOERASEBK))
    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, &m_rcF);

    if (m_bLBtnDown)
        pDC->FillRectangle(&m_rcF, m_pBrPressed);
    else if (m_bHot)
        pDC->FillRectangle(&m_rcF, m_pBrHot);

    pDC->DrawBitmap(m_pBmp, m_rcfImg, 1.f, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);

    BkDbg_DrawElemFrame();
}

BOOL CUIButton::InitElem()
{
    SAFE_RELEASE(m_pBrHot);
    SAFE_RELEASE(m_pBrPressed);
    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.6f), &m_pBrHot);
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.9f), &m_pBrPressed);
    return TRUE;
}

BOOL CUIButton::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
	{
	case WM_MOUSEMOVE:
	{
        if (PtInRect(&m_rc, GET_PT_LPARAM(lParam)))
        {
            if (!m_bHot)
            {
                m_bHot = TRUE;
                Redraw(TRUE);
            }
        }
        else if (m_bHot)
        {
            m_bHot = FALSE;
            Redraw(TRUE);
        }
	}
	return FALSE;
    case WM_MOUSELEAVE:
    {
        if (m_bHot)
        {
            m_bHot = FALSE;
            Redraw(TRUE);
        }
    }
    return FALSE;
    case WM_LBUTTONDOWN:
    {
        if (PtInRect(&m_rc, GET_PT_LPARAM(lParam)))
        {
            m_bLBtnDown = TRUE;
            SetCapture(m_pBK->m_hWnd);
            Redraw(TRUE);
            return TRUE;
        }
    }
    return FALSE;
    case WM_LBUTTONUP:
    {
        if (m_bLBtnDown)
        {
            ReleaseCapture();
            m_bLBtnDown = FALSE;
            Redraw(TRUE);
        }
    }
    return FALSE;
	}

    return FALSE;
}

LRESULT CUIButton::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    auto lResult = DefElemEventProc(uEvent, wParam, lParam);
    switch (uEvent)
    {
    case UIEE_SETRECT:
    {
        const float cx = m_rcfImg.right - m_rcfImg.left;
        const float cy = m_rcfImg.bottom - m_rcfImg.top;
        m_rcfImg.left = m_rcF.left + ((m_cx - cx) / 2);
        m_rcfImg.top = m_rcF.top + ((m_cy - cy) / 2);
        m_rcfImg.right = m_rcfImg.left + cx;
        m_rcfImg.bottom = m_rcfImg.top + cy;
    }
    break;
    }
    return lResult;
}

CUIRoundButton::CUIRoundButton()
{
    m_uType = UIET_ROUNDBUTTON;
    m_uFlags = UIEF_NONE;
}

CUIRoundButton::~CUIRoundButton()
{
    SAFE_RELEASE(m_pBrNormal);
}

void CUIRoundButton::Redraw()
{
	auto pDC = m_pBK->m_pDC;
	if (!eck::IsBitSet(m_uStyle, UIES_NOERASEBK))
		pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, &m_rcF);

    if (m_bLBtnDown)
        pDC->FillEllipse(&m_Ellipse, m_pBrPressed);
    else if (m_bHot)
        pDC->FillEllipse(&m_Ellipse, m_pBrHot);
    else
        pDC->FillEllipse(&m_Ellipse, m_pBrNormal);

    pDC->DrawBitmap(m_pBmp, m_rcfImg, 1.f, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);

    BkDbg_DrawElemFrame();
}

BOOL CUIRoundButton::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_MOUSEMOVE:
    {
        POINT pt GET_PT_LPARAM(lParam);
		if (PtInRect(&m_rc, pt) &&
            powf(m_Ellipse.point.x - pt.x, 2.f) + powf(m_Ellipse.point.y - pt.y, 2.f) <= powf(m_Ellipse.radiusX, 2.f))
        {
            if (!m_bHot)
            {
                m_bHot = TRUE;
                CUIButton::Redraw(TRUE);
            }
        }
        else if (m_bHot)
        {
            m_bHot = FALSE;
            CUIButton::Redraw(TRUE);
        }
    }
    return FALSE;
    case WM_LBUTTONDOWN:
    {
        if (PtInRect(&m_rc, GET_PT_LPARAM(lParam)))
        {
            m_bLBtnDown = TRUE;
            SetCapture(m_pBK->m_hWnd);
            CUIButton::Redraw(TRUE);
            return TRUE;
        }
    }
    return FALSE;
    }
    return CUIButton::OnEvent(uMsg, wParam, lParam);
}

LRESULT CUIRoundButton::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
	auto lResult = CUIButton::OnElemEvent(uEvent, wParam, lParam);
	if (uEvent == UIEE_SETRECT)
	{
		const float fRadius = std::min(m_cxHalf, m_cyHalf);
		m_Ellipse =
		{
			{
				m_rcF.left + m_cxHalf,
				m_rcF.top + m_cyHalf
			},
			fRadius,
			fRadius
		};
	}
	return lResult;
}

BOOL CUIRoundButton::InitElem()
{
    CUIButton::InitElem();
    SAFE_RELEASE(m_pBrNormal);
    auto pDC = m_pBK->m_pDC;
	pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.4f), &m_pBrNormal);
    return TRUE;
}



CUIPlayingCtrl::CUIPlayingCtrl()
{
    
}

void CUIPlayingCtrl::Redraw()
{
    auto pDC = m_pBK->m_pDC;
    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, &m_rcF);
    pDC->PushAxisAlignedClip(&m_rcF, D2D1_ANTIALIAS_MODE_ALIASED);
    for (auto pElem : m_vChildren)
        pElem->Redraw();
    pDC->PopAxisAlignedClip();

    BkDbg_DrawElemFrame();
}

BOOL CUIPlayingCtrl::InitElem()
{
    m_pBTOptions = new CUIButton;
    m_pBTPlayOpt = new CUIButton;
    m_pBTRepeatMode = new CUIButton;
    m_pBTPrev = new CUIButton;
    m_pBTPlay = new CUIRoundButton;
    m_pBTNext = new CUIButton;
    m_pBTStop = new CUIButton;
    m_pBTLrc = new CUIButton;
    m_pBTAbout = new CUIButton;
    m_pBTOptions->m_uStyle |= UIES_NOERASEBK;
    m_pBTPlayOpt->m_uStyle |= UIES_NOERASEBK;
    m_pBTRepeatMode->m_uStyle |= UIES_NOERASEBK;
    m_pBTPrev->m_uStyle |= UIES_NOERASEBK;
    m_pBTPlay->m_uStyle |= UIES_NOERASEBK;
    m_pBTNext->m_uStyle |= UIES_NOERASEBK;
    m_pBTStop->m_uStyle |= UIES_NOERASEBK;
    m_pBTLrc->m_uStyle |= UIES_NOERASEBK;
    m_pBTAbout->m_uStyle |= UIES_NOERASEBK;
    m_pBK->AddElem(m_pBTOptions);
    m_pBK->AddElem(m_pBTPlayOpt);
    m_pBK->AddElem(m_pBTRepeatMode);
    m_pBK->AddElem(m_pBTPrev);
    m_pBK->AddElem(m_pBTPlay);
    m_pBK->AddElem(m_pBTNext);
    m_pBK->AddElem(m_pBTStop);
    m_pBK->AddElem(m_pBTLrc);
    m_pBK->AddElem(m_pBTAbout);
    m_pBTOptions->SetParent(this);
    m_pBTPlayOpt->SetParent(this);
    m_pBTRepeatMode->SetParent(this);
    m_pBTPrev->SetParent(this);
    m_pBTPlay->SetParent(this);
    m_pBTNext->SetParent(this);
    m_pBTStop->SetParent(this);
    m_pBTLrc->SetParent(this);
    m_pBTAbout->SetParent(this);

    m_pBTOptions->InitElem();
    m_pBTPlayOpt->InitElem();
    m_pBTRepeatMode->InitElem();
    m_pBTPrev->InitElem();
    m_pBTPlay->InitElem();
    m_pBTNext->InitElem();
    m_pBTStop->InitElem();
    m_pBTLrc->InitElem();
    m_pBTAbout->InitElem();
    m_pBTOptions->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Options]);
    m_pBTPlayOpt->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_PlayOpt]);
    m_pBTRepeatMode->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_RMAllLoop]);
    m_pBTPrev->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Prev]);
    m_pBTPlay->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Play]);
    m_pBTNext->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Next]);
    m_pBTStop->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Stop]);
    m_pBTLrc->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Lrc]);
    m_pBTAbout->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_About]);

    for (auto pElem : m_vChildren)
        if (pElem->m_uType == UIET_BUTTON || pElem->m_uType == UIET_ROUNDBUTTON)
            ((CUIButton*)pElem)->SetImgSize(m_pBK->m_Ds.cxIcon, m_pBK->m_Ds.cyIcon);

    return TRUE;
}

LRESULT CUIPlayingCtrl::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    auto lResult = DefElemEventProc(uEvent, wParam, lParam);
    switch (uEvent)
    {
    case UIEE_SETRECT:
    {
		const int cxTotal = m_pBK->m_Ds.iPCBTGap * 8 + m_pBK->m_Ds.cxPCBT * 8 + m_pBK->m_Ds.cxPCBTBig;
        int xLeft = (m_cx - cxTotal) / 2;
        const int yTop = (m_cy - m_pBK->m_Ds.cyPCBT) / 2;
        const int cxBT = m_pBK->m_Ds.cxPCBT;
        const int iGap = m_pBK->m_Ds.iPCBTGap;
        RECT rc{ 0,yTop,0,yTop + m_pBK->m_Ds.cyPCBT };
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTOptions->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTPlayOpt->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTRepeatMode->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTPrev->SetElemRect(&rc);

        xLeft += (cxBT + iGap);
        const int xBigBT = xLeft;
		xLeft += (iGap + m_pBK->m_Ds.cxPCBTBig);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTNext->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTStop->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTLrc->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTAbout->SetElemRect(&rc);

        rc.left = xBigBT;
        rc.top = (m_cy - m_pBK->m_Ds.cyPCBTBig) / 2;
        rc.right = rc.left + m_pBK->m_Ds.cxPCBTBig;
        rc.bottom = rc.top + m_pBK->m_Ds.cyPCBTBig;
        m_pBTPlay->SetElemRect(&rc);
    }
    break;
    case UIEE_CHILDREDRAW:
    {
        auto pDC = m_pBK->m_pDC;
        auto pElem = (CUIElem*)wParam;
        pDC->BeginDraw();
        pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &pElem->m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, &pElem->m_rcF);
        pElem->Redraw();
        pDC->EndDraw();
    }
    break;
    }

    return lResult;
}

BOOL CUIPlayingCtrl::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DispatchEvent(uMsg, wParam, lParam);
    return FALSE;
}