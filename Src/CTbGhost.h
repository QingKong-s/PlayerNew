#pragma once
#include "CApp.h"

constexpr inline PCWSTR WCN_TBGHOST = L"PlayerNew.WndClass.TaskbarGhost";

class CWndMain;
class CTbGhost final :public eck::CWnd
{
private:
	CWndMain& m_WndMain;
public:
	static ATOM RegisterWndClass() { return eck::EzRegisterWndClass(WCN_TBGHOST, 0); }

	CTbGhost(CWndMain& w) :m_WndMain{ w } {}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WCN_TBGHOST, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, eck::g_hInstance, NULL);
	}
};