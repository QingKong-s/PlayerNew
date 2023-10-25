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
	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, p->OnSize);
	case WM_NCCREATE:
		p = (CWndBK*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		break;
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, p->OnCreate);
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

BOOL CWndBK::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
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
	return TRUE;
}

void CWndBK::OnSize(HWND hWnd, UINT state, int cx, int cy)
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





LRESULT CUIElem::DefElemEventProc(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    switch (uEvent)
    {
    case UIEE_SETRECT:
    {
        auto prc = (RECT*)wParam;
        m_rc = *prc;
        m_rcInWnd = m_rc;
        m_rcF = eck::MakeD2DRcF(m_rc);
        m_cx = m_rc.right - m_rc.left;
        m_cy = m_rc.bottom - m_rc.top;
        m_cxHalf = m_cx / 2;
        m_cyHalf = m_cy / 2;

        if (m_rcInWnd.left < 0)
            m_rcInWnd.left = 0;
        else if (m_rcInWnd.top < 0)
            m_rcInWnd.top = 0;
        else if (m_rcInWnd.right > m_cx)
            m_rcInWnd.right = m_cx;
        else if (m_rcInWnd.bottom > m_cy)
            m_rcInWnd.bottom = m_cy;

        m_pBK->m_vDirtyRect.push_back(m_rc);
    }
    return 0;

    }
    return 0;
}

CUIElem::~CUIElem()
{
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
        D2D1_SIZE_F D2DSize = pBmp->GetSize();
        if (D2DSize.width > D2DSize.height)// 宽度较大
        {
            fScaleFactor = fSize / D2DSize.height;
            xStart -= ((D2DSize.width - D2DSize.height) / 2.f * fSize / D2DSize.height);
        }
        else// 高度较大
        {
            fScaleFactor = fSize / D2DSize.width;
            yStart -= ((D2DSize.height - D2DSize.width) / 2.f * fSize / D2DSize.width);
        }

        D2D1_MATRIX_3X2_F Matrix, Matrix2;

        Matrix = D2D1::Matrix3x2F::Translation(xStart, yStart);// 制平移矩阵
        D2D1::Matrix3x2F* MatrixObj1 = D2D1::Matrix3x2F::ReinterpretBaseType(&Matrix);// 转类

        Matrix2 = D2D1::Matrix3x2F::Scale(fScaleFactor, fScaleFactor, D2D1::Point2F(xStart, yStart));// 制缩放矩阵
        D2D1::Matrix3x2F* MatrixObj2 = D2D1::Matrix3x2F::ReinterpretBaseType(&Matrix2);// 转类

        Matrix = ((*MatrixObj1) * (*MatrixObj2));// 矩阵相乘

        D2D1_BRUSH_PROPERTIES D2DBrushProp = { 1.f,Matrix };
        m_pBK->m_pDC->CreateBitmapBrush(pBmp, NULL, &D2DBrushProp, &m_pBrAlbum);
    }
}

