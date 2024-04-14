#include "CWndBK.h"

void CUILrc::ScrollProc(int iPos, int iPrevPos, LPARAM lParam)
{
	if (iPos == iPrevPos)
		return;
	auto p = (CUILrc*)lParam;

	if (!p->m_AnEnlarge.IsEnd())
		p->m_fAnValue = p->m_AnEnlarge.Tick((float)p->m_psv->GetCurrTickInterval());
	else
		p->m_bEnlarging = FALSE;

	p->CalcTopItem();
	p->InvalidateRect();
}

void CUILrc::CalcTopItem()
{
	const auto fScale = App->GetOptionsMgr().LrcCurrFontScale;
	auto it = LowerBound(m_vItem.begin(), m_vItem.end(), m_psv->GetPos(),
		[this, fScale](decltype(m_vItem)::iterator it, int iPos)
		{
			const auto& e = *it;
			float y = e.y;
			const int idx = (int)std::distance(m_vItem.begin(), it);
			if (idx > m_idxPrevAnItem && m_idxPrevAnItem >= 0)
				y += (m_vItem[m_idxPrevAnItem].cy * (fScale - m_fAnValue));
			if (idx > m_idxCurrAnItem && m_idxCurrAnItem >= 0)
				y += (m_vItem[m_idxCurrAnItem].cy * (m_fAnValue - 1.f));
			return y < iPos;
		});
	EckAssert(it != m_vItem.end());

	if (it == m_vItem.begin())
	{
		m_idxTop = 0;
		return;
	}
	--it;
	m_idxTop = (int)std::distance(m_vItem.begin(), it);
}

BOOL CUILrc::DrawItem(int idx, float& y)
{
	EckAssert(idx >= 0 && idx < (int)m_vItem.size());
	auto& Item = m_vItem[idx];
	const auto fScale = App->GetOptionsMgr().LrcCurrFontScale;

	y = GetItemY(idx);
	const D2D1_RECT_F rc{ 0,y,Item.cx,y + Item.cy };

	D2D1_MATRIX_3X2_F mat0;
	m_pDC->GetTransform(&mat0);
	auto mat = mat0 * D2D1::Matrix3x2F::Translation(0, y);

	if (!Item.bCacheValid)
	{
		ID2D1PathGeometry* pPathGeometry;
		eck::GetTextLayoutPathGeometry(Item.pLayout, m_pDC, 0.f, 0.f, pPathGeometry);
		float xDpi, yDpi;
		m_pDC->GetDpi(&xDpi, &yDpi);

		ID2D1GeometryRealization* pGr = NULL;
		m_pDC1->CreateFilledGeometryRealization(
			pPathGeometry,
			D2D1::ComputeFlatteningTolerance(
				D2D1::Matrix3x2F::Identity(), xDpi, yDpi, fScale),
			&pGr);

		if (Item.pGr)
			Item.pGr->Release();
		Item.pGr = pGr;
		pPathGeometry->Release();
		Item.bCacheValid = TRUE;
	}

	if (idx == m_idxPrevAnItem)
	{
		const auto matScale = D2D1::Matrix3x2F::Scale(
			fScale + 1.f - m_fAnValue, fScale + 1.f - m_fAnValue);
		if (Item.bSel || idx == m_idxHot)
		{
			m_pDC->SetTransform(D2D1::Matrix3x2F::Scale(
				fScale + 1.f - m_fAnValue,
				fScale + 1.f - m_fAnValue,
				{ 0,y }) * mat0);
			FillItemBkg(idx, rc);
		}
		m_pDC->SetTransform(D2D1::Matrix3x2F::Scale(
			fScale + 1.f - m_fAnValue,
			fScale + 1.f - m_fAnValue) * mat);
		m_pDC1->DrawGeometryRealization(Item.pGr,
			m_idxPrevCurr == idx ? m_pBrTextHighlight : m_pBrTextNormal);
		m_pDC->SetTransform(mat0);
		return TRUE;
	}

	if (idx == m_idxCurrAnItem)
	{
		const auto matScale = D2D1::Matrix3x2F::Scale(
			m_fAnValue, m_fAnValue);
		if (Item.bSel || idx == m_idxHot)
		{
			m_pDC->SetTransform(D2D1::Matrix3x2F::Scale(
				m_fAnValue,
				m_fAnValue,
				{ 0,y }) * mat0);
			FillItemBkg(idx, rc);
		}
		m_pDC->SetTransform(D2D1::Matrix3x2F::Scale(
			m_fAnValue,
			m_fAnValue) * mat);
		m_pDC1->DrawGeometryRealization(Item.pGr,
			m_idxPrevCurr == idx ? m_pBrTextHighlight : m_pBrTextNormal);

		m_pDC->SetTransform(mat0);
		return TRUE;
	}

	FillItemBkg(idx, rc);
	m_pDC->SetTransform(mat);
	m_pDC1->DrawGeometryRealization(Item.pGr,
		m_idxPrevCurr == idx ? m_pBrTextHighlight : m_pBrTextNormal);
	m_pDC->SetTransform(mat0);
	return TRUE;
}

