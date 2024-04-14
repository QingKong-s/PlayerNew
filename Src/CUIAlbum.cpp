#include "CWndBK.h"
#include <DirectXMath.h>

LRESULT CUIAlbum::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        Dui::ELEMPAINTSTRU ps;
		BeginPaint(ps, wParam, lParam);

        const auto pBk = (CWndBK*)GetWnd();

        if (!pBk->m_pBmpAlbum)
        {
            BkDbg_DrawElemFrame();
            EndPaint(ps);
            return 0;
        }

        const auto pDC = GetD2DDC();

        float f;
        const int cx0 = pBk->m_cxAlbum, cy0 = pBk->m_cyAlbum;
        D2D1_RECT_F rcF;

        const float cxElem = GetViewWidthF();
        const float cyElem = GetViewHeightF();

        if ((float)cxElem / (float)cyElem > (float)cx0 / (float)cy0)// y对齐
        {
            f = cx0 * cyElem / cy0;
            rcF.left = (cxElem - f) / 2;
            rcF.right = rcF.left + f;
            rcF.top = 0;
            rcF.bottom = rcF.top + cyElem;
        }
        else// x对齐
        {
            f = cxElem * cy0 / cx0;
            rcF.left = 0;
            rcF.right = rcF.left + cxElem;
            rcF.top = (cyElem - f) / 2;
            rcF.bottom = rcF.top + (FLOAT)f;
        }

        pDC->DrawBitmap(pBk->m_pBmpAlbum, &rcF);

        BkDbg_DrawElemFrame();
        EndPaint(ps);
    }
    return 0;
    }

    return __super::OnEvent(uMsg, wParam, lParam);
}