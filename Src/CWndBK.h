#pragma once
#include "CApp.h"
#include "COptionsMgr.h"

#include "CDlgEffect.h"

#include "eck\DuiBase.h"
#include "eck\CDuiCircleButton.h"
#include "eck\CDuiTrackBar.h"
#include "eck\CDuiScrollBar.h"
#include "eck\CInertialScrollView.h"
#include "eck\CMenu.h"

#include <thread>

constexpr inline D2D1_COLOR_F c_D2DClrCyanDeeper{ 0.f,0.3764f,0.7529f,1.f };
constexpr inline D2D1_COLOR_F c_D2DCrUIProgBarTempMark{ 0.0f,0.502f,1.0f,1.0f };

namespace Dui
{
	using namespace eck::Dui;
}

// UI元素类型
enum UIELEMTYPE
{
	UIET_INVALID,
	UIET_BK,
	UIET_ALBUM,
	UIET_ALBUMROTATING,
	UIET_WAVES,
	UIET_SPE,
	UIET_SPE2,
	UIET_LRC,
	UIET_PROGBAR,
	UIET_TOOLBAR,
	UIET_INFO,
	UIET_BUTTON,
	UIET_ROUNDBUTTON,
	UIET_PLAYINGCTRL,
};

// UI元素标志
enum UIELEMFLAGS
{
	UIEF_NONE = 0,						// 无
	UIEF_STATIC = (1u << 0),			// 静态元素
	UIEF_NOEVENT = (1u << 1),			// 无需转发事件
	UIEF_WANTTIMEREVENT = (1u << 2),	// 需要定时器周期事件
	UIEF_ONLYPAINTONTIMER = (1u << 3),	// 定时器触发时仅重画
};

enum UIELEMSTYLE
{
	UIES_NOERASEBK = (1u << 0),			// 不使用静态图擦除背景
};

// UI元素事件
enum UIELEMEVENT
{
	UIEE_FIRST = Dui::WM_PRIVBEGIN,
	UIEE_ONPLAYINGCTRL,		// (type, 0)	播放控制操作已发生
	UIEE_ONSETTINGSCHANGE,	// (0, 0)		设置已被更改
	UIEE_DWMCOLORCHANGED,	// (0, 0)
};


enum class ThreadState
{
	Stopped,
	Running,
	Error
};

enum
{
	IDE_BT_OPT = 101,
	IDE_BT_PLAYOPT,
	IDE_BT_REPEATMODE,
	IDE_BT_PREV,
	IDE_BT_PLAY,
	IDE_BT_NEXT,
	IDE_BT_STOP,
	IDE_BT_LRC,
	IDE_BT_ABOUT,

	IDE_TB_PROGRESS,
	IDE_TB_VOL,
};

enum CUIBTNNOTIFY
{
	CUIBN_CLICK,
	CUIBN_RCLICK,
};

#define CUI_FRIEND_ALL_ELEM \
	friend class CUIElem; \
	friend class CUIAlbum; \
	friend class CUIAlbumRotating; \
	friend class CUIWaves; \
	friend class CUISpe; \
	friend class CUISpe2; \
	friend class CUILrc; \
	friend class CUIProgBar; \
	friend class CUIToolBar; \
	friend class CUIInfo; \
	friend class CUIButton; \
	friend class CUIRoundButton; \
	friend class CUIPlayingCtrl;

constexpr UINT WM_BK_LRCANIMATION = eck::WM_USER_SAFE;

class CUIElem;
class CWndBK :public eck::Dui::CDuiWnd
{
	friend class CWndMain;

	CUI_FRIEND_ALL_ELEM
private:
	ID2D1Bitmap1* m_pBmpBKStatic = NULL;		// 静态背景，清除区域用
	ID2D1Bitmap1* m_pBmpAlbum = NULL;			// 封面原始位图
	ID2D1SolidColorBrush* m_pBrWhite = NULL;
	ID2D1SolidColorBrush* m_pBrWhite2 = NULL;

	int m_cxAlbum = 0;
	int m_cyAlbum = 0;

	int m_cxClient = 0;
	int m_cyClient = 0;

	D2D1_RECT_F m_rcfClient{};

	std::vector<CUIElem*> m_vElemsWantTimer{};
	std::vector<CUIElem*> m_vAllElems{};

	UINT m_uMsgCUIButton = 0;// (uCode, pElem)

	CDlgEffect m_DlgFx{};

	std::vector<eck::ILayout*> m_vLayout{};
	eck::ILayout* m_pLayout = NULL;

