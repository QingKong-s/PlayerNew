#include "pch.h"


void CUIGroupList::CalcTopItem()
{
	if (m_bGroup)
	{
		if (!m_Group.size())
		{
			m_idxTopGroup = m_idxTop = 0;
			return;
		}
		const auto it = std::lower_bound(m_Group.begin(), m_Group.end(), m_psv->GetPos(),
			[](const GROUPITEM& x, int iPos)
			{
				return x.y < iPos;
			});

#ifdef _DEBUG
		if (it == m_Group.end())
			EckDbgBreak();
#endif

		if (it == m_Group.begin())
			m_idxTopGroup = 0;
		else
			m_idxTopGroup = (int)std::distance(m_Group.begin(), it - 1);
		EckDbgPrint(m_idxTopGroup);
		const auto& e = m_Group[m_idxTopGroup];
		m_oyTop = e.y - m_psv->GetPos();
		m_idxTop = (m_psv->GetPos() - (e.y + m_cyGroupHeader)) / m_cyItem;
	}
	else
	{
		m_idxTop = m_psv->GetPos() / m_cyItem;
		m_oyTop = m_psv->GetPos() - m_idxTop * m_cyItem;
	}
}

LRESULT CUIGroupList::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		Dui::ELEMPAINTSTRU eps;
		BeginPaint(eps, wParam, lParam);
		m_pBr->SetColor(GetColorTheme()->Get().crBkNormal);
		m_pDC->FillRectangle(eps.rcfClipInElem, m_pBr);
		if (m_Group.empty())
		{
			EndPaint(eps);
			return 0;
		}

		const int iSbPos = m_psv->GetPos();
		auto it = std::lower_bound(m_Group.begin() + m_idxTopGroup, m_Group.end(),
			eps.prcClip->top + iSbPos, [](const GROUPITEM& x, int iPos)
			{
				return x.y < iPos;
			});

		if (it != m_Group.begin())
			--it;
		for (int i = (int)std::distance(m_Group.begin(), it); i < GetGroupCount(); ++i)
		{
			const auto& e = m_Group[i];
			if (e.y >= (int)eps.rcfClipInElem.bottom + iSbPos)
				break;
			DrawGroup(i, eck::MakeRect(eps.rcfClipInElem));
			for (int j = (i == m_idxTopGroup ? m_idxTop : 0); j < (int)e.Item.size(); ++j)
			{
				const auto& f = e.Item[j];
				if (f.y >= (int)eps.rcfClipInElem.bottom + iSbPos)
					break;
				DrawGroupItem(i, j);
			}
		}
		EndPaint(eps);
	}
	return 0;

	case WM_SIZE:
	{
		m_SB.SetRect({ GetWidth() - (int)GetWnd()->GetDs().CommSBCxy,0,GetWidth(),GetHeight() });
		m_psv->SetPage(GetHeight());
	}
	return 0;

	case WM_MOUSEMOVE:
	{
		POINT pt ECK_GET_PT_LPARAM(lParam);
		ClientToElem(pt);
		int idxHot;
		SLHITTEST slht;
		if (m_bGroup)
		{
			idxHot = HitTest(pt, slht);
			if (idxHot != m_idxHot || m_idxHotItemSGroup != slht.idxGroup)
			{
				std::swap(idxHot, m_idxHot);
				std::swap(slht.idxGroup, m_idxHotItemSGroup);
				if (slht.idxGroup >= 0 && idxHot >= 0)
					InvalidateItem(slht.idxGroup, idxHot);
				if (m_idxHotItemSGroup >= 0 && m_idxHot >= 0)
					InvalidateItem(m_idxHotItemSGroup, m_idxHot);
			}
		}
		else
		{
			idxHot = HitTest(pt, slht);
			if (idxHot != m_idxHot)
			{
				std::swap(idxHot, m_idxHot);
				//Redraw({ idxHot,m_idxHot });
			}
		}
	}
	return 0;

	case WM_MOUSELEAVE:
	{
		int idx = -1;
		if (m_bGroup)
		{
			int idxGroup = -1;
			std::swap(idx, m_idxHot);
			std::swap(idxGroup, m_idxHotItemSGroup);
			if (idxGroup >= 0 && idx >= 0)
				InvalidateItem(idxGroup, idx);
		}
		else
		{
			//std::swap(idx, m_idxHot);
			//if (idx >= 0)
			//	InvalidateItem( idx );
		}
	}
	return 0;

	case WM_MOUSEWHEEL:
		m_psv->OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
		GetWnd()->WakeRenderThread();
		return 0;

	case WM_NOTIFY:
	{
		if ((Dui::CElem*)wParam == &m_SB)
		{
			switch (((Dui::DUINMHDR*)lParam)->uCode)
			{
			case Dui::EE_VSCROLL:
				ECK_DUILOCK;
				CalcTopItem();
				InvalidateRect();
				return TRUE;
			}
		}
	}
	return 0;

	case WM_LBUTTONDOWN:
	{

	}
	return 0;

	case WM_LBUTTONUP:
	{

	}
	return 0;

	case WM_CREATE:
	{
		eck::UpdateDpiSizeF(m_DsF, GetWnd()->GetDpiValue());

		m_SB.Create(NULL, Dui::DES_VISIBLE | Dui::DES_TRANSPARENT, 0,
			0, 0, 0, 0, this, GetWnd());
		m_SB.SetRect({ GetWidth() - (int)GetWnd()->GetDs().CommSBCxy,0,GetWidth(),GetHeight() });
		m_psv = m_SB.GetScrollView();
		m_psv->AddRef();
		m_psv->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
			{
				const auto p = (CUIGroupList*)lParam;
				p->CalcTopItem();
				p->InvalidateRect();
			}, (LPARAM)this);

		App->m_pDwFactory->CreateTextFormat(L"Î¢ÈíÑÅºÚ", NULL, DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, m_DsF.EmText, L"zh-cn", &m_pTextFormat);
		m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		App->m_pDwFactory->CreateTextFormat(L"Î¢ÈíÑÅºÚ", NULL, DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, m_DsF.EmGroupText, L"zh-cn", &m_pTfGroupTitle);
		m_pTfGroupTitle->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		m_pDC->CreateSolidColorBrush({}, &m_pBr);
	}
	break;

	case WM_DESTROY:
	{
		SafeRelease(m_pTextFormat);
		SafeRelease(m_pTfGroupTitle);
		SafeRelease(m_pBr);
		SafeRelease(m_psv);
	}
	break;
	}
	return __super::OnEvent(uMsg, wParam, lParam);
}

