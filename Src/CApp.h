﻿#pragma once
#include "eck\ECK.h"

#include <d2d1effects_2.h>

#include <vector>
#include <string>
#include <format>

#include <assert.h>

#include "Utils.h"
#include "CPlayer.h"
#include "COptionsMgr.h"

#include "eck\CForm.h"
#include "eck\SystemHelper.h"
#include "eck\ImageHelper.h"

using namespace eck::Literals;

constexpr inline int c_cyLVItem = 26;

constexpr inline PCWSTR c_szDefMainWndText = L"PlayerNew - 未播放";
constexpr inline PCWSTR c_szVer = L"0.1";
constexpr inline PCWSTR c_szArtistSplitter = L"、/&";

constexpr inline int c_cxBtnIcon = 20;
constexpr inline int c_cyBtnIcon = 20;

constexpr inline int c_DlgMargin = 10;

class CApp;
extern CApp* App;

enum
{
	PNWM_BEGIN = eck::WM_USER_SAFE,
	PNWM_PLAYINGCTRL,
	PNWM_CHANNELENDED,
	PNWM_SETTINGCHANGED,
	PNWM_DWMCOLORCHANGED,

	PNWM_OPTDLG_APPLY,

	PNWM_MIDLG_GETIDX,		// 发送给选择夹窗口
	PNWM_MIDLG_UPDATEIDX,	// 发送给子窗口
	PNWM_MIDLG_INIT,		// 发送给子窗口
};

enum
{
	IIDX_ArrowCross,
	IIDX_ArrowRight,
	IIDX_ArrowRight3,
	IIDX_Balance,
	IIDX_Block,
	IIDX_BrowseFile,
	IIDX_Circle,
	IIDX_CircleOne,
	IIDX_Cross,
	IIDX_CrossSolid,
	IIDX_Disk,
	IIDX_Gear,
	IIDX_Info,
	IIDX_ListManage,
	IIDX_LoadFile,
	IIDX_Locate,
	IIDX_LockSolid,
	IIDX_Lrc,
	IIDX_Magnifier,
	IIDX_Next,
	IIDX_NextSolid,
	IIDX_Pause,
	IIDX_PauseSolid,
	IIDX_PlayList,
	IIDX_PlayOpt,
	IIDX_Plus,
	IIDX_Prev,
	IIDX_PrevSolid,
	IIDX_Reserve,
	IIDX_Silent,
	IIDX_SoundFont,
	IIDX_Speaker,
	IIDX_Speed,
	IIDX_Tempo,
	IIDX_Tick,
	IIDX_TickRed,
	IIDX_Triangle,
	IIDX_TriangleSolid,
	IIDX_DefCover,
	IIDX_Pitch,

	IIDX__MAX,
};

class CWndMain;
class CApp
{
private:
	constexpr static PCWSTR c_szResFile[]
	{
		LR"(\Img\ArrowCross.png)",
		LR"(\Img\ArrowRight.png)",
		LR"(\Img\ArrowRight3.png)",
		LR"(\Img\Balance.png)",
		LR"(\Img\Block.png)",
		LR"(\Img\BrowseFile.png)",
		LR"(\Img\Circle.png)",
		LR"(\Img\CircleOne.png)",
		LR"(\Img\Cross.png)",
		LR"(\Img\CrossSolid.png)",
		LR"(\Img\Disk.png)",
		LR"(\Img\Gear.png)",
		LR"(\Img\Info.png)",
		LR"(\Img\ListManage.png)",
		LR"(\Img\LoadFile.png)",
		LR"(\Img\Locate.png)",
		LR"(\Img\LockSolid.png)",
		LR"(\Img\Lrc.png)",
		LR"(\Img\Magnifier.png)",
		LR"(\Img\Next.png)",
		LR"(\Img\NextSolid.png)",
		LR"(\Img\Pause.png)",
		LR"(\Img\PauseSolid.png)",
		LR"(\Img\PlayList.png)",
		LR"(\Img\PlayOpt.png)",
		LR"(\Img\Plus.png)",
		LR"(\Img\Prev.png)",
		LR"(\Img\PrevSolid.png)",
		LR"(\Img\Reserve.png)",
		LR"(\Img\Silent.png)",
		LR"(\Img\SoundFont.png)",
		LR"(\Img\Speaker.png)",
		LR"(\Img\Speed.png)",
		LR"(\Img\Tempo.png)",
		LR"(\Img\Tick.png)",
		LR"(\Img\TickRed.png)",
		LR"(\Img\Triangle.png)",
		LR"(\Img\TriangleSolid.png)",
		LR"(\Img\DefCover.png)",
		LR"(\Img\Pitch.png)",
	};
	static_assert(IIDX__MAX == ARRAYSIZE(CApp::c_szResFile));
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

	CWndMain* m_pWndMain = NULL;
private:
	HINSTANCE m_hInstance = NULL;

	CPlayer m_Player{};
	COptionsMgr m_OptionsMgr{};

	CLIPFORMAT m_cfListDrag = 0;

	IWICBitmap* m_pWicRes[IIDX__MAX]{};

	static HRESULT WICCreateBitmap(IWICBitmapDecoder* pDecoder, IWICBitmap** ppBitmap);

	void LoadRes();
public:
	~CApp();

	void Init(HINSTANCE hInstance);

	PNInline HINSTANCE GetHInstance() const { return m_hInstance; }

	PNInline CPlayer& GetPlayer() { return m_Player; }

	PNInline CLIPFORMAT GetListDragClipFormat() const { return m_cfListDrag; }

	PNInline auto GetWicRes() { return m_pWicRes; }

	PNInline CWndMain* GetMainWnd() { return m_pWndMain; }

	PNInline COptionsMgr& GetOptionsMgr() { return m_OptionsMgr; }

	PNInline IWICBitmap* ScaleImageForButton(int idxImg, int iDpi, HRESULT* phr = NULL)
	{
		IWICBitmap* pBmp;
		const auto hr = eck::ScaleWicBitmap(GetWicRes()[idxImg], pBmp,
			eck::DpiScale(c_cxBtnIcon, iDpi), eck::DpiScale(c_cxBtnIcon, iDpi),
			WICBitmapInterpolationModeHighQualityCubic);
		if (phr)
			*phr = hr;
		return pBmp;
	}

	void InvertIconColor();

	static HRESULT WICCreateBitmap(PCWSTR pszFile, IWICBitmap** ppWICBitmap);

	static HRESULT WICCreateBitmap(IStream* pStream, IWICBitmap** ppWICBitmap);

	static void ShowError(HWND hWnd, EckOpt(DWORD, dwErrCode), ErrSrc uSrc, PCWSTR pszInfo, PCWSTR pszDetail = NULL, PCWSTR pszTitle = NULL);
};