#include "CWndMain.h"
#include "CDlgListFile.h"

constexpr PCWSTR c_szDefListName = L"当前无播放列表";

LRESULT CWndList::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CWndList*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		if (((NMHDR*)lParam)->hwndFrom == p->m_LVList.GetHWND())
			switch (((NMHDR*)lParam)->code)
			{
			case LVN_GETDISPINFOW:
				p->OnLVNGetDispInfo((NMLVDISPINFOW*)lParam);
				return 0;
			case NM_DBLCLK:
				p->OnLVNDbLClick((NMITEMACTIVATE*)lParam);
				return 0;
			case NM_CUSTOMDRAW:
				return p->OnLVNCustomDraw((NMLVCUSTOMDRAW*)lParam);
			}
	}
	break;
	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, p->OnSize);
	case WM_COMMAND:
	{
		if (lParam)
		{
			if ((HWND)lParam != p->m_TBManage.GetHWND())
				break;
			switch (LOWORD(wParam))
			{
			case TBCID_ADD:
			{
				RECT rc;
				p->m_TBManage.GetRect(TBCID_ADD, &rc);
				eck::ClientToScreen(p->m_TBManage.GetHWND(), &rc);
				int iRet = TrackPopupMenu(p->m_hMenuAdd, TPM_NONOTIFY | TPM_RETURNCMD | TPM_VERNEGANIMATION,
					rc.left, rc.bottom, 0, hWnd, NULL);
				switch (iRet)
				{
				case IDMI_ADDFILE:
					p->OnMenuAddFile();
					return 0;
				case IDMI_ADDDIR:
					p->OnMenuAddDir();
					return 0;
				}
			}
			return 0;
			case TBCID_LOADLIST:
				p->OnCmdLoadList();
				return 0;
			case TBCID_SAVELIST:
				p->OnCmdSaveList();
				return 0;
			}
		}
		else
		{

		}
	}
	break;
	case WM_NCCREATE:
		p = (CWndList*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		break;
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, p->OnCreate);
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

BOOL CWndList::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
	UpdateDpiInit(eck::GetDpi(hWnd));

	m_hMenuAdd = CreatePopupMenu();
	AppendMenuW(m_hMenuAdd, 0, IDMI_ADDFILE, L"添加文件...");
	AppendMenuW(m_hMenuAdd, 0, IDMI_ADDDIR, L"添加文件夹...");


	int y = m_Ds.iGap;
	m_LAListName.Create(c_szDefListName, WS_VISIBLE, 0, 0, y, 0, 0, hWnd, IDC_LA_LIST_NAME);
	m_LAListName.SetTransparent(TRUE);
	m_LAListName.SetClr(0, eck::Colorref::CyanBlue);
	y += (m_Ds.cyListName + m_Ds.iGap);

	m_EDSearch.SetMultiLine(FALSE);
	m_EDSearch.Create(NULL, WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, y, 0, 0, hWnd, IDC_ED_SEARCH);
	m_EDSearch.SetFrameType(1);

	m_BTSearch.Create(L"搜索", WS_VISIBLE, 0, 0, y, m_Ds.cySearch, m_Ds.cySearch, hWnd, IDC_BT_SEARCH);
	y += (m_Ds.cySearch + m_Ds.iGap);

	m_TBManage.Create(NULL,
		TBSTYLE_LIST | TBSTYLE_TRANSPARENT | TBSTYLE_WRAPABLE |
		CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER, 0,
		0, y, 0, 0, hWnd, IDC_TB_MANAGE);
	m_TBManage.SetButtonStructSize();
	TBBUTTON TBBtns[]
	{
		{0,TBCID_LOCATE,TBSTATE_ENABLED,0,{},0,(INT_PTR)L"定位"},
		{0,TBCID_ADD,TBSTATE_ENABLED,0,{},0,(INT_PTR)L"添加"},
		{0,TBCID_LOADLIST,TBSTATE_ENABLED,0,{},0,(INT_PTR)L"读取"},
		{0,TBCID_SAVELIST,TBSTATE_ENABLED,0,{},0,(INT_PTR)L"保存"},
		{0,TBCID_EMPTY,TBSTATE_ENABLED,0,{},0,(INT_PTR)L"清空"},
		{0,TBCID_MANAGE,TBSTATE_ENABLED,0,{},0,(INT_PTR)L"管理"},
	};
	m_TBManage.AddButtons(ARRAYSIZE(TBBtns), TBBtns);
	m_TBManage.Show(SW_SHOWNOACTIVATE);
	y += (m_Ds.cyTool + m_Ds.iGap);

	m_LVList.Create(NULL, WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA, 0, 0, y, 0, 0, hWnd, IDC_LV_LIST);
	constexpr DWORD dwLVExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	m_LVList.SetLVExtendStyle(dwLVExStyle);
	m_LVList.InsertColumn(L"名称", 0, 300);
	m_LVList.InsertColumn(L"时长", 1, 100);
	m_LVList.SetExplorerTheme();
	m_hThemeLV = OpenThemeData(m_LVList.GetHWND(), L"ListView");

	eck::LVSetItemHeight(m_LVList, eck::DpiScale(24, m_iDpi));

	eck::SetFontForWndAndCtrl(hWnd, m_hFont);
	m_LAListName.SetFont(m_hFontListName);
	m_TBManage.SetButtonSize(m_Ds.cxToolBtn, m_Ds.cyTool);
	return TRUE;
}

