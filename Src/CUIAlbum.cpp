#include "CWndBK.h"
#include <DirectXMath.h>

CUIAlbum::CUIAlbum()
{
    m_uType = UIET_ALBUM;
    m_uFlags = UIEF_STATIC;
}

void CUIAlbum::Redraw()
{
    if (!m_pBK->m_pBmpAlbum)
        return;

    auto pDC = m_pBK->m_pDC;

    int cxRgn = m_rc.right - m_rc.left,
        cyRgn = m_rc.bottom - m_rc.top;

    int iSize;
    int cx0 = m_pBK->m_cxAlbum, cy0 = m_pBK->m_cyAlbum;
    D2D1_RECT_F rcF;

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