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
	int idxGroup;
};

struct SLGROUPINFO
{
	PCWSTR pszText;
	int cchText;
	int idxItem;
	ID2D1DeviceContext* pDC;
	ID2D1Bitmap1* pBmp;
};

struct SLGETDISPINFO
{
	BOOL bItem;
	union
	{
		SLITEMINFO Item;
		SLGROUPINFO Group;
	};
};

struct SLHITTEST
{
	int idxGroup;
	BITBOOL bHitThumb : 1;
	BITBOOL bHitCover : 1;
};

class CSimpleList : public eck::CWnd
{
private:
	struct ITEM
	{
		int y = 0;
	};

	struct GROUPITEM
	{
		int y = 0;
		std::vector<ITEM> Item{};
	};

	std::function<int(SLGETDISPINFO*)> m_fnGetDispInfo{};
	int m_cItems = 0;

	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBmpBK = NULL;

	IDWriteTextFormat* m_pTextFormat = NULL;
	IDWriteTextFormat* m_pTfGroupTitle = NULL;

	ID2D1SolidColorBrush* m_pBrText = NULL;
	ID2D1SolidColorBrush* m_pBrHot = NULL;
	ID2D1SolidColorBrush* m_pBrBk = NULL;
	ID2D1SolidColorBrush* m_pBrGroup = NULL;
	ID2D1SolidColorBrush* m_pBrGroupText = NULL;
	ID2D1SolidColorBrush* m_pBrSBThumb = NULL;

	int m_cxClient = 0;
	int m_cyClient = 0;
	int m_cxView = 0;// 排除滚动条后的区域宽度

	int m_cyItem = 0;
	int m_cyGroupHeader = 40;

	int m_idxTop = 0;
	int m_idxHot = -1;
	int m_idxHotItemSGroup = -1;// 热点项所在的组
	int m_idxHotGroup = -1;
	std::unordered_set<int> m_SelItems{};
	int m_idxSel = -1;

	int m_cyInvisibleTop = 0;

	int m_cxCover = 0;

	float m_cxSBThumb = 0.f;

	eck::CInertialScrollView m_ScrollView{};

	BOOL m_bGroup = TRUE;
	BOOL m_bHoverThumb = FALSE;
	BOOL m_bThumbLBtnDown = FALSE;
	std::vector<ITEM> m_Item{};
	std::vector<GROUPITEM> m_Group{};

	D2D1_RECT_F m_rcfThumb{};
	eck::CEasingAn<eck::Easing::FOutSine> m_AnThumb{};

	constexpr static int c_iMinLinePerGroup = 3;

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_F(cxTextPadding, 4.f)
		ECK_DS_ENTRY_F(EmText, 12.f)
		ECK_DS_ENTRY_F(EmGroupText, 16.f)
		ECK_DS_ENTRY_F(cyGroupLine, 1.f)
		ECK_DS_ENTRY_F(cxScrollBarMini, 1.5f)
		ECK_DS_ENTRY_F(cxScrollBar, 8.f)
		ECK_DS_ENTRY_F(cyMinSBThumb, 30.f)
		;
	ECK_DS_END_VAR(m_DsF);

	enum
	{
		IDT_THUMBAN = 101,
	};

	enum
	{
		ELAPSE_THUMBAN = 20,
	};

	static void ScrollProc(int iPos, int iPrevPos, LPARAM lParam);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnSize(HWND hWnd, UINT uState, int cx, int cy);

