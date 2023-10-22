#include "CDlgListFile.h"

LRESULT CALLBACK CDlgListFile::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CDlgListFile*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		if (((NMHDR*)lParam)->code == LVN_ITEMCHANGED &&
			((NMHDR*)lParam)->idFrom == IDC_LV_FILE)
		{
			p->OnLVNItemChanged((NMLISTVIEW*)lParam);
			return 0;
		}
	}
	break;

	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, p->OnSize);

	case WM_INITDIALOG:
	{
		p = (CDlgListFile*)lParam;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		p->OnInitDialog(hWnd);
	}
	return FALSE;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_DESTROY:
		return HANDLE_WM_DESTROY(hWnd, wParam, lParam, p->OnDestroy);

	case WM_DPICHANGED:
		return HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, p->OnDpiChanged);

	case WM_COMMAND:
	{
		if (lParam && HIWORD(wParam) == BN_CLICKED)
			switch (LOWORD(wParam))
			{
			case IDC_BT_OK:
				p->OnCmdOk();
				return 0;
			case IDC_BT_CANCEL:
				p->OnCmdCancel();
				return 0;
			}
	}
	break;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void CDlgListFile::OnInitDialog(HWND hWnd)
{
	UpdateDpiInit(eck::GetDpi(hWnd));
	m_hFont = eck::EzFont(L"΢���ź�", 9, 400, FALSE, FALSE, FALSE, m_hWnd);
	m_EDFile.Create(NULL, WS_TABSTOP | WS_GROUP | WS_CHILD | WS_VISIBLE | (m_pParam->uType == Type::Load ? ES_READONLY : 0), 0,
		m_Ds.iMargin, m_Ds.iMargin, 0, 0, hWnd, IDC_ED_FILE);
	m_EDFile.SetFrameType(1);
	m_EDFile.SetClr(2, GetSysColor(COLOR_WINDOW));

	m_LVFile.Create(NULL, WS_TABSTOP | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 0,
		m_Ds.iMargin, m_Ds.iMargin + m_Ds.cyEdit + m_Ds.iGap, 0, 0, hWnd, IDC_LV_FILE);
	constexpr DWORD dwExLVStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	eck::LVSetItemHeight(m_LVFile, m_Ds.cyLVItem);
	m_LVFile.SetLVExtendStyle(dwExLVStyle);
	m_LVFile.SetExplorerTheme();
	m_LVFile.EnableGroupView(TRUE);
	m_LVFile.InsertColumn(L"�ļ���", 0, m_Ds.cxColumn1);
	m_LVFile.InsertColumn(L"�޸�ʱ��", 1, m_Ds.cxColumn2);

	const auto& vPath = COptionsMgr::GetInst().vListPath;

	LVGROUP lvg;
	lvg.cbSize = sizeof(LVGROUP);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_STATE;
	lvg.state = LVGS_NORMAL | LVGS_COLLAPSIBLE;
	LVITEMW li;
	li.mask = LVIF_TEXT | LVIF_GROUPID;

	FILETIME ft;
	SYSTEMTIME st;
	WCHAR szTime[20];

	WIN32_FIND_DATAW wfd;
	HANDLE hFind;
	int idx = 0;
	EckCounter(vPath.size(), i)
	{
		lvg.pszHeader = vPath[i].Data();
		lvg.iGroupId = i;
		m_LVFile.InsertGroup(i, &lvg);

		hFind = FindFirstFileW((vPath[i] + L"\\*.PNList").Data(), &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
			continue;

		li.iGroupId = i;
		do
		{
			li.iItem = idx;
			li.iSubItem = 0;
			PathRemoveExtensionW(wfd.cFileName);
			li.pszText = wfd.cFileName;
			idx = m_LVFile.InsertItem(&li);

			FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &ft);
			FileTimeToSystemTime(&ft, &st);

			swprintf_s(szTime, L"%d-%02d-%02d %02d:%02d:%02d",
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

			li.iSubItem = 1;
			li.pszText = szTime;
			m_LVFile.SetItemText(idx, &li);
			++idx;
		} while (FindNextFileW(hFind, &wfd));
		FindClose(hFind);
	}
	m_LVFile.Show(SW_SHOWNOACTIVATE);

	m_BTOk.Create(L"ȷ��", WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 0,
		0, 0, m_Ds.cxBtn, m_Ds.cyBtn, hWnd, IDC_BT_OK);
	m_BTCancel.Create(L"ȡ��", WS_TABSTOP | WS_CHILD | WS_VISIBLE, 0,
		0, 0, m_Ds.cxBtn, m_Ds.cyBtn, hWnd, IDC_BT_CANCEL);
	eck::SetFontForWndAndCtrl(hWnd, m_hFont);

	RECT rc;
	GetClientRect(hWnd, &rc);
	SendMsg(WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));

	SetFocus(m_LVFile.GetHWND());
}

