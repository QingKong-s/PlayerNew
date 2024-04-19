#include "CWndMain.h"

LRESULT CUISpe::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        Dui::ELEMPAINTSTRU ps;
        BeginPaint(ps, wParam, lParam);

        float fData[128];
        if (App->GetPlayer().GetBass().GetData(fData, BASS_DATA_FFT256) == -1)
        {
            ZeroMemory(m_piOldHeight, m_cbPerUnit);
            ZeroMemory(m_piHeight, m_cbPerUnit);
            ZeroMemory(fData, sizeof(fData));
        }

        const float cx = GetViewWidthF();
        const float cy = GetViewHeightF();

        for (int i = 0; i < m_iCount; i++)
        {
            ++m_piTime[i];
            m_piHeight[i] = abs((int)(fData[i] * cy * 2.f));
            //////////////////频谱条
            if (m_piHeight[i] > cy)// 超高
                m_piHeight[i] = (int)cy;// 回来
#define SPESTEP_BAR					7
#define SPESTEP_MAX					11
            if (m_piHeight[i] > m_piOldHeight[i])// 当前的大于先前的
                m_piOldHeight[i] = m_piHeight[i];// 顶上去
            else
                m_piOldHeight[i] -= SPESTEP_BAR;// 如果不大于就继续落

            if (m_piOldHeight[i] < 3)// 太低了
                m_piOldHeight[i] = 3;// 回来

            //////////////////峰值
            if (m_piTime[i] > 10)// 时间已到
                m_piOldMaxPos[i] -= SPESTEP_MAX;// 下落

            if (m_piOldHeight[i] > m_piOldMaxPos[i])// 频谱条大于峰值
            {
                m_piOldMaxPos[i] = m_piOldHeight[i];// 峰值顶上去，重置时间
                m_piTime[i] = 0;
            }

            if (m_piOldMaxPos[i] < 3)// 太低了
                m_piOldMaxPos[i] = 3;// 回来
            //////////////////绘制
            //////////频谱条
            D2D_RECT_F D2DRectF;
            D2DRectF.left = (FLOAT)((m_cxBar + m_cxGap) * i);
            D2DRectF.top = (FLOAT)(cy - m_piOldHeight[i]);
            D2DRectF.right = D2DRectF.left + m_cxBar;
            D2DRectF.bottom = (FLOAT)(cy);
            if (D2DRectF.right > cx)
                break;
            m_pDC->FillRectangle(&D2DRectF, m_pBrBar);

            //////////峰值指示
            D2DRectF.top = (FLOAT)(cy - m_piOldMaxPos[i]);
            D2DRectF.bottom = D2DRectF.top + 3;
            m_pDC->FillRectangle(&D2DRectF, m_pBrBar);
        }

        BkDbg_DrawElemFrame();

        EndPaint(ps);
    }
    return 0;

	case WM_SIZE:
    {
		if (m_iCount)
			m_cxBar = (GetViewWidthF() - (m_iCount - 1) * m_cxGap) / m_iCount;
	}
	break;

    case UIEE_DWMCOLORCHANGED:
    {
        ECK_DUILOCK;
        m_pBrBar->SetColor(App->GetMainWnd()->GetDwmColor());
    }
    break;

    case WM_CREATE:
    {
        SafeRelease(m_pBrBar);
        m_pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrBar);
    }
    break;

    case WM_DESTROY:
    {
		SafeRelease(m_pBrBar);
	}
    break;
	}
	return __super::OnEvent(uMsg, wParam, lParam);
}

void CUISpe::SetCount(int i)
{
    ECK_DUILOCK;
    if (i > 0)
    {
        // c * cxBar + (c - 1) * cxDiv = cx
        // 解得：cxBar = (cx - (c - 1) * cxDiv) / c
        m_iCount = i;
        m_cxBar = (GetViewWidthF() - (i - 1) * m_cxGap) / i;
        if (m_cxBar <= 0)
            goto Fail;
        m_cbPerUnit = sizeof(int) * i;
        m_vBuf.resize(i * 4);
        m_piOldHeight = m_vBuf.data();
        m_piHeight = m_vBuf.data() + i;
        m_piOldMaxPos = m_vBuf.data() + i * 2;
        m_piTime = m_vBuf.data() + i * 3;
        return;
    }
Fail:
    m_iCount = 0;
    m_cxBar = 0.f;
    m_cbPerUnit = 0;
    m_vBuf.clear();
    m_piOldHeight = m_piHeight = m_piOldMaxPos = m_piTime = NULL;
}