void CWndList::OnSize(HWND hWnd, UINT state, int cx, int cy)
{
	HDWP hDwp = BeginDeferWindowPos(10);
	cx -= m_Ds.iGap;
	hDwp = DeferWindowPos(hDwp, m_LAListName.GetHWND(), NULL, 0, 0,
		cx,
		m_Ds.cyListName,
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_EDSearch.GetHWND(), NULL, 0, 0,
		cx - m_Ds.cySearch,
		m_Ds.cySearch,
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_BTSearch.GetHWND(), NULL,
		cx - m_Ds.cySearch,
		m_Ds.cyListName + m_Ds.iGap * 2,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_TBManage.GetHWND(), NULL, 0, 0,
		cx,
		m_Ds.cyTool,
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_LVList.GetHWND(), NULL,
		0,
		0,
		cx,
		cy - (m_Ds.cyListName + m_Ds.cySearch + m_Ds.cyTool + m_Ds.iGap * 4),
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	EndDeferWindowPos(hDwp);
}

void CWndList::OnCmdLocate()
{
	int idx = App->GetPlayer().GetCurrFile();
	if (idx >= 0)
		m_LVList.EnsureVisible(idx);
}

void CWndList::OnMenuAddFile()
{
	IFileOpenDialog* pfod;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfod));
	if (FAILED(hr))
	{
		CApp::ShowError(m_hWnd, hr, CApp::ErrSrc::HResult, L"IFileOpenDialog创建失败");
		return;
	}
	pfod->SetTitle(L"打开音频文件");
	pfod->SetOptions(FOS_ALLOWMULTISELECT | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);
	constexpr COMDLG_FILTERSPEC cdfs[]
	{
		{L"音频文件(*.mp1;*.mp2;*.xm;*.mp3;*.flac;*.wma;*.wav;*.m4a;*.ogg;*.acc;*.ape;*.aiff)",
			L"*.mp1;*.mp2;*.xm;*.mp3;*.flac;*.wma;*.wav;*.m4a;*.ogg;*.acc;*.ape;*.aiff"},
		{L"所有文件",L"*.*"}
	};
	pfod->SetFileTypes(ARRAYSIZE(cdfs), cdfs);
	pfod->Show(m_hWnd);
	IShellItemArray* psia;
	hr = pfod->GetResults(&psia);
	if (FAILED(hr))
	{
		pfod->Release();
		return;
	}
	DWORD cItem;
	hr = psia->GetCount(&cItem);
	if (FAILED(hr))
	{
		psia->Release();
		pfod->Release();
		return;
	}
	IShellItem* psi;
	PWSTR pszFile;

	auto& pl = App->GetPlayer().GetList();
	LISTFILEITEM_1 Info{};
	EckCounter(cItem, i)
	{
		psia->GetItemAt(i, &psi);
		psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszFile);
		if (pszFile)
		{
			Info.cchFile = (int)wcslen(pszFile);
			pl.Insert(-1, Info, NULL, NULL, pszFile);
			CoTaskMemFree(pszFile);
		}
		psi->Release();
	}

	psia->Release();
	pfod->Release();

	m_LVList.SetItemCount(pl.GetCount());

}

