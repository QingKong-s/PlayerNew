#pragma once
#include "CApp.h"
#include "CPlayList.h"

#include "eck\CLabel.h"
#include "eck\CEditExt.h"
#include "eck\CButton.h"
#include "eck\CListView.h"

constexpr inline auto WCN_LIST = L"PlayerNew.WndClass.List";

class CWndList : public eck::CWnd
{
private:
	eck::CLabel m_LAListName{};
	eck::CEditExt m_EDSearch{};
	eck::CPushButton m_BTSearch{};
	eck::CPushButton m_BTTool[6]{};
	eck::CListView m_LVList{};

	CPlayList m_PlayList{};

	enum
	{
		TBBTI_LOCATE = 0,
		TBBTI_ADD,
		TBBTI_LOADLIST,
		TBBTI_SAVELIST,
		TBBTI_EMPTY,
		TBBTI_MANAGE,
	};

	enum IDC
	{
		IDC_LA_LIST_NAME = 100,
		IDC_ED_SEARCH,
		IDC_BT_SEARCH,
		IDC_LV_LIST,
		IDC_BT_LOCATE,
		IDC_BT_ADD,
		IDC_BT_LOADLIST,
		IDC_BT_SAVELIST,
		IDC_BT_EMPTY,
		IDC_BT_MANAGE,
	};

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(iEdgePadding, 4)
		ECK_DS_ENTRY(cyListName, 20)
		ECK_DS_ENTRY(cySearch, 26)
		ECK_DS_ENTRY(cyTool, 24)
		;
	ECK_DS_END_VAR(m_Ds);

	PNInline void UpdateDpiSize(int iDpi)
	{
		eck::UpdateDpiSize(m_Ds, iDpi);
	}

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnSize(HWND hWnd, UINT state, int cx, int cy);
public:
	static ATOM RegisterWndClass();

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;
};