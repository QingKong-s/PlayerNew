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
		RECT rcMainClient;
		GetClientRect(m_WndMain.HWnd, &rcMainClient);
		if (!App->GetOptionsMgr().ProgShowCoverLivePreview ||
			(!rcMainClient.right || !rcMainClient.bottom))
		{
			eck::CDib dib{};
			dib.Create(1, 1);
			DwmSetIconicLivePreviewBitmap(hWnd, dib.GetHBitmap(), NULL, 0);
			return 0;
		}

        if (m_hbmLivePreviewCache)
        {
            DwmSetIconicLivePreviewBitmap(hWnd, m_hbmLivePreviewCache, NULL, 0);
            return 0;
        }

        const int iDpi = eck::GetDpi(m_WndMain.HWnd);
        const UINT xMargin = eck::DpiScale(50, iDpi);
        const UINT yMargin = xMargin;
        const UINT cxMax = rcMainClient.right - xMargin * 2;
        const UINT cyMax = rcMainClient.bottom - yMargin * 2;

        const auto& rbCover = App->GetPlayer().GetMusicInfo().rbCover;
        const auto pStream = new eck::CStreamView(rbCover);

        HBITMAP hBitmap;
        GpBitmap* pBitmap;
        GdipCreateBitmapFromStream(pStream, &pBitmap);
        if (!pBitmap)
        {
            eck::CDib dib{};
            DwmSetIconicLivePreviewBitmap(hWnd, dib.Create(1, 1), NULL, 0);
            return 0;
        }

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
        GdipGraphicsClear(pGraphics, eck::ColorrefToARGB(GetSysColor(COLOR_WINDOW), 0x90));
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
            UnitPixel, NULL, NULL, NULL);
        GpPen* pPen;
        const auto cxPen = eck::DpiScaleF(1.f, iDpi);
		GdipCreatePen1(0xFF000000, cxPen, UnitPixel, &pPen);
        GdipDrawRectangleI(pGraphics, pPen, rc[0], rc[1], rc[2], rc[3]);
        GdipSetPenColor(pPen, 0xFFFFFFFF);
        rc[0] -= 1;
        rc[1] -= 1;
        rc[2] += 2;
        rc[3] += 2;
        GdipDrawRectangleI(pGraphics, pPen, rc[0], rc[1], rc[2], rc[3]);

        GdipDeleteGraphics(pGraphics);
        GdipCreateHBITMAPFromBitmap(pBitmapBK, &hBitmap, 0x00000000);

        DwmSetIconicLivePreviewBitmap(hWnd, hBitmap, 0, 0);

        GdipDisposeImage(pBitmap);
        GdipDisposeImage(pBitmapBK);
        m_hbmLivePreviewCache = hBitmap;
        pStream->LeaveRelease();
    }
    return 0;

    case WM_DWMSENDICONICTHUMBNAIL:
    {
        const UINT cxMax = HIWORD(lParam);
        const UINT cyMax = LOWORD(lParam);

        if (m_hbmThumbnailCache)
        {
            BITMAP bmp;
            GetObjectW(m_hbmThumbnailCache, sizeof(bmp), &bmp);
            if (bmp.bmWidth == cxMax && bmp.bmHeight == cyMax)
            {
                DwmSetIconicThumbnail(hWnd, m_hbmThumbnailCache, 0);
                return 0;
            }
            else
                InvalidateThumbnailCache();
        }

        const auto& rbCover = App->GetPlayer().GetMusicInfo().rbCover;
        const auto pStream = new eck::CStreamView(rbCover);

        HBITMAP hBitmap;
        GpBitmap* pBitmap;
        GdipCreateBitmapFromStream(pStream, &pBitmap);
        if (!pBitmap)
        {
            // XXX : 应替换为默认图
            eck::CDib dib{};
            DwmSetIconicThumbnail(hWnd, dib.Create(cxMax, cyMax), 0);
            return 0;
        }

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
        GdipCreateBitmapFromScan0(cxMax, cyMax, 0, PixelFormat32bppARGB, NULL, &pBitmapBK);
        GdipGetImageGraphicsContext(pBitmapBK, &pGraphics);
        GdipGraphicsClear(pGraphics, 0x00000000);
        GdipDrawImageRectRectI(pGraphics, pBitmap,
            (cxMax - cx) / 2, (cyMax - cy) / 2, cx, cy,
            0, 0, cx0, cy0,
            UnitPixel, NULL, NULL, NULL);
        GdipDeleteGraphics(pGraphics);
        GdipCreateHBITMAPFromBitmap(pBitmapBK, &hBitmap, 0x00000000);

        DwmSetIconicThumbnail(hWnd, hBitmap, 0);

        GdipDisposeImage(pBitmap);
        GdipDisposeImage(pBitmapBK);
        m_hbmThumbnailCache = hBitmap;
        pStream->LeaveRelease();
    }
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
