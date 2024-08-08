#include "pch.h"
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

        if (GetStyle() & Dui::DES_COMPOSITED)
            m_pDC->Clear({});
        if (!pBk->m_pBmpAlbum)
        {
            BkDbg_DrawElemFrame();
            EndPaint(ps);
            return 0;
        }

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

        m_pDC->DrawBitmap(pBk->m_pBmpAlbum, &rcF);

        BkDbg_DrawElemFrame();
        EndPaint(ps);
    }
    return 0;

    case WM_SIZE:
    {
        ECK_DUILOCK;
        const auto dx = 10 * 6;
        const auto dy = dx;

        SetPostCompositedRect({ -dx / 2, -dy / 2,GetWidth()+ dx / 2,GetHeight()+ dy / 2 });
    }
    break;
    }

    return __super::OnEvent(uMsg, wParam, lParam);
}

LRESULT CUIAlbum::OnComposite(const RECT& rcClip, float ox, float oy)
{
    ID2D1Effect* pFx;
    m_pDC->CreateEffect(CLSID_D2D1Shadow, &pFx);
    pFx->SetInput(0, m_pBitmapComp);
    pFx->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, 40.0f);
    pFx->SetValue(D2D1_SHADOW_PROP_COLOR, D2D1::ColorF(1.f, 0.0f, 0.0f, 0.5f));
    m_pDC->DrawImage(pFx);
    pFx->Release();

    const D2D1_RECT_F rcF{ ox,oy,ox + GetWidthF(),oy + GetHeightF() };
    m_pDC->DrawBitmap(m_pBitmapComp, &rcF);
    return 0;
}
