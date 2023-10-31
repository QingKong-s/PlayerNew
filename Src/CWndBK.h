#pragma once
#include "CApp.h"
#include "COptionsMgr.h"

#include "eck\CWnd.h"

#include <thread>

constexpr inline auto WCN_MAINBK = L"PlayerNew.WndClass.MainBK";

constexpr inline D2D1_COLOR_F c_D2DClrCyanDeeper{ 0.f,0.3764f,0.7529f,1.f };
constexpr inline D2D1_COLOR_F c_D2DCrUIProgBarTempMark{ 0.0f,0.502f,1.0f,1.0f };

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
	UIEE_SETRECT = 1,			// (&rc, 0)		矩形将被更改
	UIEE_ONPLAYINGCTRL = 2,		// (type, 0)	播放控制操作已发生
	UIEE_ONSETTINGSCHANGE = 3,	// (0, 0)		设置已被更改
	UIEE_CHILDREDRAW = 4,		// (pElem, 0)	子元素请求重画，只会在有UIES_NOERASEBK样式的子元素执行即时重画时发送
	UIEE_DESTROY = 5,			// (0, 0)		销毁元素
};

// 播放控制类型
enum PLAYINGCTRLTYPE
{
	PCT_PLAY,
	PCT_PAUSE,
	PCT_STOP
};

enum class ThreadState
{
	Stopped,
	Running,
	Error
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

class CUIElem;
class CWndBK :public eck::CWnd
{
	friend class CWndMain;

	CUI_FRIEND_ALL_ELEM
private:
	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBmpBK = NULL;
	ID2D1Bitmap1* m_pBmpBKStatic = NULL;		// 静态背景，清除区域用
	ID2D1Bitmap1* m_pBmpAlbum = NULL;			// 封面原始位图
	ID2D1SolidColorBrush* m_pBrWhite = NULL;
	ID2D1SolidColorBrush* m_pBrWhite2 = NULL;

	int m_cxAlbum = 0;
	int m_cyAlbum = 0;

	int m_cxClient = 0;
	int m_cyClient = 0;

	D2D1_RECT_F m_rcfClient{};

	std::vector<CUIElem*> m_vElems{};
	std::vector<CUIElem*> m_vElemsWantTimer{};
	std::vector<CUIElem*> m_vAllElems{};

	std::vector<RECT> m_vDirtyRect{};

	enum
	{
		IDT_PGS = 101,
	};

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
		;
	ECK_DS_END_VAR(m_DsF);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnSize(HWND hWnd, UINT state, int cx, int cy);

	void OnDestroy(HWND hWnd);

	/// <summary>
	/// 生成元素事件
	/// </summary>
	/// <param name="uEvent"></param>
	/// <param name="wParam"></param>
	/// <param name="lParam"></param>
	void GenElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam);
public:
	CWndBK();

	~CWndBK();

	static ATOM RegisterWndClass();

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;
	
	/// <summary>
	/// 添加元素
	/// </summary>
	/// <param name="pElem">元素对象</param>
	void AddElem(CUIElem* pElem);

	/// <summary>
	/// 新歌曲初始化完毕
	/// </summary>
	void OnPlayingControl(PLAYINGCTRLTYPE uType);

	/// <summary>
	/// 设置被改变
	/// </summary>
	PNInline void OnSettingsChanged() { GenElemEvent(UIEE_ONSETTINGSCHANGE, 0, 0); }

	/// <summary>
	/// 重画
	/// </summary>
	void Redraw();

	/// <summary>
	/// 更新静态位图
	/// </summary>
	void UpdateStaticBmp();

	template<class T>
	PNInline T Dpi(T Size) const
	{
		return Size * (T)m_iDpi / (T)96;
	}
};

#ifndef NDEBUG
#if 1
// 【调试用】画元素边框
#define BkDbg_DrawElemFrame() \
	{ \
		ID2D1SolidColorBrush* UIDBG_pBrush___; \
		pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &UIDBG_pBrush___); \
		if (UIDBG_pBrush___) \
		{ \
			pDC->DrawRectangle(m_rcF, UIDBG_pBrush___); \
			UIDBG_pBrush___->Release(); \
		} \
	}
#else
#define BkDbg_DrawElemFrame()
#endif // 1
#else
// 【调试用】画元素边框
// Release模式下已被定义为空宏
#define BkDbg_DrawElemFrame()
#endif // !NDEBUG

