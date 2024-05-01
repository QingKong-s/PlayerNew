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

void CUILrc::ReCreateTextFormat()
{
	const auto& om = App->GetOptionsMgr();
	SafeRelease(m_pTextFormat);
	const auto& Font = om.ScLrcFontMain;
	App->m_pDwFactory->CreateTextFormat(Font.rsFontName.Data(), NULL,
		(DWRITE_FONT_WEIGHT)Font.iWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		GetBk()->Dpi(Font.fFontSize), L"zh-cn", &m_pTextFormat);
	m_pTextFormat->SetTextAlignment((DWRITE_TEXT_ALIGNMENT)om.ScLrcAlign);

	SafeRelease(m_pTextFormatTrans);
	const auto& Font2 = App->GetOptionsMgr().ScLrcFontTranslation;
	App->m_pDwFactory->CreateTextFormat(Font2.rsFontName.Data(), NULL,
		(DWRITE_FONT_WEIGHT)Font2.iWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		GetBk()->Dpi(Font2.fFontSize), L"zh-cn", &m_pTextFormatTrans);
	m_pTextFormat->SetTextAlignment((DWRITE_TEXT_ALIGNMENT)om.ScLrcAlign);
}

void CUILrc::ReCreateEmptyText()
{
	SafeRelease(m_pGrEmptyText);

	auto& Player = App->GetPlayer();
	auto& rsName = Player.GetList().At(Player.GetCurrFile()).rsName;
	IDWriteTextLayout* pLayout;
	App->m_pDwFactory->CreateTextLayout(rsName.Data(), rsName.Size(), m_pTextFormat,
		GetViewWidthF(), GetViewHeightF(), &pLayout);
	if (!pLayout)
		return;
	pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);

	ID2D1PathGeometry* pPathGeometry;
	eck::GetTextLayoutPathGeometry(pLayout, m_pDC, 0.f, 0.f, pPathGeometry);
	pLayout->Release();
	float xDpi, yDpi;
	m_pDC->GetDpi(&xDpi, &yDpi);

	m_pDC1->CreateFilledGeometryRealization(
		pPathGeometry,
		D2D1::ComputeFlatteningTolerance(
			D2D1::Matrix3x2F::Identity(), xDpi, yDpi),
		&m_pGrEmptyText);
	pPathGeometry->Release();
}

