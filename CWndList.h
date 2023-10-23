﻿#pragma once
#include "CApp.h"
#include "CPlayList.h"

#include "eck\CLabel.h"
#include "eck\CEditExt.h"
#include "eck\CButton.h"
#include "eck\CListView.h"
#include "eck\CToolBar.h"

#include <shlobj_core.h>

constexpr inline auto WCN_LIST = L"PlayerNew.WndClass.List";

class CWndMain;
class CWndList : public eck::CWnd
{
private:
	eck::CLabel m_LAListName{};
	eck::CEditExt m_EDSearch{};
	eck::CPushButton m_BTSearch{};
	eck::CToolBar m_TBManage{};
	eck::CListView m_LVList{};

	HFONT m_hFont = NULL;
	HFONT m_hFontListName = NULL;

	HMENU m_hMenuAdd = NULL;
	HMENU m_hMenuLV = NULL;

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;


	HTHEME m_hThemeLV = NULL;
	CWndMain& m_WndMain;

	enum
	{
		TBBTI_LOCATE = 0,
		TBBTI_ADD,
		TBBTI_LOADLIST,
		TBBTI_SAVELIST,
		TBBTI_EMPTY,
		TBBTI_MANAGE,
	};

	enum// 控件ID
	{
		IDC_LA_LIST_NAME = 100,
		IDC_ED_SEARCH,
		IDC_BT_SEARCH,
		IDC_LV_LIST,
		IDC_TB_MANAGE
	};
	enum// 工具条按钮ID
	{
		TBCID_LOCATE = 200,
		TBCID_ADD,
		TBCID_LOADLIST,
		TBCID_SAVELIST,
		TBCID_EMPTY,
		TBCID_MANAGE,
	};
	enum// 菜单ID
	{
		IDMI_ADDFILE = 300,
		IDMI_ADDDIR,


		IDMI_PLAY,
		IDMI_DELETE_FROM_LIST,
		IDMI_DELETE_FROM_DISK,
		IDMI_OPEN_IN_EXPLORER,
		IDMI_RENAME,
		IDMI_INFO,
		IDMI_BOOKMARK,
		IDMI_NEXTBOOKMARK,
		IDMI_PREVBOOKMARK,
		IDMI_IGNORE,
		IDMI_PLAYLATER,
	};

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(iEdgePadding, 4)
		ECK_DS_ENTRY(cyListName, 24)
		ECK_DS_ENTRY(cySearch, 30)
		ECK_DS_ENTRY(cyTool, 32)
		ECK_DS_ENTRY(cxToolBtn, 60)
		ECK_DS_ENTRY(iGap, 4)
		ECK_DS_ENTRY(cxLVTextSpace, 4)
		;
	ECK_DS_END_VAR(m_Ds);

	PNInline void UpdateDpiInit(int iDpi)
	{
		m_iDpi = iDpi;
		eck::UpdateDpiSize(m_Ds, iDpi);
		DeleteObject(m_hFont);
		m_hFont = eck::EzFont(L"微软雅黑", 9);
		DeleteObject(m_hFontListName);
		m_hFontListName = eck::EzFont(L"微软雅黑", 13);
	}

	PNInline void UpdateDpi(int iDpi)
	{
		UpdateDpiInit(iDpi);
		eck::SetFontForWndAndCtrl(m_hWnd, m_hFont, TRUE);
	}

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnSize(HWND hWnd, UINT state, int cx, int cy);

	void OnDestroy(HWND hWnd);

	void OnCmdLocate();

	void OnMenuAddFile();
	
	void OnMenuAddDir();

	void OnCmdLoadList();

	void OnCmdSaveList();

	void OnCmdEmpty();

	void OnLVNDbLClick(NMITEMACTIVATE* pnmia);

	LRESULT OnLVNCustomDraw(NMLVCUSTOMDRAW* pnmlvcd);

	void OnLVNRClick(NMITEMACTIVATE* pnmia);

	void OnMenuOpenInExplorer();
public:
	CWndList(CWndMain& Main) :m_WndMain(Main) {}

	static ATOM RegisterWndClass();

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;

	void PlayListItem(int idx);
};