#pragma once
#include "CApp.h"

#include "eck\CWnd.h"

constexpr inline auto WCN_MAINBK = L"PlayerNew.WndClass.MainBK";

class CMainBK :eck::CWnd
{
private:
	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBmpBK = NULL;

	int m_cxClient = 0;
	int m_cyClient = 0;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnSize(HWND hWnd, UINT state, int cx, int cy);
public:
	CMainBK();

	~CMainBK();

	static ATOM RegisterWndClass();

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;
};