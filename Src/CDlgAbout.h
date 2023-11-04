#pragma once
#include "eck\CDialog.h"
#include "eck\ImageHelper.h"

#include "CApp.h"
#include "resource.h"

class CDlgAbout :public eck::CDialog
{
private:
	GpBitmap* m_pGpBitmap = NULL;
	int m_cxImg = 0, m_cyImg = 0;
	int m_cxClient = 0, m_cyClient = 0;

	static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL OnInitDialog(HWND hDlg, HWND hCtrl, LPARAM lParam);
public:
	INT_PTR DlgBox(HWND hParent, void* pData = NULL) override
	{
		auto iRet = DialogBoxParamW(App->GetHInstance(), MAKEINTRESOURCEW(IDD_ABOUT), hParent, DlgProc, (LPARAM)this);
		GdipDisposeImage(m_pGpBitmap);
		return iRet;
	}
};