// 在UI元素类中声明可执行立即显示的重绘方法，需要Redraw方法
#define CUI_DeclRedrawAndPresent \
	PNInline void Redraw(BOOL bImmdShow) override \
	{ \
		auto pDC = m_pBK->m_pDC; \
		if (m_pParent) \
		{ \
			m_pParent->OnElemEvent(UIEE_CHILDREDRAW, (WPARAM)this, 0); \
			goto CUIRedrawImmdShow; \
		} \
		pDC->BeginDraw(); \
		Redraw(); \
		pDC->EndDraw(); \
	CUIRedrawImmdShow: \
		if (bImmdShow) \
		{ \
			DXGI_PRESENT_PARAMETERS pp; \
			pp.DirtyRectsCount = 1; \
			pp.pDirtyRects = &m_rc; \
			pp.pScrollOffset = NULL; \
			pp.pScrollRect = NULL; \
			m_pBK->m_pSwapChain->Present1(0, 0, &pp); \
		} \
	}

// UI元素类
class CUIElem
{
	friend class CWndBK;
	friend class CUIBK;
	CUI_FRIEND_ALL_ELEM
protected:
	UINT m_uType = UIET_INVALID;
	UINT m_uFlags = UIEF_NONE;

	D2D1_RECT_F m_rcF{};	// 元素矩形
	RECT m_rc{};			// 元素矩形
	RECT m_rcInWnd{};		// 元素矩形与窗口矩形的交集
	RECT m_rcRelative{};	// 相对父元素的位置
	int m_cxHalf = 0,		// 元素宽度的一半
		m_cyHalf = 0;		// 元素高度的一半
	int m_cx = 0,			// 元素宽度
		m_cy = 0;			// 元素高度

	BOOL m_bShow = TRUE;
	CWndBK* m_pBK = NULL;

	CUIElem* m_pParent = NULL;			// 父元素
	std::vector<CUIElem*> m_vChildren{};// 子元素

	/// <summary>
	/// 默认属性更改处理过程
	/// </summary>
	LRESULT DefElemEventProc(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam);

	PNInline void DispatchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		for (auto pElem : m_vChildren)
		{
			if (!eck::IsBitSet(pElem->m_uFlags, UIEF_NOEVENT))
				if (pElem->OnEvent(uMsg, wParam, lParam))
					break;
		}
	}
public:
	UINT m_uStyle = 0u;

	virtual ~CUIElem();

	/// <summary>
	/// 通用方法：通知元素事件
	/// </summary>
	/// <param name="uEvent">事件标识符</param>
	/// <param name="wParam"></param>
	/// <param name="lParam"></param>
	/// <returns></returns>
	virtual LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
	{
		return DefElemEventProc(uEvent, wParam, lParam);
	}

	/// <summary>
	/// 通用方法：处理窗口消息
	/// </summary>
	/// <param name="uMsg"></param>
	/// <param name="wParam"></param>
	/// <param name="lParam"></param>
	/// <returns></returns>
	virtual BOOL OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) { return 0; }

	/// <summary>
	/// 通用方法：定时器周期事件
	/// </summary>
	/// <param name="uTimerID">定时器ID</param>
	virtual void OnTimer(UINT uTimerID) {}

	/// <summary>
	/// 通用方法：重画
	/// </summary>
	virtual void Redraw() {}

	/// <summary>
	/// 通用方法：重画。
	/// 独立调用BeginDraw/EndDraw
	/// </summary>
	/// <param name="bImmdShow">是否立即显示</param>
	virtual void Redraw(BOOL bImmdShow) {}

	/// <summary>
	/// 通用方法：初始化元素信息
	/// </summary>
	/// <returns>初始化成功返回TRUE，否则返回FALSE</returns>
	virtual BOOL InitElem() { return TRUE; }

	/// <summary>
	/// 置元素矩形
	/// </summary>
	/// <param name="rc">矩形</param>
	PNInline void SetElemRect(RECT* prc) { OnElemEvent(UIEE_SETRECT, (WPARAM)prc, 0); }

	/// <summary>
	/// 取元素矩形
	/// </summary>
	/// <param name="prc">RECT指针</param>
	/// <returns></returns>
	PNInline void GetElemRect(RECT* prc) const { *prc = m_rc; }

	/// <summary>
	/// 元素是否可见
	/// </summary>
	/// <returns>可见性</returns>
	PNInline BOOL IsElemVisible() const { return m_bShow; }

	/// <summary>
	/// 置元素可见性
	/// </summary>
	/// <param name="bShow">是否可见</param>
	PNInline void ShowElem(BOOL bShow) { m_bShow = bShow; }

	void SetParent(CUIElem* pParent);
};