void CUILrc::CalcTopItem()
{
	const auto fScale = App->GetOptionsMgr().ScLrcCurrFontScale;
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
	const auto fScale = App->GetOptionsMgr().ScLrcCurrFontScale;
	const auto iAlign = App->GetOptionsMgr().ScLrcAlign;

	y = GetItemY(idx);
	const D2D1_RECT_F rc{ Item.x,0,Item.x + Item.cx,Item.cy };

	D2D1_MATRIX_3X2_F mat0;
	m_pDC->GetTransform(&mat0);

	D2D1_POINT_2F ptScale{};
	switch (iAlign)
	{
	case DWRITE_TEXT_ALIGNMENT_CENTER:
		ptScale.x = GetViewWidthF() / 2.f;
		break;
	case DWRITE_TEXT_ALIGNMENT_TRAILING:
		ptScale.x = GetViewWidthF();
		break;
	}

	D2D1_MATRIX_3X2_F mat = mat0 * D2D1::Matrix3x2F::Translation(0, y);

	if (!Item.bCacheValid)
	{
		const float cyPadding[]{ GetBk()->Dpi(5.f),0.f };

		float x[2]{};
		switch (iAlign)
		{
		case DWRITE_TEXT_ALIGNMENT_CENTER:
			x[0] = (GetViewWidthF() - GetViewWidthF() / fScale) / 2.f;
			x[1] = (GetViewWidthF() - Item.cxTrans) / 2.f;
			break;
		case DWRITE_TEXT_ALIGNMENT_TRAILING:
			x[0] = (GetViewWidthF() - GetViewWidthF() / fScale);
			x[1] = (GetViewWidthF() - Item.cxTrans);
			break;
		}

		ID2D1PathGeometry* pPathGeometry;
		eck::GetTextLayoutPathGeometry(&Item.pLayout, 2, cyPadding, m_pDC, x, 0.f, pPathGeometry);
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
		m_pDC->SetTransform(D2D1::Matrix3x2F::Scale(
			fScale + 1.f - m_fAnValue,
			fScale + 1.f - m_fAnValue,
			ptScale) * mat);
		if (Item.bSel || idx == m_idxHot)
			FillItemBkg(idx, rc);
		m_pDC1->DrawGeometryRealization(Item.pGr,
			m_idxPrevCurr == idx ? m_pBrTextHighlight : m_pBrTextNormal);
		m_pDC->SetTransform(mat0);
		return TRUE;
	}

	if (idx == m_idxCurrAnItem)
	{
		m_pDC->SetTransform(D2D1::Matrix3x2F::Scale(
			m_fAnValue,
			m_fAnValue,
			ptScale) * mat);
		if (Item.bSel || idx == m_idxHot)
			FillItemBkg(idx, rc);
		m_pDC1->DrawGeometryRealization(Item.pGr,
			m_idxPrevCurr == idx ? m_pBrTextHighlight : m_pBrTextNormal);
		m_pDC->SetTransform(mat0);
		return TRUE;
	}

	m_pDC->SetTransform(mat);
	FillItemBkg(idx, rc);
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

void CUILrc::GetItemRect(int idx, RECT& rc)
{
	auto& e = m_vItem[idx];
	const auto y = GetItemY(idx);
	const auto fScale = App->GetOptionsMgr().ScLrcCurrFontScale;
	rc.left = (long)e.x;
	rc.top = (long)y;
	if (idx == m_idxPrevAnItem)
	{
		const auto cx = (long)(e.cx * (fScale + 1.f - m_fAnValue));
		switch (App->GetOptionsMgr().ScLrcAlign)
		{
		case DWRITE_TEXT_ALIGNMENT_CENTER:
			rc.left = (GetViewWidth() - cx) / 2;
			break;
		case DWRITE_TEXT_ALIGNMENT_TRAILING:
			rc.left = GetViewWidth() - cx;
			break;
		}
		rc.right = rc.left + cx;
		rc.bottom = (long)(y + e.cy * (fScale + 1.f - m_fAnValue));
	}
	else if (idx == m_idxCurrAnItem)
	{
		const auto cx = (long)(e.cx * m_fAnValue);
		switch (App->GetOptionsMgr().ScLrcAlign)
		{
		case DWRITE_TEXT_ALIGNMENT_CENTER:
			rc.left = (GetViewWidth() - cx) / 2;
			break;
		case DWRITE_TEXT_ALIGNMENT_TRAILING:
			rc.left = GetViewWidth() - cx;
			break;
		}
		rc.right = rc.left + cx;
		rc.bottom = (long)(y + e.cy * m_fAnValue);
	}
	else ECKLIKELY
	{
		rc.right = rc.left + (long)e.cx;
		rc.bottom = (long)(y + e.cy);
	}
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

		if (m_vItem.empty())
		{
			if (m_pGrEmptyText)
				m_pDC1->DrawGeometryRealization(m_pGrEmptyText, m_pBrTextNormal);
		}
		else
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

	case WM_MOUSEMOVE:
	{
		if (m_vItem.empty())
			break;
		POINT pt ECK_GET_PT_LPARAM(lParam);
		ClientToElem(pt);

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
			if (App->GetPlayer().GetLrc().empty())
			{
				m_vItem.clear();
				ReCreateEmptyText();
			}
			else
			{
				m_idxPrevCurr = -1;
				LayoutItems();
				m_psv->SetPos(m_psv->GetMin());
				CalcTopItem();
			}
			InvalidateRect();
		}
	}
	return 0;

	case UIEE_ONSETTINGSCHANGE:
	{
		ReCreateTextFormat();
		if (!App->GetPlayer().GetLrc().empty())
		{
			LayoutItems();
			CalcTopItem();
		}
		InvalidateRect();
	}
	return 0;

	case WM_CREATE:
	{
		ReCreateTextFormat();

		m_SB.Create(NULL, Dui::DES_VISIBLE, 0,
			0, 0, GetWnd()->GetDs().CommSBCxy, GetViewHeight(),
			this, GetWnd());

		m_pDC->QueryInterface(&m_pDC1);

		m_pDC->CreateSolidColorBrush(eck::ColorrefToD2dColorF(eck::Colorref::Silver), &m_pBrTextNormal);
		m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBrTextHighlight);
		m_pDC->CreateSolidColorBrush(eck::ColorrefToD2dColorF(eck::Colorref::Silver), &m_pBrTextHighlight);
		m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBrTextNormal);
		m_pDC->CreateSolidColorBrush({}, &m_pBrush);

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

			if (m_tMouseIdle <= 0 && !m_bCtxMenuOpen)
			{
				const auto fScale = App->GetOptionsMgr().ScLrcCurrFontScale;
				m_bEnlarging = TRUE;
				m_idxPrevAnItem = idxPrev;
				m_idxCurrAnItem = m_idxPrevCurr;
				m_AnEnlarge.Begin(1.f, fScale - 1.f, (float)m_psv->GetDuration());
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
	const auto fScale = App->GetOptionsMgr().ScLrcCurrFontScale;
	m_bEnlarging = TRUE;
	m_AnEnlarge.Begin(1.f, fScale - 1.f, 400);
	const auto& CurrItem = m_vItem[m_idxPrevCurr];
	float yDest = CurrItem.y + CurrItem.cy * fScale / 2.f;
	m_psv->InterruptAnimation();
	m_psv->SmoothScrollDelta(int((yDest - GetViewHeight() / 3) - m_psv->GetPos()));
	GetWnd()->WakeRenderThread();
}