	enum
	{
		ICIDX_Prev,
		ICIDX_Play,
		ICIDX_Stop,
		ICIDX_Next,
		ICIDX_Lrc,
		ICIDX_RMAllLoop,
		ICIDX_RMAll,
		ICIDX_RMRadom,
		ICIDX_RMSingleLoop,
		ICIDX_RMSingle,
		ICIDX_PlayOpt,
		ICIDX_PlayList,
		ICIDX_Options,
		ICIDX_About,
		ICIDX_Pause,

		ICIDX__MAX
	};

	ID2D1Bitmap1* m_pBmpIcon[ICIDX__MAX]{};

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxTime, 100)
		ECK_DS_ENTRY(cxPCBT, 36)
		ECK_DS_ENTRY(cyPCBT, 36)
		ECK_DS_ENTRY(cxPCBTBig, 52)
		ECK_DS_ENTRY(cyPCBTBig, 52)
		ECK_DS_ENTRY(iPCBTGap, 8)
		ECK_DS_ENTRY(cyVolTrack, 14)
		ECK_DS_ENTRY(cyVolParent, 40)
		ECK_DS_ENTRY(cxVolTrack, 170)
		ECK_DS_ENTRY(cxyMinSBThumb, 10)
		ECK_DS_ENTRY(ScrollDelta, 80)
		ECK_DS_ENTRY(UIMargin, 12)
		ECK_DS_ENTRY(UIMarginInt, 8)
		;
	ECK_DS_END_VAR(m_Ds);

	ECK_DS_BEGIN(DPIS_F)
		ECK_DS_ENTRY_F(sizeAlbumLevel, 15)
		ECK_DS_ENTRY_F(cxWaveLine, 2.f)
		ECK_DS_ENTRY_F(cxSepLine, 2.f)
		ECK_DS_ENTRY_F(cyProgBarTrack, 6.f)
		ECK_DS_ENTRY_F(cxTopTip, 50.f)
		ECK_DS_ENTRY_F(cyTopTip, 18.f)
		ECK_DS_ENTRY_F(cyTopTitle, 30.f)
		ECK_DS_ENTRY_F(sizeTopTipGap, 6.f)
		ECK_DS_ENTRY_F(cxIcon, 20.f)
		ECK_DS_ENTRY_F(cyIcon, 20.f)
		ECK_DS_ENTRY_F(cxySBTrack, 10.f)
		;
	ECK_DS_END_VAR(m_DsF);


	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnSize(HWND hWnd, UINT state, int cx, int cy);

	void OnDestroy(HWND hWnd);

	void SetupElem();
public:
	enum
	{
		IDT_PGS = 101,
	};

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	LRESULT OnElemEvent(Dui::CElem* pElem, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	/// <summary>
	/// 播放控制操作已发生
	/// </summary>
	void OnPlayingControl(PLAYINGCTRLTYPE uType);

	/// <summary>
	/// 设置被改变
	/// </summary>
	PNInline void OnSettingsChanged() { BroadcastEvent(UIEE_ONSETTINGSCHANGE, 0, 0); }

	/// <summary>
	/// 更新静态位图
	/// </summary>
	void UpdateStaticBmp();

	void RegisterTimerElem(CUIElem* pElem)
	{
		m_vElemsWantTimer.emplace_back(pElem);
	}

	template<class T>
	PNInline T Dpi(T Size) const
	{
		return Size * (T)m_iDpi / (T)96;
	}

	PNInline const auto& GetDpiSize() const { return m_Ds; }

	PNInline const auto& GetDpiSizeF() const { return m_DsF; }
};

#ifndef NDEBUG
#if 1
// 【调试用】画元素边框
#define BkDbg_DrawElemFrame()					\
	{											\
		ID2D1SolidColorBrush* UIDBG_pBrush___;	\
		m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &UIDBG_pBrush___); \
		if (UIDBG_pBrush___)					\
		{										\
			m_pDC->DrawRectangle(GetViewRectF(), UIDBG_pBrush___); \
			UIDBG_pBrush___->Release();			\
		}										\
	}
#else
#define BkDbg_DrawElemFrame() ;
#endif // 1
#else
// 【调试用】画元素边框
// Release模式下已被定义为空宏
#define BkDbg_DrawElemFrame() ;
#endif // !NDEBUG


