#pragma once
#include <functional>
#include <unordered_set>

#include "eck\CWnd.h"
#include "eck\WndHelper.h"
#include "eck\CScrollView.h"

#include "CApp.h"

#define PNProp(getter, setter) __declspec(property(get = getter, put = setter))

constexpr PCWSTR WCN_SIMPLELIST = L"PlayerNew.WndClass.SimpleList";

using eck::PCVOID;

struct SLITEMINFO
{
	PCWSTR pszText;
	int cchText;
	int idxItem;

};

struct SLGETDISPINFO
{
	SLITEMINFO* pItemInfo;
};

class CSimpleList : public eck::CWnd
{
private:
	std::function<int(SLGETDISPINFO*)> m_fnGetDispInfo{};
	int m_cItems = 0;

	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBmpBK = NULL;

	IDWriteTextFormat* m_pTextFormat = NULL;
	ID2D1SolidColorBrush* m_pBrText = NULL;
	ID2D1SolidColorBrush* m_pBrHot = NULL;
	ID2D1SolidColorBrush* m_pBrBk = NULL;

	int m_cxClient = 0;
	int m_cyClient = 0;

	int m_cyItem = 0;

	int m_idxTop = 0;
	int m_idxHot = -1;
	std::unordered_set<int> m_SelItems{};
	int m_idxSel = -1;

	int m_cyInvisibleTop = 0;

	eck::CInertialScrollView m_ScrollView{};

	static void ScrollProc(int iPos, int iPrevPos, LPARAM lParam);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnSize(HWND hWnd, UINT uState, int cx, int cy);

	void OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags);

	void OnMouseLeave(HWND hWnd);

	void OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT fwKeys);

	void RedrawItem(int idxItem, D2D1_RECT_F& rcItem)
	{
		SLGETDISPINFO sldi{};
		SLITEMINFO sli{};
		sldi.pItemInfo = &sli;

		sli.cchText = -1;
		sli.idxItem = idxItem;
		m_fnGetDispInfo(&sldi);

		rcItem =
		{
			0.f,
			(float)(idxItem * m_cyItem - m_ScrollView.GetPos()),
			(float)m_cxClient,
			(float)((idxItem + 1) * m_cyItem - m_ScrollView.GetPos()),
		};

		if (sli.pszText)
		{
			if (sli.cchText < 0)
				sli.cchText = (int)wcslen(sli.pszText);

			if (idxItem == m_idxHot)
				m_pDC->FillRectangle(rcItem, m_pBrHot);
			else
				m_pDC->FillRectangle(rcItem, m_pBrBk);

			m_pDC->DrawTextW(sli.pszText, sli.cchText, m_pTextFormat, rcItem, m_pBrText, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
		}
		else
			m_pDC->FillRectangle(rcItem, m_pBrBk);
	}
public:
	CSimpleList()
	{
	}

	static ATOM RegisterWndClass();

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;

	void SetDispInfoProc(std::function<int(SLGETDISPINFO*)> fn)
	{
		m_fnGetDispInfo = fn;
	}

	void SetItemCount(int cItems)
	{
		m_cItems = cItems;
		m_ScrollView.SetMax(m_cItems * m_cyItem);
		Redraw();
	}

	int GetItemHeight()
	{
		return m_cyItem;
	}

	void SetItemHeight(int cyItem)
	{
		m_cyItem = cyItem;
	}

	void Redraw();

	void Redraw(std::initializer_list<int> idxItem);

	int HitTest(POINT pt);
};

