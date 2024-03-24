#pragma once
#include "CApp.h"

#include "eck\CToolTip.h"
#include "eck\GraphicsHelper.h"

constexpr inline PCWSTR WCN_LRC = L"PlayerNew.WndClass.Lrc";

enum
{
	IDLBT_NONE = -1,
	IDLBT_PREV,
	IDLBT_PLAY,
	IDLBT_NEXT,
	IDLBT_LOCK,
	IDLBT_CLOSE,
};

class CWndLrc;
class CLrcBtnBox
{
	friend class CWndLrc;
private:
	constexpr static int c_cBtn = 5;
	CWndLrc& m_Wnd;

	ID2D1Bitmap* m_pBmpPlay = NULL;
	struct BTN
	{
		PCWSTR pszTip;
		ID2D1Bitmap* pBmp;
		BITBOOL bChecked : 1;
	};

	BTN m_Btn[c_cBtn]
	{
		{L"上一首"},
		{L"播放"},
		{L"下一首"},
		{L"锁定"},
		{L"关闭"},
	};

	int m_idxHot = -1;

	BITBOOL m_bLBtnDown : 1 = FALSE;

public:
	CLrcBtnBox(CWndLrc& x) :m_Wnd(x) {}

	~CLrcBtnBox()
	{
		for (auto& x : m_Btn)
			SafeRelease(x.pBmp);
		SafeRelease(m_pBmpPlay);
	}

	void PostCreateRenderTarget();

	void MouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

	int HitTest(POINT pt);

	void Draw();
};

class CWndLrc :public eck::CWnd
{
	friend class CLrcBtnBox;
private:
	constexpr static int c_InvalidCacheIdx = std::numeric_limits<int>::min();
	constexpr static float c_ReCalcDx = std::numeric_limits<float>::min();

	CLrcBtnBox m_BtnBox{ *this };

	int m_cxClient = 0,
		m_cyClient = 0;

	eck::CEzCDC m_DC{};
	ID2D1DCRenderTarget* m_pRT = NULL;
	ID2D1SolidColorBrush* m_pBrHot = NULL;
	ID2D1SolidColorBrush* m_pBrPressed = NULL;

	ID2D1SolidColorBrush* m_pBrBk = NULL;
	ID2D1SolidColorBrush* m_pBrFrame = NULL;
	ID2D1SolidColorBrush* m_pBrTextBorder = NULL;

	IDWriteTextFormat* m_pTfMain = NULL;
	IDWriteTextFormat* m_pTfTranslation = NULL;

	BITBOOL m_bShowBk : 1 = TRUE;
	BITBOOL m_bLocked : 1 = FALSE;
	BITBOOL m_bShow : 1 = FALSE;
	BITBOOL m_bNcLBtnDown : 1 = FALSE;
	BITBOOL m_bDrawLockBtn : 1 = FALSE;
	BITBOOL m_bHoverLockBtn : 1 = FALSE;

	struct TEXTCACHE
	{
		IDWriteTextLayout* pLayout = NULL;
		IDWriteTextLayout* pLayoutTrans = NULL;
		D2D1_SIZE_F size{};
		D2D1_SIZE_F sizeTrans{};
		int idxLrc = c_InvalidCacheIdx;
		BITBOOL bTooLong : 1 = FALSE;
		BITBOOL bTooLongTrans : 1 = FALSE;
		BITBOOL bScrolling : 1 = FALSE;
	};

	TEXTCACHE m_TextCache[2]{};
	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxyBtn, 34)
		ECK_DS_ENTRY(cxBtnPadding, 6)
		;
	ECK_DS_END_VAR(m_Ds);

	ECK_DS_BEGIN(DPISF)
		ECK_DS_ENTRY_F(Margin, 2.f)
		ECK_DS_ENTRY_F(TextPadding, 2.f)
		ECK_DS_ENTRY_F(cxyBtn, 34.f)
		ECK_DS_ENTRY_F(cxBtnPadding, 6.f)
		;
	ECK_DS_END_VAR(m_DsF);

	enum
	{
		IDT_MOUSELEAVE = 101,
		TE_MOUSELEAVE = 800,
		TE_MOUSELEAVE_FIRST = 1600,

		IDT_LOCK = 102,
		TE_LOCK = 500,
	};

	enum
	{
		LRCIDX_APPNAME = c_InvalidCacheIdx + 1,
		LRCIDX_FILENAME,
	};

	void ReSizeRenderStuff(int cx, int cy);

	void UpdateSysColorBrush();

	float DrawLrcLine(int idxLrc, float y, BOOL bSecondLine);

	void DrawStaticLine(int idxFake, float y);

	void UpdateTextFormat();

	void UpdateTextBrush();

	void UpdateOptionRes();

	void UpdateDpi(int iDpi);

	PNInline float CalcMaxLrcWidth() { return (float)m_cxClient - m_DsF.Margin * 6.f; }

	PNInline float CalcLrcMargin() { return m_DsF.Margin * 3.f; }
public:
	static ATOM RegisterWndClass() { return eck::EzRegisterWndClass(WCN_LRC); }

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
			WCN_LRC, pszText, WS_POPUP | WS_VISIBLE,
			x, y, cx, cy, NULL, NULL, App->GetHInstance(), NULL);
	}

	EckInline void InvalidateCache()
	{
		m_TextCache[0].idxLrc = c_InvalidCacheIdx;
		m_TextCache[1].idxLrc = c_InvalidCacheIdx;
	}

	EckInline int GetDpiValue() const { return m_iDpi; }

	void Draw();

	void DoCmd(int idx);

	void ShowWnd(BOOL bShow);

	void Lock(BOOL bLock);

	EckInline BOOL IsCacheLayoutTooLong() const
	{ 
		return
			(m_TextCache[0].idxLrc != c_InvalidCacheIdx &&
				(m_TextCache[0].bTooLong || m_TextCache[0].bTooLongTrans)) ||
			(m_TextCache[1].idxLrc != c_InvalidCacheIdx &&
				(m_TextCache[1].bTooLong || m_TextCache[1].bTooLongTrans));
	}

	EckInline BOOL IsBkVisible() const { return m_bShowBk; }
};