int CUILrc::HitTest(POINT pt)
{
	RECT rc;
	for (int i = m_idxTop; i < (int)m_vItem.size(); ++i)
	{
		GetItemRect(i, rc);
		if (eck::PtInRect(rc, pt))
			return i;
		if (rc.top > GetViewHeight())
			break;
	}
	return -1;
}

LRESULT CUILrc::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ECK_DUILOCK;
	switch (uMsg)
	{
	case WM_PAINT:
	{
		Dui::ELEMPAINTSTRU ps;
		BeginPaint(ps, wParam, lParam);

		if (!m_vItem.empty())
		{
			m_pBrush->SetColor(eck::ColorrefToD2dColorF(eck::Colorref::CyanBlue, 0.5f));
			float y;
			for (int i = m_idxTop; i < (int)m_vItem.size(); ++i)
			{
				DrawItem(i, y);
				if (y > GetViewHeightF())
					break;
			}
			//DrawScrollBar();
		}
		else
		{

		}

		BkDbg_DrawElemFrame();

		EndPaint(ps);
	}
	return 0;

	case WM_MOUSEMOVE:
	{
		if (m_vItem.empty())
			break;
		POINT pt ECK_GET_PT_LPARAM(lParam);
		ClientToElem(pt);

		if (m_bThumbLBtnDown)
		{
			m_psv->OnMouseMove(pt.y);
			CalcTopItem();
			InvalidateRect();
		}
		else
		{
			int idx = HitTest(pt);
			if (idx != m_idxHot)
			{
				std::swap(idx, m_idxHot);
				SetRedraw(FALSE);
				if (idx >= 0)
					InvalidateItem(idx);
				if (m_idxHot >= 0)
					InvalidateItem(m_idxHot);
				SetRedraw(TRUE);
			}
		}
	}
	return 0;

	case WM_MOUSELEAVE:
	{
		if (m_vItem.empty())
			break;
		int idx = -1;
		if (idx != m_idxHot)
		{
			std::swap(idx, m_idxHot);
			if (idx >= 0)
				InvalidateItem(idx);
		}
	}
	return 0;

	case WM_MOUSEWHEEL:
	{
		if (m_vItem.empty())
			break;
		BeginMouseIdleDetect();
		m_psv->OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
		GetWnd()->WakeRenderThread();
	}
	return 0;

	case WM_LBUTTONDOWN:
	{
		SetFocus();
		if (m_vItem.empty())
			break;
		POINT pt ECK_GET_PT_LPARAM(lParam);
		ClientToElem(pt);

		RECT rcThumb;
		GetSBThumbRect(rcThumb);
		if (eck::PtInRect(rcThumb, pt))
		{
			m_bThumbLBtnDown = TRUE;
			SetCapture();
			m_psv->OnLButtonDown(pt.y);
		}
		else
		{
			int idx = HitTest(pt);
			SetRedraw(FALSE);
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				if (idx >= 0)
					m_idxMark = idx;
			}
			else if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
			{
				if (m_idxMark < 0 || idx < 0)
				{
					SetRedraw(TRUE);
					break;
				}
				const int idx0 = std::min(m_idxMark, idx);
				const int idx1 = std::max(m_idxMark, idx);
				int i = 0;
				for (; i < idx0; ++i)
				{
					if (m_vItem[i].bSel)
					{
						m_vItem[i].bSel = FALSE;
						InvalidateItem(i);
					}
				}
				for (; i <= idx1; ++i)
				{
					if (!m_vItem[i].bSel)
					{
						m_vItem[i].bSel = TRUE;
						InvalidateItem(i);
					}
				}
				for (; i < (int)m_vItem.size(); ++i)
				{
					if (m_vItem[i].bSel)
					{
						m_vItem[i].bSel = FALSE;
						InvalidateItem(i);
					}
				}
				SetRedraw(TRUE);
				break;
			}
			else
			{
				if (idx >= 0)
					m_idxMark = idx;
				EckCounter((int)m_vItem.size(), i)
				{
					if (m_vItem[i].bSel)
					{
						m_vItem[i].bSel = FALSE;
						InvalidateItem(i);
					}
				}
			}

			if (idx >= 0)
				if (!m_vItem[idx].bSel)
				{
					m_vItem[idx].bSel = TRUE;
					InvalidateItem(idx);
				}
			SetRedraw(TRUE);
		}
	}
	return 0;

	case WM_RBUTTONDOWN:
	{
		if (m_vItem.empty())
			break;
		m_bCtxMenuOpen = TRUE;
		POINT pt ECK_GET_PT_LPARAM(lParam);
		POINT ptSrc{ pt };
		ClientToScreen(GetWnd()->HWnd, &ptSrc);
		ClientToElem(pt);
		const int idx = HitTest(pt);
		if (idx >= 0)
		{
			m_idxMark = idx;

		}

		const int iID = m_Menu.TrackPopupMenu(GetWnd()->HWnd, ptSrc.x, ptSrc.y, TPM_RETURNCMD | TPM_NONOTIFY);
		switch (iID)
		{
		case IDMI_PLAY_FROM_THIS:
		{
			if (idx < 0)
				break;
			auto& Player = App->GetPlayer();
			Player.GetBass().SetPosition(Player.GetLrc()[idx].fTime);
		}
		break;

		case IDMI_COPY_LRC:
		{
			if (idx < 0)
				break;
			const auto& e = App->GetPlayer().GetLrc()[idx];
			eck::SetClipboardString(e.pszLrc, e.cchTotal, GetWnd()->HWnd);
		}
		break;
		case IDMI_COPY_LRC_1:
		{
			if (idx < 0)
				break;
			const auto& e = App->GetPlayer().GetLrc()[idx];
			eck::SetClipboardString(e.pszLrc, e.cchLrc, GetWnd()->HWnd);
		}
		break;
		case IDMI_COPY_LRC_2:
		{
			if (idx < 0)
				break;
			const auto& e = App->GetPlayer().GetLrc()[idx];
			eck::SetClipboardString(e.pszTranslation, e.cchTotal - e.cchLrc - 1, GetWnd()->HWnd);
		}
		break;
		}

		m_bCtxMenuOpen = FALSE;
	}
	return 0;

	case WM_KEYDOWN:
	{
		if (m_vItem.empty())
			break;
		if (wParam == VK_ESCAPE)
		{
			m_tMouseIdle = 0;
			KillTimer(GetWnd()->HWnd, IDT_MOUSEIDLE);
			ScrollToCurrPos();
		}
	}
	return 0;

	case WM_LBUTTONUP:
	{
		if (m_vItem.empty())
			break;
		if (m_bThumbLBtnDown)
		{
			ReleaseCapture();
			m_bThumbLBtnDown = FALSE;
			m_psv->OnLButtonUp();
		}
	}
	return 0;

	case WM_NOTIFY:
	{
		if ((Dui::CElem*)wParam == &m_SB)
		{
			switch (((Dui::DUINMHDR*)lParam)->uCode)
			{
			case Dui::EE_VSCROLL:
				CalcTopItem();
				InvalidateRect();
				return TRUE;
			}
		}
	}
	return 0;

	case WM_SIZE:
	{
		RECT rc;
		rc.left = GetViewWidth() - m_SB.GetViewWidth();
		rc.top = 0;
		rc.right = rc.left + m_SB.GetViewWidth();
		rc.bottom = rc.top + GetViewHeight();
		m_SB.SetRect(rc);
	}
	break;

	case UIEE_ONPLAYINGCTRL:
	{
		if (wParam == PCT_PLAYNEW)
		{
			m_idxPrevCurr = -1;
			const auto fScale = App->GetOptionsMgr().LrcCurrFontScale;

			auto& Player = App->GetPlayer();
			const auto& vLrc = Player.GetLrc();
			const float cx = GetViewWidthF(), cy = GetViewHeightF();

			m_vItem.clear();
			if (!vLrc.empty())
			{
				m_vItem.resize(vLrc.size());
				const float cxMax = cx / fScale;
				IDWriteTextLayout* pLayout;
				DWRITE_TEXT_METRICS Metrics;
				float y = 0.f;
				const float cyPadding = GetBk()->Dpi(App->GetOptionsMgr().LrcPaddingHeight);
				EckCounter(vLrc.size(), i)
				{
					App->m_pDwFactory->CreateTextLayout(vLrc[i].pszLrc, vLrc[i].cchTotal,
						m_pTextFormat, cxMax, (float)cy, &pLayout);
					pLayout->GetMetrics(&Metrics);
					m_vItem[i].y = y;
					m_vItem[i].cx = Metrics.width;
					m_vItem[i].cy = Metrics.height;
					m_vItem[i].pLayout = pLayout;

					y += (Metrics.height + cyPadding);
				}

				m_psv->SetMin(int(-cy / 2.f));
				m_psv->SetMax(int(y - cyPadding + cy / 2.f));
				m_psv->SetPos(m_psv->GetMin());
				m_psv->SetPage((int)cy);
				CalcTopItem();
				InvalidateRect();
			}
		}
	}
	return 0;

	case WM_CREATE:
	{
		m_SB.Create(NULL, Dui::DES_VISIBLE, 0,
			0, 0, GetWnd()->GetDs().CommSBCxy, GetViewHeight(),
			this, GetWnd());

		m_pDC->QueryInterface(&m_pDC1);

		m_pDC->CreateSolidColorBrush(eck::ColorrefToD2dColorF(eck::Colorref::DeepGray), &m_pBrTextNormal);
		m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBrTextHighlight);
		m_pDC->CreateSolidColorBrush({}, &m_pBrush);
		const auto& Font = App->GetOptionsMgr().LrcFont;
		App->m_pDwFactory->CreateTextFormat(Font.rsFontName.Data(), NULL,
			(DWRITE_FONT_WEIGHT)Font.iWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
			GetBk()->Dpi(Font.fFontSize), L"zh-cn", &m_pTextFormat);

		m_psv = m_SB.GetScrollView();
		m_psv->AddRef();
		m_psv->SetMinThumbSize(GetBk()->GetDpiSize().cxyMinSBThumb);
		m_psv->SetCallBack(ScrollProc, (LPARAM)this);
		m_psv->SetDelta(GetBk()->GetDpiSize().ScrollDelta);

		GetBk()->RegisterTimerElem(this);
	}
	break;

	case WM_DESTROY:
	{
		SafeRelease(m_pBrTextNormal);
		SafeRelease(m_pBrTextHighlight);
		SafeRelease(m_pBrush);
		SafeRelease(m_pTextFormat);
		SafeRelease(m_psv);
		SafeRelease(m_pDC1);
		m_vItem.clear();
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
		if (m_vItem.empty())
			break;
		BOOL bSwitchLrc = FALSE;
		GetWnd()->GetCriticalSection()->Enter();
		if (m_idxPrevCurr != App->GetPlayer().GetCurrLrc())// 进度已更新
		{
			const int idxPrev = m_idxPrevCurr;
			m_idxPrevCurr = App->GetPlayer().GetCurrLrc();

			if (m_tMouseIdle <= 0 && !m_bThumbLBtnDown && !m_bCtxMenuOpen)
			{
				const auto fScale = App->GetOptionsMgr().LrcCurrFontScale;
				m_bEnlarging = TRUE;
				m_idxPrevAnItem = idxPrev;
				m_idxCurrAnItem = m_idxPrevCurr;
				m_AnEnlarge.Begin(1.f, fScale - 1.f, m_psv->GetDuration());
				const auto& CurrItem = m_vItem[m_idxPrevCurr];
				float yDest = CurrItem.y + CurrItem.cy * fScale / 2.f;
				GetWnd()->GetCriticalSection()->Leave();
				m_psv->InterruptAnimation();
				GetWnd()->GetCriticalSection()->Enter();
				m_psv->SmoothScrollDelta(int((yDest - GetViewHeight() / 3) - m_psv->GetPos()));
				GetWnd()->WakeRenderThread();
			}
			else
			{
				SetRedraw(FALSE);
				if (idxPrev >= 0)
					InvalidateItem(idxPrev);
				if (m_idxPrevCurr >= 0)
					InvalidateItem(m_idxPrevCurr);
				SetRedraw(TRUE);
			}
		}
		GetWnd()->GetCriticalSection()->Leave();
	}
	return;
	case IDT_MOUSEIDLE:
	{
		ECK_DUILOCK;
		if (m_vItem.empty())
			break;
		m_tMouseIdle -= TE_MOUSEIDLE;
		if (m_tMouseIdle <= 0)
		{
			m_tMouseIdle = 0;
			KillTimer(GetWnd()->HWnd, IDT_MOUSEIDLE);
			ScrollToCurrPos();
		}
	}
	return;
	}
}

void CUILrc::ScrollToCurrPos()
{
	const auto fScale = App->GetOptionsMgr().LrcCurrFontScale;
	m_bEnlarging = TRUE;
	m_AnEnlarge.Begin(1.f, fScale - 1.f, 400);
	const auto& CurrItem = m_vItem[m_idxPrevCurr];
	float yDest = CurrItem.y + CurrItem.cy * fScale / 2.f;
	m_psv->InterruptAnimation();
	m_psv->SmoothScrollDelta(int((yDest - GetViewHeight() / 3) - m_psv->GetPos()));
	GetWnd()->WakeRenderThread();
}