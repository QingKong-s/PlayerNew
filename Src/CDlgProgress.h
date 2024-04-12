#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CStatic.h"
#include "eck\CProgressBar.h"
#include "eck\CButton.h"

class CDlgProgress:public eck::CDialog
{
private:
	eck::CProgressBar m_PB{};

	HANDLE m_hThread{};
	std::atomic<BOOL> m_bShouldExit = FALSE;
public:
	PNInline INT_PTR DlgBox(HWND hParent, void* pData = NULL) 
	{ 
		return IntCreateModalDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_PROGRESS), hParent);
	}

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;

	void OnCancel(HWND hCtrl) override;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};