void CUIGroupList::InvalidateItem(int idxGroup, int idxItemInGroup)
{
	RECT rc;
	GetGroupPartRect(rc, Part::Item, idxGroup, idxItemInGroup);
	InvalidateRect(&rc);
}

int CUIGroupList::HitTest(POINT pt, SLHITTEST& pslht)
{
	pslht.bHitCover = FALSE;
	pslht.idxGroup = -1;
	if (pt.x < 0 || pt.x >= GetWidth() || pt.y < 0 || pt.y >= GetHeight())
		return -1;
	if (m_bGroup)
	{
		auto it = std::lower_bound(m_Group.begin() + m_idxTop, m_Group.end(),
			pt.y + m_psv->GetPos(), [](const GROUPITEM& x, int iPos)
			{
				return x.y < iPos;
			});

		if (it == m_Group.begin())
			return -1;
		else if (it == m_Group.end())
			it = (m_Group.rbegin() + 1).base();
		else
			--it;
		const int idxGroup = (int)std::distance(m_Group.begin(), it);
		pslht.idxGroup = idxGroup;
		if (pt.y > it->y && pt.y < it->y + m_cyGroupHeader)
			return -1;

		const int y = pt.y + m_psv->GetPos() - it->y - m_cyGroupHeader;
		if (y >= 0)
		{
			int idx = y / m_cyItem;
			if (idx < (int)it->Item.size())
			{
				if (pt.x <= m_cxCover)
				{
					pslht.bHitCover = TRUE;
					return -1;
				}
				return idx;
			}
		}
		return -1;
	}
	else
	{
		const int idx = (pt.y + m_psv->GetPos()) / m_cyItem;
		if (idx >= (int)m_Item.size())
			return -1;
		else
			return idx;
	}
}

