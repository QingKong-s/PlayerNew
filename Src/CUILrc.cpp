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
    p->CUIElem::Redraw(TRUE);
}

CUILrc::CUILrc()
{
    m_uType = UIET_LRC;
    m_uFlags = UIEF_WANTTIMEREVENT;
}

CUILrc::~CUILrc()
{
    eck::SafeRelease(m_pBrTextNormal);
    eck::SafeRelease(m_pBrTextHighlight);
    eck::SafeRelease(m_pTextFormat);
}

BOOL CUILrc::InitElem()
{
    eck::SafeRelease(m_pBrTextNormal);
    eck::SafeRelease(m_pBrTextHighlight);
    eck::SafeRelease(m_pTextFormat);

    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrTextNormal);
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pBrTextHighlight);

    const auto& Font = App->GetOptionsMgr().LrcFont;
    App->m_pDwFactory->CreateTextFormat(Font.rsFontName.Data(), NULL,
        (DWRITE_FONT_WEIGHT)Font.iWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		m_pBK->Dpi(Font.fFontSize), L"zh-cn", &m_pTextFormat);

    m_hEventSV = CreateEventW(NULL, TRUE, FALSE, NULL);
    m_ScrollView.SetHWND(m_pBK->GetHWND());
    return TRUE;
}

void CUILrc::Redraw()
{
    auto pDC = m_pBK->m_pDC;
    if (!m_vItem.size())
        return;
    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);
    pDC->PushAxisAlignedClip(&m_rcF, D2D1_ANTIALIAS_MODE_ALIASED);
    float y;
    for (int i = m_idxTop; i < (int)m_vItem.size(); ++i)
    {
        DrawItem(i, y);
        if (y > m_rcF.bottom)
            break;
    }
    pDC->PopAxisAlignedClip();
    BkDbg_DrawElemFrame();
}

BOOL CUILrc::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const HWND hWnd = m_pBK->m_hWnd;
    switch (uMsg)
    {
    case WM_MOUSEWHEEL:
    {
        POINT pt ECK_GET_PT_LPARAM(lParam);
        ScreenToClient(hWnd, &pt);
        if (PtInRect(&m_rc, pt))
        {
            BeginMouseIdleDetect();
            m_ScrollView.OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA, ScrollProc, (LPARAM)this);
            return TRUE;
        }
	}
	return FALSE;
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
				if (DrawItem(idxPrev, rcF))
					m_pBK->m_vDirtyRect.emplace_back(eck::MakeRect(rcF));
			}

			if (m_idxPrevCurr >= 0)
			{
				if (DrawItem(m_idxPrevCurr, rcF))
					m_pBK->m_vDirtyRect.emplace_back(eck::MakeRect(rcF));
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
				m_ScrollView.SmoothScrollDelta((yDest - m_cy / 3) - m_ScrollView.GetPos(), 
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
			KillTimer(m_pBK->HWnd, IDT_MOUSEIDLE);
        }
    }
    return;
    }
}

LRESULT CUILrc::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
	auto lResult = DefElemEventProc(uEvent, wParam, lParam);
	switch (uEvent)
	{
	case UIEE_SETRECT:
	{

	}
	break;
	case UIEE_ONPLAYINGCTRL:
	{
        if (wParam == PCT_PLAY)
        {
            m_idxPrevCurr = -1;
            constexpr float fScale = 1.4f;
            auto& Player = App->GetPlayer();
            const auto& vLrc = Player.GetLrc();
            m_vItem.clear();
            m_vItem.resize(vLrc.size());
            const float cxMax = m_cx / fScale;
            IDWriteTextLayout* pLayout;
            DWRITE_TEXT_METRICS Metrics;
            float y = 0.f;
            const float cyPadding = m_pBK->Dpi(App->GetOptionsMgr().cyLrcPadding);
            EckCounter(vLrc.size(), i)
            {
                App->m_pDwFactory->CreateTextLayout(vLrc[i].pszLrc, vLrc[i].cchTotal,
                    m_pTextFormat, cxMax, (float)m_cy, &pLayout);
                pLayout->GetMetrics(&Metrics);
                m_vItem[i].y = y;
                m_vItem[i].cy = Metrics.height;
                m_vItem[i].pLayout = pLayout;

                y += (Metrics.height + cyPadding);
            }
            m_ScrollView.SetMin(-m_cyHalf);
			m_ScrollView.SetMax(y - cyPadding + m_cyHalf);
            m_ScrollView.SetPos(m_ScrollView.GetMin());
            m_ScrollView.SetPage(m_cy);
            CalcTopItem();
            CUIElem::Redraw();
        }
	}
	break;
	}
	return lResult;
}