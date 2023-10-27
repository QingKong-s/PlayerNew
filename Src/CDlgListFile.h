#pragma once
#include "CApp.h"
#include "COptionsMgr.h"

#include "eck\CDialog.h"
#include "eck\CEditExt.h"
#include "eck\CListView.h"
#include "eck\CButton.h"
#include "eck\CBk.h"


class CDlgListFile :public eck::CDialog
{
public:
	enum class Type
	{
		Load,
		Save
	};

	struct PARAM
	{
		Type uType;
		eck::CRefStrW rsRetFile;
	};
private:
	eck::CEditExt m_EDFile{};
	eck::CListView m_LVFile{};
	eck::CPushButton m_BTOk{};
	eck::CPushButton m_BTCancel{};

	PARAM* m_pParam = NULL;
	HFONT m_hFont = NULL;

	BOOL m_bRet = FALSE;

	enum
	{
		IDC_LA_TIP = 101,
		IDC_ED_FILE,
		IDC_LV_FILE,
		IDC_BT_OK,
		IDC_BT_CANCEL
	};

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(iMargin, 10)
		ECK_DS_ENTRY(cxColumn1, 300)
		ECK_DS_ENTRY(cxColumn2, 170)
		ECK_DS_ENTRY(cyEdit, 30)
		ECK_DS_ENTRY(iGap, 6)
		ECK_DS_ENTRY(cxBtn, 80)
		ECK_DS_ENTRY(cyBtn, 30)
		ECK_DS_ENTRY(cyLVItem, c_cyLVItem)
		;
	ECK_DS_END_VAR(m_Ds);


	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnInitDialog(HWND hWnd);

	void OnSize(HWND hWnd, UINT uState, int cx, int cy);

	void OnDestroy(HWND hWnd);

	void OnDpiChanged(HWND hWnd, int xDpi, int yDpi, RECT* pRect);

	void OnCmdOk();

	void OnCmdCancel();

	void OnLVNItemChanged(NMLISTVIEW* pnmlv);

	void UpdateDpiInit(int iDpi);

	void UpdateDpi(int iDpi);
public:
	INT_PTR DlgBox(HWND hParent, void* pData = NULL) override;
};