void CUIAlbumRotating::Redraw()
{
    auto pDC = m_pBK->m_pDC;
    pDC->PushAxisAlignedClip(&m_rcF, D2D1_ANTIALIAS_MODE_ALIASED);// 设个剪辑区，防止边缘残留
    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);// 刷背景

    DWORD dwLevel = -1;
    float cx = m_rcF.right - m_rcF.left, cy = m_rcF.bottom - m_rcF.top;
    const int iAlbumLevel = m_pBK->m_Ds.sizeAlbumLevel;
    if (App->GetPlayer().GetBass().GetHStream())
    {
        dwLevel = App->GetPlayer().GetBass().GetLevel();
        /////////////////////////////画封面边缘
        D2D1_ELLIPSE D2DEllipse;
        D2DEllipse.point = { m_rcF.left + cx / 2.f,m_rcF.top + cy / 2.f };
        float fRadius;
        fRadius = std::min(cx / 2.f, cy / 2.f);
        D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;

        pDC->FillEllipse(&D2DEllipse, m_pBrUV);// 画外圈
        float fOffset = 0.f;
        fRadius -= iAlbumLevel;

        if (dwLevel != -1)
        {
            fOffset = ((float)(LOWORD(dwLevel) + HIWORD(dwLevel)) / 2.f) / 32768.f * iAlbumLevel;
            fRadius += fOffset;
            D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;
            pDC->FillEllipse(&D2DEllipse, m_pBrUV2);// 画电平指示
        }

        /////////////////////////////画封面
        D2D1_MATRIX_3X2_F Matrix = D2D1::Matrix3x2F::Rotation(m_fAngle, D2DEllipse.point);// 制旋转矩阵
        pDC->SetTransform(Matrix);// 置旋转变换

        fRadius = fRadius - fOffset;
        D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;
        pDC->FillEllipse(&D2DEllipse, m_pBrAlbum);

        Matrix = D2D1::Matrix3x2F::Identity();
        pDC->SetTransform(Matrix);// 还原空变换
    }
    else
    {
        D2D1_ELLIPSE D2DEllipse;
        D2DEllipse.point = { m_rcF.left + cx / 2.f,m_rcF.top + cy / 2.f };
        float fRadius;
        fRadius = std::min(cx / 2.f, cy / 2.f);
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
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBrUV);
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.6f), &m_pBrUV2);
    return TRUE;
}


void CUIWaves::GetWavesData()
{
	auto p = App;
    std::thread Thread([this, p]()
        {
            CBass Bass;
            Bass.Open(p->GetPlayer().GetCurrFileName().Data(), BASS_STREAM_DECODE, BASS_MUSIC_DECODE | BASS_MUSIC_PRESCAN);
            ULONGLONG ullLength = (ULONGLONG)(Bass.GetLength() * 1000.);
            if (ullLength == 0)
            {
                EckDbgPrint(CBass::GetError());
                m_ThreadState = ThreadState::Error;
                return;
            }

            size_t cBars = ullLength / 20ull;
            m_vWavesData.resize(cBars);

            HANDLE hEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, c_szWaveEventName);
            if (!hEvent)
            {
                EckDbgPrintFormatMessage(GetLastError());
                m_ThreadState = ThreadState::Error;
                return;
            }
            EckCounter(cBars, i)
            {
                m_vWavesData[i] = Bass.GetLevel();
                if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
                {
                    m_vWavesData.clear();
                    m_ThreadState = ThreadState::Stopped;
                    return;
                }
            }

            m_ThreadState = ThreadState::Stopped;
        });

    Thread.detach();
    m_hThread = Thread.native_handle();
}

CUIWaves::CUIWaves()
{
    m_uType = UIET_WAVES;
    m_uFlags = UIEF_NOEVENT | UIEF_WANTTIMEREVENT;
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
        13, L"zh-cn", &m_pTfTip);
    m_pTfTip->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_pTfTip->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);// 水平垂直都居中
    return TRUE;
}

CUIWaves::~CUIWaves()
{
    SAFE_RELEASE(m_pBrLine);
    SAFE_RELEASE(m_pBrCenterMark);
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
        SetRedraw(FALSE);
        pDC->DrawTextW(pszText, lstrlenW(pszText), m_pTfTip, &m_rcF, m_pBrLine);
		BkDbg_DrawElemFrame();
		return;
	}

    const int idxCurr = (int)(m_pBK->m_ullMusicPos / 20ull);// 算数组索引    20ms一单位
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

