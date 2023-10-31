#include "CWndBK.h"

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
    rcF.top = m_rcF.top + (m_cy - m_pBK->m_DsF.cyProgBarTrack) / 2.f;
    rcF.bottom = rcF.top + m_pBK->m_DsF.cyProgBarTrack;
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