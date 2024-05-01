#pragma once
#include "CApp.h"

#include "eck\CListView.h"

class CWndList;

class CLVNew :public eck::CListView
{
protected:
	static void PaintLVItem(CLVNew& LV, CWndList& WndList, PLAYLISTUNIT& Item,
		NMLVCUSTOMDRAW* pnmlvcd, HTHEME hTheme, int idx, int idxLVItem);
};

class CLVSearch :public CLVNew
{
	CWndList& m_WndList;
	HTHEME m_hTheme = NULL;
public:
	CLVSearch(CWndList& x) : m_WndList(x) {}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	LRESULT OnNotifyMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override;
};

class CLVPlay :public CLVNew
{
private:
	CLVSearch& m_LVSearch;
	CWndList& m_WndList;
	HTHEME m_hTheme = NULL;
public:
	CLVPlay(CWndList& x, CLVSearch& y) : m_WndList(x), m_LVSearch(y) {}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	LRESULT OnNotifyMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override;
};