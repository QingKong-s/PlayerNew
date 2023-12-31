﻿#pragma once
#include <vector>

#include "eck\CSplitBar.h"

#include "CApp.h"
#include "CWndBK.h"
#include "CWndList.h"




#include "CSimpleList.h"


struct DRAGDROPINFO
{
	BOOL bValid;
};

class CDropTargetList;

class CWndMain : public eck::CWnd
{
	friend class CDropTargetList;
private:
	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBmpBK = NULL;
	ID2D1Bitmap1* m_pBmpPauseBK = NULL;

	CWndBK m_BK{};
	CWndList m_List{ *this };
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
	static LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnSize(HWND hWnd, UINT uState, int cx, int cy);

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnDestroy();

	void OnDpiChanged(HWND hWnd, int xDpi, int yDpi, RECT* pRect);
public:
	CWndMain()
	{
	}
	
	static ATOM RegisterWndClass();

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;
};