void CUIGroupList::GetGroupPartRect(RECT& rc, Part ePart, int idxGroup, int idxItemInGroup)
{
	auto& e = m_Group[idxGroup];
	switch (ePart)
	{
	case Part::GroupHeader:
		rc =
		{
			(int)m_DsF.cxTextPadding,
			e.y - m_psv->GetPos(),
			GetWidth(),
			e.y - m_psv->GetPos() + m_cyGroupHeader,
		};
		break;
	case Part::GroupText:
		EckDbgBreak();
		break;
	case Part::Item:
		rc =
		{
			m_cxCover,
			e.Item[idxItemInGroup].y - m_psv->GetPos(),
			GetWidth(),
			e.Item[idxItemInGroup].y - m_psv->GetPos() + m_cyItem,
		};
		break;
	case Part::Cover:
		rc =
		{
			0,
			e.y - m_psv->GetPos() + m_cyGroupHeader,
			m_cxCover,
			e.y - m_psv->GetPos() + m_cyGroupHeader + m_cxCover
		};
		break;
	default:
		EckDbgBreak();
		break;
	}
}

void CUIGroupList::GetGroupPartRect(D2D1_RECT_F& rc, Part ePart, int idxGroup, int idxItemInGroup)
{
	RECT rc2;
	GetGroupPartRect(rc2, ePart, idxGroup, idxItemInGroup);
	rc = eck::MakeD2DRcF(rc2);
}

void CUIGroupList::ReCalc(int idxBegin)
{
	int y = 0;
	for (int i = idxBegin; i < m_Group.size(); ++i)
	{
		auto& Group = m_Group[i];
		Group.y = y;
		y += m_cyGroupHeader;
		for (auto& Item : Group.Item)
		{
			Item.y = y;
			y += m_cyItem;
		}

		if (Group.Item.size() < c_iMinLinePerGroup)
			y += ((c_iMinLinePerGroup - Group.Item.size()) * m_cyItem);
	}
	m_psv->SetMax(y);
}

BOOL CUIGroupList::DrawGroup(int idxGroup, const RECT& rcPaint)
{
	auto& e = m_Group[idxGroup];

	D2D1_RECT_F rcGroup;
	GetGroupPartRect(rcGroup, Part::GroupHeader, idxGroup, 0);

	D2D1_RECT_F rcCover;
	GetGroupPartRect(rcCover, Part::Cover, idxGroup, 0);

	const BOOL bText = !(rcGroup.bottom <= rcPaint.top || rcGroup.top >= rcPaint.bottom);
	const BOOL bCover = !(rcCover.bottom <= rcPaint.top || rcCover.top >= rcPaint.bottom);
	if (!bText && !bCover)
		return FALSE;

	SLGETDISPINFO sldi{};
	if (bText || bCover)
	{
		sldi.nmhdr.uCode = UIN_GROUPLIST_GETDISPINFO;
		sldi.bItem = FALSE;
		sldi.Group.cchText = -1;
		sldi.Group.idxItem = idxGroup;
		sldi.Group.pDC = m_pDC;
		GenElemNotify(&sldi);
	}

	if (bText)
	{
		if (!(e.uFlags & ITF_LAYOUT_CACHE_VALID))
		{
			SafeRelease(e.pLayout);
			App->m_pDwFactory->CreateTextLayout(sldi.Group.pszText, sldi.Group.cchText, m_pTfGroupTitle,
				GetWidthF(), (float)m_cyGroupHeader, &e.pLayout);
			DWRITE_TEXT_METRICS tm;
			e.pLayout->GetMetrics(&tm);
			e.cxText = tm.width;
			e.uFlags |= ITF_LAYOUT_CACHE_VALID;
		}
		if (sldi.Group.pszText)
		{
			m_pBr->SetColor(m_pGroupColor->Get().crTextNormal);
			m_pDC->DrawTextLayout({ rcGroup.left,rcGroup.top }, e.pLayout,
				m_pBr, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);

			const float yLine = rcGroup.top + (float)(m_cyGroupHeader / 2);
			m_pDC->DrawLine({ rcGroup.left + m_DsF.cxTextPadding * 2.f + e.cxText,yLine },
				{ GetWidthF() - m_DsF.cxTextPadding,yLine }, m_pBr, m_DsF.cyGroupLine);
		}
	}

	if (bCover && sldi.Group.pBmp)
		m_pDC->DrawBitmap(sldi.Group.pBmp, &rcCover);
	return TRUE;
}

