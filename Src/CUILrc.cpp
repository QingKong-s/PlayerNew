#include "CWndBK.h"


void CUILrc::ScrollProc(int iPos, int iPrevPos, LPARAM lParam)
{
	if (iPos == iPrevPos)
		return;
    auto p = (CUILrc*)lParam;

	if (!p->m_AnEnlarge.IsEnd())
	{
        p->m_fAnValue = p->m_AnEnlarge.Tick(30);
	}
	else
	{
		//m_idxCurrAnItem = m_idxPrevCurr = -1;
        p->m_bEnlarging = FALSE;
	}

    p->CalcTopItem();
    p->InvalidateRect();
}

LRESULT CUILrc::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        Dui::ELEMPAINTSTRU ps;
        BeginPaint(ps, wParam, lParam);

        if (!m_vItem.empty())
        {
            float y;
            for (int i = m_idxTop; i < (int)m_vItem.size(); ++i)
            {
                DrawItem(i, y);
                if (y > GetViewHeightF())
                    break;
            }
        }
        BkDbg_DrawElemFrame();

        EndPaint(ps);
    }
    return 0;

	case WM_MOUSEWHEEL:
	{
		BeginMouseIdleDetect();
		m_ScrollView.OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA, ScrollProc, (LPARAM)this);
	}
	return 0;

    case WM_LBUTTONDOWN:
        SetFocus();
        return 0;

    case UIEE_ONPLAYINGCTRL:
    {
        if (wParam == PCT_PLAYNEW)
        {
            m_idxPrevCurr = -1;
            constexpr float fScale = 1.4f;

            auto& Player = App->GetPlayer();
            const auto& vLrc = Player.GetLrc();
            const float cx = GetViewWidthF(), cy = GetViewHeightF();

            m_vItem.clear();
            m_vItem.resize(vLrc.size());
            const float cxMax = cx / fScale;
            IDWriteTextLayout* pLayout;
            DWRITE_TEXT_METRICS Metrics;
            float y = 0.f;
            const float cyPadding = GetBk()->Dpi(App->GetOptionsMgr().cyLrcPadding);
            EckCounter(vLrc.size(), i)
            {
                App->m_pDwFactory->CreateTextLayout(vLrc[i].pszLrc, vLrc[i].cchTotal,
                    m_pTextFormat, cxMax, (float)cy, &pLayout);
                pLayout->GetMetrics(&Metrics);
                m_vItem[i].y = y;
                m_vItem[i].cy = Metrics.height;
                m_vItem[i].pLayout = pLayout;

                y += (Metrics.height + cyPadding);
            }
			m_ScrollView.SetMin(-cy / 2.f);
			m_ScrollView.SetMax(y - cyPadding + cy / 2.f);
            m_ScrollView.SetPos(m_ScrollView.GetMin());
            m_ScrollView.SetPage(cy);
            CalcTopItem();
            InvalidateRect();
        }
    }
    return 0;

    case WM_CREATE:
    {
        eck::SafeRelease(m_pBrTextNormal);
        eck::SafeRelease(m_pBrTextHighlight);
        eck::SafeRelease(m_pTextFormat);

        //m_pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrTextNormal);
        //m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pBrTextHighlight);

        m_pDC->CreateSolidColorBrush(eck::ColorrefToD2dColorF(eck::Colorref::DeepGray), &m_pBrTextNormal);
        m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBrTextHighlight);

        const auto& Font = App->GetOptionsMgr().LrcFont;
        App->m_pDwFactory->CreateTextFormat(Font.rsFontName.Data(), NULL,
            (DWRITE_FONT_WEIGHT)Font.iWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
            GetBk()->Dpi(Font.fFontSize), L"zh-cn", &m_pTextFormat);

        m_hEventSV = CreateEventW(NULL, TRUE, FALSE, NULL);
        m_ScrollView.SetHWND(GetWnd()->GetHWND());

        GetBk()->RegisterTimerElem(this);
    }
    break;

    case WM_DESTROY:
    {
        eck::SafeRelease(m_pBrTextNormal);
        eck::SafeRelease(m_pBrTextHighlight);
        eck::SafeRelease(m_pTextFormat);
    }
    break;
    }
    return FALSE;
}

void CUILrc::OnTimer(UINT uTimerID)
{
    switch (uTimerID)
    {
    case CWndBK::IDT_PGS:
    {
        BOOL bSwitchLrc = FALSE;
        if (m_idxPrevCurr != App->GetPlayer().GetCurrLrc())// 进度已更新
        {
			const int idxPrev = m_idxPrevCurr;
			m_idxPrevCurr = App->GetPlayer().GetCurrLrc();
			D2D1_RECT_F rcF;
			if (idxPrev >= 0)
			{
                DrawItem(idxPrev, rcF);
			}

			if (m_idxPrevCurr >= 0)
			{
                DrawItem(m_idxPrevCurr, rcF);
			}

			if (m_tMouseIdle <= 0)
			{
				m_bEnlarging = TRUE;
                m_idxPrevAnItem = idxPrev;
                m_idxCurrAnItem = m_idxPrevCurr;
				m_AnEnlarge.Begin(1.f, 0.4f, 400);
				const auto& CurrItem = m_vItem[m_idxPrevCurr];
				float yDest = CurrItem.y + CurrItem.cy * 1.4f / 2.f;
				m_ScrollView.InterruptAnimation();
				m_ScrollView.SmoothScrollDelta((yDest - GetViewHeight() / 3) - m_ScrollView.GetPos(), 
                    ScrollProc, (LPARAM)this);
			}
			return;
		}

        if (m_bEnlarging)
        {
            
        }
	}
    return;
    case IDT_MOUSEIDLE:
    {
        m_tMouseIdle -= TE_MOUSEIDLE;
        if (m_tMouseIdle <= 0)
        {
			m_tMouseIdle = 0;
			KillTimer(GetWnd()->HWnd, IDT_MOUSEIDLE);
        }
    }
    return;
    }
}