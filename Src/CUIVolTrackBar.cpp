#include "pch.h"
#include "CWndBK.h"

LRESULT CUIVolTrackBar::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		Dui::ELEMPAINTSTRU ps;
		BeginPaint(ps, wParam, lParam);

		m_pDC->Clear({});

		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 0.6f));
		m_pDC->FillRectangle(GetViewRectF(), m_pBrush);

		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Gray, 0.9f));
		m_pDC->DrawRectangle(GetViewRectF(), m_pBrush);

		EndPaint(ps);
	}
	return 0;
	case WM_NOTIFY:
	{
		auto pElem = (Dui::CElem*)wParam;
		if (pElem == &m_TrackBar)
		{
			switch (((Dui::DUINMHDR*)lParam)->uCode)
			{
			case Dui::EE_KILLFOCUS:
				SetVisible(FALSE);
			}
		}
	}
	break;
	case WM_SIZE:
		SetPostCompositedRect({ -120, -120,GetViewWidth() + 120, GetViewHeight() + 120 });
		m_TrackBar.SetSize(GetViewWidth(), GetViewHeight());
		break;
	case WM_CREATE:
	{
		m_TrackBar.Create(NULL, Dui::DES_VISIBLE, 0,
			0, 0, GetViewWidth(), GetViewHeight(),
			this, GetWnd(), IDE_TB_VOL);
		m_TrackBar.SetRange(0.f, 200.f);
		m_TrackBar.SetTrackSize((float)GetBk()->GetDpiSize().cyVolTrack);
		m_TrackBar.SetGenEventWhenDragging(TRUE);
		m_TrackBar.SetFocus();

		m_pDC->CreateSolidColorBrush({}, &m_pBrush);

		m_yBegin = GetRect().top;
		m_pec = new eck::CEasingCurve{};
		m_pec->SetRange(0.f, 1.f);
		m_pec->SetDuration(400);
		m_pec->SetCallBack([](float fCurrValue, float fOldValue, LPARAM lParam)
			{
				const auto p = (CUIVolTrackBar*)lParam;
				const auto rc = p->GetRect();
				p->SetPos(rc.left, p->m_yBegin - int(fCurrValue * 40));
			});
		InitEasingCurve(m_pec);
		m_pec->Begin();
	}
	break;
	case WM_DESTROY:
		SafeRelease(m_pBrush);
		GetWnd()->UnregisterTimeLine(m_pec);
		SafeRelease(m_pec);
		break;
	}
	return __super::OnEvent(uMsg, wParam, lParam);
}

LRESULT CUIVolTrackBar::OnComposite(const RECT& rcClip, float ox, float oy)
{
	ID2D1Effect* pFx;
	m_pDC->CreateEffect(CLSID_D2D1Shadow, &pFx);
	pFx->SetInput(0, m_pBitmapComp);
	pFx->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, 40.0f);
	pFx->SetValue(D2D1_SHADOW_PROP_COLOR, D2D1::ColorF(1.f, 0.0f, 0.0f, 0.5f));
	m_pDC->DrawImage(pFx);
	pFx->Release();

	m_pDC->DrawBitmap(m_pBitmapComp, { ox,oy,ox + GetWidthF(),oy + GetHeightF() },
		m_pec->GetCurrValue(), D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	return 0;
}
