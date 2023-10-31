#include "CWndBK.h"

BOOL CUISpe2::InitElem()
{
    SAFE_RELEASE(m_pBrLine);
    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrLine);
    return TRUE;
}

CUISpe2::CUISpe2()
{
    m_uType = UIET_SPE2;
    m_uFlags = UIEF_NOEVENT | UIEF_WANTTIMEREVENT | UIEF_ONLYPAINTONTIMER;
}

CUISpe2::~CUISpe2()
{
    SAFE_RELEASE(m_pBrLine);
}

void CUISpe2::Redraw()
{
    auto pDC = m_pBK->m_pDC;
    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);// 刷背景

    App->GetPlayer().GetBass().GetData(m_pfBuf, (DWORD)m_cbBuf);// 取频谱数据

    D2D1_POINT_2F PtF1, PtF2;
    int k, l;
    for (int j = 0; j < 2; ++j)
    {
        for (int i = 0; i < m_cSample - 1; ++i)
        {
            k = (int)((1 - m_pfBuf[i * 2 + j]) * m_cy / 2);// 直接从Bass示例里狠狠地抄
            if (k < 0)
                k = 0;
            else if (k > m_cy)
                k = m_cy;
            PtF1 = { m_rcF.left + i * m_cxStep,m_rcF.top + k };

            l = (int)((1 - m_pfBuf[(i + 1) * 2 + j]) * m_cy / 2);
            if (l < 0)
                l = 0;
            else if (l > m_cy)
                l = m_cy;
            PtF2 = { m_rcF.left + (i + 1) * m_cxStep,m_rcF.top + l };
            pDC->DrawLine(PtF1, PtF2, m_pBrLine, m_pBK->m_DsF.cxSepLine);
        }
    }

    BkDbg_DrawElemFrame();
}

void CUISpe2::SetSampleCount(int i)
{
    delete[] m_pfBuf;
    if (i > 1)
    {
        m_cSample = i;
        m_cbBuf = 2 * i * sizeof(float);
        m_pfBuf = new float[m_cbBuf];
        m_cxStep = (float)m_cx / (float)(i - 1);
    }
    else
    {
        m_cSample = 0;
        m_cbBuf = 0;
        m_pfBuf = NULL;
        m_cxStep = 0.f;
        EckDbgPrint(L"样本数不正确");
        EckDbgBreak();
    }
}

void CUISpe2::OnTimer(UINT uTimerID)
{
    if (uTimerID == CWndBK::IDT_PGS)
    {
        Redraw();
    }
}

LRESULT CUISpe2::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    auto lResult = DefElemEventProc(uEvent, wParam, lParam);
    switch (uEvent)
    {
    case UIEE_SETRECT:
    {
        if (m_cSample > 1)
            m_cxStep = (float)m_cx / (float)(m_cSample - 1);
    }
    break;
    }
    return lResult;
}