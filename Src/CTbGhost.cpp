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
        eck::CDib dib{};
        dib.Create(1, 1);
	    DwmSetIconicLivePreviewBitmap(hWnd, dib.GetHBitmap(), NULL, 0);
        return 0;
        // TODO:设置、最小化处理
        const UINT xMargin = 80;
        const UINT yMargin = 80;
        const UINT cxMax = m_WndMain.ClientWidth - xMargin * 2;
        const UINT cyMax = m_WndMain.ClientHeight - yMargin * 2;

        const auto& rbCover = App->GetPlayer().GetMusicInfo().rbCover;
        const auto pStream = new eck::CStreamView(rbCover);

        HBITMAP hBitmap;
        GpBitmap* pBitmap;
        GdipCreateBitmapFromStream(pStream, &pBitmap);
        if (!pBitmap)
        {
            eck::CDib dib{};
            DwmSetIconicLivePreviewBitmap(hWnd, dib.Create(cxMax, cyMax), 0, 0);
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
        GdipCreateBitmapFromScan0(m_WndMain.ClientWidth, m_WndMain.ClientHeight, 0, PixelFormat32bppARGB, NULL, &pBitmapBK);
        GdipGetImageGraphicsContext(pBitmapBK, &pGraphics);
        GdipGraphicsClear(pGraphics, 0x90FFFFFF);
        int rc[4]{ (cxMax - cx) / 2 + xMargin, (cyMax - cy) / 2 + yMargin, cx, cy };
        GdipDrawImageRectRectI(pGraphics, pBitmap,
            rc[0],rc[1], rc[2], rc[3],
            0, 0, cx0, cy0,
            UnitPixel, NULL, NULL, NULL);
        GpPen* pPen;
        GdipCreatePen1(0xFF000000, 2, UnitPixel, &pPen);
        GdipDrawRectangleI(pGraphics, pPen, rc[0], rc[1], rc[2], rc[3]);
        GdipDeleteGraphics(pGraphics);
        GdipCreateHBITMAPFromBitmap(pBitmapBK, &hBitmap, 0x00000000);

        DwmSetIconicLivePreviewBitmap(hWnd, hBitmap, 0, 0);

        GdipDisposeImage(pBitmap);
        GdipDisposeImage(pBitmapBK);
        DeleteObject(hBitmap);
        pStream->LeaveRelease();
    }
    return 0;

    case WM_DWMSENDICONICTHUMBNAIL:
    {
        const UINT cxMax = HIWORD(lParam);
        const UINT cyMax = LOWORD(lParam);

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
        DeleteObject(hBitmap);
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
        return 0;
    }
    return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}
