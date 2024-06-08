#include "CWndMain.h"
#include "CDlgListFile.h"
#include "CDlgBookmark.h"
#include "CDlgNewBookmark.h"
#include "DragDrop.h"

constexpr static PCWSTR c_szDefListName = L"<无列表>";

class CDropSourceList :public CDropSource
{
public:
	// IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject)
	{
		const static QITAB qit[]
		{
			QITABENT(CDropSourceList, IDropSource),
			{}
		};

		return QISearch(this, qit, iid, ppvObject);
	}
	// IDropSource
	HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
	{
		if (fEscapePressed)
			return DRAGDROP_S_CANCEL;
		if (!eck::IsBitSet(grfKeyState, MK_LBUTTON))
			return DRAGDROP_S_DROP;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect)
	{
		if (dwEffect == DROPEFFECT_NONE)
		{
			SetCursor(LoadCursorW(NULL, IDC_NO));
			return S_OK;
		}
		
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}
};



void CWndList::UpdateDpi(int iDpi)
{
	const int iDpiOld = m_iDpi;
	UpdateDpiInit(iDpi);

	const HFONT hOldFont1 = m_hFont, hOldFont2 = m_hFontListName;
	m_hFont = eck::ReCreateFontForDpiChanged(m_hFont, iDpi, iDpiOld);
	eck::SetFontForWndAndCtrl(m_hWnd, m_hFont);
	DeleteObject(hOldFont1);
	m_hFontListName = eck::ReCreateFontForDpiChanged(m_hFontListName, iDpi, iDpiOld);
	m_LAListName.SetFont(m_hFontListName);
	DeleteObject(hOldFont2);

	UpdateUISize();
}