// 封面
class CUIAlbum final :public CUIElem
{
public:
	CUIAlbum();
	void Redraw() override;
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
	CUIAlbumRotating();
	~CUIAlbumRotating();
	void Redraw() override;
	void OnTimer(UINT uTimerID) override;
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;
	BOOL InitElem() override;
	CUI_DeclRedrawAndPresent;

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
	CUIWaves();
	~CUIWaves();
	void Redraw() override;
	BOOL InitElem() override;
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;

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
	CUISpe();
	~CUISpe();
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;
	void Redraw() override;
	BOOL InitElem() override;

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
	void Redraw() override;
	BOOL InitElem() override;
	void OnTimer(UINT uTimerID) override;
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;

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
// 滚动歌词
class CUILrc final :public CUIElem
{
private:
	ID2D1SolidColorBrush* m_pBrTextNormal = NULL;
	ID2D1SolidColorBrush* m_pBrTextHighlight = NULL;

	IDWriteTextFormat* m_pTextFormat = NULL;

	int		m_idxCenter = -1;// 中间歌词
	int		m_idxCurr = -1;	// 当前歌词
	int		m_idxMouseHover = -1;	// 当前热点歌词
	int		m_idxLastMouseHover = -1;	// 上次热点歌词
	int		m_idxLastHighlight = -1;	// 上次高亮歌词
	float	m_yCenterItem = 0.f;	// 中间句水平中线的顶边

	int		m_idxTopItem = 0;	// 第一句可见歌词
	float	m_fOffsetTopIndex = 0.f;	// 第一句可见歌词的遮挡高度

	//LRCHSCROLLINFO  m_LrcHScrollInfo{ -1 };	// 歌词水平滚动信息，仅适用于当前播放行
	//LRCVSCROLLINFO  m_LrcVScrollInfo{};		// 垂直滚动动画信息
	BOOL	m_bVAnOngoing = FALSE;// 垂直滚动动画是否在进行，此成员为TRUE时不能进行在歌词区域内执行其他呈现交换链的操作，否则会有一些奇怪的问题。。。。

	int		m_iDrawingID = 0;

	ULONGLONG m_ullLastDrawingTime = 0ull;

	//int             m_iLastLrcIndex[2] = { -1,-1 };// 0：上次中心；1：上次高亮

	int m_idxAnStart = -1;
	int m_idxAnEnd = -1;

	RECT m_rcLrc = {};
	D2D1_RECT_F m_rcFLrc = {};
	int m_cxLrc = 0,
		m_cyLrc = 0;
	float m_cxLrcF = 0.f,
		m_cyLrcF = 0.f;

	BOOL		m_bShowSB = FALSE;

	RECT m_rcSB = {};
	D2D1_RECT_F m_rcFSB = {};
	BOOL m_bSBLBtnDown = FALSE;
	int m_iSBCursorOffset = 0;
	int m_iThumbSize = 0;
	RECT m_rcThumb = {};
	D2D1_RECT_F m_rcFThumb = {};

	UINT m_uSBMax = 0u;
	UINT m_uSBPos = 0u;

	int m_iDelayTime = 0;
public:
	CUILrc();
	~CUILrc();
	BOOL OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void Redraw() override;
	CUI_DeclRedrawAndPresent;
	void OnTimer(UINT uTimerID) override;
	BOOL InitElem() override;
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;

	/// <summary>
	/// 从m_idxTopItem项开始向下绘画所有歌词
	/// </summary>
	void DrawAllLrc_Top();

	/// <summary>
	/// 以当前歌词为中间一句绘画所有歌词，中间句歌词位置受m_yCenterItem控制
	/// </summary>
	void DrawAllLrc_Center();

	/// <summary>
	/// 画滚动条
	/// </summary>
	/// <param name="bImmdShow">是否立即显示</param>
	void RedrawSB(BOOL bImmdShow);

	/// <summary>
	/// 画一句歌词
	/// </summary>
	/// <param name="iIndex">索引</param>
	/// <param name="y">高度，若要使用上次高度，设置为INFINITY</param>
	/// <param name="bTop">指定"y"参数是否为顶边，TRUE为顶边，FALSE为底边，若y==INFINITY，则此参数必须设为TRUE</param>
	/// <param name="bClearBK"></param>
	/// <param name="bImmdShow">是否立即显示</param>
	/// <returns>高度</returns>
	float RedrawItem(int iIndex, float y, BOOL bTop, BOOL bClearBK, BOOL bImmdShow, BOOL bCenterLine = FALSE, int* yOut = NULL);

	/// <summary>
	/// 命中测试
	/// </summary>
	/// <param name="pt">测试点</param>
	/// <returns>成功返回歌词索引，失败返回-1</returns>
	int HitTest(POINT pt);

