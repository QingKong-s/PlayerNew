#include "CWndBK.h"

LRESULT CUIVolTrackBar::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		Dui::ELEMPAINTSTRU ps;
		BeginPaint(ps, wParam, lParam, Dui::EBPF_DO_NOT_FILLBK);

		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 0.6f));
		m_pDC->FillRectangle(GetViewRectF(), m_pBrush);
		m_pDC->Flush();
		eck::BlurD2dDC(m_pDC, m_pWnd->GetD2D().GetBitmap(), m_pWnd->m_pBmpBlurCache,
			ps.rcfClip, {}, 10.f);

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
			case Dui::ENM_KILLFOCUS:
				SetVisible(FALSE);
			}
		}
	}
	break;
	case WM_SIZE:
		m_TrackBar.SetSize(GetViewWidth(), GetViewHeight());
		break;
	case WM_CREATE:
	{
		m_TrackBar.Create(NULL, Dui::DES_VISIBLE, 0,
			0, 0, GetViewWidth(), GetViewHeight(),
			this, GetWnd(), IDE_TB_VOL);
		m_TrackBar.SetRange(0.f, 200.f);
		m_TrackBar.SetTrackSize(40);
		m_TrackBar.SetGenEventWhenDragging(TRUE);
		m_TrackBar.SetFocus();

		m_pDC->CreateSolidColorBrush({}, &m_pBrush);
	}
	break;
	case WM_DESTROY:
		SafeRelease(m_pBrush);
		break;
	}
	return __super::OnEvent(uMsg, wParam, lParam);
}