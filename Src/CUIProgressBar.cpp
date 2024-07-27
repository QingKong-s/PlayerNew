#include "pch.h"
#include "CWndBK.h"
#include "CWndMain.h"

LRESULT CUIProgressBar::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		GetBk()->RegisterTimerElem(this);
		m_TrackBar.Create(NULL, Dui::DES_TRANSPARENT | Dui::DES_VISIBLE, 0,
			0, 0, GetViewWidth(), GetViewHeight(), this, GetWnd(), IDE_TB_PROGRESS);
		m_TrackBar.SetTrackSize(20.f);
	}
	break;
	case WM_SIZE:
		m_TrackBar.SetRect({ 0,0,GetViewWidth(),GetViewHeight() });
		break;
	case WM_PAINT:
	{
		Dui::ELEMPAINTSTRU ps;
		BeginPaint(ps, wParam, lParam);
		BkDbg_DrawElemFrame();
		EndPaint(ps);
	}
	return 0;

	case UIEE_ONPLAYINGCTRL:
	{
		if (wParam == PCT_PLAYNEW)
			m_TrackBar.SetRange(0.f, App->GetPlayer().GetLength() / 1000.f);
	}
	return 0;

	case UIEE_DWMCOLORCHANGED:
	{
		auto cr = m_TrackBar.GetColorTheme()->Get();
		cr.crBkSelected = App->GetMainWnd()->GetDwmColor();
		auto pTheme = new Dui::CColorTheme(cr);
		m_TrackBar.SetColorTheme(pTheme);
		pTheme->DeRef();
	}
	return 0;
	}
	return __super::OnEvent(uMsg, wParam, lParam);
}

void CUIProgressBar::OnTimer(UINT uTimerID)
{
	if (uTimerID == CWndBK::IDT_PGS)
	{
		const auto f = App->GetPlayer().GetPosF();
		if (fabs(f - m_TrackBar.GetPos()) > 0.5f)
		{
			m_TrackBar.SetPos(f);
			m_TrackBar.InvalidateRect();
		}
	}
}