void CWndList::UpdateUISize()
{
	HDWP hDwp = BeginDeferWindowPos(10);
	hDwp = DeferWindowPos(hDwp, m_LAListName.GetHWND(), NULL,
		0,
		m_Ds.iGap,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_EDSearch.GetHWND(), NULL,
		0,
		m_Ds.cyListName + m_Ds.iGap * 2,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_TBManage.GetHWND(), NULL,
		0,
		m_Ds.cySearch + m_Ds.cyListName + m_Ds.iGap * 3,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_BTSearch.GetHWND(), NULL, 0, 0,
		m_Ds.cySearch,
		m_Ds.cySearch,
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	EndDeferWindowPos(hDwp);

	const auto pBmp = App->ScaleImageForButton(IIDX_Magnifier, m_iDpi);
	DestroyIcon(m_hiSearch);
	m_hiSearch = eck::CreateHICON(pBmp);
	m_BTSearch.SetImage(m_hiSearch, IMAGE_ICON);
	pBmp->Release();

	eck::LVSetItemHeight(m_LVList.HWnd, m_Ds.cyLVItem);
	m_TBManage.SetButtonSize(m_Ds.cxToolBtn, m_Ds.cyTool);
}

BOOL CWndList::AutoSaveList()
{
	if (!App->GetOptionsMgr().ListAutoSave)
		return TRUE;
	auto& Player = App->GetPlayer();
	const auto& Info = Player.GetList().GetInfo();
	if (Info.rsFileName.IsEmpty())
		return TRUE;
	return Player.SaveList(Info.rsFileName.Data());
}

void CWndList::ListSwitched()
{
	const auto& Info = App->GetPlayer().GetList().GetInfo();
	m_LAListName.SetText(Info.rsName.IsEmpty() ? c_szDefListName : Info.rsName.Data());
}

BOOL CWndList::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
#pragma region 创建菜单
	m_hMenuAdd = CreatePopupMenu();
	AppendMenuW(m_hMenuAdd, 0, IDMI_ADDFILE, L"添加文件...");
	AppendMenuW(m_hMenuAdd, 0, IDMI_ADDDIR, L"添加文件夹...");

	m_hMenuLV = CreatePopupMenu();
	AppendMenuW(m_hMenuLV, 0, IDMI_PLAY, L"播放");
	SetMenuDefaultItem(m_hMenuLV, IDMI_PLAY, FALSE);
	AppendMenuW(m_hMenuLV, 0, IDMI_PLAYLATER, L"稍后播放");
	AppendMenuW(m_hMenuLV, MF_SEPARATOR, 0, NULL);
	AppendMenuW(m_hMenuLV, 0, IDMI_OPEN_IN_EXPLORER, L"打开文件位置");
	AppendMenuW(m_hMenuLV, 0, IDMI_DELETE_FROM_LIST, L"从播放列表中删除");
	AppendMenuW(m_hMenuLV, 0, IDMI_DELETE_FROM_DISK, L"从磁盘中删除");
	AppendMenuW(m_hMenuLV, MF_SEPARATOR, 0, NULL);
	AppendMenuW(m_hMenuLV, 0, IDMI_IGNORE, L"忽略/取消忽略");
	AppendMenuW(m_hMenuLV, 0, IDMI_RENAME, L"重命名");
	AppendMenuW(m_hMenuLV, 0, IDMI_INFO, L"详细信息");
	AppendMenuW(m_hMenuLV, MF_SEPARATOR, 0, NULL);
	AppendMenuW(m_hMenuLV, 0, IDMI_PREVBOOKMARK, L"跳到上一书签");
	AppendMenuW(m_hMenuLV, 0, IDMI_NEXTBOOKMARK, L"跳到下一书签");
	AppendMenuW(m_hMenuLV, 0, IDMI_ADDBOOKMARK, L"添加/删除书签");

	m_hMenuManage = CreatePopupMenu();
	AppendMenuW(m_hMenuManage, 0, IDMI_SORT_DEF, L"默认排序");
	AppendMenuW(m_hMenuManage, 0, IDMI_SORT_FILENAME, L"按文件名排序");
	AppendMenuW(m_hMenuManage, 0, IDMI_SORT_NAME, L"按名称排序");
	AppendMenuW(m_hMenuManage, 0, IDMI_SORT_CREATE_TIME, L"按创建时间排序");
	AppendMenuW(m_hMenuManage, 0, IDMI_SORT_MODIFY_TIME, L"按修改时间排序");
	AppendMenuW(m_hMenuManage, 0, IDMI_SORT_REVERSE, L"倒置排序");
	AppendMenuW(m_hMenuManage, MF_SEPARATOR, 0, NULL);
	AppendMenuW(m_hMenuManage, 0, IDMI_SORT_ASCENDING, L"升序");
	AppendMenuW(m_hMenuManage, 0, IDMI_SORT_DESCENDING, L"降序");
	AppendMenuW(m_hMenuManage, 0, IDMI_FIXSORT, L"固定为默认排序");
	AppendMenuW(m_hMenuManage, MF_SEPARATOR, 0, NULL);
	AppendMenuW(m_hMenuManage, 0, IDMI_BOOKMARK_MGR, L"书签...");
	AppendMenuW(m_hMenuManage, 0, IDMI_DETAIL, L"详细信息...");
#pragma endregion
	m_hFont = eck::EzFont(L"微软雅黑", 9);
	m_hFontListName = eck::EzFont(L"微软雅黑", 13);

	m_TBManage.Create(NULL,WS_CHILD | WS_VISIBLE | 
		TBSTYLE_LIST | TBSTYLE_TRANSPARENT | TBSTYLE_WRAPABLE |
		CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER, 0,
		0, 0, 0, 0, hWnd, IDC_TB_MANAGE);
	m_TBManage.SetButtonStructSize();
	TBBUTTON TBBtns[]
	{
		{ 0,TBCID_LOCATE,	TBSTATE_ENABLED,0,{},0,(INT_PTR)L"定位" },
		{ 1,TBCID_ADD,		TBSTATE_ENABLED,0,{},0,(INT_PTR)L"添加" },
		{ 2,TBCID_LOADLIST,	TBSTATE_ENABLED,0,{},0,(INT_PTR)L"读取" },
		{ 3,TBCID_SAVELIST,	TBSTATE_ENABLED,0,{},0,(INT_PTR)L"保存" },
		{ 4,TBCID_EMPTY,	TBSTATE_ENABLED,0,{},0,(INT_PTR)L"清空" },
		{ 5,TBCID_MANAGE,	TBSTATE_ENABLED,0,{},0,(INT_PTR)L"管理" },
	};
	m_TBManage.AddButtons(ARRAYSIZE(TBBtns), TBBtns);

	UpdateDpiInit(eck::GetDpi(hWnd));

	m_LAListName.Create(c_szDefListName, WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_LA_LIST_NAME);
	m_LAListName.SetTransparent(TRUE);
	m_LAListName.SetClr(0, GetSysColor(COLOR_HIGHLIGHT));

	m_EDSearch.SetMultiLine(FALSE);
	m_EDSearch.Create(NULL, WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, 0, hWnd, IDC_ED_SEARCH);
	m_EDSearch.SetFrameType(1);

	m_BTSearch.Create(L"搜索", WS_VISIBLE | BS_ICON, 0, 
		0, 0, m_Ds.cySearch, m_Ds.cySearch, hWnd, IDC_BT_SEARCH);

	constexpr DWORD dwLVStyle = LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_EDITLABELS;
	m_LVSearch.Create(NULL, dwLVStyle, 0, 0, 0, 0, 0, hWnd, IDC_LV_LIST);
	m_LVSearch.SetExplorerTheme();

	m_LVList.Create(NULL, WS_VISIBLE | dwLVStyle, 0, 0, 0, 0, 0, hWnd, IDC_LV_LIST);
	constexpr DWORD dwLVExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	m_LVList.SetLVExtendStyle(dwLVExStyle);
	m_LVList.InsertColumn(L"名称", 0, m_Ds.cxColumn1);
	m_LVList.InsertColumn(L"时长", 1, m_Ds.cxColumn2, LVCFMT_CENTER);

	eck::SetFontForWndAndCtrl(hWnd, m_hFont);
	m_LAListName.SetFont(m_hFontListName);

	UpdateUISize();
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
	const int cBtnsPerRow = cx / m_Ds.cxToolBtn;
	int cRows;
	if (cBtnsPerRow > 0)
	{
		cRows = TBBTCOUNT / cBtnsPerRow;
		if (cRows * cBtnsPerRow < TBBTCOUNT)
			++cRows;
	}
	else
		cRows = 0;
	const int cyTB = m_Ds.cyTool * cRows;
	hDwp = DeferWindowPos(hDwp, m_TBManage.GetHWND(), NULL, 0, 0,
		cx,
		cyTB,
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	const int xLV = m_Ds.cyListName + m_Ds.cySearch + cyTB + m_Ds.iGap * 4;
	const int cyLV = cy - (m_Ds.cyListName + m_Ds.cySearch + cyTB + m_Ds.iGap * 4);
	hDwp = DeferWindowPos(hDwp, m_LVList.GetHWND(), NULL,
		0, xLV, cx, cyLV,
		SWP_NOZORDER | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_LVSearch.GetHWND(), NULL,
		0, xLV, cx, cyLV,
		SWP_NOZORDER | SWP_NOACTIVATE);
	EndDeferWindowPos(hDwp);
}

void CWndList::OnDestroy(HWND hWnd)
{
	AutoSaveList();
	DestroyMenu(m_hMenuAdd);
	DestroyMenu(m_hMenuLV);
	DestroyMenu(m_hMenuManage);
	DeleteObject(m_hFont);
	DeleteObject(m_hFontListName);
	DestroyIcon(m_hiSearch);
	ImageList_Destroy(m_hIml);
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
	constexpr COMDLG_FILTERSPEC c_cdfs[]
	{
		{L"音频文件(*.mp1;*.mp2;*.xm;*.mp3;*.flac;*.wma;*.wav;*.m4a;*.ogg;*.acc;*.ape;*.aiff)",
			L"*.mp1;*.mp2;*.xm;*.mp3;*.flac;*.wma;*.wav;*.m4a;*.ogg;*.acc;*.ape;*.aiff"},
		{L"所有文件",L"*.*"}
	};
	pfod->SetFileTypes(ARRAYSIZE(c_cdfs), c_cdfs);

	eck::GetThreadCtx()->bEnableDarkModeHook = FALSE;
	pfod->Show(HWnd);
	eck::GetThreadCtx()->bEnableDarkModeHook = TRUE;
	IShellItemArray* psia;
	hr = pfod->GetResults(&psia);
	if (FAILED(hr))
	{
		pfod->Release();
		return;
	}
	DWORD cItems;
	hr = psia->GetCount(&cItems);
	if (FAILED(hr))
	{
		psia->Release();
		pfod->Release();
		return;
	}
	IShellItem* psi;
	PWSTR pszFile;

	auto& Player = App->GetPlayer();
	auto& List = Player.GetList();
	LISTFILEITEM_1 Info{};
	Info.s.bNeedUpdated = TRUE;
	List.Reserve(List.GetCount() + cItems);
	int idx = m_LVList.GetCurrSel();
	if (idx < 0)
		idx = m_LVList.GetItemCount();
	const int idxBegin = idx;
	const BOOL bSort = Player.BeginAddOperation();
	EckCounter(cItems, i)
	{
		psia->GetItemAt(i, &psi);
		psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszFile);
		if (pszFile)
		{
			Info.cchFile = (int)wcslen(pszFile);
			Player.Insert(idx, Info, NULL, pszFile, NULL, NULL, NULL, NULL);
			++idx;
			CoTaskMemFree(pszFile);
		}
		psi->Release();
	}
	Player.EndAddOperation(bSort);
	psia->Release();
	pfod->Release();
	m_LVList.SetItemCount(List.GetCount());
}

