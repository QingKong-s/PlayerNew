#pragma once
#include "CApp.h"
#include "COptionsMgr.h"

#include "eck\CDialog.h"
#include "eck\CColorPicker.h"
#include "eck\CEditExt.h"
#include "eck\CListView.h"
#include "eck\CButton.h"
#include "eck\CToolBar.h"

class CDlgBookmark :public eck::CDialogNew
{
public:
	struct PARAM
	{
		eck::CListView& LVList;
	};
private:
	eck::CListView m_LVBookmark{};
	eck::CToolBar m_TBOp{};
	eck::CEditExt m_EDName{};
	eck::CColorPicker m_CLPColor{};

	HFONT m_hFont = NULL;

	PARAM* m_pParam = NULL;

	int m_idxEditing = -1;
	int m_idxSubEditing = -1;

	int m_idxLBtnDown = -1;
	int m_idxSubLBtnDown = -1;

	enum
	{
		TBCID_DELETE = 100,
		TBCID_JUMP
	};

	enum
	{
		IDC_LV_BOOKMARK = 200,
		IDC_TB_OP,
		IDC_ED_NAME,
		IDC_CLP_COLOR
	};

	enum
	{
		IDT_LVCLICKDELAY = 300,
		TELAPSE_LVCLICKDELAY = 600
	};

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxColumn1, 50)
		ECK_DS_ENTRY(cxColumn2, 250)
		ECK_DS_ENTRY(cxColumn3, 80)
		ECK_DS_ENTRY(cyLVItem, c_cyLVItem)
		ECK_DS_ENTRY(iLVClrBlockPadding, 3)
		ECK_DS_ENTRY(cyTB, 36)
		ECK_DS_ENTRY(iMargin, 10)
		;
	ECK_DS_END_VAR(m_Ds);


	void UpdateDpiInit(int iDpi);

	void UpdateDpi(int iDpi);

	static LRESULT CALLBACK SubclassProc_LV(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK SubclassProc_ED(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnInitDialog(HWND hWnd);

	void OnCmdDelete();

	void OnCmdJump();

	LRESULT OnLVNCustomDraw(NMLVCUSTOMDRAW* pnmlvcd);

	void OnLVNDbLClick(NMITEMACTIVATE* pnmia);

	void OnSize(HWND hWnd, UINT uState, int cx, int cy);

	void OnLVLBtnDown(HWND hWnd, BOOL bDoubleClick, int x, int y, UINT uKeyFlags);

	void EnterEditing(int idx, int idxSub);

	void CancelEditing(BOOL bSave);
public:
	INT_PTR DlgBox(HWND hParent, void* pData = NULL) override;
};