void CUILrc::LayoutItems()
{
	const auto fScale = App->GetOptionsMgr().ScLrcCurrFontScale;
	const auto cyMainTransPadding = GetBk()->Dpi(5.f);

	auto& Player = App->GetPlayer();
	const auto& vLrc = Player.GetLrc();
	const float cx = GetViewWidthF(), cy = GetViewHeightF();

	m_vItem.clear();

	m_vItem.resize(vLrc.size());
	const float cxMax = cx / fScale;
	DWRITE_TEXT_METRICS Metrics;
	float y = 0.f;
	const float cyPadding = GetBk()->Dpi(App->GetOptionsMgr().ScLrcPaddingHeight);
	const auto iAlign = App->GetOptionsMgr().ScLrcAlign;

	EckCounter(vLrc.size(), i)
	{
		auto& e = m_vItem[i];
		App->m_pDwFactory->CreateTextLayout(vLrc[i].pszLrc, vLrc[i].cchLrc,
			m_pTextFormat, cxMax, (float)cy, &e.pLayout);
		e.pLayout->GetMetrics(&Metrics);
		e.y = y;
		e.cx = Metrics.width;
		e.cy = Metrics.height;

		if (vLrc[i].pszTranslation)
		{
			App->m_pDwFactory->CreateTextLayout(vLrc[i].pszTranslation, vLrc[i].cchTotal - vLrc[i].cchLrc,
				m_pTextFormatTrans, cxMax, (float)cy, &e.pLayoutTrans);
			e.pLayoutTrans->GetMetrics(&Metrics);
			e.cxTrans = Metrics.width;
			e.cx = std::max(e.cx, Metrics.width);
			e.cy += (Metrics.height + cyMainTransPadding);
		}

		switch (iAlign)
		{
		case DWRITE_TEXT_ALIGNMENT_CENTER:
			e.x = (cx - e.cx) / 2.f;
			break;
		case DWRITE_TEXT_ALIGNMENT_TRAILING:
			e.x = cx - e.cx;
			break;
		}

		y += (e.cy + cyPadding);
	}

	m_psv->SetMin(int(-cy / 3.f - m_vItem.front().cy));
	m_psv->SetMax(int(y - cyPadding + cy / 3.f * 2.f));
	m_psv->SetPage((int)cy);
}