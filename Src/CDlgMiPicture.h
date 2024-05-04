#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CListBoxNew.h"
#include "eck\CPictureBox.h"
#include "eck\CListView.h"
#include "eck\CSplitLayout.h"

class CDlgMiPicture :public eck::CDialogNew
{
private:
	enum
	{
		IDMI_LB_SAVE = 100,
		IDMI_LB_PUT_NEW,
		IDMI_LB_DELETE
	};

	struct ITEM
	{
		Utils::MUSICPIC Pic{};
		GpBitmap* pBitmap{};
		eck::CRefStrW rsText{};
	};

	std::vector<ITEM> m_vItem{};

	eck::CListBoxNew m_LBN{};
	eck::CPictureBox m_PKB{};
	eck::CListView m_LV{};
	eck::CMenu m_MenuLB
	{
		{L"保存到文件",IDMI_LB_SAVE},
		{L"选择新图片",IDMI_LB_PUT_NEW},
		{L"删除",IDMI_LB_DELETE},
	};

	eck::CSplitLayoutH m_Layout{};
	eck::CSplitLayoutV m_LayoutV{};

	CDlgMusicInfo* m_pParent{};

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxPadding, 10)
		ECK_DS_ENTRY(cyItem, 150)
		;
	ECK_DS_END_VAR(m_Ds);

	EckInline void UpdateDpi(int iDpi)
	{
		eck::UpdateDpiSize(m_Ds, iDpi);
	}

	void UpdatePic();
public:
	HWND CreateDlg(HWND hParent, void* pData = NULL) override
	{
		return IntCreateModelessDlg(0, eck::WCN_DLG, L"图片", 0,
			0, 0, 0, 0, hParent, NULL, App->GetHInstance(), NULL);
	}

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};