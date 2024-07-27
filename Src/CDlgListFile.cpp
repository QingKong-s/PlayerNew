#include "pch.h"
#include "CDlgListFile.h"

BOOL CDlgListFile::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	UpdateDpiInit(eck::GetDpi(hDlg));
	m_hFont = eck::EzFont(L"微软雅黑", 9, 400, FALSE, FALSE, FALSE, m_hWnd);
	m_EDFile.Create(NULL, WS_TABSTOP | WS_GROUP | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0,
		m_Ds.iMargin, m_Ds.iMargin, 0, 0, hDlg, IDC_ED_FILE);
	m_EDFile.SetFrameType(5);

	m_LVFile.Create(NULL, 
		WS_TABSTOP | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 
		WS_EX_CLIENTEDGE,
		m_Ds.iMargin, m_Ds.iMargin + m_Ds.cyEdit + m_Ds.iGap, 0, 0, hDlg, IDC_LV_FILE);
	constexpr DWORD dwExLVStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	eck::LVSetItemHeight(m_LVFile.HWnd, m_Ds.cyLVItem);
	m_LVFile.SetLVExtendStyle(dwExLVStyle);
	m_LVFile.EnableGroupView(TRUE);
	m_LVFile.InsertColumn(L"文件名", 0, m_Ds.cxColumn1);
	m_LVFile.InsertColumn(L"修改时间", 1, m_Ds.cxColumn2);

	auto& vPath = App->GetOptionsMgr().ListFilePath;

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
	EckCounter((int)vPath.size(), i)
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

	m_BTOk.Create(L"确定", WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 0,
		0, 0, m_Ds.cxBtn, m_Ds.cyBtn, hDlg, IDOK);
	m_BTOk.SetTextImageShowing(TRUE);
	m_BTOk.SetImage(m_hiTick, IMAGE_ICON);
	m_BTCancel.Create(L"取消", WS_TABSTOP | WS_CHILD | WS_VISIBLE, 0,
		0, 0, m_Ds.cxBtn, m_Ds.cyBtn, hDlg, IDCANCEL);
	m_BTCancel.SetImage(m_hiCross, IMAGE_ICON);
	m_BTCancel.SetTextImageShowing(TRUE);

	eck::SetFontForWndAndCtrl(hDlg, m_hFont);

	RECT rc;
	GetClientRect(hDlg, &rc);
	SendMsg(WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));

	SetFocus(m_LVFile.GetHWND());
	return FALSE;
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
	DestroyIcon(m_hiTick);
	DestroyIcon(m_hiCross);
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

void CDlgListFile::OnOk(HWND hCtrl)
{
	m_pParam->rsRetFile = m_EDFile.GetText();
	if (m_pParam->rsRetFile.IsEmpty())
	{
		Utils::MsgBox(L"未指定文件", NULL, L"错误", 1, (HICON)TD_ERROR_ICON, m_hWnd);
		return;
	}
	else if (m_pParam->uType == Type::Load && !PathFileExistsW(m_pParam->rsRetFile.Data()))
	{
		m_pParam->rsRetFile.Clear();
		Utils::MsgBox(L"指定的文件不存在", NULL, L"错误", 1, (HICON)TD_ERROR_ICON, m_hWnd);
		return;
	}
	EndDlg(TRUE);
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
	cchPath += m_LVFile.GetItemText(pnmlv->iItem, 0, szPath + cchPath, MAX_PATH - cchPath);

	wcscpy_s(szPath + cchPath, MAX_PATH - cchPath, L".PNList");

	m_EDFile.SetText(szPath);
}

void CDlgListFile::UpdateDpiInit(int iDpi)
{
	m_iDpi = iDpi;
	eck::UpdateDpiSize(m_Ds, iDpi);

	DestroyIcon(m_hiTick);
	IWICBitmap* pBmp = App->ScaleImageForButton(IIDX_Tick, iDpi);
	m_hiTick = eck::CreateHICON(pBmp);
	pBmp->Release();
	if (m_BTOk.IsValid())
		m_BTOk.SetImage(m_hiTick, IMAGE_ICON);

	DestroyIcon(m_hiCross);
	pBmp = App->ScaleImageForButton(IIDX_Cross, iDpi);
	m_hiCross = eck::CreateHICON(pBmp);
	pBmp->Release();
	if (m_BTCancel.IsValid())
		m_BTCancel.SetImage(m_hiCross, IMAGE_ICON);
}

void CDlgListFile::UpdateDpi(int iDpi)
{
	const int iDpiOld = m_iDpi;
	UpdateDpiInit(iDpi);

	const HFONT hOldFont = m_hFont;
	m_hFont = eck::ReCreateFontForDpiChanged(m_hFont, iDpi, iDpiOld);
	eck::SetFontForWndAndCtrl(m_hWnd, m_hFont);
	DeleteObject(hOldFont);

	eck::LVSetItemHeight(m_LVFile.GetHWND(), m_Ds.cyLVItem);
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

LRESULT CDlgListFile::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
#pragma warning (suppress:26454)// 算术溢出
		if (((NMHDR*)lParam)->code == LVN_ITEMCHANGED &&
			((NMHDR*)lParam)->idFrom == IDC_LV_FILE)
		{
			OnLVNItemChanged((NMLISTVIEW*)lParam);
			return 0;
		}
	}
	break;

	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, OnSize);

	case WM_DESTROY:
		return HANDLE_WM_DESTROY(hWnd, wParam, lParam, OnDestroy);

	case WM_DPICHANGED:
		return ECK_HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, OnDpiChanged);
	}

	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

INT_PTR CDlgListFile::DlgBox(HWND hParent, void* pData)
{
	m_pParam = (PARAM*)pData;
	constexpr PCWSTR c_szTile[]{ L"读入列表",L"保存列表" };
	const int iDpi = eck::GetDpi(hParent);
	const int cx = eck::DpiScale(560, iDpi);
	const int cy = eck::DpiScale(500, iDpi);

	return IntCreateModalDlg(0, eck::WCN_DLG, c_szTile[(UINT)m_pParam->uType], WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0, cx, cy, hParent, NULL, eck::g_hInstance, NULL, eck::DLGNCF_CENTERPARENT);
}
