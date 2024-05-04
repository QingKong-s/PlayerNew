#pragma once
#include "CApp.h"
#include "resource.h"

#include "CDlgMiMetaData.h"
#include "CDlgMiPicture.h"

#include "eck\CTab.h"
#include "eck\CButton.h"

class CDlgMusicInfo : public eck::CDialog
{
private:
	CDlgMiMetaData m_DlgMetaData{};
	CDlgMiPicture m_DlgPic{};
	int m_idxCurr{};// LV索引

	eck::CTab m_Tab{};
	eck::CDialog* m_pDlg[2]
	{
		&m_DlgMetaData,
		&m_DlgPic,
	};

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxTab, 70)
		ECK_DS_ENTRY(cyTab, 24)
		;
	ECK_DS_END_VAR(m_Ds);

	void UpdateDpi(int iDpi)
	{
		eck::UpdateDpiSize(m_Ds, iDpi);
	}
public:
	HWND CreateDlg(HWND hParent, void* pData = NULL) override
	{
		m_idxCurr = eck::pToI32<int>(pData);
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_FILEINFO), hParent);
	}

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;
	
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	EckInline int GetCurrIndex() const { return m_idxCurr; }
};