#pragma once
#include "eck\CWnd.h"

#include "CApp.h"


class CWndMain : public eck::CWnd
{
private:
	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBmpBK = NULL;
	ID2D1Bitmap1* m_pBmpPauseBK = NULL;

private:
	static LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnSize(int cx, int cy);

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnDestroy();
public:
	CWndMain()
	{
	}
	
	static ATOM RegisterWndClass();

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;
};