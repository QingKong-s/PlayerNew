#pragma once
#include <Windows.h>
#include <windowsx.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wincodec.h>
#include <CommCtrl.h>
#include <Shlwapi.h>

#include <vector>
#include <string>
#include <format>

#include <assert.h>

#include "Utils.h"
#include "CPlayer.h"

#include "eck\CWnd.h"

constexpr inline int c_cyLVItem = 24;

constexpr PCWSTR c_szDefMainWndText = L"PlayerNew - 未播放";

class CApp;
extern CApp* App;

class CApp
{
public:
	enum class ErrSrc
	{
		None,
		Win32,
		HResult,
		Bass
	};


	ID2D1Factory1* m_pD2dFactory = NULL;
	IDWriteFactory* m_pDwFactory = NULL;
	IWICImagingFactory* m_pWicFactory = NULL;

	ID2D1Device* m_pD2dDevice = NULL;

	IDXGIDevice1* m_pDxgiDevice = NULL;
	IDXGIFactory2* m_pDxgiFactory = NULL;
private:
	HINSTANCE			m_hInstance = NULL;

	CPlayer m_Player{};

	CLIPFORMAT m_cfListDrag = 0;

	static HRESULT WICCreateBitmap(IWICBitmapDecoder* pDecoder, IWICBitmap** ppBitmap);
public:
	~CApp();

	void Init(HINSTANCE hInstance);

	PNInline HINSTANCE GetHInstance() const { return m_hInstance; }

	PNInline CPlayer& GetPlayer() { return m_Player; }

	PNInline CLIPFORMAT GetListDragClipFormat() const { return m_cfListDrag; }

	static HRESULT WICCreateBitmap(PWSTR pszFile, IWICBitmap** ppWICBitmap);

	static HRESULT WICCreateBitmap(IStream* pStream, IWICBitmap** ppWICBitmap);

	static void ShowError(HWND hWnd, EckOpt(DWORD, dwErrCode), ErrSrc uSrc, PCWSTR pszInfo, PCWSTR pszTitle = NULL);
};