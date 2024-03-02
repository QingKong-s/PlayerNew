#pragma once
#include "CApp.h"

#include "eck\CToolBar.h"

enum// 工具条按钮ID
{
	TBCID_BEGIN = 200,
	TBCID_LOCATE = TBCID_BEGIN,
	TBCID_ADD,
	TBCID_LOADLIST,
	TBCID_SAVELIST,
	TBCID_EMPTY,
	TBCID_MANAGE,

	TBBTCOUNT = 6
};
class CTBList :public eck::CToolBar
{
private:
	HTHEME m_hTheme = NULL;
	HFONT m_hFont = NULL;
	HICON m_hi[TBBTCOUNT]{};
	BYTE m_uBTFlags[TBBTCOUNT]{};

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxIcon, c_cxBtnIcon)
		ECK_DS_ENTRY(cyIcon, c_cyBtnIcon)
		ECK_DS_ENTRY(cyTool, 32)
		ECK_DS_ENTRY(cxToolBtn, 60)
		;
	ECK_DS_END_VAR(m_Ds);

	constexpr static int c_idxIcon[]
	{
		IIDX_Locate,
		IIDX_Plus,
		IIDX_LoadFile,
		IIDX_Disk,
		IIDX_Cross,
		IIDX_ListManage
	};

	void UpdateIcon(int iDpi);
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override;
};
