#pragma once
#include <vector>

#include "eck\CSplitBar.h"

#include "CApp.h"
#include "CWndBK.h"
#include "CWndList.h"
#include "CWndLrc.h"
#include "CTbGhost.h"

constexpr inline PCWSTR c_pszWndClassMain = L"PlayerNew.WndClass.Main";

enum
{
	PWM_DWMCOLORCHANGED = eck::WM_USER_SAFE + 1000,

};


struct DRAGDROPINFO
{
	BOOL bValid;
};

class CDropTargetList;

class CWndMain : public eck::CWnd
{
	friend class CDropTargetList;
	friend class CWndBK;
	friend class CWndList;
	friend class CTbGhost;
private:
	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBmpBK = NULL;
	ID2D1Bitmap1* m_pBmpPauseBK = NULL;
	ITaskbarList4* m_pTbList = NULL;

	CWndBK m_BK{};
	CWndList m_List{ *this };
	CWndLrc m_Lrc{};
	eck::CSplitBar m_SPB{};
	CTbGhost m_TbGhost{ *this };

	int m_xSeparateBar = 0;

	CDropTargetList* m_pDropTarget = NULL;
	DRAGDROPINFO m_DragDropInfo{};

	D2D1_COLOR_F m_crDwm{};
	ARGB m_argbDwm = 0;
	BOOL m_bDarkColor = FALSE;

	enum
	{
		IDC_BK = 100,
		IDC_LIST,
		IDC_SPB,
	};

	enum
	{
		IDTBB_PREV = 10001,
		IDTBB_PLAY,
		IDTBB_NEXT
	};

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxSeparateBar, 6)
		;
	ECK_DS_END_VAR(m_Ds);

	std::vector<eck::CRefStrW> m_vStr{};

	struct GROUP
	{
		eck::CRefStrW rs{};
		std::vector<Utils::MUSICINFO*> Items{};
		ID2D1Bitmap1* pBmp = NULL;

		GROUP(const eck::CRefStrW& rs_) :rs{ rs_ }, Items{}, pBmp{}
		{
		}
	};
	std::vector<GROUP> m_vGroup{};
	std::vector<Utils::MUSICINFO*> m_vItem;

	void UpdateDpi(int iDpi);

	void InitBK();
private:
	void OnSize(HWND hWnd, UINT uState, int cx, int cy);

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnDestroy();

	void OnDpiChanged(HWND hWnd, int xDpi, int yDpi, RECT* pRect);
public:
	static UINT s_uMsgTaskbarButtonCreated;
	
	static ATOM RegisterWndClass();

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		m_TbGhost.Create(NULL, WS_OVERLAPPEDWINDOW,
			WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
			-32000, -32000, 0, 0, HWnd, NULL);

		const auto hWnd = IntCreate(dwExStyle, c_pszWndClassMain, pszText, dwStyle,
			x, y, cx, cy, NULL, NULL, App->GetHInstance(), NULL);

		EckAssert(!m_pTbList);
		const auto hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&m_pTbList));
		if (FAILED(hr))
			CApp::ShowError(HWnd, hr, CApp::ErrSrc::HResult, L"ITaskbarList4创建失败");
		else
		{
			m_pTbList->HrInit();
			SetupTaskbarStuff();
		}

		return hWnd;
	}

	void ShowLrc(BOOL bShow)
	{
		if (bShow)
			if (m_Lrc.IsValid())
				SetForegroundWindow(m_Lrc.HWnd);
			else
				m_Lrc.Create(NULL, 0, 0,
					700, 1100, 1100, 400, NULL, NULL, NULL);
		else
			if (m_Lrc.IsValid())
				m_Lrc.Destroy();
	}

	PNInline ARGB GetDwmColorArgb() const { return m_argbDwm; }

	PNInline const auto& GetDwmColor() const { return m_crDwm; }

	void SetupTaskbarStuff();
};
inline UINT CWndMain::s_uMsgTaskbarButtonCreated = RegisterWindowMessageW(L"TaskbarButtonCreated");