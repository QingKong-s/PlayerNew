#include "pch.h"
#include "CDlgMusicInfo.h"

void CDlgMiMetaData::UpdateDpi(int iDpi)
{
	if (m_hFont)
	{
		auto hFontNew = eck::ReCreateFontForDpiChanged(m_hFont, iDpi, m_iDpi);
		eck::SetFontForWndAndCtrl(HWnd, hFontNew);
		std::swap(m_hFont, hFontNew);
		DeleteObject(hFontNew);
	}
	else
		m_hFont = eck::CreateDefFont(iDpi);
	m_iDpi = iDpi;
	eck::UpdateDpiSize(m_Ds, m_iDpi);
}

void CDlgMiMetaData::BuildTree()
{
	m_vRoot.clear();
	const int idx = m_pParent->GetCurrIndex();
	if (idx < 0)
	{
		m_TL.BuildTree();
		m_TL.Redraw();
		m_Clkt.FreeAll();
		return;
	}
	m_Clkt.FreeAll();
	const auto& e = App->GetPlayer().GetList().At(idx);
	m_vRoot.push_back(m_Clkt.Alloc(1, eck::TLNODE{}, 0u, L"ID3v2"));
	auto& v = m_vRoot.back()->vChildren;
	if (!e.rsTitle.IsEmpty())
	{
		v.emplace_back(m_Clkt.Alloc(1, eck::TLNODE{}, Utils::MIF_TITLE, L"标题", e.rsTitle));
	}
	if (!e.rsArtist.IsEmpty())
	{
		auto& v2 = v.emplace_back(
			m_Clkt.Alloc(1, eck::TLNODE{}, Utils::MIF_ARTIST, L"艺术家", e.rsArtist))->vChildren;
		std::vector<eck::CRefStrW> vArtist{};
		eck::SplitStrWithMultiChar(e.rsArtist.Data(), c_szArtistSplitter, vArtist);
		for (auto& e : vArtist)
		{
			v2.emplace_back(m_Clkt.Alloc(1, eck::TLNODE{}, 0u, std::move(e)));
		}
	}
	if (!e.rsAlbum.IsEmpty())
	{
		v.emplace_back(m_Clkt.Alloc(1, eck::TLNODE{}, Utils::MIF_ALBUM, L"专辑", e.rsAlbum));
	}
	if (!e.rsGenre.IsEmpty())
	{
		v.emplace_back(m_Clkt.Alloc(1, eck::TLNODE{}, Utils::MIF_GENRE, L"流派", e.rsGenre));
	}
	m_TL.BuildTree();
	m_TL.Redraw();
}

BOOL CDlgMiMetaData::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	UpdateDpi(eck::GetDpi(hDlg));
	m_TL.Create(NULL, WS_CHILD | WS_VISIBLE, 0,
		0, 0, 0, 0, hDlg, 0);
	auto& Header = m_TL.GetHeader();
	Header.InsertItem(L"名称", 0, 200);
	Header.InsertItem(L"值", 1, 200);
	eck::SetFontForWndAndCtrl(hDlg, m_hFont);
	return 0;
}

LRESULT CDlgMiMetaData::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		int cx, cy;
		ECK_GET_SIZE_LPARAM(cx, cy, lParam);
		SetWindowPos(m_TL.HWnd, NULL,
			m_Ds.iMargin, m_Ds.iMargin, cx - m_Ds.iMargin * 2, cy - m_Ds.iMargin * 2,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	return 0;

	case WM_NOTIFY:
	{
		if (((NMHDR*)lParam)->hwndFrom == m_TL.HWnd)
			switch (((NMHDR*)lParam)->code)
			{
			case eck::NM_TL_GETDISPINFO:
			{
				const auto p = (eck::NMTLGETDISPINFO*)lParam;
				const auto pNode = (NODE*)p->Item.pNode;
				if (p->Item.idxSubItem)
				{
					p->Item.pszText = pNode->V.Data();
					p->Item.cchText = pNode->V.Size();
				}
				else
				{
					p->Item.pszText = pNode->K.Data();
					p->Item.cchText = pNode->K.Size();
				}
			}
			return 0;
			case eck::NM_TL_FILLCHILDREN:
			{
				const auto p = (eck::NMTLFILLCHILDREN*)lParam;
				if (p->bQueryRoot)
				{
					p->pChildren = (eck::TLNODE**)m_vRoot.data();
					p->cChildren = (int)m_vRoot.size();
				}
				else
				{
					const auto pNode = (NODE*)p->pParent;
					p->pChildren = (eck::TLNODE**)pNode->vChildren.data();
					p->cChildren = (int)pNode->vChildren.size();
				}
			}
			return 0;
			}
	}
	return 0;

	case PNWM_MIDLG_UPDATEIDX:
		BuildTree();
		return 0;

	case PNWM_MIDLG_INIT:
		m_pParent = (CDlgMusicInfo*)lParam;
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}