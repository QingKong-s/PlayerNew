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
		for (int i = m_idxTopGroup; i < GetGroupCount(); ++i)
		{
			(DrawGroup(i));
			{
				EckCounter((int)m_Group[i].Item.size(), j)
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
				InvalidateItem(slht.idxGroup,  idxHot );
				InvalidateItem(m_idxHotItemSGroup, m_idxHot );
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
			InvalidateItem(idxGroup,  idx );
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
		m_psv->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
			{
				const auto p = (CUIGroupList*)lParam;
				p->CalcTopItem();
				p->InvalidateRect();
			},(LPARAM)this);

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
	}
	break;
	}
	return __super::OnEvent(uMsg, wParam, lParam);
}

void CUIGroupList::InvalidateItem(int idxGroup, int idxItemInGroup)
{
	InvalidateRect();
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

BOOL CUIGroupList::DrawGroup(int idxGroup)
{
	auto& Group = m_Group[idxGroup];
	SLGETDISPINFO sldi{};
	sldi.nmhdr.uCode = UIN_GROUPLIST_GETDISPINFO;
	sldi.bItem = FALSE;
	sldi.Group.cchText = -1;
	sldi.Group.idxItem = idxGroup;
	sldi.Group.pDC = m_pDC;
	GenElemNotify(&sldi);

	if (!(Group.uFlags & ITF_LAYOUT_CACHE_VALID))
	{
		SafeRelease(Group.pLayout);
		App->m_pDwFactory->CreateTextLayout(sldi.Group.pszText, sldi.Group.cchText, m_pTfGroupTitle,
			GetWidthF(), (float)m_cyGroupHeader, &Group.pLayout);
		DWRITE_TEXT_METRICS tm;
		Group.pLayout->GetMetrics(&tm);
		Group.cxText = tm.width;
		Group.uFlags |= ITF_LAYOUT_CACHE_VALID;
	}

	D2D1_RECT_F rcGroup
	{
		0.f,
		(float)(Group.y - m_psv->GetPos()),
		GetWidthF(),
		(float)(Group.y + m_cyGroupHeader - m_psv->GetPos()),
	};

	const int cSubItem = (int)Group.Item.size();

	D2D1_RECT_F rcTemp
	{
		0.f,
		rcGroup.bottom,
		(float)m_cxCover,
		rcGroup.bottom + (float)m_cxCover
	};

	if(sldi.Group.pBmp)
	m_pDC->DrawBitmap(sldi.Group.pBmp, &rcTemp);

	//rcTemp.top = rcTemp.bottom;
	//rcTemp.bottom = rcGroup.bottom + (float)(cSubItem < c_iMinLinePerGroup ? c_iMinLinePerGroup * m_cyItem : cSubItem * m_cyItem);
	//m_pDC->FillRectangle(&rcTemp, m_pBrBk);
	//if (cSubItem < c_iMinLinePerGroup)
	//{
	//	rcTemp =
	//	{
	//		(float)m_cxCover,
	//		(float)(Group.Item.back().y + m_cyItem),
	//		GetWidthF(),
	//		(float)(Group.Item.front().y + m_cyItem * c_iMinLinePerGroup)
	//	};
	//	m_pDC->FillRectangle(rcTemp, m_pBrBk);
	//}

	if (rcGroup.bottom < 0.f || rcGroup.top > GetHeight())
		return FALSE;

	if (sldi.Group.pszText)
	{
		if (sldi.Group.cchText < 0)
			sldi.Group.cchText = (int)wcslen(sldi.Item.pszText);

		//if (idxItem == m_idxHot)
		//	m_pDC->FillRectangle(rcItem, m_pBrHot);
		//else
		//m_pDC->FillRectangle(rcGroup, m_pBrBk);
		//m_pDC->FillRectangle(rcGroup, m_pBrGroup);

		m_pBr->SetColor(m_pGroupColor->Get().crTextNormal);
		m_pDC->DrawTextLayout({ rcGroup.left + m_DsF.cxTextPadding,rcGroup.top }, Group.pLayout,
			m_pBr, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);

		const float yLine = rcGroup.top + (float)(m_cyGroupHeader / 2);
		m_pDC->DrawLine({ rcGroup.left + m_DsF.cxTextPadding * 2.f + Group.cxText,yLine },
			{ GetWidthF() - m_DsF.cxTextPadding,yLine }, m_pBr, m_DsF.cyGroupLine);
	}
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
