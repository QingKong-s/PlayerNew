#include "CDlgBookmark.h"

constexpr static UINT_PTR c_uScidLVList = 20231027'01;

void CDlgBookmark::UpdateDpiInit(int iDpi)
{
	m_iDpi = iDpi;
	eck::UpdateDpiSize(m_Ds, iDpi);

	const int
		cxIcon = eck::DpiScale(c_cxBtnIcon, iDpi),
		cyIcon = eck::DpiScale(c_cyBtnIcon, iDpi);
	HIMAGELIST hIml = ImageList_Create(cxIcon, cyIcon, ILC_COLOR32, 0, 2);
	HICON hIcon;
	auto pBmp = App->ScaleImageForButton(IIDX_Cross, iDpi);
	hIcon = eck::CreateHICON(pBmp);
	ImageList_AddIcon(hIml, hIcon);
	pBmp->Release();
	DestroyIcon(hIcon);

	pBmp = App->ScaleImageForButton(IIDX_Locate, iDpi);
	hIcon = eck::CreateHICON(pBmp);
	ImageList_AddIcon(hIml, hIcon);
	pBmp->Release();
	DestroyIcon(hIcon);

	m_TBOp.SetImageList(hIml);
	std::swap(m_hIml, hIml);
	ImageList_Destroy(hIml);

	m_TBOp.SetButtonSize(m_Ds.cxTB, m_Ds.cyTB);
}

void CDlgBookmark::UpdateDpi(int iDpi)
{
	const int iDpiOld = m_iDpi;
	UpdateDpiInit(iDpi);

	const HFONT hOldFont = m_hFont;
	m_hFont = eck::ReCreateFontForDpiChanged(m_hFont, iDpi, iDpiOld);
	eck::SetFontForWndAndCtrl(m_hWnd, m_hFont);
	DeleteObject(hOldFont);

	eck::LVSetItemHeight(m_LVBookmark.GetHWND(), m_Ds.cyLVItem);
}

LRESULT CDlgBookmark::SubclassProc_LV(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto p = (CDlgBookmark*)dwRefData;
	switch (uMsg)
	{
	case WM_COMMAND:
		if ((HWND)lParam == p->m_EDName.GetHWND() && HIWORD(wParam) == EN_KILLFOCUS)
		{
			p->CancelEditing(TRUE);
			return 0;
		}
		else if ((HWND)lParam == p->m_CLPColor.GetHWND() && HIWORD(wParam) == CBN_KILLFOCUS)
		{
			p->CancelEditing(TRUE);
			return 0;
		}
		break;
	case WM_LBUTTONDOWN:
		HANDLE_WM_MBUTTONDOWN(hWnd, wParam, lParam, p->OnLVLBtnDown);
		break;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
// XXX : 项目编辑
LRESULT CDlgBookmark::SubclassProc_ED(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto p = (CDlgBookmark*)dwRefData;
	switch (uMsg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RETURN:
			p->CancelEditing(TRUE);
			return 0;
		case VK_ESCAPE:
			p->CancelEditing(FALSE);
			return 0;
		}
		break;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CDlgBookmark::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		if (((NMHDR*)lParam)->hwndFrom == m_LVBookmark.GetHWND())
			switch (((NMHDR*)lParam)->code)
			{
			case NM_CUSTOMDRAW:
				return OnLVNCustomDraw((NMLVCUSTOMDRAW*)lParam);
			case NM_DBLCLK:
				OnLVNDbLClick((NMITEMACTIVATE*)lParam);
				return 0;
			}
	}
	break;
	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, OnSize);
	case WM_COMMAND:
		if ((HWND)lParam == m_TBOp.GetHWND() && HIWORD(wParam) == BN_CLICKED)
			switch (LOWORD(wParam))
			{
			case TBCID_DELETE:
				OnCmdDelete();
				return 0;
			case TBCID_JUMP:
				OnCmdJump();
				return 0;
			}
		break;
	case WM_TIMER:
		if (wParam == IDT_LVCLICKDELAY)
		{
			KillTimer(hWnd, IDT_LVCLICKDELAY);
			EnterEditing(m_idxLBtnDown, m_idxSubLBtnDown);
			return 0;
		}
		break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CDlgBookmark::OnInitDialog(HWND hWnd, HWND hFocus, LPARAM lParam)
{
	m_TBOp.Create(NULL, WS_TABSTOP | WS_GROUP |
		TBSTYLE_LIST | TBSTYLE_TRANSPARENT | TBSTYLE_WRAPABLE |
		CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER, 0, 0, 0, 0, 0, hWnd, IDC_TB_OP);

	UpdateDpiInit(eck::GetDpi(hWnd));
	m_hFont = eck::EzFont(L"微软雅黑", 9);

	m_TBOp.SetButtonStructSize();
	TBBUTTON TBBtns[]
	{
		{0,TBCID_DELETE,TBSTATE_ENABLED,0,{},0,(INT_PTR)L"删除"},
		{1,TBCID_JUMP,TBSTATE_ENABLED,0,{},0,(INT_PTR)L"跳转"},
	};
	m_TBOp.AddButtons(ARRAYSIZE(TBBtns), TBBtns);
	m_TBOp.Show(SW_SHOWNOACTIVATE);

	m_LVBookmark.Create(NULL, WS_CLIPCHILDREN | WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, WS_EX_CLIENTEDGE,
		0, 0, 0, 0, hWnd, IDC_LV_BOOKMARK);
	constexpr DWORD dwLVExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	m_LVBookmark.SetLVExtendStyle(dwLVExStyle);
	m_LVBookmark.InsertColumn(L"位置", 0, m_Ds.cxColumn1);
	m_LVBookmark.InsertColumn(L"名称", 1, m_Ds.cxColumn2);
	m_LVBookmark.InsertColumn(L"颜色", 2, m_Ds.cxColumn3, LVCFMT_CENTER);
	m_LVBookmark.SetExplorerTheme();
	eck::LVSetItemHeight(m_LVBookmark.GetHWND(), m_Ds.cyLVItem);
	const auto& hmBookmark = App->GetPlayer().GetList().GetBookmark();
	LVITEMW lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = 0;
	std::wstring sTemp;
	m_LVBookmark.SetItemCount((int)hmBookmark.size());
	for (auto& x : hmBookmark)
	{
		lvi.iSubItem = 0;
		sTemp = std::to_wstring(x.first);
		lvi.pszText = (PWSTR)sTemp.c_str();
		lvi.lParam = x.first;
		m_LVBookmark.InsertItem(&lvi);

		lvi.iSubItem = 1;
		lvi.pszText = (PWSTR)x.second.rsName.Data();
		m_LVBookmark.SetItemText(lvi.iItem, &lvi);
	}
	SetWindowSubclass(m_LVBookmark.GetHWND(), SubclassProc_LV, c_uScidLVList, (DWORD_PTR)this);
	m_LVBookmark.Show(SW_SHOWNOACTIVATE);

	m_EDName.Create(NULL, ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, 0, 0, 0, 0, m_LVBookmark.GetHWND(), IDC_ED_NAME);
	SetWindowSubclass(m_EDName.GetHWND(), SubclassProc_ED, 41434163, (DWORD_PTR)this);
	m_CLPColor.Create(NULL, 0, 0, 0, 0, 0, 0, m_LVBookmark.GetHWND(), IDC_CLP_COLOR);

	eck::SetFontForWndAndCtrl(hWnd, m_hFont);

	RECT rc;
	GetClientRect(hWnd, &rc);
	SendMsg(WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));

	SetFocus(m_LVBookmark.HWnd);
	return FALSE;
}

