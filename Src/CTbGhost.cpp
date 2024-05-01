#include "CWndMain.h"

LRESULT CTbGhost::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        constexpr BOOL b = TRUE;
        DwmSetWindowAttribute(hWnd, DWMWA_HAS_ICONIC_BITMAP, &b, sizeof(b));
        DwmSetWindowAttribute(hWnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &b, sizeof(b));
    }
    return 0;

	case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
	{
		if (!App->GetOptionsMgr().ProgShowCoverLivePreview)
		{
        SetEmptyBitmap:
            eck::CDib dib{};
            DwmSetIconicLivePreviewBitmap(hWnd, dib.Create(1, 1), NULL, 0);
			return 0;
		}

        if (m_hbmLivePreviewCache)
        {
            DwmSetIconicLivePreviewBitmap(hWnd, m_hbmLivePreviewCache, NULL, 0);
            return 0;
		}

		RECT rcMainClient;
		if (!eck::GetWindowClientRect(m_WndMain.HWnd, rcMainClient))
			goto SetEmptyBitmap;
		const int iDpi = eck::GetDpi(m_WndMain.HWnd);
		const UINT xMargin = eck::DpiScale(50, iDpi);
		const UINT yMargin = xMargin;
		const UINT cxMax = rcMainClient.right - xMargin * 2;
		const UINT cyMax = rcMainClient.bottom - yMargin * 2;

        const auto pPic = App->GetPlayer().GetMusicInfo().GetMainCover();
        if (!pPic)
            goto SetEmptyBitmap;

        GpBitmap* pBitmap;
        if (pPic->bLink)
            GdipCreateBitmapFromFile(std::get<1>(pPic->varPic).Data(), &pBitmap);
        else
        {
            const auto pStream = new eck::CStreamView(std::get<0>(pPic->varPic));
            GdipCreateBitmapFromStream(pStream, &pBitmap);
            pStream->Release();
        }

        if (!pBitmap)
            goto SetEmptyBitmap;

        UINT cx, cy, cx0, cy0;
        GdipGetImageWidth(pBitmap, &cx0);
        GdipGetImageHeight(pBitmap, &cy0);

        if ((float)cxMax / (float)cyMax > (float)cx0 / (float)cy0)// y对齐
        {
            cy = cyMax;
            cx = cx0 * cy / cy0;
        }
        else// x对齐
        {
            cx = cxMax;
            cy = cx * cy0 / cx0;
        }

        GpBitmap* pBitmapBK;
        GpGraphics* pGraphics;
        GdipCreateBitmapFromScan0(rcMainClient.right, rcMainClient.bottom, 0, PixelFormat32bppARGB, NULL, &pBitmapBK);
        GdipGetImageGraphicsContext(pBitmapBK, &pGraphics);
        GdipGraphicsClear(pGraphics, eck::ColorrefToARGB(eck::GetThreadCtx()->crDefBkg, 0x90));
        int rc[4]
        {
            (int)((cxMax - cx) / 2 + xMargin),
            (int)((cyMax - cy) / 2 + yMargin),
            (int)cx,
            (int)cy
        };
        GdipDrawImageRectRectI(pGraphics, pBitmap,
            rc[0],rc[1], rc[2], rc[3],
            0, 0, cx0, cy0,
            Gdiplus::UnitPixel, NULL, NULL, NULL);
        GpPen* pPen;
        const auto cxPen = eck::DpiScaleF(1.f, iDpi);
		GdipCreatePen1(0xFF000000, cxPen, Gdiplus::UnitPixel, &pPen);
        GdipDrawRectangleI(pGraphics, pPen, rc[0], rc[1], rc[2], rc[3]);
        GdipSetPenColor(pPen, 0xFFFFFFFF);
        rc[0] -= 1;
        rc[1] -= 1;
        rc[2] += 2;
        rc[3] += 2;
        GdipDrawRectangleI(pGraphics, pPen, rc[0], rc[1], rc[2], rc[3]);

        GdipDeleteGraphics(pGraphics);
        HBITMAP hBitmap;
        GdipCreateHBITMAPFromBitmap(pBitmapBK, &hBitmap, 0x00000000);

        DwmSetIconicLivePreviewBitmap(hWnd, hBitmap, 0, 0);

        GdipDisposeImage(pBitmap);
        GdipDisposeImage(pBitmapBK);
        m_hbmLivePreviewCache = hBitmap;
    }
    return 0;

    case WM_DWMSENDICONICTHUMBNAIL:
        SetIconicThumbnail(HIWORD(lParam), LOWORD(lParam));
        return 0;

    case WM_ACTIVATE:
    {
        m_WndMain.m_pTbList->SetTabActive(hWnd, m_WndMain.HWnd, 0);
        if (IsIconic(m_WndMain.HWnd))
            m_WndMain.Show(SW_RESTORE);
        SetForegroundWindow(m_WndMain.HWnd);
    }
    return 0;

    case WM_COMMAND:
    case WM_SYSCOMMAND:
        return m_WndMain.SendMsg(uMsg, wParam, lParam);

    case WM_DESTROY:
        m_WndMain.m_pTbList->UnregisterTab(hWnd);
        InvalidateLivePreviewCache();
        InvalidateThumbnailCache();
        return 0;
    }
    return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