// UI元素类
class CUIElem :public eck::Dui::CElem
{
	friend class CWndBK;
	friend class CUIBK;
	CUI_FRIEND_ALL_ELEM
protected:
	UINT m_uType = UIET_INVALID;
	UINT m_uFlags = UIEF_NONE;
public:
	UINT m_uStyle = 0u;

	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	virtual void OnTimer(UINT uTimerID) {}

	EckInline CWndBK* GetBk() { return (CWndBK*)GetWnd(); }
};

// 封面
class CUIAlbum final :public CUIElem
{
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	LRESULT OnComposite() override;
};

// 旋转封面
class CUIAlbumRotating final :public CUIElem
{
private:
	float m_fAngle = 0.f;// 当前旋转角度

	ID2D1BitmapBrush1* m_pBrAlbum = NULL;// 封面位图画刷
	ID2D1SolidColorBrush* m_pBrUV = NULL;// 电平画刷
	ID2D1SolidColorBrush* m_pBrUV2 = NULL;// 电平背景画刷
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void OnTimer(UINT uTimerID) override;

	/// <summary>
	/// 更新位图画刷
	/// </summary>
	void UpdateAlbumBrush();
};

// 波形
class CUIWaves final :public CUIElem
{
private:
	int m_cxLine = 0;// 线宽

	ID2D1SolidColorBrush* m_pBrLine = NULL;// 线画刷
	ID2D1SolidColorBrush* m_pBrCenterMark = NULL;// 中线画刷
	IDWriteTextFormat* m_pTfTip = NULL;

	ThreadState m_ThreadState = ThreadState::Stopped;// 线程状态
	HANDLE m_hThread = NULL;
	HANDLE m_hEvent = NULL;
	std::vector<DWORD> m_vWavesData{};

	void GetWavesData();
public:

	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void OnTimer(UINT uTimerID) override;

	/// <summary>
	/// 置绘制线宽
	/// </summary>
	/// <param name="i">宽度</param>
	PNInline void SetLineWidth(int i) { m_cxLine = i; }

	/// <summary>
	/// 取绘制线宽
	/// </summary>
	/// <returns>返回线宽</returns>
	PNInline int GetLineWidth() { return m_cxLine; }
};

// 频谱
class CUISpe final :public CUIElem
{
private:
	int m_iCount = 0;			// 频谱条数量
	float m_cxBar = 0.f;		// 频谱条宽度
	float m_cxGap = 0.f;		// 间隔宽度

	int* m_piOldHeight = NULL;	// 旧高度信息
	int* m_piHeight = NULL;		// 高度
	int* m_piOldMaxPos = NULL;	// 旧峰值
	int* m_piTime = NULL;		// 计时

	std::vector<int> m_vBuf{};	// 缓冲区
	SIZE_T m_cbPerUnit = 0u;	// 每单元字节数

	ID2D1SolidColorBrush* m_pBrBar = NULL;// 频谱条画刷
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void OnTimer(UINT uTimerID) override
	{
		if (uTimerID == CWndBK::IDT_PGS)
			InvalidateRect();
	}

	/// <summary>
	/// 置频谱条数量
	/// </summary>
	/// <param name="i">数量</param>
	void SetCount(int i);

	/// <summary>
	/// 取频谱条数目
	/// </summary>
	/// <returns>返回数目</returns>
	PNInline int GetCount() { return m_iCount; }

	/// <summary>
	/// 置间隔宽度
	/// </summary>
	/// <param name="i">宽度</param>
	PNInline void SetGapWidth(float i)
	{
		if (i < 0)
			i = 0;
		m_cxGap = i;
	}

	/// <summary>
	/// 取间隔宽度
	/// </summary>
	/// <returns>返回间隔宽度</returns>
	PNInline float GetGapWidth() { return m_cxGap; }
};

// 频谱2
class CUISpe2 final :public CUIElem
{
private:
	int m_cSample = 0;		// 样本数
	float* m_pfBuf = NULL;	// 样本缓冲区
	SIZE_T m_cbBuf = 0;		// 缓冲区大小
	float m_cxStep = 0.f;	// 样本显示间隔

	ID2D1SolidColorBrush* m_pBrLine = NULL;
public:
	CUISpe2();
	~CUISpe2();

	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void OnTimer(UINT uTimerID) override;

	/// <summary>
	/// 置样本数
	/// </summary>
	/// <param name="i">样本数</param>
	void SetSampleCount(int i);

	/// <summary>
	/// 取样本数
	/// </summary>
	/// <returns>返回样本数</returns>
	PNInline int GetSampleCount() { return m_cSample; }
};

struct LRCITEMHEIGHTDATA
{
	float cy;
};

