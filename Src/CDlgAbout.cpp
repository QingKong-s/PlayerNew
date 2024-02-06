#include "CDlgAbout.h"

#include "eck\SystemHelper.h"
#include "eck\Utility.h"

constexpr static PCWSTR c_szPropAbout = L"PlayerNew.Prop.About";

INT_PTR CDlgAbout::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CDlgAbout*)GetPropW(hDlg, c_szPropAbout);
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
#pragma warning(suppress:26454)
		if (((NMHDR*)lParam)->code == NM_CLICK)
			switch (((NMHDR*)lParam)->idFrom)
			{
			case IDC_LK_AUTHOR:
			case IDC_LK_3RDPARTYLIB:
			case IDC_LK_OPENGITHUB:
				ShellExecuteW(NULL, L"open", ((NMLINK*)lParam)->item.szUrl, NULL, NULL, SW_SHOW);
				return TRUE;
			}
	}
	return FALSE;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hDlg, &ps);
		GpGraphics* pGraphics;
		GdipCreateFromHDC(ps.hdc, &pGraphics);
		GdipSetSmoothingMode(pGraphics, SmoothingModeHighQuality);
		GdipDrawImageRectI(pGraphics, p->m_pGpBitmap,
			0, 0,
			p->m_cxClient,
			p->m_cxClient * p->m_cyImg / p->m_cxImg);
		GdipDeleteGraphics(pGraphics);
		EndPaint(hDlg, &ps);
	}
	return TRUE;
	case WM_INITDIALOG:
		p = (CDlgAbout*)lParam;
		SetPropW(hDlg, c_szPropAbout, (HANDLE)lParam);
		return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, p->OnInitDialog);
	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) && HIWORD(wParam) == BN_CLICKED)
		{
			EndDialog(hDlg, 0);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

BOOL CDlgAbout::OnInitDialog(HWND hDlg, HWND hCtrl, LPARAM lParam)
{
	m_hWnd = hDlg;

	using namespace std::literals;
	SetDlgItemTextW(hDlg, IDC_ST_PNVER, (L"PlayerNew  "s + c_szVer).c_str());

	SetDlgItemTextW(hDlg, IDC_ST_BASSVER, (L"Bass版本："s + CBass::VerToString(CBass::GetVer())).c_str());

	constexpr PCWSTR c_szBin = L"PNBIN";
	const HINSTANCE hInst = App->GetHInstance();
	HGLOBAL hRes;
	HRSRC hResInfo = FindResourceW(hInst, MAKEINTRESOURCEW(IDR_BIN_COMPILETIME), c_szBin);
	if (hResInfo)
	{
		hRes = LoadResource(hInst, hResInfo);
		if (hRes)
		{
			SetDlgItemTextW(hDlg, IDC_ST_COMPILETIME,
				(L"编译时间："s + (PCWSTR)LockResource(hRes)).c_str());
		}
	}

	hResInfo = FindResourceW(hInst, MAKEINTRESOURCEW(IDR_BIN_COMPILECOUNT), c_szBin);
	if (hResInfo)
	{
		hRes = LoadResource(hInst, hResInfo);
		if (hRes)
		{
			SetDlgItemTextW(hDlg, IDC_ST_COMPILECOUNT,
				(L"累计编译次数："s + std::to_wstring(*(UINT*)LockResource(hRes))).c_str());
		}
	}

	GdipCreateBitmapFromFile((eck::GetRunningPath() + LR"(\Img\PlayerNew.png)").Data(), &m_pGpBitmap);
	GdipGetImageWidth(m_pGpBitmap, (UINT*)&m_cxImg);
	GdipGetImageHeight(m_pGpBitmap, (UINT*)&m_cyImg);

	RECT rc;
	GetClientRect(hDlg, &rc);
	m_cxClient = rc.right;
	m_cyClient = rc.bottom;
	return FALSE;
}