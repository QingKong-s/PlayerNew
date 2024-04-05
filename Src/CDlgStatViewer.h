#pragma once
#include "CStatistics.h"

#include "eck\CDialog.h"
#include "eck\CTreeView.h"
#include "eck\CSplitBar.h"
#include "eck\CListView.h"

class CDlgStatViewer :public eck::CDialogNew
{
private:
	eck::CTreeView m_TVDate{};
	eck::CSplitBar m_SPB{};
	eck::CListView m_LVSong{};
	eck::CListView m_LVArtist{};
public:
	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	HWND CreateDlg(HWND hParent, void* pData = NULL) override;
};