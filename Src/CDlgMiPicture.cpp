#include "pch.h"
#include "CDlgMusicInfo.h"

void CDlgMiPicture::UpdatePic()
{
	for (const auto& e : m_vItem)
	{
		if (e.pBitmap)
			GdipDisposeImage(e.pBitmap);
	}
	m_vItem.clear();

	const int idx = m_pParent->GetCurrIndex();
	if (idx < 0)
	{
		m_LBN.SetItemCount(0);
		m_LBN.Redraw();
		return;
	}
	const auto& e = App->GetPlayer().GetList().At(idx);

	Utils::MUSICINFO mi{ Utils::MIF_COVER };
	Utils::GetMusicInfo(e.rsFile.Data(), mi);
	for (auto& f : mi.vImage)
	{
		GpBitmap* pBitmap{};
		if (f.bLink) ECKUNLIKELY
		{
			// TODO:解析相对链接
			GdipCreateBitmapFromFile(std::get<1>(f.varPic).Data(), &pBitmap);
		}
		else
		{
			const auto pStream = new eck::CStreamView(std::get<0>(f.varPic));
			GdipCreateBitmapFromStream(pStream, &pBitmap);
			pStream->Release();
		}
		eck::CRefStrW rs(Utils::PicTypeToString(f.eType));
		rs.PushBackChar(L'\n');
		if (pBitmap)
		{
			rs.PushBack(f.rsMime);
			rs.PushBackChar(L'\n');

			UINT cx, cy;
			GdipGetImageWidth(pBitmap, &cx);
			GdipGetImageHeight(pBitmap, &cy);
			rs.AppendFormat(L"%ux%u\n", cx, cy);

			rs.PushBack(f.rsDesc);
		}
		else
		{
			rs.PushBack(L"图片数据无效\n");
			rs.PushBack(f.rsDesc);
		}
		m_vItem.emplace_back(std::move(f), pBitmap, std::move(rs));
	}
	m_LBN.SetItemCount((int)m_vItem.size());
	m_LBN.Redraw();
}

BOOL CDlgMiPicture::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	UpdateDpi(eck::GetDpi(hDlg));
	m_LBN.Create(NULL, WS_CHILD | WS_VISIBLE, 0,
		0, 0, 0, 0, hDlg, 0);
	m_LBN.SetItemHeight(m_Ds.cyItem);
	m_Layout.Add(&m_LBN, {}, 0u, 1);

	{
		m_PKB.Create(NULL, WS_CHILD | WS_VISIBLE, 0,
			0, 0, 0, 0, hDlg, 0);
		m_LayoutV.Add(&m_PKB, {}, 0u, 2);

		m_LV.Create(NULL, WS_CHILD | WS_VISIBLE, 0,
			0, 0, 0, 0, hDlg, 0);
		m_LayoutV.Add(&m_LV, {}, 0u, 1);
	}
	m_Layout.Add(&m_LayoutV, {}, 0u, 1);

	return 0;
}

LRESULT CDlgMiPicture::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		if(((NMHDR*)lParam)->hwndFrom==m_LBN.HWnd )
			switch (((NMHDR*)lParam)->code)
			{
			case eck::NM_LBN_CUSTOMDRAW:
			{
				const auto p = (eck::NMLBNCUSTOMDRAW*)lParam;
				if (p->nmcd.iStage == eck::NMECDS_POSTDRAWBK)
				{
					const auto& e = m_vItem[p->nmcd.idx];
					const auto& rcItem = p->nmcd.rcItem;
					const int cyItem = rcItem.bottom - rcItem.top;
					RECT rc{ rcItem };
					eck::InflateRect(rc, -m_Ds.cxPadding, -m_Ds.cxPadding);
					rc.right = rc.left + (rc.bottom - rc.top);
					if (e.pBitmap)
					{
						GpGraphics* pGraphics;
						GdipCreateFromHDC(p->nmcd.hDC, &pGraphics);
						// TODO:按比例缩放
						GdipDrawImageRectI(pGraphics, e.pBitmap,
							rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top));
						GdipDeleteGraphics(pGraphics);
					}
					rc.left = rc.right + m_Ds.cxPadding;
					rc.right = rcItem.right - m_Ds.cxPadding;
					DrawTextW(p->nmcd.hDC, e.rsText.Data(), e.rsText.Size(), &rc,
						DT_NOPREFIX | DT_END_ELLIPSIS | DT_NOCLIP);

					return eck::NMECDR_SKIPDEF;
				}
			}
			return 0;
			}
	}
	break;

	case WM_SIZE:
	{
		m_Layout.Arrange(LOWORD(lParam), HIWORD(lParam));
	}
	return 0;

	case PNWM_MIDLG_UPDATEIDX:
		UpdatePic();
		return 0;

	case PNWM_MIDLG_INIT:
		m_pParent = (CDlgMusicInfo*)lParam;
		return 0;

	case WM_DESTROY:
	{
		m_Layout.Clear();
		m_LayoutV.Clear();
	}
	return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}