void CDlgBookmark::OnCmdJump()
{
	int idxCurr = m_LVBookmark.GetCurrSel();
	if (idxCurr < 0)
		return;
	const auto& hmBookmark = App->GetPlayer().GetList().GetBookmark();
	LVITEMW lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = idxCurr;
	lvi.iSubItem = 0;
	if (m_LVBookmark.GetItem(&lvi))
		m_pParam->LVList.EnsureVisible((int)lvi.lParam);
}

LRESULT CDlgBookmark::OnLVNCustomDraw(NMLVCUSTOMDRAW* pnmlvcd)
{
	switch (pnmlvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
		return CDRF_NOTIFYSUBITEMDRAW;
	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
	{
		if (pnmlvcd->iSubItem != 2)
			return CDRF_DODEFAULT;
		const HDC hDC = pnmlvcd->nmcd.hdc;
		const auto& hmBookmark = App->GetPlayer().GetList().GetBookmark();
		RECT rc;
		m_LVBookmark.GetHeaderCtrl().GetItemRect(2, &rc);
		rc.top = pnmlvcd->nmcd.rc.top;
		rc.bottom = pnmlvcd->nmcd.rc.bottom;
		const COLORREF cr = hmBookmark.at((int)pnmlvcd->nmcd.lItemlParam).crColor;
		WCHAR szTemp[24];
		swprintf_s(szTemp, L"0x%06X", eck::ReverseColorref(cr));
		const COLORREF crOld = SetTextColor(hDC, cr);
		DrawTextW(hDC, szTemp, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		SetTextColor(hDC, crOld);
	}
	return CDRF_SKIPDEFAULT;
	}
	return CDRF_DODEFAULT;
}

void CDlgBookmark::OnLVNDbLClick(NMITEMACTIVATE* pnmia)
{
	OnCmdJump();
}

void CDlgBookmark::OnSize(HWND hWnd, UINT uState, int cx, int cy)
{
	HDWP hDwp = BeginDeferWindowPos(5);
	hDwp = DeferWindowPos(hDwp, m_TBOp.GetHWND(), NULL,
		m_Ds.iMargin,
		m_Ds.iMargin,
		cx - m_Ds.iMargin * 2,
		m_Ds.cyTB,
		SWP_NOZORDER | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_LVBookmark.GetHWND(), NULL,
		m_Ds.iMargin,
		m_Ds.cyTB + m_Ds.iMargin,
		cx - m_Ds.iMargin * 2,
		cy - (m_Ds.cyTB + m_Ds.iMargin * 2),
		SWP_NOZORDER | SWP_NOACTIVATE);
	EndDeferWindowPos(hDwp);
	m_TBOp.SetButtonSize(m_Ds.cxTB, m_Ds.cyTB);
}

void CDlgBookmark::OnLVLBtnDown(HWND hWnd, BOOL bDoubleClick, int x, int y, UINT uKeyFlags)
{
	KillTimer(m_hWnd, IDT_LVCLICKDELAY);
	LVHITTESTINFO lvhti;
	lvhti.pt = { x,y };
	m_LVBookmark.SubItemHitTest(&lvhti);
	if (lvhti.iItem < 0 || lvhti.iSubItem < 1 || lvhti.iSubItem > 2)
		return;
	if (m_LVBookmark.GetItemState(lvhti.iItem, LVIS_SELECTED) == LVIS_SELECTED)
	{
		m_idxLBtnDown = lvhti.iItem;
		m_idxSubLBtnDown = lvhti.iSubItem;
		SetTimer(m_hWnd, IDT_LVCLICKDELAY, TELAPSE_LVCLICKDELAY, NULL);
	}
}

void CDlgBookmark::EnterEditing(int idx, int idxSub)
{
	if (idx < 0 || idxSub < 1 || idxSub > 2)
	{
		EckDbgBreak();
		return;
	}
	RECT rcItem;
	m_LVBookmark.GetSubItemRect(idx, idxSub, &rcItem);
	rcItem.right -= rcItem.left;
	rcItem.bottom -= rcItem.top;

	LVITEMW lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = idx;
	lvi.iSubItem = 0;
	m_LVBookmark.GetItem(&lvi);

	const auto& hmBookmark = App->GetPlayer().GetList().GetBookmark();
	const auto& Bm = hmBookmark.at((int)lvi.lParam);
	CancelEditing(FALSE);
	if (idxSub == 1)
	{
		m_EDName.SetText(Bm.rsName.Data());
		SetFocus(m_EDName.GetHWND());
		SetWindowPos(m_EDName.GetHWND(), NULL,
			rcItem.left, rcItem.top, rcItem.right, rcItem.bottom, SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}
	else if (idxSub == 2)
	{
		m_CLPColor.SetItemHeight(rcItem.bottom);
		m_CLPColor.SetColor(Bm.crColor);
		SetFocus(m_CLPColor.GetHWND());
		SetWindowPos(m_CLPColor.GetHWND(), NULL,
			rcItem.left, rcItem.top, rcItem.right, rcItem.bottom, SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}
	m_idxEditing = idx;
	m_idxSubEditing = idxSub;
}

void CDlgBookmark::CancelEditing(BOOL bSave)
{
	if (m_idxSubEditing < 1 || m_idxSubEditing>2 || m_idxEditing < 0)
		return;
	LVITEMW lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = m_idxEditing;
	lvi.iSubItem = 0;
	m_LVBookmark.GetItem(&lvi);

	auto& hmBookmark = App->GetPlayer().GetList().GetBookmark();
	auto& Bm = hmBookmark.at((int)lvi.lParam);
	if (m_idxSubEditing == 1)
	{
		if(bSave)
		{
			Bm.rsName = m_EDName.GetText();
			lvi.iSubItem = 1;
			lvi.pszText = Bm.rsName.Data();
			m_LVBookmark.SetItemText(m_idxEditing, &lvi);
		}

		m_EDName.Show(SW_HIDE);
	}
	else if (m_idxSubEditing == 2)
	{
		if(bSave)
		Bm.crColor = m_CLPColor.GetColor();

		m_CLPColor.Show(SW_HIDE);
	}

	m_LVBookmark.RedrawItem(m_idxEditing);
	m_idxEditing = -1;
	m_idxSubEditing = -1;
}

void CDlgBookmark::OnCmdDelete()
{
	const int cItems = m_LVBookmark.GetItemCount();
	auto& Player = App->GetPlayer();
	auto& hmBookmark = Player.GetList().GetBookmark();
	
	LVITEMW lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;
	EckCounter(cItems, i)
	{
		if (m_LVBookmark.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
		{
			lvi.iItem = i;
			m_LVBookmark.GetItem(&lvi);
			auto it = hmBookmark.find((int)lvi.lParam);
			Player.Delete(i);
		}
	}
}

INT_PTR CDlgBookmark::DlgBox(HWND hParent, void* pData)
{
	m_pParam = (PARAM*)pData;

	const int iDpi = eck::GetDpi(hParent);
	const int cx = eck::DpiScale(450, iDpi);
	const int cy = eck::DpiScale(400, iDpi);

	return IntCreateModalDlg(0, eck::WCN_DLG, L"书签管理", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0, cx, cy, hParent, NULL, eck::g_hInstance, pData, eck::DLGNCF_CENTERPARENT);
}