void CWndList::OnMenuAddDir()
{
	IFileOpenDialog* pfod;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfod));
	if (FAILED(hr))
	{
		CApp::ShowError(m_hWnd, hr, CApp::ErrSrc::HResult, L"IFileOpenDialog创建失败");
		return;
	}
	pfod->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
	eck::GetThreadCtx()->bEnableDarkModeHook = FALSE;
	if (FAILED(pfod->Show(m_hWnd)))
	{
		eck::GetThreadCtx()->bEnableDarkModeHook = TRUE;
		pfod->Release();
		return;
	}
	eck::GetThreadCtx()->bEnableDarkModeHook = TRUE;
	IShellItem* psi;
	hr = pfod->GetResult(&psi);
	if (FAILED(hr))
	{
		pfod->Release();
		return;
	}
	PWSTR pszPath;
	hr = psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszPath);
	psi->Release();
	pfod->Release();
	if (FAILED(hr))
		return;

	const int cchPath = (int)wcslen(pszPath);
	PWSTR pszFindingPattern = (PWSTR)_malloca(eck::Cch2Cb(cchPath + 10));
	EckAssert(pszFindingPattern);
	wcscpy(pszFindingPattern, pszPath);

	constexpr PCWSTR c_szExt[]
	{
		L"*\\.mp1",L"\\*.mp2",L"\\*.xm",L"\\*.mp3",L"\\*.flac",L"\\*.wma",
		L"\\*.wav",L"\\*.m4a",L"\\*.ogg",L"\\*.acc",L"\\*.ape",L"\\*.aiff"
	};

	WIN32_FIND_DATAW wfd;
	HANDLE hFind;

	PWSTR pszTemp = (PWSTR)_malloca(eck::Cch2Cb(cchPath + MAX_PATH + 10));
	EckAssert(pszTemp);
	wcscpy(pszTemp, pszPath);

	int cchFileName;
	LISTFILEITEM_1 Info{};
	Info.s.bNeedUpdated = TRUE;
	auto& Player = App->GetPlayer();

	int idx = m_LVList.GetCurrSel();
	if (idx < 0)
		idx = m_LVList.GetItemCount();
	const int idxBegin = idx;
	const BOOL bSort = Player.BeginAddOperation();
	for (auto pszExt : c_szExt)
	{
		wcscpy(pszFindingPattern + cchPath, pszExt);
		hFind = FindFirstFileW(pszFindingPattern, &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
			continue;
		do
		{
			cchFileName = (int)wcslen(wfd.cFileName);
			*(pszTemp + cchPath) = L'\\';
			wcscpy(pszTemp + cchPath + 1, wfd.cFileName);

			Info.cchFile = cchPath + cchFileName + 1;
			Player.Insert(idx, Info, NULL, pszTemp, NULL, NULL, NULL, NULL);
			++idx;

		} while (FindNextFileW(hFind, &wfd));
		FindClose(hFind);
	}
	Player.EndAddOperation(bSort);
	CoTaskMemFree(pszPath);
	_freea(pszTemp);
	_freea(pszFindingPattern);
	m_LVList.SetItemCount(Player.GetList().GetCount());
}

