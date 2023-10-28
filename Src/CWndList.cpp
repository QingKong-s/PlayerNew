#include "CWndMain.h"
#include "CDlgListFile.h"
#include "CDlgBookmark.h"
#include "CDlgNewBookmark.h"
#include "DragDrop.h"

constexpr static PCWSTR c_szDefListName = L"当前无播放列表";
constexpr static UINT_PTR c_uScidLVList = 20231026'01;
constexpr static UINT_PTR c_uScidLVSearch = 20231028'01;

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
			case NM_CUSTOMDRAW:
				return p->OnListLVNCustomDraw((NMLVCUSTOMDRAW*)lParam);
			case NM_DBLCLK:
				p->OnListLVNDbLClick((NMITEMACTIVATE*)lParam);
				return 0;
			case NM_RCLICK:
				return p->OnListLVNRClick((NMITEMACTIVATE*)lParam);
			case LVN_BEGINDRAG:
				p->OnListLVNBeginDrag((NMLISTVIEW*)lParam);
				return 0;
			}
		else if (((NMHDR*)lParam)->hwndFrom == p->m_LVSearch.GetHWND())
			switch (((NMHDR*)lParam)->code)
			{
			case NM_CUSTOMDRAW:
				return p->OnSearchLVNCustomDraw((NMLVCUSTOMDRAW*)lParam);
			case NM_DBLCLK:
				p->OnSearchLVNDbLClick((NMITEMACTIVATE*)lParam);
				return 0;
			case NM_RCLICK:
				return p->OnSearchLVNRClick((NMITEMACTIVATE*)lParam);
			case LVN_BEGINDRAG:
				p->OnSearchLVNBeginDrag((NMLISTVIEW*)lParam);
				return 0;
			}
	}
	break;
	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, p->OnSize);
	case WM_COMMAND:
	{
		if (lParam)
		{
			if ((HWND)lParam == p->m_TBManage.GetHWND() || HIWORD(wParam) == BN_CLICKED)
				switch (LOWORD(wParam))
				{
				case TBCID_LOCATE:
					p->OnCmdLocate();
					return 0;
				case TBCID_ADD:
				{
					RECT rc;
					p->m_TBManage.GetRect(TBCID_ADD, &rc);
					eck::ClientToScreen(p->m_TBManage.GetHWND(), &rc);
					const int iRet = TrackPopupMenu(p->m_hMenuAdd, TPM_NONOTIFY | TPM_RETURNCMD | TPM_VERNEGANIMATION | TPM_RIGHTBUTTON,
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
				case TBCID_EMPTY:
					p->OnCmdEmpty();
					return 0;
				case TBCID_MANAGE:
					p->OnCmdManage();
					return 0;
				}
			else if ((HWND)lParam == p->m_EDSearch.GetHWND() || HIWORD(wParam) == EN_CHANGE)
			{
				p->OnENChange();
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
	case WM_DESTROY:
		return HANDLE_WM_DESTROY(hWnd, wParam, lParam, p->OnDestroy);
	case WM_DPICHANGED:
	{
		auto prc = (RECT*)lParam;
		p->UpdateDpi(LOWORD(wParam));
		SetWindowPos(hWnd, NULL, prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	return 0;
	case WM_DPICHANGED_BEFOREPARENT:
		p->UpdateDpi(eck::GetDpi(hWnd));
		return 0;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CWndList::SubclassProc_LVList(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto p = (CWndList*)dwRefData;
	switch (uMsg)
	{
	case LVM_DELETECOLUMN:
	case LVM_INSERTCOLUMNW:
	case LVM_SETBKCOLOR:
	case LVM_SETBKIMAGEW:
	case LVM_SETCOLUMNW:
	case LVM_SETCOLUMNORDERARRAY:
	case LVM_SETCOLUMNWIDTH:
	case LVM_SETEXTENDEDLISTVIEWSTYLE:
	case LVM_SETOUTLINECOLOR:
	case LVM_SETTEXTBKCOLOR:
	case LVM_SETTEXTCOLOR:
	case LVM_SETVIEW:
		SendMessageW(p->m_LVSearch.GetHWND(), uMsg, wParam, lParam);
		break;
	case LVM_SETIMAGELIST:
	{
		HIMAGELIST hIml = ImageList_Duplicate((HIMAGELIST)lParam);
		SendMessageW(p->m_LVSearch.GetHWND(), uMsg, wParam, (LPARAM)hIml);
	}
	break;
	case WM_KEYDOWN:
		if (wParam == VK_RETURN)// 按回车键播放曲目
		{
			const int idx = p->m_LVList.GetCurrSel();
			if (idx < 0)
				break;
			p->PlayListItem(idx);
		}
		else if (wParam == 0x41)// A键按下
			if (GetKeyState(VK_CONTROL) & 0x80000000)// Ctrl + A 全选
				p->m_LVList.SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
		break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CWndList::SubclassProc_LVSearch(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto p = (CWndList*)dwRefData;
	switch (uMsg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_RETURN)// 按回车键播放曲目
		{
			const int idx = p->m_LVSearch.GetCurrSel();
			if (idx < 0)
				break;
			p->PlayListItem(App->GetPlayer().AtSearchingIndex(idx));
		}
		else if (wParam == 0x41)// A键按下
			if (GetKeyState(VK_CONTROL) & 0x80000000)// Ctrl + A 全选
				p->m_LVSearch.SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
		break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

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

	eck::LVSetItemHeight(m_LVList, m_Ds.cyLVItem);
	m_TBManage.SetButtonSize(m_Ds.cxToolBtn, m_Ds.cyTool);
}

BOOL CWndList::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
	UpdateDpiInit(eck::GetDpi(hWnd));
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

	m_hbrLaterPlaying = CreateSolidBrush(eck::Colorref::CyanBlue);
	m_hbrGray = CreateSolidBrush(0xF3F3F3);
	m_hbrCurrPlaying = CreateSolidBrush(0xE6E8B1);

	m_LAListName.Create(c_szDefListName, WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_LA_LIST_NAME);
	m_LAListName.SetTransparent(TRUE);
	m_LAListName.SetClr(0, eck::Colorref::CyanBlue);

	m_EDSearch.SetMultiLine(FALSE);
	m_EDSearch.Create(NULL, WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, 0, hWnd, IDC_ED_SEARCH);
	m_EDSearch.SetFrameType(1);

	m_BTSearch.Create(L"搜索", WS_VISIBLE, 0, 0, 0, m_Ds.cySearch, m_Ds.cySearch, hWnd, IDC_BT_SEARCH);

	m_TBManage.Create(NULL,
		TBSTYLE_LIST | TBSTYLE_TRANSPARENT | TBSTYLE_WRAPABLE |
		CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER, 0,
		0, 0, 0, 0, hWnd, IDC_TB_MANAGE);
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

	constexpr DWORD dwLVStyle = LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA;
	m_LVSearch.Create(NULL, dwLVStyle, 0, 0, 0, 0, 0, hWnd, IDC_LV_LIST);
	SetWindowSubclass(m_LVSearch.GetHWND(), SubclassProc_LVSearch, c_uScidLVSearch, (DWORD_PTR)this);
	m_LVSearch.SetExplorerTheme();

	m_LVList.Create(NULL, WS_VISIBLE | dwLVStyle, 0, 0, 0, 0, 0, hWnd, IDC_LV_LIST);
	SetWindowSubclass(m_LVList.GetHWND(), SubclassProc_LVList, c_uScidLVList, (DWORD_PTR)this);
	constexpr DWORD dwLVExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	m_LVList.SetLVExtendStyle(dwLVExStyle);
	m_LVList.InsertColumn(L"名称", 0, m_Ds.cxColumn1);
	m_LVList.InsertColumn(L"时长", 1, m_Ds.cxColumn2, LVCFMT_CENTER);
	m_LVList.SetExplorerTheme();
	m_hThemeLV = OpenThemeData(m_LVList.GetHWND(), L"ListView");

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
	DestroyMenu(m_hMenuAdd);
	DestroyMenu(m_hMenuLV);
	DestroyMenu(m_hMenuManage);
	DeleteObject(m_hFont);
	DeleteObject(m_hFontListName);
	DeleteObject(m_hbrCurrPlaying);
	DeleteObject(m_hbrGray);
	DeleteObject(m_hbrLaterPlaying);
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
	pfod->Show(m_hWnd);
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
	List.Reserve(List.GetCount() + cItems);
	int idx = m_LVList.GetCurrSel();
	if (idx < 0)
		idx = m_LVList.GetItemCount();
	const int idxBegin = idx;
	const BOOL bSort = Player.BeginSortProtect();
	EckCounter(cItems, i)
	{
		psia->GetItemAt(i, &psi);
		psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszFile);
		if (pszFile)
		{
			Info.cchFile = (int)wcslen(pszFile);
			Player.Insert(idx, Info, NULL, NULL, pszFile);
			++idx;
			CoTaskMemFree(pszFile);
		}
		psi->Release();
	}
	Player.EndSortProtect(bSort);
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
	if (FAILED(pfod->Show(m_hWnd)))
	{
		pfod->Release();
		return;
	}
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
	auto& Player = App->GetPlayer();

	int idx = m_LVList.GetCurrSel();
	if (idx < 0)
		idx = m_LVList.GetItemCount();
	const int idxBegin = idx;
	const BOOL bSort = Player.BeginSortProtect();
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
			Player.Insert(idx, Info, NULL, NULL, pszTemp);
			++idx;

		} while (FindNextFileW(hFind, &wfd));
		FindClose(hFind);
	}
	Player.EndSortProtect(bSort);
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
		auto& Player = App->GetPlayer();
		auto& List = Player.GetList();
		Player.Delete(-1);
		CPlayListFileReader ListFile{};
		if (!ListFile.Open(Param.rsRetFile.Data()))
		{
			CApp::ShowError(m_hWnd, std::nullopt, CApp::ErrSrc::Win32, L"打开列表文件失败");
			return;
		}

		Player.Delete(-1);
		List.Reserve(ListFile.GetItemCount());
		ListFile.For([&Player](const LISTFILEITEM_1* pItem, PCWSTR pszName, PCWSTR pszFile, PCWSTR pszTime)->BOOL
			{
				Player.Insert(-1, *pItem, pszName, pszTime, pszFile);
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

LRESULT CWndList::OnListLVNCustomDraw(NMLVCUSTOMDRAW* pnmlvcd)
{
	switch (pnmlvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
	{
		auto& Item = App->GetPlayer().GetList().At((int)pnmlvcd->nmcd.dwItemSpec);
		const HDC hDC = pnmlvcd->nmcd.hdc;
		const int idx = (int)pnmlvcd->nmcd.dwItemSpec;

		if (idx == App->GetPlayer().GetCurrFile())// 标记现行播放项
			FillRect(hDC, &pnmlvcd->nmcd.rc, m_hbrCurrPlaying);
		else if (idx % 2)// 交替行色
			FillRect(hDC, &pnmlvcd->nmcd.rc, m_hbrGray);

		int iState;
		if (m_LVList.GetItemState(idx, LVIS_SELECTED) == LVIS_SELECTED)// 选中
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
		if (iState)// 画表项框
			DrawThemeBackground(m_hThemeLV, hDC, LVP_LISTITEM, iState, &pnmlvcd->nmcd.rc, NULL);

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

		if (idx == App->GetPlayer().GetLaterPlaying())// 稍后播放
			FrameRect(hDC, &pnmlvcd->nmcd.rc, m_hbrLaterPlaying);

		if (Item.bBookmark)
		{
			auto& bm = App->GetPlayer().GetList().AtBookmark(idx);
			HGDIOBJ hOldPen = SelectObject(hDC, CreatePen(PS_SOLID, 1, bm.crColor));
			MoveToEx(hDC, pnmlvcd->nmcd.rc.left, pnmlvcd->nmcd.rc.top, NULL);
			LineTo(hDC, pnmlvcd->nmcd.rc.right, pnmlvcd->nmcd.rc.top);
			DeleteObject(SelectObject(hDC, hOldPen));
		}
	}
	return CDRF_SKIPDEFAULT;
	}
	return CDRF_SKIPDEFAULT;
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
		OnMenuDelFromList();
		break;
	case IDMI_DELETE_FROM_DISK:
#pragma message("TODO：删除文件")
		break;
	case IDMI_IGNORE:
		EckBoolNot(List.At(idx).bIgnore);
		m_LVList.RedrawItem(idx);
		break;
	case IDMI_RENAME:
#pragma message("TODO：重命名")
		break;
	case IDMI_DETAIL:
#pragma message("TODO：详细信息")
		break;
	case IDMI_PREVBOOKMARK:
	{
		for (int i = idx; i >= 0; --i)
		{
			if (List.At(i).bBookmark)
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
			if (List.At(i).bBookmark)
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

	CloseHandle(eck::CRTCreateThread([](void* pParam)->UINT
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

void CWndList::OnMenuDelFromList()
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
				m_LVList.SetItemState(i, 0, LVIS_SELECTED);
				vIdx.emplace_back(List.At(i).idxSortMapping);
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
	if (Item.bBookmark)
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

LRESULT CWndList::OnSearchLVNCustomDraw(NMLVCUSTOMDRAW* pnmlvcd)
{
	switch (pnmlvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
	{
		const HDC hDC = pnmlvcd->nmcd.hdc;
		const int idx = (int)pnmlvcd->nmcd.dwItemSpec;
		const int idxReal = App->GetPlayer().AtSearchingIndex(idx);
		auto& Item = App->GetPlayer().AtSearching(idx);

		if (idxReal == App->GetPlayer().GetCurrFile())// 标记现行播放项
			FillRect(hDC, &pnmlvcd->nmcd.rc, m_hbrCurrPlaying);
		else if (idx % 2)// 交替行色
			FillRect(hDC, &pnmlvcd->nmcd.rc, m_hbrGray);

		int iState;
		if (m_LVSearch.GetItemState(idx, LVIS_SELECTED) == LVIS_SELECTED)// 选中
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
		if (iState)// 画表项框
			DrawThemeBackground(m_hThemeLV, hDC, LVP_LISTITEM, iState, &pnmlvcd->nmcd.rc, NULL);

		if (Item.bIgnore)
			SetTextColor(hDC, eck::Colorref::Gray);
		else
			SetTextColor(hDC, eck::Colorref::Black);

		const int xColumn2 = pnmlvcd->nmcd.rc.left + m_LVSearch.GetColumnWidth(0);
		RECT rc = pnmlvcd->nmcd.rc;
		rc.left += m_Ds.cxLVTextSpace;
		rc.right = xColumn2;
		const RECT rcColumn1{ rc };
		DrawTextW(hDC, Item.rsName.Data(), Item.rsName.Size(), &rc, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
		rc.left = rc.right;
		rc.right = pnmlvcd->nmcd.rc.right;
		DrawTextW(hDC, Item.rsTime.Data(), Item.rsTime.Size(), &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS);

		if (idxReal == App->GetPlayer().GetLaterPlaying())// 稍后播放
			FrameRect(hDC, &pnmlvcd->nmcd.rc, m_hbrLaterPlaying);

		int pos = 0;
		SIZE sizeKeyword;
		GetTextExtentPoint32W(hDC, m_rsCurrKeyword.Data(), m_rsCurrKeyword.Size(), &sizeKeyword);
		RECT rcBK{ 0,pnmlvcd->nmcd.rc.top,m_LVSearch.GetColumnWidth(0),pnmlvcd->nmcd.rc.bottom };

		IntersectClipRect(hDC, rcColumn1.left, rcColumn1.top, rcColumn1.right, rcColumn1.bottom);
		while ((pos = eck::FindStr(Item.rsName.Data(), m_rsCurrKeyword.Data(), pos)) != eck::INVALID_STR_POS)
		{
			if (pos)
			{
				rc = pnmlvcd->nmcd.rc;
				DrawTextW(hDC, Item.rsName.Data(), pos, &rc,
					DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_CALCRECT);
				rcBK.left = rc.right + m_Ds.cxLVTextSpace;
			}
			else
				rcBK.left = m_Ds.cxLVTextSpace;
			rcBK.right = rcBK.left + sizeKeyword.cx;
			if (rcBK.left < xColumn2)
			{
				const int iOldMode = SetBkMode(hDC, OPAQUE);
				const COLORREF crOld = SetBkColor(hDC, eck::Colorref::Yellow);
				DrawTextW(hDC, m_rsCurrKeyword.Data(), m_rsCurrKeyword.Size(), &rcBK, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
				SetBkMode(hDC, iOldMode);
				SetBkColor(hDC, crOld);
			}
			++pos;
		}
		SelectClipRgn(hDC, NULL);
	}
	return CDRF_SKIPDEFAULT;
	}
	return CDRF_SKIPDEFAULT;
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

void CWndList::EnsureVisibleBookmark(int idx)
{
	m_LVList.EnsureVisible(idx);
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
	int iBassErr = BASS_OK;
	auto pszErrMsg = CPlayer::GetErrMsgToShow(uRet, &iBassErr);
	if (!pszErrMsg)
	{
		m_LVList.RedrawItem(idx);
		if (Player.IsSearching())
		{
			const auto& vResult = Player.GetSearchingResult();
			auto it = std::find(std::execution::par_unseq, vResult.begin(), vResult.end(), idx);
			if (it != vResult.end())
				m_LVSearch.RedrawItem((int)std::distance(vResult.begin(), it));
		}
		m_WndMain.SetText(Player.GetList().At(idx).rsName.Data());
	}
	else
	{
		m_WndMain.SetText(c_szDefMainWndText);
		std::wstring s;
		if (iBassErr != BASS_OK)
		{
			s = std::format(L"{}错误代码：0x{:08X}。", pszErrMsg, iBassErr);
			pszErrMsg = s.c_str();
		}
		CApp::ShowError(m_hWnd, (DWORD)uRet, CApp::ErrSrc::None, pszErrMsg);
	}
}