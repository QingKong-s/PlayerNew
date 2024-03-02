#include "CWndBK.h"

LRESULT CUIAlbumRotating::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        Dui::ELEMPAINTSTRU ps;
        BeginPaint(ps, wParam, lParam);

        const auto pBk = (CWndBK*)GetWnd();
        const int cx = GetViewWidth(), cy = GetViewHeight();


        DWORD dwLevel = -1;
        const auto fAlbumLevel = pBk->m_DsF.sizeAlbumLevel;
        if (App->GetPlayer().IsFileActive())
        {
            dwLevel = App->GetPlayer().GetBass().GetLevel();
            /////////////////////////////画封面边缘
            float fRadius;
            fRadius = std::min(cx / 2.f, cy / 2.f);
            D2D1_ELLIPSE Ellipse
            {
                { cx / 2.f,cy / 2.f },
                fRadius,
                fRadius
            };

            m_pDC->FillEllipse(&Ellipse, m_pBrUV);// 画外圈
            float fOffset = 0.f;
            fRadius -= fAlbumLevel;
            if (dwLevel != -1)
            {
                fOffset = ((float)(LOWORD(dwLevel) + HIWORD(dwLevel)) / 2.f) / 32768.f * fAlbumLevel;
                fRadius += fOffset;
                Ellipse.radiusX = Ellipse.radiusY = fRadius;
                m_pDC->FillEllipse(&Ellipse, m_pBrUV2);// 画电平指示
            }
            /////////////////////////////画封面
            m_pDC->SetTransform(D2D1::Matrix3x2F::Rotation(m_fAngle, Ellipse.point));// 置旋转变换

            fRadius = fRadius - fOffset;
            Ellipse.radiusX = Ellipse.radiusY = fRadius;
            m_pDC->FillEllipse(&Ellipse, m_pBrAlbum);

            m_pDC->SetTransform(D2D1::Matrix3x2F::Identity());// 还原空变换
        }
        else
        {
            
            D2D1_ELLIPSE D2DEllipse;
            D2DEllipse.point = { cx / 2.f,cy / 2.f };
            float fRadius;
            fRadius = std::min(cx / 2.f, cy / 2.f);
            D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;

            m_pDC->FillEllipse(&D2DEllipse, m_pBrUV);// 画外圈

            fRadius -= fAlbumLevel;
            D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;
            m_pDC->FillEllipse(&D2DEllipse, m_pBrUV2);
        }

        m_pDC->PopAxisAlignedClip();
        BkDbg_DrawElemFrame();

        EndPaint(ps);
    }
    return 0;

    case UIEE_ONPLAYINGCTRL:
    {
        if (wParam == PCT_PLAYNEW)
        {
            m_fAngle = 0.f;
            UpdateAlbumBrush();
            InvalidateRect();
        }
    }
    break;
    }
    return __super::OnEvent(uMsg, wParam, lParam);
}

void CUIAlbumRotating::UpdateAlbumBrush()
{
    const auto pBk = (CWndBK*)GetWnd();
    auto pBmp = pBk->m_pBmpAlbum;
    if (pBmp)
    {
        if (m_pBrAlbum)
            m_pBrAlbum->Release();

        float fRadius, cx, cy;
        const auto fAlbumLevel = pBk->m_DsF.sizeAlbumLevel;
        cx = GetViewWidthF() - fAlbumLevel * 2;
        cy = GetViewHeightF() - fAlbumLevel * 2;
        fRadius = std::min(cx / 2.f, cy / 2.f);
        float xStart = fAlbumLevel + cx / 2.f - fRadius, 
            yStart = fAlbumLevel + cy / 2.f - fRadius;
        float fSize = fRadius * 2;
        float fScaleFactor;
        const int cxImg = pBk->m_cxAlbum, cyImg = pBk->m_cyAlbum;
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
        GetD2DDC()->CreateBitmapBrush(pBmp, NULL, &BrushProp, &m_pBrAlbum);
    }
}

void CUIAlbumRotating::OnTimer(UINT uTimerID)
{
    if (uTimerID == CWndBK::IDT_PGS)
    {
        m_fAngle += 0.5f;
        if (m_fAngle >= 360.f)
            m_fAngle = 0;
        InvalidateRect();
    }
}