void CUIWaves::OnTimer(UINT uTimerID)
{
    if (uTimerID == CWndBK::IDT_PGS)
    {
        Redraw();
    }
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
        ZeroMemory(m_piOldHeight, m_cbDataUnit);
        ZeroMemory(m_piHeight, m_cbDataUnit);
        ZeroMemory(fData, sizeof(fData));
    }

    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);// 刷背景
    for (int i = 0; i < m_iCount; i++)
    {
        ++m_piTime[i];
        m_piHeight[i] = abs((int)(fData[i] * 300.0f));

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
    delete[] m_pBaseData;
    if (i > 0)
    {
        // c * cxBar + (c - 1) * cxDiv = cx
        // 解得：cxBar = (cx - (c - 1) * cxDiv) / c
        m_iCount = i;
        m_cxBar = (m_cx - (i - 1) * m_cxGap) / i;
        if (m_cxBar <= 0)
            goto Fail;

        m_cbDataUnit = sizeof(int) * i * 4;

        m_pBaseData = new int[i * 4];
        m_piOldHeight = m_pBaseData;
        m_piHeight = m_pBaseData + i;
        m_piOldMaxPos = m_pBaseData + i * 2;
        m_piTime = m_pBaseData + i * 3;
        return;
    }
Fail:
    m_iCount = 0;
    m_cxBar = 0.f;
    m_cbDataUnit = 0;
    m_pBaseData = m_piOldHeight = m_piHeight = m_piOldMaxPos = m_piTime = NULL;
}

CUISpe::CUISpe()
{
    m_uType = UIET_SPE;
    m_uFlags = UIEF_NOEVENT | UIEF_WANTTIMEREVENT;
}

CUISpe::~CUISpe()
{
    delete[] m_pBaseData;
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

void CUISpe::OnTimer(UINT uTimerID)
{
    if (uTimerID == CWndBK::IDT_PGS)
    {
        Redraw();
    }
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
    m_uFlags = UIEF_NOEVENT | UIEF_WANTTIMEREVENT;
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

LRESULT CUIProgBar::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
        m_ullPos = m_pBK->m_ullMusicPos;
        Redraw();
    }
}

LRESULT CUIProgBar::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    auto lResult = DefElemEventProc(uEvent, wParam, lParam);
    switch (uEvent)
    {
    case UIEE_ONPLAYINGCTRL:
    {
        SetMax(m_pBK->m_ullMusicLength);
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
    HWND hWndBK = m_pBK->m_hWnd;
    m_hToolTip = CreateWindowExW(0, TOOLTIPS_CLASSW, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP,
        0, 0, 0, 0, hWndBK, NULL, NULL, NULL);// 创建工具提示

    m_ti = { sizeof(TTTOOLINFOW),TTF_TRACK | TTF_IDISHWND | TTF_ABSOLUTE,hWndBK,(UINT_PTR)hWndBK,{0},App->GetHInstance(),NULL,0};
    SendMessageW(m_hToolTip, TTM_ADDTOOLW, 0, (LPARAM)&m_ti);

    m_rcFTimeLabel.left = m_rcF.left;
    m_rcFTimeLabel.top = m_rcF.top;
    m_rcFTimeLabel.right = m_rcFTimeLabel.left + (float)m_pBK->m_Ds.cxTime;
    m_rcFTimeLabel.bottom = m_rcF.bottom;
    m_rcTimeLabel = eck::MakeRect(m_rcFTimeLabel);

    m_cchTime = lstrlenW(m_szTime);
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

LRESULT CUIToolBar::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
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


void CUIInfo::UpdateTF()
{
    SAFE_RELEASE(m_pTfTitle);
    SAFE_RELEASE(m_pTfTip);
    App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        13, L"zh-cn", &m_pTfTitle);
    App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        9, L"zh-cn", &m_pTfTip);

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
    const PCWSTR pszTip[4] =
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
        pDC->DrawTextW(pszTip[i], cchTip2[i], m_pTfTip, &rcF, m_pBrSmallTip);
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
    UpdateTF();
    return TRUE;
}

CUIInfo::~CUIInfo()
{
    SAFE_RELEASE(m_pBrBigTip);
    SAFE_RELEASE(m_pBrSmallTip);
    SAFE_RELEASE(m_pTfTitle); 
    SAFE_RELEASE(m_pTfTip);
}