void CWndList::OnMenuAddDir()
{
}

void CWndList::OnCmdLoadList()
{
	CDlgListFile Dlg{};
	CDlgListFile::PARAM Param{ CDlgListFile::Type::Load };
	if (Dlg.DlgBox(m_hWnd, &Param))
	{
		auto& List = App->GetPlayer().GetList();
		List.Delete(-1);
		CPlayListFileReader ListFile{};
		if (!ListFile.Open(Param.rsRetFile.Data()))
		{
			CApp::ShowError(m_hWnd, std::nullopt, CApp::ErrSrc::Win32, L"打开列表文件失败");
			return;
		}
		
		List.Reserve(ListFile.GetItemCount());
		ListFile.For([&List](const LISTFILEITEM_1* pItem, PCWSTR pszName, PCWSTR pszFile, PCWSTR pszTime)->BOOL
			{
				List.Insert(-1, *pItem, pszName, pszTime, pszFile);
				return TRUE;
			});
		ListFile.ForBookmark([&List](const BOOKMARKITEM* pItem, PCWSTR pszName)->BOOL
			{
				List.InsertBookmark(pItem->idxItem, pszName, pItem->cchName, pItem->cr);
				return TRUE;
			});

		m_LVList.SetItemCount(List.GetCount());
	}
}

void CWndList::OnCmdSaveList()
{
	CDlgListFile Dlg{};
	CDlgListFile::PARAM Param{ CDlgListFile::Type::Save };
	if (Dlg.DlgBox(m_hWnd, &Param))
	{
		auto& List = App->GetPlayer().GetList();
		CPlayListFileWriter ListFile{};
		if (!ListFile.Open(Param.rsRetFile.Data()))
		{
			CApp::ShowError(m_hWnd, std::nullopt, CApp::ErrSrc::Win32, L"打开列表文件失败");
			return;
		}

		LISTFILEITEM_1 Info{};
		EckCounter(List.GetCount(), i)
		{
			auto& x = List.AtAbs(i);
			Info.cchName = x.rsName.Size();
			Info.cchTime = x.rsTime.Size();
			Info.cchFile = x.rsFile.Size();
			Info.bBookmark = x.bBookmark;
			Info.bDelayPlaying = x.bDelayPlaying;
			Info.bIgnore = x.bIgnore;
			ListFile.PushBack(Info, x.rsName.Data(), x.rsFile.Data(), x.rsTime.Data());
		}
		ListFile.BeginBookMark();
		BOOKMARKITEM BmInfo{};
		for (auto& x : List.GetBookmark())
		{
			BmInfo.idxItem = x.first;
			BmInfo.cr = x.second.crColor;
			BmInfo.cchName = x.second.rsName.Size();
			ListFile.PushBackBookmark(BmInfo, x.second.rsName.Data());
		}
		ListFile.End();
	}
}

void CWndList::OnLVNGetDispInfo(NMLVDISPINFOW* pnmlvdi)
{
	auto& pl = App->GetPlayer().GetList();
	switch (pnmlvdi->item.iSubItem)
	{
	case 0:
		pnmlvdi->item.pszText = pl.At(pnmlvdi->item.iItem).rsName.Data();
		break;
	}
}

void CWndList::OnLVNDbLClick(NMITEMACTIVATE* pnmia)
{
	if (pnmia->iItem < 0)
		return;
	PlayListItem(pnmia->iItem);
}