template <class FwdIt, class Ty, class Pr>
[[nodiscard]] constexpr FwdIt LowerBound(FwdIt First, const FwdIt Last, const Ty& Val, Pr Pred) {

	auto UFirst = (First);
	std::iter_difference_t<FwdIt> Count = std::distance(UFirst, (Last));

	while (0 < Count) {
		const std::iter_difference_t<FwdIt> Count2 = Count / 2;
		const auto UMid = std::next(UFirst, Count2);
		if (Pred(UMid, Val)) {
			UFirst = std::next(UMid);
			Count -= Count2 + 1;
		}
		else
			Count = Count2;
	}

	return UFirst;
}


// 滚动歌词
class CUILrc final :public CUIElem
{
private:
	struct ITEM
	{
		// 不要修改前两个字段的位置
		IDWriteTextLayout* pLayout = NULL;
		IDWriteTextLayout* pLayoutTrans = NULL;
		ID2D1GeometryRealization* pGr = NULL;
		float x = 0.f;
		float y = 0.f;
		float cy = 0.f;
		float cx = 0.f;

		float cxTrans = 0.f;
		BITBOOL bSel : 1 = FALSE;
		BITBOOL bCacheValid : 1 = FALSE;

		ITEM() = default;
		ITEM(const ITEM& x) = delete;
		ITEM& operator=(const ITEM& x) = delete;
		ITEM(ITEM&& x) noexcept
		{
			memcpy(this, &x, sizeof(*this));
			x.pLayout = NULL;
			x.pGr = NULL;
		}
		ITEM& operator=(ITEM&& x) noexcept
		{
			memcpy(this, &x, sizeof(*this));
			x.pLayout = NULL;
			x.pGr = NULL;
		}
		~ITEM()
		{
			if (pLayout)
				pLayout->Release();
			if (pGr)
				pGr->Release();
		}
	};
private:
	ID2D1SolidColorBrush* m_pBrTextNormal = NULL;
	ID2D1SolidColorBrush* m_pBrTextHighlight = NULL;
	ID2D1SolidColorBrush* m_pBrush = NULL;

	IDWriteTextFormat* m_pTextFormat = NULL;
	IDWriteTextFormat* m_pTextFormatTrans = NULL;

	ID2D1GeometryRealization* m_pGrEmptyText = NULL;

	ID2D1DeviceContext1* m_pDC1 = NULL;

	Dui::CScrollBar m_SB{};
	eck::CInertialScrollView* m_psv = NULL;

	int m_idxTop = -1;
	int m_idxHot = -1;
	int m_idxMark = -1;
	int m_idxPrevCurr = -1;
	std::vector<ITEM> m_vItem{};

	eck::CEasingAn<eck::Easing::FOutCubic> m_AnEnlarge{};
	BOOL m_bEnlarging = FALSE;
	int m_idxPrevAnItem = -1,
		m_idxCurrAnItem = -1;
	float m_fAnValue = 1.f;

	int m_tMouseIdle = 0;

	BITBOOL m_bCtxMenuOpen : 1 = FALSE;

	enum
	{
		IDMI_PLAY_FROM_THIS = 100,
		IDMI_COPY_LRC,
		IDMI_COPY_LRC_1,
		IDMI_COPY_LRC_2,
		IDMI_GEN_SHARE_IMAGE,
	};

	eck::CMenu m_Menu
	{
		{ L"从此处播放",IDMI_PLAY_FROM_THIS },
		{ L"复制歌词",IDMI_COPY_LRC },
		{ L"复制歌词原文",IDMI_COPY_LRC_1 },
		{ L"复制歌词翻译",IDMI_COPY_LRC_2 },
		{ NULL,0,MF_SEPARATOR },
		{ L"生成分享图片",IDMI_GEN_SHARE_IMAGE },
	};

	enum
	{
		IDT_MOUSEIDLE = 1001,
		TE_MOUSEIDLE = 500,
		T_MOUSEIDLEMAX = 4500,
	};

	static void ScrollProc(int iPos, int iPrevPos, LPARAM lParam);

	void ReCreateTextFormat();

	void ReCreateEmptyText();

