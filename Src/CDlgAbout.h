#pragma once
#include "eck\CDialog.h"
#include "eck\ImageHelper.h"

#include "CApp.h"
#include "resource.h"

class CDlgAbout final :public eck::CDialog
{
private:
	GpBitmap* m_pGpBitmap = NULL;
	int m_cxImg = 0, 
		m_cyImg = 0;
	int m_cxClient = 0, 
		m_cyClient = 0;
	BOOL m_bDark = FALSE;

	void LoadPlayNewImage();
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL OnInitDialog(HWND hDlg, HWND hCtrl, LPARAM lParam) override;

	PNInline INT_PTR DlgBox(HWND hParent, void* pData = NULL) override
	{
		return IntCreateModalDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_ABOUT), hParent);
	}
};