LRESULT CWndList::OnLVNCustomDraw(NMLVCUSTOMDRAW* pnmlvcd)
{
	switch (pnmlvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
	{
		HDC hDC = pnmlvcd->nmcd.hdc;
		int iState;
		if (SendMessageW(pnmlvcd->nmcd.hdr.hwndFrom, LVM_GETITEMSTATE, pnmlvcd->nmcd.dwItemSpec, LVIS_SELECTED) == LVIS_SELECTED)//选中
		{
			if (pnmlvcd->nmcd.uItemState & CDIS_HOT)
				iState = LISS_HOTSELECTED;
			else
				iState = LISS_SELECTED;
		}
		else if (pnmlvcd->nmcd.uItemState & CDIS_HOT)
			iState = LISS_HOT;
		else
			iState = 0;

		HBRUSH hBrush;

		if (pnmlvcd->nmcd.dwItemSpec % 2)//交替行色
		{
			hBrush = CreateSolidBrush(0xF3F3F3);
			FillRect(hDC, &pnmlvcd->nmcd.rc, hBrush);
			DeleteObject(hBrush);
		}
		if (pnmlvcd->nmcd.dwItemSpec == App->GetPlayer().GetCurrFile())//标记现行播放项
		{
			hBrush = CreateSolidBrush(0xE6E8B1);
			FillRect(hDC, &pnmlvcd->nmcd.rc, hBrush);
			DeleteObject(hBrush);
		}
		if (iState)//画表项框
			DrawThemeBackground(m_hThemeLV, hDC, LVP_LISTITEM, iState, &pnmlvcd->nmcd.rc, NULL);

		auto& Item = App->GetPlayer().GetList().At((int)pnmlvcd->nmcd.dwItemSpec);

		if (Item.bIgnore)
			SetTextColor(hDC, eck::Colorref::Gray);
		else
			SetTextColor(hDC, eck::Colorref::Black);

		RECT rc = pnmlvcd->nmcd.rc;
		rc.left += m_Ds.cxLVTextSpace;
		rc.right = rc.left + m_LVList.GetColumnWidth(0) - m_Ds.cxLVTextSpace;
		DrawTextW(hDC, Item.rsName.Data(), Item.rsName.Size(), &rc, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
		rc.left = rc.right;
		rc.right = pnmlvcd->nmcd.rc.right;
		DrawTextW(hDC, Item.rsTime.Data(), Item.rsTime.Size(), &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS);
	
		if (Item.bBookmark)
		{
			auto& bm = App->GetPlayer().GetList().AtBookmark((int)pnmlvcd->nmcd.dwItemSpec);
			HGDIOBJ hOldPen = SelectObject(hDC, CreatePen(PS_SOLID, 1, bm.crColor));
			MoveToEx(hDC, pnmlvcd->nmcd.rc.left, pnmlvcd->nmcd.rc.top, NULL);
			LineTo(hDC, pnmlvcd->nmcd.rc.right, pnmlvcd->nmcd.rc.top);
			DeleteObject(SelectObject(hDC, hOldPen));
		}
	}
	return CDRF_SKIPDEFAULT;
	}
}


ATOM CWndList::RegisterWndClass()
{
	WNDCLASSEX wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = App->GetHInstance();
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = WCN_LIST;
	wcex.cbWndExtra = sizeof(void*);
	return RegisterClassExW(&wcex);
}

HWND CWndList::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	m_hWnd = CreateWindowExW(dwExStyle, WCN_LIST, pszText, dwStyle,
		x, y, cx, cy, hParent, eck::i32ToP<HMENU>(nID), App->GetHInstance(), this);
	return m_hWnd;
}

void CWndList::PlayListItem(int idx)
{
	auto& Player = App->GetPlayer();
	int idxOld = Player.GetCurrFile();
	Player.Play(idx);
	if (idxOld >= 0)
		m_LVList.RedrawItems(idxOld, idxOld);
	m_LVList.RedrawItems(idx, idx);
	m_WndMain.SetText((Player.GetList().At(idx).rsName + L" - PlayerNew").Data());
}