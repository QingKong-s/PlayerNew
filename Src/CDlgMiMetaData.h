#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CTreeList.h"
#include "eck\CFixedBlockCollection.h"

class CDlgMusicInfo;
class CDlgMiMetaData :public eck::CDialogNew
{
private:
	struct NODE
	{
		eck::TLNODE TlNode{};
		UINT uType{};
		eck::CRefStrW K{};
		eck::CRefStrW V{};
		std::vector<NODE*> vChildren{};
	};
	eck::CFixedBlockCollection<NODE> m_Clkt{};
	std::vector<NODE*> m_vRoot{};

	eck::CTreeList m_TL{};
	HFONT m_hFont{};

	CDlgMusicInfo* m_pParent{};

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(iMargin, c_DlgMargin)
		;
	ECK_DS_END_VAR(m_Ds);

	void UpdateDpi(int iDpi);

	void BuildTree();
public:
	HWND CreateDlg(HWND hParent, void* pData = NULL) override
	{
		return IntCreateModelessDlg(0, eck::WCN_DLG, L"元数据", 0,
			0, 0, 0, 0, hParent, NULL, App->GetHInstance(), NULL);
	}

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};