void CWndList::OnCmdLoadList()
{
	CDlgListFile Dlg{};
	CDlgListFile::PARAM Param{ CDlgListFile::Type::Load };
	if (Dlg.DlgBox(m_hWnd, &Param))
	{
		AutoSaveList();
		auto& Player = App->GetPlayer();
		if (!Player.LoadList(Param.rsRetFile.Data()))
		{
			CApp::ShowError(m_hWnd, std::nullopt, CApp::ErrSrc::Win32, L"打开列表文件失败");
			return;
		}
		ListSwitched();
		m_LVList.SetItemCount(Player.GetList().GetCount());
	}
}

void CWndList::OnCmdSaveList()
{
	CDlgListFile Dlg{};
	CDlgListFile::PARAM Param{ CDlgListFile::Type::Save };
	if (Dlg.DlgBox(m_hWnd, &Param))
	{
		auto& List = App->GetPlayer().GetList();
		if (!App->GetPlayer().SaveList(Param.rsRetFile.Data()))
			CApp::ShowError(m_hWnd, std::nullopt, CApp::ErrSrc::Win32, L"保存列表文件失败");
		if (List.GetInfo().rsFileName.IsEmpty())
		{
			List.SetFileName(Param.rsRetFile.Data());
			ListSwitched();
		}
	}
}

void CWndList::OnCmdEmpty()
{
	if (Utils::MsgBox(L"确定要清空列表？", NULL, L"确认删除", 2, (HICON)TD_WARNING_ICON, m_hWnd) == Utils::MBBID_1)
	{
		App->GetPlayer().Delete(-1);
		m_LVList.SetItemCount(0);
	}
}

void CWndList::OnCmdManage()
{
	auto& Player = App->GetPlayer();
	RECT rc;
	m_TBManage.GetRect(TBCID_MANAGE, &rc);
	eck::ClientToScreen(m_TBManage.GetHWND(), &rc);
	const BOOL bSorting = Player.GetList().IsSorting();
	EnableMenuItem(m_hMenuManage, IDMI_SORT_DEF, bSorting ? MF_ENABLED : MF_GRAYED);
	const int iRet = TrackPopupMenu(m_hMenuManage, TPM_NONOTIFY | TPM_RETURNCMD | TPM_VERNEGANIMATION | TPM_RIGHTBUTTON,
		rc.left, rc.bottom, 0, m_hWnd, NULL);
	switch (iRet)
	{
	case IDMI_SORT_DEF:
		Player.CancelSort();
		m_LVList.Redraw();
		return;
	case IDMI_SORT_FILENAME:
		Player.Sort(PNSF_FILE);
		m_LVList.Redraw();
		return;
	case IDMI_SORT_NAME:
		Player.Sort(PNSF_NAME);
		m_LVList.Redraw();
		return;
	case IDMI_BOOKMARK_MGR:
	{
		CDlgBookmark Dlg{};
		CDlgBookmark::PARAM Param{ m_LVList };
		Dlg.DlgBox(m_hWnd, &Param);
	}
	return;
	}
}

void CWndList::OnListLVNDbLClick(NMITEMACTIVATE* pnmia)
{
	if (pnmia->iItem < 0)
		return;
	PlayListItem(pnmia->iItem);
}

