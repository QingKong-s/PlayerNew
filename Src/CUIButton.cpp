#include "CWndBK.h"

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