	/// <summary>
	/// 初始化竖直切换动画
	/// </summary>
	void InitVAnimation();

	void InitAnimation(int idxPrevCenter, int idxNewCenter);
};

// 进度条
class CUIProgBar final :public CUIElem
{
private:
	ULONGLONG m_ullPos = 0ull;		// 位置
	ULONGLONG m_ullMax = 0ull;		// 最大位置

	ULONGLONG m_ullTempPos = 0ull;	// 临时位置
	BOOL m_bShowTempMark = FALSE;	// 是否启用临时位置

	BOOL m_bLBtnDown = FALSE;		// 左键是否按下

	ID2D1SolidColorBrush* m_pBrNormal = NULL;	// 进度画刷
	ID2D1SolidColorBrush* m_pBrBK = NULL;		// 背景画刷
	ID2D1SolidColorBrush* m_pBrTempMark = NULL;	// 临时进度画刷
public:
	CUIProgBar();
	~CUIProgBar();
	BOOL OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void Redraw() override;
	BOOL InitElem() override;
	void OnTimer(UINT uTimerID) override;
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;
	CUI_DeclRedrawAndPresent;

	/// <summary>
	/// 置位置
	/// </summary>
	/// <param name="ullPos">位置</param>
	PNInline void SetPos(ULONGLONG ullPos)
	{
		if (ullPos > m_ullMax)
			ullPos = m_ullMax;
		m_ullPos = ullPos;
	}

	/// <summary>
	/// 取位置
	/// </summary>
	/// <returns>返回位置</returns>
	PNInline ULONGLONG GetPos() { return m_ullPos; }

	/// <summary>
	/// 置最大值
	/// </summary>
	/// <param name="ullMax">最大值</param>
	PNInline void SetMax(ULONGLONG ullMax) { m_ullMax = ullMax; }

	/// <summary>
	/// 取最大值
	/// </summary>
	/// <returns>返回最大值</returns>
	PNInline ULONGLONG GetMax() { return m_ullMax; }

	/// <summary>
	/// 启用临时进度
	/// </summary>
	/// <param name="b">是否启用</param>
	PNInline void EnableTempPosMark(BOOL b) { m_bShowTempMark = b; }

	/// <summary>
	/// 置临时进度位置
	/// </summary>
	/// <param name="ullPos">位置</param>
	PNInline void SetTempPosMark(ULONGLONG ullPos) { m_ullTempPos = ullPos; }

	/// <summary>
	/// 命中测试
	/// </summary>
	/// <param name="pt">相对背景的坐标</param>
	/// <param name="pullPos">返回进度条位置</param>
	/// <returns>测试成功返回非零，否则返回零</returns>
	int HitTest(POINT pt, ULONGLONG* pullPos);
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
	CUIInfo();
	~CUIInfo();
	void Redraw() override;
	BOOL InitElem() override;
};

class CUIButton :public CUIElem
{
protected:
	ID2D1SolidColorBrush* m_pBrHot = NULL;
	ID2D1SolidColorBrush* m_pBrPressed = NULL;
	ID2D1Bitmap1* m_pBmp = NULL;

	BOOL m_bHot = FALSE;
	BOOL m_bLBtnDown = FALSE;

	D2D1_RECT_F m_rcfImg{};
public:
	CUIButton();

	~CUIButton();

	void Redraw() override;

	CUI_DeclRedrawAndPresent;

	BOOL InitElem() override;

	BOOL OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;

	PNInline void SetImg(ID2D1Bitmap1* pBmp)
	{
		m_pBmp = pBmp;
	}

	PNInline void SetImgSize(float cx, float cy)
	{
		m_rcfImg.left = m_rcF.left + (m_cx - cx) / 2.f;
		m_rcfImg.top = m_rcF.top + (m_cy - cy) / 2.f;
		m_rcfImg.right = m_rcfImg.left + cx;
		m_rcfImg.bottom = m_rcfImg.top + cy;
	}

	/// <summary>
	/// 置颜色
	/// </summary>
	/// <param name="i">0 - 热点   1 - 按下</param>
	/// <param name="cr">颜色</param>
	PNInline void SetColor(int i, const D2D1_COLOR_F& cr)
	{
		if (i == 0)
			m_pBrHot->SetColor(cr);
		else if (i == 1)
			m_pBrPressed->SetColor(cr);
#ifdef _DEBUG
		else
			EckDbgBreak();
#endif
	}
};

class CUIRoundButton final :public CUIButton
{
private:
	ID2D1SolidColorBrush* m_pBrNormal = NULL;