void CTbGhost::SetIconicThumbnail(UINT cxMax, UINT cyMax)
{
    if (cxMax == UINT_MAX || cyMax == UINT_MAX)
    {
        cxMax = (m_cxPrev ? m_cxPrev : eck::DpiScale(120, m_WndMain.m_iDpi));
        cyMax = (m_cyPrev ? m_cyPrev : eck::DpiScale(120, m_WndMain.m_iDpi));
    }
    m_cxPrev = cxMax;
    m_cyPrev = cyMax;

    if (m_hbmThumbnailCache)
    {
        BITMAP bmp;
        GetObjectW(m_hbmThumbnailCache, sizeof(bmp), &bmp);
        if (bmp.bmWidth <= (int)cxMax && bmp.bmHeight <= (int)cyMax)
        {
            DwmSetIconicThumbnail(HWnd, m_hbmThumbnailCache, 0);
            return;
        }
        else
			InvalidateThumbnailCache();
	}

	auto pOrg = App->GetPlayer().GetWicBmpCover();
    if (!pOrg)
        pOrg = App->GetWicRes()[IIDX_DefCover];
	UINT cx, cy, cx0, cy0;
	pOrg->GetSize(&cx0, &cy0);
	if ((float)cxMax / (float)cyMax > (float)cx0 / (float)cy0)// y对齐
	{
		cy = cyMax;
		cx = cx0 * cy / cy0;
	}
	else// x对齐
	{
		cx = cxMax;
		cy = cx * cy0 / cx0;
	}
	IWICBitmap* pBmp;
	eck::ScaleWicBitmap(pOrg, pBmp, cx, cy, WICBitmapInterpolationModeFant);
	m_hbmThumbnailCache = eck::CreateHBITMAP(pBmp);
	DwmSetIconicThumbnail(HWnd, m_hbmThumbnailCache, 0);
	pBmp->Release();
	return;

	//const auto& rbCover = App->GetPlayer().GetMusicInfo().rbCover;
	//const auto pStream = new eck::CStreamView(rbCover);

	//HBITMAP hBitmap;
	//GpBitmap* pBitmap;
	//GdipCreateBitmapFromStream(pStream, &pBitmap);
    //if (!pBitmap)
    //{
    //    EckDbgBreak();
    //    eck::CDib dib{};
    //    DwmSetIconicThumbnail(HWnd, dib.Create(cxMax, cyMax), 0);
    //    return;
    //}

    //UINT cx, cy, cx0, cy0;
    //GdipGetImageWidth(pBitmap, &cx0);
    //GdipGetImageHeight(pBitmap, &cy0);

    //if ((float)cxMax / (float)cyMax > (float)cx0 / (float)cy0)// y对齐
    //{
    //    cy = cyMax;
    //    cx = cx0 * cy / cy0;
    //}
    //else// x对齐
    //{
    //    cx = cxMax;
    //    cy = cx * cy0 / cx0;
    //}

    //GpBitmap* pBitmapBK;
    //GpGraphics* pGraphics;
    //GdipCreateBitmapFromScan0(cx, cy, 0, GpPixelFormat::PF32bppARGB, NULL, &pBitmapBK);
    //GdipGetImageGraphicsContext(pBitmapBK, &pGraphics);
    //GdipGraphicsClear(pGraphics, 0x00000000);
    //GdipSetInterpolationMode(pGraphics, GpInterpolationMode::HighQuality);
    //GdipDrawImageRectRectI(pGraphics, pBitmap,
    //    0, 0, cx, cy,
    //    0, 0, cx0, cy0,
    //    Gdiplus::UnitPixel, NULL, NULL, NULL);
    //GdipDeleteGraphics(pGraphics);
    //GdipCreateHBITMAPFromBitmap(pBitmapBK, &hBitmap, 0x00000000);

    //DwmSetIconicThumbnail(HWnd, hBitmap, 0);

    //GdipDisposeImage(pBitmap);
    //GdipDisposeImage(pBitmapBK);
    //m_hbmThumbnailCache = hBitmap;
    //pStream->LeaveRelease();
}