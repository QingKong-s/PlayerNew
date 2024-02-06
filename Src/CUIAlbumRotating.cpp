#include "CWndBK.h"

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
        const auto fAlbumLevel = m_pBK->m_DsF.sizeAlbumLevel;
        cx = m_rcF.right - m_rcF.left - fAlbumLevel * 2;
        cy = m_rcF.bottom - m_rcF.top - fAlbumLevel * 2;
        fRadius = std::min(cx / 2.f, cy / 2.f);
        float xStart = m_rcF.left + fAlbumLevel + cx / 2.f - fRadius, yStart = m_rcF.top + fAlbumLevel + cy / 2.f - fRadius;
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
    const auto fAlbumLevel = m_pBK->m_DsF.sizeAlbumLevel;
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
        fRadius -= fAlbumLevel;
        if (dwLevel != -1)
        {
            fOffset = ((float)(LOWORD(dwLevel) + HIWORD(dwLevel)) / 2.f) / 32768.f * fAlbumLevel;
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

        fRadius -= fAlbumLevel;
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
            CUIElem::Redraw(TRUE);
        }
    }
    break;
    }
    return lResult;
}

BOOL CUIAlbumRotating::InitElem()
{
    eck::SafeRelease(m_pBrUV);
    eck::SafeRelease(m_pBrUV2);
    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBrUV2);
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.6f), &m_pBrUV);
    return TRUE;
}