#pragma once
#include <vector>

#include "eck\CWnd.h"

#include "CApp.h"
#include "CWndBK.h"
#include "CWndList.h"




#include "CSimpleList.h"



class CWndMain : public eck::CWnd
{
private:
	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBmpBK = NULL;
	ID2D1Bitmap1* m_pBmpPauseBK = NULL;

	CWndBK m_Bk{};
	CWndList m_List{ *this };

	int m_xSeparateBar = 0;

	enum
	{
		IDC_BK = 100,
		IDC_LIST
	};


	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxSeparateBar, 5)
		;
	ECK_DS_END_VAR(m_Ds);


	CSimpleList m_Sl{};
	std::vector<eck::CRefStrW> m_vStr{};

	void UpdateDpi(int iDpi);

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