#pragma once
#include <Windows.h>
#include <windowsx.h>
#include <d2d1_2.h>
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
#include "COptionsMgr.h"

#include "eck\CWnd.h"
#include "eck\SystemHelper.h"

using namespace eck::Literals;

constexpr inline int c_cyLVItem = 24;

constexpr inline PCWSTR c_szDefMainWndText = L"PlayerNew - 未播放";
constexpr inline PCWSTR c_szVer = L"0.1";

class CApp;
extern CApp* App;

enum
{
	IIDX_ArrowCross,
	IIDX_ArrowRight,
	IIDX_ArrowRight3,
	IIDX_Blance,
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
	IIDX_Slient,
	IIDX_SoundFont,
	IIDX_Speaker,
	IIDX_Speed,
	IIDX_Tempo,
	IIDX_Tick,
	IIDX_TickRed,
	IIDX_Triangle,
	IIDX_TriangleSolid,
	IIDX_DefCover,

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
	ID2D1Multithread* m_pD2dMultiThread = NULL;
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

	IWICBitmap* m_pWicRes[ARRAYSIZE(c_szResFile)]{};
	HICON m_hIcon[ARRAYSIZE(c_szResFile)]{};

	static HRESULT WICCreateBitmap(IWICBitmapDecoder* pDecoder, IWICBitmap** ppBitmap);

	void LoadRes();
public:
	~CApp();

	void Init(HINSTANCE hInstance);

	PNInline HINSTANCE GetHInstance() const { return m_hInstance; }

	PNInline CPlayer& GetPlayer() { return m_Player; }

	PNInline CLIPFORMAT GetListDragClipFormat() const { return m_cfListDrag; }

	PNInline const auto GetWicRes() { return m_pWicRes; }

	PNInline const auto GetHIconRes() { return m_hIcon; }

	PNInline CWndMain* GetMainWnd() { return m_pWndMain; }

	PNInline COptionsMgr& GetOptionsMgr() { return m_OptionsMgr; }

	static HRESULT WICCreateBitmap(PCWSTR pszFile, IWICBitmap** ppWICBitmap);

	static HRESULT WICCreateBitmap(IStream* pStream, IWICBitmap** ppWICBitmap);

	static void ShowError(HWND hWnd, EckOpt(DWORD, dwErrCode), ErrSrc uSrc, PCWSTR pszInfo, PCWSTR pszDetail = NULL, PCWSTR pszTitle = NULL);
};