	D2D1_ELLIPSE m_Ellipse{};
public:
	CUIRoundButton();

	~CUIRoundButton();

	void Redraw() override;

	BOOL OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;

	BOOL InitElem() override;

	/// <summary>
	/// 置颜色
	/// </summary>
	/// <param name="i">0 - 热点   1 - 按下   2 - 正常</param>
	/// <param name="cr">颜色</param>
	PNInline void SetColor(int i, const D2D1_COLOR_F& cr)
	{
		if (i == 2)
			m_pBrNormal->SetColor(cr);
		else
			CUIButton::SetColor(i, cr);
	}
};

class CUIPlayingCtrl final :public CUIElem
{
private:
	CUIButton* m_pBTOptions{};
	CUIButton* m_pBTPlayOpt{};
	CUIButton* m_pBTRepeatMode{};
	CUIButton* m_pBTPrev{};
	CUIRoundButton* m_pBTPlay{};
	CUIButton* m_pBTNext{};
	CUIButton* m_pBTStop{};
	CUIButton* m_pBTLrc{};
	CUIButton* m_pBTAbout{};
public:
	CUIPlayingCtrl();

	void Redraw() override;

	BOOL InitElem() override;

	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;

	BOOL OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};


// 工具条按钮状态
enum UITOOLBARBTNSTATE
{
	TBBS_NORMAL = 0x00000000,
	TBBS_CHECK = 0x00000001
};
// 工具条按钮索引
enum UITOOLBARBTNINDEX
{
	TBBI_INVALID = -1,

	TBBI_PREV = 0,
	TBBI_PLAY = 1,
	TBBI_STOP = 2,
	TBBI_NEXT = 3,
	TBBI_LRC = 4,
	TBBI_REPEATMODE = 5,
	TBBI_PLAYINGOPT = 6,
	TBBI_PLAYLIST = 7,
	TBBI_OPTIONS = 8,
	TBBI_ABOUT = 9,

	TBBI_COUNT = 10
};

#define BUFSIZE_TOOLBATTIME			48
#define BUFSIZE_TOOLBATTIMEMAXCH	47
// 工具条
class CUIToolBar final :public CUIElem
{
private:
	BOOL m_bLBtnDown = FALSE;		// 左键是否按下
	BOOL m_bInToolBar = FALSE;		// 光标是否在工具条内

	D2D1_RECT_F m_rcFTimeLabel = {};// 时间标签矩形
	RECT m_rcTimeLabel = {};		// 时间标签矩形
	WCHAR m_szTime[BUFSIZE_TOOLBATTIME] = { L"00:00/00:00" };// 时间标签内容
	UINT32 m_cchTime = 0;			// 时间标签文本长度

	UITOOLBARBTNINDEX m_idxHot = TBBI_INVALID;		// 热点索引
	UITOOLBARBTNINDEX m_idxLastHot = TBBI_INVALID;	// 上次热点索引
	UITOOLBARBTNINDEX m_idxPush = TBBI_INVALID;		// 按下索引
	UITOOLBARBTNINDEX m_idxLastHover = TBBI_INVALID;// 上次悬停索引

	UINT m_uBtnsCheckState = 0;		// 按钮检查状态，使用 1<<索引 设置位标志

	HWND m_hToolTip = NULL;			// 工具提示窗口句柄
	TTTOOLINFOW m_ti = {};			// 工具调试结构

	ID2D1SolidColorBrush* m_pBrText = NULL;			// 文本画刷
	ID2D1SolidColorBrush* m_pBrBtnHot = NULL;		// 热点背景画刷
	ID2D1SolidColorBrush* m_pBrBtnPushed = NULL;	// 按下背景画刷
	ID2D1SolidColorBrush* m_pBrBtnChecked = NULL;	// 检查背景画刷
	IDWriteTextFormat* m_pTfTime = NULL;
public:
	CUIToolBar();
	~CUIToolBar();
	BOOL OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void Redraw() override;
	void OnTimer(UINT uTimerID) override;
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;
	BOOL InitElem() override;
	CUI_DeclRedrawAndPresent;

	/// <summary>
	/// 重画时间标签
	/// </summary>
	void RedrawTimeInfo();

	/// <summary>
	/// 命中测试
	/// </summary>
	/// <param name="pt">测试点</param>
	/// <returns>按钮索引</returns>
	UITOOLBARBTNINDEX HitTest(POINT pt);

	/// <summary>
	/// 执行按钮动作
	/// </summary>
	/// <param name="idx">索引</param>
	void DoCmd(UITOOLBARBTNINDEX idx);
};