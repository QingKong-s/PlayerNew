#pragma once
#include <vector>

#include "eck\CSplitBar.h"

#include "CApp.h"
#include "CWndBK.h"
#include "CWndList.h"
#include "CWndLrc.h"




#include "CSimpleList.h"

constexpr inline PCWSTR c_pszWndClassMain = L"PlayerNew.WndClass.Main";


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
private:
	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBmpBK = NULL;
	ID2D1Bitmap1* m_pBmpPauseBK = NULL;

	CWndBK m_BK{};
	CWndList m_List{ *this };
	CWndLrc m_Lrc{};
	eck::CSplitBar m_SPB{};

	int m_xSeparateBar = 0;

	CDropTargetList* m_pDropTarget = NULL;
	DRAGDROPINFO m_DragDropInfo{};

	enum
	{
		IDC_BK = 100,
		IDC_LIST,
		IDC_SPB,
	};

	constexpr static UINT SPBM_POSCHANGE = eck::WM_USER_SAFE;

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxSeparateBar, 8)
		;
	ECK_DS_END_VAR(m_Ds);


	CSimpleList m_Sl{};
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
	CWndMain()
	{
	}
	
	static ATOM RegisterWndClass();

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, c_pszWndClassMain, pszText, dwStyle,
			x, y, cx, cy, NULL, NULL, App->GetHInstance(), NULL);
	}

	void ShowLrc(BOOL bShow)
	{
		if (bShow)
			m_Lrc.Create(NULL, 0, 0, 
				700, 1100, 1100, 400, NULL, NULL, NULL);
		else
			m_Lrc.Destroy();
	}
};