	void OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags);

	void OnMouseLeave(HWND hWnd);

	void OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT fwKeys);

	void OnLButtonDown(HWND hWnd, BOOL bDoubleClick, int x, int y, UINT uKeyFlags);

	void OnLButtonUp(HWND hWnd, int x, int y, UINT uKeyFlags);

	BOOL RedrawItem(int idxItem, D2D1_RECT_F& rcItem)
	{
		SLGETDISPINFO sldi{};
		sldi.bItem = TRUE;
		sldi.Item.cchText = -1;
		sldi.Item.idxItem = idxItem;
		sldi.Item.idxGroup = -1;
		m_fnGetDispInfo(&sldi);

		rcItem =
		{
			0.f,
			(float)(idxItem * m_cyItem - m_ScrollView.GetPos()),
			(float)m_cxView,
			(float)((idxItem + 1) * m_cyItem - m_ScrollView.GetPos()),
		};
		if (rcItem.bottom < 0.f || rcItem.top > m_cyClient)
			return FALSE;

		if (sldi.Item.pszText)
		{
			if (sldi.Item.cchText < 0)
				sldi.Item.cchText = (int)wcslen(sldi.Item.pszText);

			if (idxItem == m_idxHot)
				m_pDC->FillRectangle(rcItem, m_pBrHot);
			else
				m_pDC->FillRectangle(rcItem, m_pBrBk);

			rcItem.left += m_DsF.cxTextPadding;
			m_pDC->DrawTextW(sldi.Item.pszText, sldi.Item.cchText, m_pTextFormat,
				rcItem, m_pBrText, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
			rcItem.left -= m_DsF.cxTextPadding;
		}
		else
			m_pDC->FillRectangle(rcItem, m_pBrBk);
		return TRUE;
	}

	BOOL DrawGroupItem(int idxGroup, int idxItem, D2D1_RECT_F& rcItem)
	{
		SLGETDISPINFO sldi{};
		sldi.bItem = TRUE;
		sldi.Item.cchText = -1;
		sldi.Item.idxItem = idxItem;
		sldi.Item.idxGroup = idxGroup;
		m_fnGetDispInfo(&sldi);

		auto& Item = m_Group[idxGroup].Item[idxItem];
		rcItem =
		{
			(float)m_cxCover,
			(float)(Item.y - m_ScrollView.GetPos()),
			(float)m_cxView,
			(float)(Item.y + m_cyItem - m_ScrollView.GetPos()),
		};

		if (rcItem.bottom < 0.f || rcItem.top > m_cyClient)
			return FALSE;

		if (sldi.Item.pszText)
		{
			if (sldi.Item.cchText < 0)
				sldi.Item.cchText = (int)wcslen(sldi.Item.pszText);

			if (idxItem == m_idxHot && idxGroup == m_idxHotItemSGroup)
				m_pDC->FillRectangle(rcItem, m_pBrHot);
			else
				m_pDC->FillRectangle(rcItem, m_pBrBk);

			rcItem.left += m_DsF.cxTextPadding;
			m_pDC->DrawTextW(sldi.Item.pszText, sldi.Item.cchText, m_pTextFormat,
				rcItem, m_pBrText, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
			rcItem.left -= m_DsF.cxTextPadding;
		}
		else
			m_pDC->FillRectangle(rcItem, m_pBrBk);
		return TRUE;
	}

	BOOL DrawGroup(int idxGroup, D2D1_RECT_F& rcGroup)
	{
		SLGETDISPINFO sldi{};
		sldi.bItem = FALSE;
		sldi.Group.cchText = -1;
		sldi.Group.idxItem = idxGroup;
		sldi.Group.pDC = m_pDC;
		m_fnGetDispInfo(&sldi);

		auto& Group = m_Group[idxGroup];
		rcGroup =
		{
			0.f,
			(float)(Group.y - m_ScrollView.GetPos()),
			(float)m_cxView,
			(float)(Group.y + m_cyGroupHeader - m_ScrollView.GetPos()),
		};

		const int cSubItem = (int)Group.Item.size();

		D2D1_RECT_F rcTemp
		{
			0.f,
			rcGroup.bottom,
			(float)m_cxCover,
			rcGroup.bottom + (float)m_cxCover
		};

		m_pDC->DrawBitmap(sldi.Group.pBmp, &rcTemp);

		rcTemp.top = rcTemp.bottom;
		rcTemp.bottom = rcGroup.bottom + (float)(cSubItem < c_iMinLinePerGroup ? c_iMinLinePerGroup * m_cyItem : cSubItem * m_cyItem);
		m_pDC->FillRectangle(&rcTemp, m_pBrBk);
		if (cSubItem < c_iMinLinePerGroup)
		{
			rcTemp =
			{
				(float)m_cxCover,
				(float)(Group.Item.back().y + m_cyItem),
				(float)m_cxView,
				(float)(Group.Item.front().y + m_cyItem * c_iMinLinePerGroup)
			};
			m_pDC->FillRectangle(&rcTemp, m_pBrBk);
		}

		if (rcGroup.bottom < 0.f || rcGroup.top > m_cyClient)
			return FALSE;

		if (sldi.Group.pszText)
		{
			if (sldi.Group.cchText < 0)
				sldi.Group.cchText = (int)wcslen(sldi.Item.pszText);

			//if (idxItem == m_idxHot)
			//	m_pDC->FillRectangle(rcItem, m_pBrHot);
			//else
				m_pDC->FillRectangle(rcGroup, m_pBrBk);
			//m_pDC->FillRectangle(rcGroup, m_pBrGroup);

			IDWriteTextLayout* pLayout;
			App->m_pDwFactory->CreateTextLayout(sldi.Group.pszText, sldi.Group.cchText, m_pTfGroupTitle,
				(float)m_cxView, (float)m_cyGroupHeader, &pLayout);
			DWRITE_TEXT_METRICS Metrics;
			pLayout->GetMetrics(&Metrics);

			m_pDC->DrawTextLayout({ rcGroup.left + m_DsF.cxTextPadding,rcGroup.top }, pLayout,
				m_pBrGroupText, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
			pLayout->Release();

			const float yLine = rcGroup.top + (float)(m_cyGroupHeader / 2);
			m_pDC->DrawLine({ rcGroup.left + m_DsF.cxTextPadding * 2.f + Metrics.width,yLine },
				{ (float)m_cxView - m_DsF.cxTextPadding,yLine }, m_pBrGroupText, m_DsF.cyGroupLine);
		}
		else
			m_pDC->FillRectangle(rcGroup, m_pBrBk);
		return TRUE;
	}

	void DrawScrollBar()
	{
		const D2D1_RECT_F rcF
		{
			(float)m_cxClient - m_DsF.cxScrollBar,
			0,
			(float)m_cxClient,
			(float)m_cyClient
		};
		m_pDC->FillRectangle(&rcF, m_pBrBk);
		m_pDC->FillRectangle(&m_rcfThumb, m_pBrSBThumb);
	}

	void CalcTopItem();

	void CalcThumb();

	void BeginSBThumbAn(BOOL bEnlarge);
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
		m_Item.resize(cItems);
		//m_cItems = cItems;
		//m_ScrollView.SetMax(m_cItems * m_cyItem);
		//Redraw();
	}

	void SetGroupCount(int cGroups)
	{
		m_Group.resize(cGroups);
	}

	void SetGroupItemCount(int idxGroup, int cItems)
	{
		m_Group[idxGroup].Item.resize(cItems);
	}

	void ReCalc(int idxBegin);

	int GetItemHeight()
	{
		return m_cyItem;
	}

	void SetItemHeight(int cyItem)
	{
		m_cyItem = cyItem;
		m_cxCover = cyItem * 3 - cyItem / 4;
	}

	void Redraw();

	void Redraw(std::initializer_list<int> idxItem);

	void Redraw(int idxGroup, std::initializer_list<int> idxItem);

	int HitTest(POINT pt, SLHITTEST* pslht);
};

