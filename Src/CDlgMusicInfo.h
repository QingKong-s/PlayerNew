#pragma once
#include "CApp.h"
#include "resource.h"

#include "CDlgMiMetaData.h"

#include "eck\CTab.h"
#include "eck\CButton.h"

class CDlgMusicInfo : public eck::CDialog
{
private:
	CDlgMiMetaData m_DlgMetaData{};

	eck::CTab m_Tab{};
	eck::CDialog* m_pDlg[1]
	{
		&m_DlgMetaData
	};
private:
	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;
};