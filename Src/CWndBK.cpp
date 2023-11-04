#include "CWndBK.h"


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
    if (uMsg == WM_NCCREATE)
    {
        p = (CWndBK*)((CREATESTRUCTW*)lParam)->lpCreateParams;
        SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
    }

    if (uMsg == p->m_uMsgCUIButton)
        p->OnPCBtnClick((UINT)wParam, (CUIElem*)lParam);// 落到元素事件处理

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
					pElem->OnTimer((UINT)wParam);
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
                pElem->OnTimer((UINT)wParam);
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
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, p->OnCreate);
    case WM_DESTROY:
        return HANDLE_WM_DESTROY(hWnd, wParam, lParam, p->OnDestroy);
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
    m_uMsgCUIButton = RegisterWindowMessageW(L"PlayerNew.Message.CUIButtonClick");
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

    m_pDC->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
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

    UpdateStaticBmp();
    Redraw();
}

void CWndBK::OnDestroy(HWND hWnd)
{
	GenElemEvent(UIEE_DESTROY, 0, 0);
	for (auto pElem : m_vAllElems)
		delete pElem;
    m_vAllElems.clear();
    m_vElems.clear();
    m_vElemsWantTimer.clear();
	m_pBrWhite->Release();
	m_pBrWhite2->Release();
	if (m_pBmpAlbum)
		m_pBmpAlbum->Release();
	m_pBmpBKStatic->Release();
	m_pBmpBK->Release();
	for (auto pBmp : m_pBmpIcon)
		pBmp->Release();
	m_pDC->Release();
	m_pSwapChain->Release();
}

void CWndBK::OnPCBtnClick(UINT uCode, CUIElem* pElem)
{
    auto pBtn = (CUIButton*)pElem;
    auto& Player = App->GetPlayer();
    PlayOpErr uErr;
    switch (pBtn->GetID())
    {
	case PCBTI_OPT:
		break;
	case PCBTI_PLAYOPT:
		break;
	case PCBTI_REPEATMODE:
        COptionsMgr::GetInst().iRepeatMode = COptionsMgr::NextRepeatMode(COptionsMgr::GetInst().iRepeatMode);
		break;
	case PCBTI_PREV:
		uErr = Player.Prev();
		goto ProcPlayErr;
	case PCBTI_PLAY:
		uErr = Player.PlayOrPause();
        goto ProcPlayErr;
	case PCBTI_NEXT:
		uErr = Player.Next();
        goto ProcPlayErr;
	case PCBTI_STOP:
		Player.Stop();
		break;
	case PCBTI_LRC:
		break;
	case PCBTI_ABOUT:
		break;
	}

    return;
ProcPlayErr:
    if (uErr != PlayOpErr::Ok)
        CPlayer::ShowPlayErr(m_hWnd, uErr);
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
    if (!pWICBitmapOrg)
        pWICBitmapOrg = App->GetWicRes()[IIDX_DefCover];
    ////////////////////绘制静态位图
    m_pDC->BeginDraw();
    m_pDC->SetTarget(m_pBmpBKStatic);

    UINT cx0, cy0;
    float cyRgn/*截取区域高*/, cx/*缩放后图片宽*/, cy/*缩放后图片高*/;

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