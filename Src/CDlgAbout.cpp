#include "CDlgAbout.h"

#include "eck\SystemHelper.h"
#include "eck\Utility.h"

BOOL CDlgAbout::OnInitDialog(HWND hDlg, HWND hCtrl, LPARAM lParam)
{
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

	LoadPlayNewImage();

	RECT rc;
	GetClientRect(hDlg, &rc);
	m_cxClient = rc.right;
	m_cyClient = rc.bottom;

	SetFocus(GetDlgItem(hDlg, IDOK));
	return FALSE;
}

void CDlgAbout::LoadPlayNewImage()
{
	GdipDisposeImage(m_pGpBitmap);

	GdipCreateBitmapFromFile((eck::GetRunningPath() + LR"(\Img\PlayerNew.png)").Data(), &m_pGpBitmap);
	GdipGetImageWidth(m_pGpBitmap, (UINT*)&m_cxImg);
	GdipGetImageHeight(m_pGpBitmap, (UINT*)&m_cyImg);

	if (eck::ShouldAppUseDarkMode())
	{
		GpImageAttributes* pIA;
		GdipCreateImageAttributes(&pIA);
		Gdiplus::ColorMatrix mat{ -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0.999f, 0.999f, 0.999f, 0 };
		GdipSetImageAttributesColorMatrix(pIA, Gdiplus::ColorAdjustTypeDefault, TRUE,
			&mat, NULL, Gdiplus::ColorMatrixFlagsDefault);

		GpBitmap* pNewBmp;
		GdipCreateBitmapFromScan0(m_cxImg, m_cyImg, 0, PixelFormat32bppARGB, NULL, &pNewBmp);
		GpGraphics* pGraphics;
		GdipGetImageGraphicsContext(pNewBmp, &pGraphics);
		GdipGraphicsClear(pGraphics, 0);
		GdipDrawImageRectRectI(pGraphics, m_pGpBitmap, 0, 0, m_cxImg, m_cyImg,
			0, 0, m_cxImg, m_cyImg, Gdiplus::UnitPixel, pIA, NULL, NULL);
		std::swap(m_pGpBitmap, pNewBmp);

		GdipDisposeImageAttributes(pIA);
		GdipDeleteGraphics(pGraphics);
		GdipDisposeImage(pNewBmp);
	}
}

LRESULT CDlgAbout::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
			case IDC_LK_THANKS:
				ShellExecuteW(NULL, L"open", ((NMLINK*)lParam)->item.szUrl, NULL, NULL, SW_SHOW);
				return TRUE;
			}
	}
	return FALSE;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		GpGraphics* pGraphics;
		GdipCreateFromHDC(ps.hdc, &pGraphics);
		GdipSetInterpolationMode(pGraphics, Gdiplus::InterpolationModeHighQuality);
		GdipDrawImageRectI(pGraphics, m_pGpBitmap,
			0, 0,
			m_cxClient,
			m_cxClient * m_cyImg / m_cxImg);
		GdipDeleteGraphics(pGraphics);
		EndPaint(hWnd, &ps);
	}
	return TRUE;

	case WM_SETTINGCHANGE:
	{
		if (eck::IsColorSchemeChangeMessage(lParam) && 
			m_bDark != eck::ShouldAppUseDarkMode())
		{
			m_bDark = eck::ShouldAppUseDarkMode();
			LoadPlayNewImage();
			Redraw();
		}
	}
	break;

	case WM_DESTROY:
		GdipDisposeImage(m_pGpBitmap);
		break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}