	PNInline void FillItemBkg(int idx, const D2D1_RECT_F& rc)
	{
		constexpr D2D1_COLOR_F crHot{ eck::ColorrefToD2dColorF(eck::Colorref::Silver,0.5f) };
		constexpr D2D1_COLOR_F crSel{ eck::ColorrefToD2dColorF(eck::Colorref::Gray,0.6f) };
		constexpr D2D1_COLOR_F crHotSel{ eck::ColorrefToD2dColorF(eck::Colorref::Gray,0.8f) };

		if (m_vItem[idx].bSel)
			if (idx == m_idxHot)
				m_pBrush->SetColor(crHotSel);
			else
				m_pBrush->SetColor(crSel);
		else
			if (idx == m_idxHot)
				m_pBrush->SetColor(crHot);
			else
				return;
		m_pDC->FillRectangle(rc, m_pBrush);
	}

	void CalcTopItem();

	BOOL DrawItem(int idx, float& y);

	PNInline void BeginMouseIdleDetect()
	{
		if (!m_tMouseIdle)
			SetTimer(GetWnd()->HWnd, IDT_MOUSEIDLE, TE_MOUSEIDLE, NULL);
		m_tMouseIdle = T_MOUSEIDLEMAX;
	}

	int HitTest(POINT pt);

	void GetItemRect(int idx, RECT& rc);

	PNInline float GetItemY(int idx)
	{
		const auto fScale = App->GetOptionsMgr().ScLrcCurrFontScale;
		float y = m_vItem[idx].y - m_psv->GetPos();
		if (idx > m_idxPrevAnItem && m_idxPrevAnItem >= 0)
			y += (m_vItem[m_idxPrevAnItem].cy * (fScale - m_fAnValue));
		if (idx > m_idxCurrAnItem && m_idxCurrAnItem >= 0)
			y += (m_vItem[m_idxCurrAnItem].cy * (m_fAnValue - 1.f));
		return y;
	}

	PNInline void InvalidateItem(int idx)
	{
		RECT rc;
		GetItemRect(idx, rc);
		ElemToClient(rc);
		rc.top -= 1;
		rc.bottom += 1;
		rc.right += 1;
		InvalidateRect(&rc);
	}

	void ScrollToCurrPos();

	void LayoutItems();

	void Scrolled();
public:
	CUILrc()
	{
		int a = 0;
	}

	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void OnTimer(UINT uTimerID) override;
};

// 歌曲信息
class CUIInfo final :public CUIElem
{
private:
	ID2D1SolidColorBrush* m_pBrBigTip = NULL;
	ID2D1SolidColorBrush* m_pBrSmallTip = NULL;
	IDWriteTextFormat* m_pTfTitle = NULL;
	IDWriteTextFormat* m_pTfTip = NULL;

	void UpdateTextFormat();
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};

class CUIVolTrackBar final :public CUIElem
{
	friend class CUIPlayingCtrl;
private:
	Dui::CTrackBar m_TrackBar{};
	ID2D1SolidColorBrush* m_pBrush = NULL;
public:
	BOOL Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent, Dui::CDuiWnd* pWnd, int iId = 0, PCVOID pData = NULL) override
	{
		dwStyle |= Dui::DES_TRANSPARENT;
		return IntCreate(pszText, dwStyle, dwExStyle, x, y, cx, cy, pParent, pWnd, iId, pData);
	}

	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};

class CUIPlayingCtrl final :public CUIElem
{
private:
	Dui::CCircleButton m_BTOptions{};
	Dui::CCircleButton m_BTPlayOpt{};
	Dui::CCircleButton m_BTRepeatMode{};
	Dui::CCircleButton m_BTPrev{};
	Dui::CCircleButton m_BTPlay{};
	Dui::CCircleButton m_BTNext{};
	Dui::CCircleButton m_BTStop{};
	Dui::CCircleButton m_BTLrc{};
	Dui::CCircleButton m_BTAbout{};
	CUIVolTrackBar m_VolTB{};
public:
	BOOL Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent, Dui::CDuiWnd* pWnd, int iId = 0, PCVOID pData = NULL) override
	{
		dwStyle |= Dui::DES_TRANSPARENT;
		return IntCreate(pszText, dwStyle, dwExStyle, x, y, cx, cy, pParent, pWnd, iId, pData);
	}

	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};

class CUIProgressBar final :public CUIElem
{
private:
	Dui::CTrackBar m_TrackBar{};
public:
	BOOL Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent, Dui::CDuiWnd* pWnd, int iId = 0, PCVOID pData = NULL) override
	{
		dwStyle |= Dui::DES_TRANSPARENT;
		return IntCreate(pszText, dwStyle, dwExStyle, x, y, cx, cy, pParent, pWnd, iId, pData);
	}

	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void OnTimer(UINT uTimerID) override;
};