#pragma once
#include "CApp.h"

#include "eck\CListView.h"

class CWndList;
class CLVSearch :public eck::CListView
{
	CWndList& m_WndList;
	HTHEME m_hTheme = NULL;
public:
	CLVSearch(CWndList& x) : m_WndList(x) {}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	LRESULT OnNotifyMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override;
};

class CLVPlay :public eck::CListView
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