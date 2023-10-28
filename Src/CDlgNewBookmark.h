#pragma once
#include "CApp.h"

#include "eck\CDialog.h"
#include "eck\CEditExt.h"
#include "eck\CLabel.h"
#include "eck\CButton.h"
#include "eck\CColorPicker.h"

class CDlgNewBookmark :public eck::CDialog
{
public:
	struct PARAM
	{
		eck::CRefStrW rsName;
		COLORREF cr;
	};
private:
	eck::CLabel m_LAName{};
	eck::CLabel m_LAColor{};
	eck::CEditExt m_EDName{};
	eck::CColorPicker m_CLPColor{};
	eck::CPushButton m_BTOk{};
	eck::CPushButton m_BTCancel{};

	enum
	{
		IDC_LA_TIP1 = 101,
		IDC_LA_TIP2,
		IDC_ED_NAME,
		IDC_CLP_CLR,
	};

	PARAM* m_pParam = NULL;
	BOOL m_bRet = FALSE;

	HFONT m_hFont = NULL;

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cyComm, 32)
		ECK_DS_ENTRY(cxLabel, 50)
		ECK_DS_ENTRY(iGap, 6)
		ECK_DS_ENTRY(iMargin, 10)
		ECK_DS_ENTRY(cxInput, 260)
		ECK_DS_ENTRY(cxBT, 80)
		;
	ECK_DS_END_VAR(m_Ds);

	void UpdateDpiInit(int iDpi);

	void UpdateDpi(int iDpi);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnInitDialog(HWND hWnd);

	void OnDestroy(HWND hWnd);

	void OnDpiChanged(HWND hWnd, int xDpi, int yDpi, RECT* pRect);

	void OnCmdOk();

	void OnCmdCancel();
public:
	INT_PTR DlgBox(HWND hParent, void* pData = NULL) override;
};