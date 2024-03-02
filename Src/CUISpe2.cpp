#include "CWndBK.h"

LRESULT CUISpe2::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
    case WM_PAINT:
    {
        Dui::ELEMPAINTSTRU ps;
        BeginPaint(ps, wParam, lParam);

        App->GetPlayer().GetBass().GetData(m_pfBuf, (DWORD)m_cbBuf);// 取频谱数据

        const float cx = GetViewWidthF();
        const float cy = GetViewHeightF();

        D2D1_POINT_2F PtF1, PtF2;
        int k, l;
        for (int j = 0; j < 2; ++j)
        {
            for (int i = 0; i < m_cSample - 1; ++i)
            {
                k = (int)((1 - m_pfBuf[i * 2 + j]) * cy / 2);
                if (k < 0)
                    k = 0;
                else if (k > cy)
                    k = cy;
                PtF1 = { i * m_cxStep,(float)k };

                l = (int)((1 - m_pfBuf[(i + 1) * 2 + j]) * cy / 2);
                if (l < 0)
                    l = 0;
                else if (l > cy)
                    l = cy;
                PtF2 = { (i + 1) * m_cxStep,(float)l };
                m_pDC->DrawLine(PtF1, PtF2, m_pBrLine, GetBk()->m_DsF.cxSepLine);
            }
        }

        BkDbg_DrawElemFrame();

        EndPaint(ps);
    }
    return 0;

    case WM_SIZE:
    {
        if (m_cSample > 1)
            m_cxStep = GetViewWidthF() / (float)(m_cSample - 1);
    }
    break;

	case WM_CREATE:
	{
		eck::SafeRelease(m_pBrLine);
		m_pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrLine);
	}
	break;

	case WM_DESTROY:
	{
		eck::SafeRelease(m_pBrLine);
	}
	break;
	}

    return __super::OnEvent(uMsg, wParam, lParam);
}

void CUISpe2::SetSampleCount(int i)
{
    delete[] m_pfBuf;
    if (i > 1)
    {
        m_cSample = i;
        m_cbBuf = 2 * i * sizeof(float);
        m_pfBuf = new float[m_cbBuf];
        m_cxStep = GetViewWidthF() / (float)(i - 1);
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
        InvalidateRect();
    }
}