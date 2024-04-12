#pragma once
#include "CApp.h"

constexpr inline PCWSTR WCN_TBGHOST = L"PlayerNew.WndClass.TaskbarGhost";

class CWndMain;
class CTbGhost final :public eck::CWnd
{
	friend class CWndMain;
private:
	CWndMain& m_WndMain;
	HBITMAP m_hbmLivePreviewCache = NULL;// 实时预览位图缓存
	HBITMAP m_hbmThumbnailCache = NULL;// 缩略图位图缓存

	UINT m_cxPrev = 0u, m_cyPrev = 0u;
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

	/// <summary>
	/// 无效化实时预览缓存
	/// </summary>
	PNInline void InvalidateLivePreviewCache()
	{
		if (m_hbmLivePreviewCache)
		{
			DeleteObject(m_hbmLivePreviewCache);
			m_hbmLivePreviewCache = NULL;
		}
	}

	/// <summary>
	/// 无效化缩略图缓存
	/// </summary>
	PNInline void InvalidateThumbnailCache()
	{
		if (m_hbmThumbnailCache)
		{
			DeleteObject(m_hbmThumbnailCache);
			m_hbmThumbnailCache = NULL;
		}
	}

	void SetIconicThumbnail(UINT cxMax = UINT_MAX, UINT cyMax = UINT_MAX);
};