void CDlgListFile::OnSize(HWND hWnd, UINT uState, int cx, int cy)
{
	HDWP hDwp = BeginDeferWindowPos(10);
	hDwp = DeferWindowPos(hDwp, m_EDFile.GetHWND(), NULL, 0, 0,
		cx - m_Ds.iMargin * 2,
		m_Ds.cyEdit,
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_LVFile.GetHWND(), NULL, 0, 0,
		cx - m_Ds.iMargin * 2,
		cy - m_Ds.iMargin * 2 - m_Ds.iGap * 2 - m_Ds.cyBtn - m_Ds.cyEdit,
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_BTOk.GetHWND(), NULL,
		cx - m_Ds.iMargin - m_Ds.cxBtn * 2 - m_Ds.iGap,
		cy - m_Ds.iMargin - m_Ds.cyBtn,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_BTCancel.GetHWND(), NULL,
		cx - m_Ds.iMargin - m_Ds.cxBtn,
		cy - m_Ds.iMargin - m_Ds.cyBtn,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	EndDeferWindowPos(hDwp);
}

void CDlgListFile::OnDestroy(HWND hWnd)
{
	DeleteObject(m_hFont);
}

void CDlgListFile::OnDpiChanged(HWND hWnd, int xDpi, int yDpi, RECT* pRect)
{
	UpdateDpi(xDpi);
	SetWindowPos(hWnd, NULL,
		pRect->left,
		pRect->top,
		pRect->right - pRect->left,
		pRect->bottom - pRect->top,
		SWP_NOZORDER | SWP_NOACTIVATE);
}

void CDlgListFile::OnCmdOk()
{
	m_bRet = TRUE;
	m_pParam->rsRetFile = m_EDFile.GetText();
	PostQuitMessage(0);
}

void CDlgListFile::OnCmdCancel()
{
	m_bRet = FALSE;
	PostQuitMessage(0);
}

void CDlgListFile::OnLVNItemChanged(NMLISTVIEW* pnmlv)
{
	if (pnmlv->iItem < 0)
		return;

	LVITEMW lvi;
	lvi.mask = LVIF_GROUPID;
	lvi.iItem = pnmlv->iItem;
	lvi.iSubItem = 0;
	m_LVFile.GetItem(&lvi);

	WCHAR szPath[MAX_PATH];
	*szPath = L'\0';
	LVGROUP lvg;
	lvg.cbSize = sizeof(LVGROUP);
	lvg.mask = LVGF_HEADER;
	lvg.pszHeader = szPath;
	lvg.cchHeader = MAX_PATH;
	m_LVFile.GetGroup(lvi.iGroupId, &lvg);

	int cchPath = (int)wcslen(szPath);
	*(szPath + cchPath) = L'\\';
	++cchPath;
	cchPath+= m_LVFile.GetItemText(pnmlv->iItem, 0, szPath + cchPath , MAX_PATH - cchPath );

	wcscpy_s(szPath + cchPath, MAX_PATH - cchPath, L".PNList");

	m_EDFile.SetText(szPath);
}

void CDlgListFile::UpdateDpiInit(int iDpi)
{
	m_iDpi = iDpi;
	eck::UpdateDpiSize(m_Ds, iDpi);
}

void CDlgListFile::UpdateDpi(int iDpi)
{
	int iDpiOld = m_iDpi;
	UpdateDpiInit(iDpi);

	m_hFont = eck::ReCreateFontForDpiChanged(m_hFont, iDpi, iDpiOld, TRUE);

	eck::SetFontForWndAndCtrl(m_hWnd, m_hFont);
	eck::LVSetItemHeight(m_LVFile, m_Ds.cyLVItem);
	HDWP hDwp = BeginDeferWindowPos(4);
	hDwp = DeferWindowPos(hDwp, m_EDFile.GetHWND(), NULL,
		m_Ds.iMargin,
		m_Ds.iMargin,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_LVFile.GetHWND(), NULL,
		m_Ds.iMargin, m_Ds.iMargin + m_Ds.cyEdit + m_Ds.iGap,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_BTOk.GetHWND(), NULL, 0, 0,
		m_Ds.cxBtn, m_Ds.cyBtn, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_BTCancel.GetHWND(), NULL, 0, 0,
		m_Ds.cxBtn, m_Ds.cyBtn, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	EndDeferWindowPos(hDwp);
}

INT_PTR CDlgListFile::DlgBox(HWND hParent, void* pData)
{
	HWND hOwner = PreModal(hParent);
	BOOL bNeedEnableOwner;
	if (hOwner && hOwner != GetDesktopWindow() && IsWindowEnabled(hOwner))
	{
		bNeedEnableOwner = TRUE;
		EnableWindow(hOwner, FALSE);
	}
	else
		bNeedEnableOwner = FALSE;

	m_pParam = (PARAM*)pData;
	constexpr PCWSTR c_szTile[]{ L"�����б�",L"�����б�" };
	RECT rc;
	if (hParent)
		GetWindowRect(hParent, &rc);
	else
		GetWindowRect(GetDesktopWindow(), &rc);

	const int iDpi = eck::GetDpi(hParent);
	const int cx = eck::DpiScale(560, iDpi);
	const int cy = eck::DpiScale(500, iDpi);

	m_hWnd = CreateWindowExW(0, eck::WCN_DLG, c_szTile[(UINT)m_pParam->uType], WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		rc.left + (rc.right - rc.left - cx) / 2, rc.top + (rc.bottom - rc.top - cy) / 2, cx, cy,
		hParent, NULL, eck::g_hInstance, pData);
	eck::SetWindowProc(m_hWnd, WndProc);
	SendMsg(WM_INITDIALOG, 0, (LPARAM)this);

	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessageW(msg.hwnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	if (bNeedEnableOwner)
		EnableWindow(hOwner, TRUE);
	if (hParent)
		SetActiveWindow(hParent);
	Destroy();

	return m_bRet && m_pParam->rsRetFile.Size();
}