BOOL CUIGroupList::DrawGroupItem(int idxGroup, int idxItem)
{
	auto& Item = m_Group[idxGroup].Item[idxItem];

	if (!(Item.uFlags & ITF_LAYOUT_CACHE_VALID))
	{
		SLGETDISPINFO sldi{};
		sldi.nmhdr.uCode = UIN_GROUPLIST_GETDISPINFO;
		sldi.bItem = TRUE;
		sldi.Item.cchText = -1;
		sldi.Item.idxItem = idxItem;
		sldi.Item.idxGroup = idxGroup;
		GenElemNotify(&sldi);

		SafeRelease(Item.pLayout);
		App->m_pDwFactory->CreateTextLayout(sldi.Item.pszText, sldi.Item.cchText, m_pTextFormat,
			GetWidthF() - (float)m_cxCover, (float)m_cyItem, &Item.pLayout);
		DWRITE_TEXT_METRICS tm;
		Item.uFlags |= ITF_LAYOUT_CACHE_VALID;
	}

	D2D1_RECT_F rcItem
	{
		(float)m_cxCover,
		(float)(Item.y - m_psv->GetPos()),
		GetWidthF(),
		(float)(Item.y + m_cyItem - m_psv->GetPos()),
	};

	if (rcItem.bottom < 0.f || rcItem.top > GetHeight())
		return FALSE;

	if (Item.pLayout)
	{
		if (idxItem == m_idxHot && idxGroup == m_idxHotItemSGroup)
		{
			m_pBr->SetColor(GetColorTheme()->Get().crBkHot);
			m_pDC->FillRectangle(rcItem, m_pBr);
		}

		rcItem.left += m_DsF.cxTextPadding;
		m_pBr->SetColor(GetColorTheme()->Get().crTextNormal);
		m_pDC->DrawTextLayout({ rcItem.left,rcItem.top }, Item.pLayout,
			m_pBr, D2D1_DRAW_TEXT_OPTIONS_NONE);
		rcItem.left -= m_DsF.cxTextPadding;
	}
	return TRUE;
}

BOOL CUIGroupList::RedrawItem(int idxItem, D2D1_RECT_F& rcItem)
{
	SLGETDISPINFO sldi{};
	sldi.nmhdr.uCode = UIN_GROUPLIST_GETDISPINFO;
	sldi.bItem = TRUE;
	sldi.Item.cchText = -1;
	sldi.Item.idxItem = idxItem;
	sldi.Item.idxGroup = -1;
	GenElemNotify(&sldi);

	rcItem =
	{
		0.f,
		(float)(idxItem * m_cyItem - m_psv->GetPos()),
		GetWidthF(),
		(float)((idxItem + 1) * m_cyItem - m_psv->GetPos()),
	};
	if (rcItem.bottom < 0.f || rcItem.top > GetHeight())
		return FALSE;

	if (sldi.Item.pszText)
	{
		if (sldi.Item.cchText < 0)
			sldi.Item.cchText = (int)wcslen(sldi.Item.pszText);

		if (idxItem == m_idxHot)
		{
			m_pBr->SetColor(GetColorTheme()->Get().crBkHot);
			m_pDC->FillRectangle(rcItem, m_pBr);
		}

		rcItem.left += m_DsF.cxTextPadding;
		m_pBr->SetColor(GetColorTheme()->Get().crTextNormal);
		m_pDC->DrawTextW(sldi.Item.pszText, sldi.Item.cchText, m_pTextFormat,
			rcItem, m_pBr, D2D1_DRAW_TEXT_OPTIONS_NONE);
		rcItem.left -= m_DsF.cxTextPadding;
	}
	return TRUE;
}