BOOL CWndList::OnListLVNRClick(NMITEMACTIVATE* pnmia)
{
	POINT pt;
	GetCursorPos(&pt);
	const int idx = pnmia->iItem;
	const BOOL bItemValid = (idx >= 0);
	const UINT uFlags = (bItemValid ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(m_hMenuLV, IDMI_PLAY, uFlags);
	EnableMenuItem(m_hMenuLV, IDMI_DELETE_FROM_LIST, uFlags);
	EnableMenuItem(m_hMenuLV, IDMI_DELETE_FROM_DISK, uFlags);
	EnableMenuItem(m_hMenuLV, IDMI_OPEN_IN_EXPLORER, uFlags);
	EnableMenuItem(m_hMenuLV, IDMI_RENAME, uFlags);
	EnableMenuItem(m_hMenuLV, IDMI_INFO, uFlags);
	EnableMenuItem(m_hMenuLV, IDMI_ADDBOOKMARK, uFlags);
	EnableMenuItem(m_hMenuLV, IDMI_NEXTBOOKMARK, uFlags);
	EnableMenuItem(m_hMenuLV, IDMI_PREVBOOKMARK, uFlags);
	EnableMenuItem(m_hMenuLV, IDMI_IGNORE, uFlags);
	EnableMenuItem(m_hMenuLV, IDMI_PLAYLATER, uFlags);

	const int iRet = TrackPopupMenu(m_hMenuLV, TPM_NONOTIFY | TPM_RETURNCMD | TPM_VERNEGANIMATION | TPM_RIGHTBUTTON,
		pt.x, pt.y, 0, m_hWnd, NULL);
	auto& Player = App->GetPlayer();
	auto& List = Player.GetList();
	switch (iRet)
	{
	case IDMI_PLAY:
		PlayListItem(idx);
		break;
	case IDMI_PLAYLATER:
		if (Player.GetLaterPlaying() == idx)
			Player.SetLaterPlaying(-1);
		else
			Player.SetLaterPlaying(idx);
		m_LVList.RedrawItem(idx);
		break;
	case IDMI_OPEN_IN_EXPLORER:
		OnMenuOpenInExplorer();
		break;
	case IDMI_DELETE_FROM_LIST:
		if (Utils::MsgBox(L"确定要从列表中移除选中的项目吗", L"移除后无法恢复",
			L"询问", 2, (HICON)TD_WARNING_ICON, HWnd) == Utils::MBBID_1)
			OnMenuDel(FALSE);
		break;
	case IDMI_DELETE_FROM_DISK:
		if (Utils::MsgBox(L"确定要从磁盘上删除选中的文件吗", L"删除后无法恢复",
			L"询问", 2, (HICON)TD_WARNING_ICON, HWnd) == Utils::MBBID_1)
			OnMenuDel(TRUE);
		break;
	case IDMI_IGNORE:
		ECKBOOLNOT(List.At(idx).s.bIgnore);
		m_LVList.RedrawItem(idx);
		break;
	case IDMI_RENAME:// TODO : 替换默认编辑控件
		m_LVList.EditLabel(idx);
		break;
	case IDMI_INFO:
	{
		m_pDlgMusicInfo->CreateDlg(HWnd,eck::i32ToP<void*>(idx));
	}
	break;
	case IDMI_PREVBOOKMARK:
	{
		for (int i = idx; i >= 0; --i)
		{
			if (List.At(i).s.bBookmark)
			{
				EnsureVisibleBookmark(i);
				break;
			}
		}
	}
	break;
	case IDMI_NEXTBOOKMARK:
	{
		for (int i = idx; i < m_LVList.GetItemCount(); ++i)
		{
			if (List.At(i).s.bBookmark)
			{
				EnsureVisibleBookmark(i);
				break;
			}
		}
	}
	break;
	case IDMI_ADDBOOKMARK:
		OnMenuAddDelBookmark(idx);
		break;
	}
	return FALSE;
}

void CWndList::OnMenuOpenInExplorer()
{
	auto& List = App->GetPlayer().GetList();
	auto pvSelPaths = new std::vector<eck::CRefStrW>{};
	const int cItems = m_LVList.GetItemCount();
	EckCounter(cItems, i)
	{
		if (m_LVList.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
			pvSelPaths->emplace_back(List.At(i).rsFile);
	}

	CloseHandle(eck::CrtCreateThread([](void* pParam)->UINT
		{
			auto pvSelPaths = (std::vector<eck::CRefStrW>*)pParam;
			if (FAILED(CoInitialize(NULL)))
			{
				delete pvSelPaths;
				return 0;
			}
			
			std::unordered_map<std::wstring_view, int> hmPaths{};// 文件夹路径->vPIDL索引
			std::vector<std::pair<LPITEMIDLIST, std::vector<LPITEMIDLIST>>> vPIDL{};// { 文件夹PIDL,{文件PIDL} }
			std::wstring_view svTemp{};
			LPITEMIDLIST pIDL;

			int idxCurr = 0;

			PWSTR pszFileName;
			PCWSTR pszPath;
			for (auto& x : *pvSelPaths)
			{
				pszPath = x.Data();
				pszFileName = PathFindFileNameW(pszPath);
				if (pszFileName != pszPath)
				{
					svTemp = std::wstring_view(pszPath, pszFileName - pszPath);
					auto it = hmPaths.find(svTemp);
					if (it == hmPaths.end())
					{
						WCHAR ch = *(pszFileName - 1);
						*(pszFileName - 1) = L'\0';
						if (FAILED(SHParseDisplayName(pszPath, NULL, &pIDL, 0, NULL)))// 文件夹转PIDL
						{
							*(pszFileName - 1) = ch;
							continue;
						}
						*(pszFileName - 1) = ch;

						it = hmPaths.insert(std::make_pair(svTemp, idxCurr)).first;
						++idxCurr;

						auto& x = vPIDL.emplace_back(pIDL, std::vector<LPITEMIDLIST>());
						if (FAILED(SHParseDisplayName(pszPath, NULL, &pIDL, 0, NULL)))// 文件转PIDL
							continue;
						x.second.emplace_back(pIDL);
					}
					else
					{
						SHParseDisplayName(pszPath, NULL, &pIDL, 0, NULL);// 文件转PIDL
						vPIDL[it->second].second.emplace_back(pIDL);
					}
				}
			}

			for (auto& x : vPIDL)
			{
				SHOpenFolderAndSelectItems(x.first, (UINT)x.second.size(), (LPCITEMIDLIST*)x.second.data(), 0);
				CoTaskMemFree(x.first);
				for (auto pidl : x.second)
					CoTaskMemFree(pidl);
			}

			delete pvSelPaths;
			CoUninitialize();
			return 0;
		}, pvSelPaths));
}

void CWndList::OnMenuDel(BOOL bDelFile)
{
	auto& Player = App->GetPlayer();
	auto& List = Player.GetList();
	const int cItems = m_LVList.GetItemCount();
	const BOOL bSort = Player.BeginSortProtect();
	if (bSort)
	{
		std::vector<int> vIdx{};
		EckCounter(cItems, i)
		{
			if (m_LVList.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
			{
				auto& e = List.At(i);
				m_LVList.SetItemState(i, 0, LVIS_SELECTED);
				vIdx.push_back(e.idxSortMapping);
				if (bDelFile)
					DeleteFileW(e.rsFile.Data());
			}
		}
		std::sort(std::execution::par_unseq, vIdx.begin(), vIdx.end());
		for (auto it = vIdx.rbegin(); it < vIdx.rend(); ++it)
			Player.Delete(*it);
	}
	else
		for (int i = cItems - 1; i >= 0; --i)
		{
			if (m_LVList.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
			{
				m_LVList.SetItemState(i, 0, LVIS_SELECTED);
				if (bDelFile)
					DeleteFileW(List.AtAbs(i).rsFile.Data());
				Player.Delete(i);
			}
		}
	Player.EndSortProtect(bSort);
	m_LVList.SetItemCount(Player.GetList().GetCount());
	if (Player.IsSearching())
	{
		Player.Search(m_rsCurrKeyword.Data());
		m_LVSearch.SetItemCount(Player.GetSearchingResultCount());
	}
}

void CWndList::OnMenuAddDelBookmark(int idx)
{
	auto& List = App->GetPlayer().GetList();
	auto& Item = List.At(idx);
	if (Item.s.bBookmark)
	{
		if (Utils::MsgBox(L"确定要删除书签吗？", NULL, L"询问", 2, (HICON)TD_INFORMATION_ICON, m_hWnd) == Utils::MBBID_1)
		{
			List.DeleteBookmark(List.AbsIndex(idx));
			m_LVList.RedrawItem(idx);
		}
	}
	else
	{
		CDlgNewBookmark Dlg{};
		CDlgNewBookmark::PARAM Param{};
		if (Dlg.DlgBox(m_hWnd, &Param))
		{
			List.InsertBookmark(List.AbsIndex(idx), Param.rsName.Data(), Param.rsName.Size(), Param.cr);
			m_LVList.RedrawItem(idx);
		}
	}
}

void CWndList::OnListLVNBeginDrag(NMLISTVIEW* pnmlv)
{
	if (pnmlv->iItem < 0)
		return;

	std::vector<int> vIdx{};
	auto& List = App->GetPlayer().GetList();
	const int cItems = m_LVList.GetItemCount();

	int cchTotal = 0;
	EckCounter(cItems, i)
	{
		if (m_LVList.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
		{
			vIdx.emplace_back(i);
			cchTotal += (List.At(i).rsFile.Size() + 1);
		}
	}
#pragma region 制HDROP
	SIZE_T cbBuf = sizeof(DROPFILES) + eck::Cch2Cb(cchTotal);
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cbBuf);
	if (!hGlobal)
	{
		EckDbgBreak();
		return;
	}
	void* p = GlobalLock(hGlobal);
	if (!p)
	{
		GlobalFree(hGlobal);
		EckDbgBreak();
		return;
	}

	DROPFILES df
	{
		sizeof(DROPFILES),
		{},
		TRUE,
		TRUE
	};

	eck::CMemWriter w(p, cbBuf);
	w << df;
	for (auto x : vIdx)
		w << List.At(x).rsFile;
	w << L'\0';

	GlobalUnlock(hGlobal);
	// 制格式描述
	FORMATETC fe =
	{
		CF_HDROP,
		NULL,
		DVASPECT_CONTENT,
		-1,
		TYMED_HGLOBAL
	};
	// 制存储介质
	STGMEDIUM sm;
	sm.tymed = TYMED_HGLOBAL;
	sm.hGlobal = hGlobal;
	sm.pUnkForRelease = NULL;

	auto pDataObject = new CDataObject;
	pDataObject->SetData(&fe, &sm, FALSE);
#pragma endregion
#pragma region 制自定义拖放信息
	LISTDRAGPARAMHEADER Header{ LDPH_VER_1,GetCurrentProcessId(),(int)vIdx.size() };
	cbBuf = sizeof(LISTDRAGPARAMHEADER) + vIdx.size() * sizeof(int);
	hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cbBuf);
	if (!hGlobal)
	{
		pDataObject->Release();
		EckDbgBreak();
		return;
	}
	p = GlobalLock(hGlobal);
	if (!p)
	{
		GlobalFree(hGlobal);
		pDataObject->Release();
		EckDbgBreak();
		return;
	}
	eck::CMemWriter w2(p, cbBuf);
	w2 << Header << vIdx;
	GlobalUnlock(hGlobal);
	fe.cfFormat = App->GetListDragClipFormat();
	sm.hGlobal = hGlobal;

	pDataObject->SetData(&fe, &sm, FALSE);
#pragma endregion
	auto pDropSource = new CDropSourceList;

	DWORD dwEffect;
	HRESULT hr = DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);

	pDropSource->Release();
	pDataObject->Release();
}

void CWndList::OnSearchLVNDbLClick(NMITEMACTIVATE* pnmia)
{
	if (pnmia->iItem < 0)
		return;
	PlayListItem(App->GetPlayer().AtSearchingIndex(pnmia->iItem));
}

BOOL CWndList::OnSearchLVNRClick(NMITEMACTIVATE* pnmia)
{
	auto& Player = App->GetPlayer();
	m_LVList.SetItemState(-1, 0, LVIS_SELECTED);
	EckCounter(m_LVSearch.GetItemCount(), i)
	{
		if (m_LVSearch.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
			m_LVList.SetItemState(Player.AtSearchingIndex(i), LVIS_SELECTED, LVIS_SELECTED);
	}
	NMITEMACTIVATE nmia{};
	nmia.iItem = Player.AtSearchingIndex(pnmia->iItem);
	OnListLVNRClick(&nmia);
	m_LVSearch.RedrawItem(pnmia->iItem);
	return FALSE;
}

void CWndList::OnSearchLVNBeginDrag(NMLISTVIEW* pnmlv)
{
	auto& Player = App->GetPlayer();
	m_LVList.SetItemState(-1, 0, LVIS_SELECTED);
	EckCounter(m_LVSearch.GetItemCount(), i)
	{
		if (m_LVSearch.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
			m_LVList.SetItemState(Player.AtSearchingIndex(i), LVIS_SELECTED, LVIS_SELECTED);
	}
	NMLISTVIEW nmlv{};
	nmlv.iItem = Player.AtSearchingIndex(pnmlv->iItem);
	OnListLVNBeginDrag(&nmlv);
}

void CWndList::OnENChange()
{
	auto& Player = App->GetPlayer();
	auto rsKeyword = m_EDSearch.GetText();
	if (rsKeyword != m_rsCurrKeyword)
	{
		m_rsCurrKeyword = std::move(rsKeyword);
		if (m_rsCurrKeyword.Size())
		{
			m_LVList.Show(SW_HIDE);
			m_LVSearch.Show(SW_SHOWNOACTIVATE);
			Player.Search(m_rsCurrKeyword.Data());
			m_LVSearch.SetItemCount(Player.GetSearchingResultCount());
			m_LVSearch.Redraw();
		}
		else if(Player.IsSearching())
		{
			m_LVList.Show(SW_SHOWNOACTIVATE);
			m_LVSearch.Show(SW_HIDE);
			m_LVSearch.SetItemCount(0);
			Player.CancelSearch();
		}
	}
}

BOOL CWndList::OnListLVNEndLabelEdit(NMLVDISPINFOW* pnmlvdi)
{
	if (pnmlvdi->item.pszText)
		App->GetPlayer().GetList().At(pnmlvdi->item.iItem).rsName = pnmlvdi->item.pszText;
	return FALSE;
}

BOOL CWndList::OnSearchLVNEndLabelEdit(NMLVDISPINFOW* pnmlvdi)
{
	if (pnmlvdi->item.pszText)
		App->GetPlayer().AtSearching(pnmlvdi->item.iItem).rsName = pnmlvdi->item.pszText;
	return FALSE;
}

void CWndList::EnsureVisibleBookmark(int idx)
{
	m_LVList.EnsureVisible(idx);
}

ATOM CWndList::RegisterWndClass()
{
	WNDCLASSEX wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DefWindowProcW;
	wcex.hInstance = App->GetHInstance();
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.lpszClassName = WCN_LIST;
	wcex.cbWndExtra = sizeof(void*);
	return RegisterClassExW(&wcex);
}

LRESULT CWndList::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		if (((NMHDR*)lParam)->hwndFrom == m_LVList.GetHWND())
			switch (((NMHDR*)lParam)->code)
			{
			case NM_DBLCLK:
				OnListLVNDbLClick((NMITEMACTIVATE*)lParam);
				return 0;
			case NM_RCLICK:
				return OnListLVNRClick((NMITEMACTIVATE*)lParam);
			case LVN_BEGINDRAG:
				OnListLVNBeginDrag((NMLISTVIEW*)lParam);
				return 0;
			case LVN_ENDLABELEDIT:
				return OnListLVNEndLabelEdit((NMLVDISPINFOW*)lParam);
			}
		else if (((NMHDR*)lParam)->hwndFrom == m_LVSearch.GetHWND())
			switch (((NMHDR*)lParam)->code)
			{
			case NM_DBLCLK:
				OnSearchLVNDbLClick((NMITEMACTIVATE*)lParam);
				return 0;
			case NM_RCLICK:
				return OnSearchLVNRClick((NMITEMACTIVATE*)lParam);
			case LVN_BEGINDRAG:
				OnSearchLVNBeginDrag((NMLISTVIEW*)lParam);
				return 0;
			case LVN_ENDLABELEDIT:
				return OnSearchLVNEndLabelEdit((NMLVDISPINFOW*)lParam);
			}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		SetDCBrushColor(ps.hdc, eck::GetThreadCtx()->crDefBkg);
		FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
		EndPaint(hWnd, &ps);
	}
	return 0;
	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, OnSize);
	case WM_COMMAND:
	{
		if (lParam)
		{
			if ((HWND)lParam == m_TBManage.GetHWND() || HIWORD(wParam) == BN_CLICKED)
				switch (LOWORD(wParam))
				{
				case TBCID_LOCATE:
					OnCmdLocate();
					return 0;
				case TBCID_ADD:
				{
					RECT rc;
					m_TBManage.GetRect(TBCID_ADD, &rc);
					eck::ClientToScreen(m_TBManage.GetHWND(), &rc);
					const int iRet = TrackPopupMenu(m_hMenuAdd, TPM_NONOTIFY | TPM_RETURNCMD | TPM_VERNEGANIMATION | TPM_RIGHTBUTTON,
						rc.left, rc.bottom, 0, hWnd, NULL);
					switch (iRet)
					{
					case IDMI_ADDFILE:
						OnMenuAddFile();
						return 0;
					case IDMI_ADDDIR:
						OnMenuAddDir();
						return 0;
					}
				}
				return 0;
				case TBCID_LOADLIST:
					OnCmdLoadList();
					return 0;
				case TBCID_SAVELIST:
					OnCmdSaveList();
					return 0;
				case TBCID_EMPTY:
					OnCmdEmpty();
					return 0;
				case TBCID_MANAGE:
					OnCmdManage();
					return 0;
				}
			else if ((HWND)lParam == m_EDSearch.GetHWND() || HIWORD(wParam) == EN_CHANGE)
			{
				OnENChange();
				return 0;
			}
		}
		else
		{

		}
	}
	break;
	case WM_SYSCOLORCHANGE:
		m_LAListName.SetClr(0, GetSysColor(COLOR_HIGHLIGHT));
		break;
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
	case WM_DESTROY:
		return HANDLE_WM_DESTROY(hWnd, wParam, lParam, OnDestroy);
	case WM_DPICHANGED:
	{
		auto prc = (RECT*)lParam;
		UpdateDpi(LOWORD(wParam));
		SetWindowPos(hWnd, NULL, prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	return 0;
	case WM_DPICHANGED_BEFOREPARENT:
		UpdateDpi(eck::GetDpi(hWnd));
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

void CWndList::PlayListItem(int idx)
{
	auto& Player = App->GetPlayer();
	int idxOld = Player.GetCurrFile();
	auto uRet = Player.Play(idx);
	if (idxOld >= 0)
	{
		m_LVList.RedrawItem(idxOld);
		if (Player.IsSearching())
		{
			const auto& vResult = Player.GetSearchingResult();
			auto it = std::find(std::execution::par_unseq, vResult.begin(), vResult.end(), idxOld);
			if (it != vResult.end())
				m_LVSearch.RedrawItem((int)std::distance(vResult.begin(), it));
		}
	}

	if (uRet == PlayOpErr::Ok)
	{
		m_LVList.RedrawItem(idx);
		if (Player.IsSearching())
		{
			const auto& vResult = Player.GetSearchingResult();
			auto it = std::find(std::execution::par_unseq, vResult.begin(), vResult.end(), idx);
			if (it != vResult.end())
				m_LVSearch.RedrawItem((int)std::distance(vResult.begin(), it));
		}
	}
	else
		CPlayer::ShowPlayErr(m_hWnd, uRet);
}