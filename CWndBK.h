#pragma once
#include "CApp.h"
#include "COptionsMgr.h"

#include "eck\CWnd.h"

#include <thread>

constexpr inline auto WCN_MAINBK = L"PlayerNew.WndClass.MainBK";

constexpr inline auto c_szWaveEventName = L"PlayerNew.Event.WaveEvent";

// UIԪ������
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
	UIET_INFO
};

// UIԪ�ر�־
enum UIELEMFLAGS
{
	UIEF_NONE = 0x00000000,// ��
	UIEF_STATIC = 0x00000001,// ��̬Ԫ��
	UIEF_NOEVENT = 0x00000002,// ����ת���¼�
	UIEF_WANTTIMEREVENT = 0x00000004 // ��Ҫ��ʱ�������¼�
};

// UIԪ���¼�
enum UIELEMEVENT
{
	UIEE_SETRECT = 1,	// (&rc,0)		���ν�������
	UIEE_ONPLAYINGCTRL = 2,	// (type,0)		���ſ��Ʋ����ѷ���
	UIEE_ONSETTINGSCHANGE = 3,	// (0,0)		�����ѱ�����
};

// ���ſ�������
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

class CUIElem;
class CWndBK :public eck::CWnd
{
	friend class CUIElem;
	friend class CUIAlbum;
	friend class CUIAlbumRotating;
	friend class CUIWaves;
	friend class CUISpe;
	friend class CUISpe2;
	friend class CUILrc;
	friend class CUIProgBar;
	friend class CUIToolBar;
	friend class CUIInfo;
private:
	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBmpBK = NULL;
	ID2D1Bitmap1* m_pBmpBKStatic = NULL;// ��̬���������������
	ID2D1Bitmap1* m_pBmpAlbum = NULL;// ����ԭʼλͼ
	ID2D1SolidColorBrush* m_pBrWhite = NULL;
	ID2D1SolidColorBrush* m_pBrWhite2 = NULL;

	int m_cxAlbum = 0;
	int m_cyAlbum = 0;

	int m_cxClient = 0;
	int m_cyClient = 0;

	D2D1_RECT_F m_rcfClient{};

	std::vector<CUIElem*> m_vElems{};
	std::vector<CUIElem*> m_vElemsWantTimer{};

	std::vector<RECT> m_vDirtyRect{};

	ULONGLONG m_ullMusicPos = 0ull;
	ULONGLONG m_ullMusicLength = 0ull;

	enum
	{
		IDT_PGS = 101,
	};

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(sizeAlbumLevel, 15)
		ECK_DS_ENTRY(cxWaveLine, 2)
		ECK_DS_ENTRY(cxSepLine, 1)
		ECK_DS_ENTRY(cyProgBarTrack, 6)
		ECK_DS_ENTRY(cxTime, 100)
		ECK_DS_ENTRY(cxTopTip, 50)
		ECK_DS_ENTRY(cyTopTip, 18)
		ECK_DS_ENTRY(cyTopTitle, 30)
		ECK_DS_ENTRY(sizeTopTipGap, 6)
		;
	ECK_DS_END_VAR(m_Ds);


	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnSize(HWND hWnd, UINT state, int cx, int cy);

	/// <summary>
	/// ����Ԫ���¼�
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
	/// ���Ԫ��
	/// </summary>
	/// <param name="pElem">Ԫ�ض���</param>
	void AddElem(CUIElem* pElem);

	/// <summary>
	/// �¸�����ʼ�����
	/// </summary>
	void OnPlayingControl(PLAYINGCTRLTYPE uType);

	/// <summary>
	/// ���ñ��ı�
	/// </summary>
	void OnSettingsChanged() { GenElemEvent(UIEE_ONSETTINGSCHANGE, 0, 0); }

	/// <summary>
	/// �ػ�
	/// </summary>
	void Redraw();

	/// <summary>
	/// ���¾�̬λͼ
	/// </summary>
	void UpdateStaticBmp();

};



#ifndef NDEBUG
#if 1
// �������á���Ԫ�ر߿�
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
// �������á���Ԫ�ر߿�
// Releaseģʽ���ѱ�����Ϊ�պ�
#define BkDbg_DrawElemFrame()
#endif // !NDEBUG

// ��UIԪ������������ִ��������ʾ���ػ淽������ҪRedraw����
#define CUI_DeclRedrawAndPresent \
	PNInline void Redraw(BOOL bImmdShow) override \
	{ \
		auto pDC = m_pBK->m_pDC; \
		pDC->BeginDraw(); \
		Redraw(); \
		pDC->EndDraw(); \
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

#define BUFSIZE_UIDIRTYRCGROW	10



// UIԪ����
class CUIElem
{
protected:
	UINT m_uType = UIET_INVALID;
	UINT m_uFlags = UIEF_NONE;

	D2D1_RECT_F		m_rcF = {};		// Ԫ�ؾ���
	RECT			m_rc = {};		// Ԫ�ؾ���
	RECT			m_rcInWnd = {};		// Ԫ�ؾ����봰�ھ��εĽ���
	int				m_cxHalf = 0,		// Ԫ�ؿ�ȵ�һ��
		m_cyHalf = 0;		// Ԫ�ظ߶ȵ�һ��
	int				m_cx = 0,		// Ԫ�ؿ��
		m_cy = 0;		// Ԫ�ظ߶�


	BOOL			m_bShow = TRUE;
	BOOL			m_bRedraw = TRUE;
	CWndBK*			m_pBK = NULL;
	int				m_iZOrder = -1;		// ��0��ʼ��Z��

	int				m_idxDirtyRect = -1;
	BOOL			m_bHasFoucs = FALSE;

	/// <summary>
	/// Ĭ�����Ը��Ĵ������
	/// </summary>
	/// <param name="uEvent"></param>
	/// <param name="wParam"></param>
	/// <param name="lParam"></param>
	/// <returns></returns>
	LRESULT DefElemEventProc(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam);
public:
	friend class CUIBK;

	virtual ~CUIElem();

	/// <summary>
	/// ͨ�÷�����֪ͨԪ���¼�
	/// </summary>
	/// <param name="uEvent">�¼���ʶ��</param>
	/// <param name="wParam"></param>
	/// <param name="lParam"></param>
	/// <returns></returns>
	virtual LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
	{
		return DefElemEventProc(uEvent, wParam, lParam);
	}

	/// <summary>
	/// ͨ�÷�������������Ϣ
	/// </summary>
	/// <param name="uMsg"></param>
	/// <param name="wParam"></param>
	/// <param name="lParam"></param>
	/// <returns></returns>
	virtual LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) { return 0; }

	/// <summary>
	/// ͨ�÷�������ʱ�������¼�
	/// </summary>
	/// <param name="uTimerID">��ʱ��ID</param>
	virtual void OnTimer(UINT uTimerID) {}

	/// <summary>
	/// ͨ�÷������ػ�
	/// </summary>
	virtual void Redraw() {}

	/// <summary>
	/// ͨ�÷������ػ���
	/// ��������BeginDraw/EndDraw
	/// </summary>
	/// <param name="bImmdShow">�Ƿ�������ʾ</param>
	virtual void Redraw(BOOL bImmdShow) {}

	/// <summary>
	/// ͨ�÷�������ʼ��Ԫ����Ϣ
	/// </summary>
	/// <returns>��ʼ���ɹ�����TRUE�����򷵻�FALSE</returns>
	virtual BOOL InitElem() { return TRUE; }

	/// <summary>
	/// ��Ԫ�ؾ���
	/// </summary>
	/// <param name="rc">����</param>
	PNInline void SetElemRect(RECT rc) { OnElemEvent(UIEE_SETRECT, (WPARAM)&rc, 0); }

	/// <summary>
	/// ȡԪ�ؾ���
	/// </summary>
	/// <param name="prc">RECTָ��</param>
	/// <returns></returns>
	PNInline void GetElemRect(RECT* prc) { *prc = m_rc; }

	/// <summary>
	/// Ԫ���Ƿ�ɼ�
	/// </summary>
	/// <returns>�ɼ���</returns>
	PNInline BOOL IsElemVisible() { return m_bShow; }

	/// <summary>
	/// ��Ԫ�ؿɼ���
	/// </summary>
	/// <param name="bShow">�Ƿ�ɼ�</param>
	PNInline void ShowElem(BOOL bShow) { m_bShow = bShow; }

	PNInline void SetZOrder(int iZOrder) { m_iZOrder = iZOrder; }

	PNInline int GetZOrder() { return  m_iZOrder; }

	PNInline void SetRedraw(BOOL b) { m_bRedraw = b; }

	/// <summary>
	/// ���Ӱ��Ԫ�ء�
	/// ��Ԫ�ص���ʾ���ݽ���ͶӰ��Ӱ��Ԫ��
	/// </summary>
	/// <param name="pElem">Ԫ��</param>
	void AddShadow(CUIElem* pElem);
};

// ����
class CUIAlbum final :public CUIElem
{
public:
	CUIAlbum();
	void Redraw() override;
};

// ��ת����
class CUIAlbumRotating final :public CUIElem
{
private:
	float m_fAngle = 0.f;// ��ǰ��ת�Ƕ�

	ID2D1BitmapBrush1* m_pBrAlbum = NULL;// ����λͼ��ˢ
	ID2D1SolidColorBrush* m_pBrUV = NULL;// ��ƽ��ˢ
	ID2D1SolidColorBrush* m_pBrUV2 = NULL;// ��ƽ������ˢ
public:
	CUIAlbumRotating();
	~CUIAlbumRotating();
	void Redraw() override;
	void OnTimer(UINT uTimerID) override;
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;
	BOOL InitElem() override;
	CUI_DeclRedrawAndPresent;

	/// <summary>
	/// ����λͼ��ˢ
	/// </summary>
	void UpdateAlbumBrush();
};

// ����
class CUIWaves final :public CUIElem
{
private:
	int m_cxLine = 0;// �߿�

	ID2D1SolidColorBrush* m_pBrLine = NULL;// �߻�ˢ
	ID2D1SolidColorBrush* m_pBrCenterMark = NULL;// ���߻�ˢ
	IDWriteTextFormat* m_pTfTip = NULL;

	ThreadState m_ThreadState = ThreadState::Stopped;// �߳�״̬
	HANDLE m_hThread = NULL;
	std::vector<DWORD> m_vWavesData{};

	void GetWavesData();
public:
	CUIWaves();
	~CUIWaves();
	void Redraw() override;
	BOOL InitElem() override;
	void OnTimer(UINT uTimerID) override;

	/// <summary>
	/// �û����߿�
	/// </summary>
	/// <param name="i">���</param>
	PNInline void SetLineWidth(int i) { m_cxLine = i; }

	/// <summary>
	/// ȡ�����߿�
	/// </summary>
	/// <returns>�����߿�</returns>
	PNInline int GetLineWidth() { return m_cxLine; }
};

// Ƶ��
class CUISpe final :public CUIElem
{
private:
	int m_iCount = 0;		// Ƶ��������
	float m_cxBar = 0.f;		// Ƶ�������
	float m_cxGap = 0.f;		// ������


	int* m_piOldHeight = NULL;		// �ɸ߶���Ϣ
	int* m_piHeight = NULL;		// �߶�
	int* m_piOldMaxPos = NULL;		// �ɷ�ֵ
	int* m_piTime = NULL;		// ��ʱ

	int* m_pBaseData = NULL;		// ��������ָ��
	SIZE_T m_cbDataUnit = 0;		// �������ܴ�С

	ID2D1SolidColorBrush* m_pBrBar = NULL;// Ƶ������ˢ
public:
	CUISpe();
	~CUISpe();
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;
	void Redraw() override;
	BOOL InitElem() override;
	void OnTimer(UINT uTimerID) override;

	/// <summary>
	/// ��Ƶ��������
	/// </summary>
	/// <param name="i">����</param>
	void SetCount(int i);

	/// <summary>
	/// ȡƵ������Ŀ
	/// </summary>
	/// <returns>������Ŀ</returns>
	PNInline int GetCount() { return m_iCount; }

	/// <summary>
	/// �ü�����
	/// </summary>
	/// <param name="i">���</param>
	PNInline void SetGapWidth(int i)
	{
		if (i < 0)
			i = 0;
		m_cxGap = i;
	}

	/// <summary>
	/// ȡ������
	/// </summary>
	/// <returns>���ؼ�����</returns>
	PNInline int GetGapWidth() { return m_cxGap; }
};
// Ƶ��2
class CUISpe2 :public CUIElem
{
private:
	int m_cSample = 0;		// ������
	float* m_pfBuf = NULL;		// ����������
	SIZE_T m_cbBuf = 0;		// ��������С
	float m_cxStep = 0.f;		// ������ʾ���

	ID2D1SolidColorBrush* m_pBrLine = NULL;
public:
	CUISpe2();
	~CUISpe2();
	void Redraw() override;
	BOOL InitElem() override;
	void OnTimer(UINT uTimerID) override;
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;

	/// <summary>
	/// ��������
	/// </summary>
	/// <param name="i">������</param>
	void SetSampleCount(int i);

	/// <summary>
	/// ȡ������
	/// </summary>
	/// <returns>����������</returns>
	PNInline int GetSampleCount() { return m_cSample; }
};

struct LRCITEMHEIGHTDATA
{
	float cy;
};
// �������
class CUILrc :public CUIElem
{
private:
	ID2D1SolidColorBrush* m_pBrTextNormal = NULL;
	ID2D1SolidColorBrush* m_pBrTextHighlight = NULL;

	IDWriteTextFormat* m_pTextFormat = NULL;

	int		m_idxCenter = -1;// �м���
	int		m_idxCurr = -1;	// ��ǰ���
	int		m_idxMouseHover = -1;	// ��ǰ�ȵ���
	int		m_idxLastMouseHover = -1;	// �ϴ��ȵ���
	int		m_idxLastHighlight = -1;	// �ϴθ������
	float	m_yCenterItem = 0.f;	// �м��ˮƽ���ߵĶ���

	int		m_idxTopItem = 0;	// ��һ��ɼ����
	float	m_fOffsetTopIndex = 0.f;	// ��һ��ɼ���ʵ��ڵ��߶�

	//LRCHSCROLLINFO  m_LrcHScrollInfo{ -1 };	// ���ˮƽ������Ϣ���������ڵ�ǰ������
	//LRCVSCROLLINFO  m_LrcVScrollInfo{};		// ��ֱ����������Ϣ
	BOOL	m_bVAnOngoing = FALSE;// ��ֱ���������Ƿ��ڽ��У��˳�ԱΪTRUEʱ���ܽ����ڸ��������ִ���������ֽ������Ĳ������������һЩ��ֵ����⡣������

	int		m_iDrawingID = 0;

	ULONGLONG m_ullLastDrawingTime = 0ull;

	//int             m_iLastLrcIndex[2] = { -1,-1 };// 0���ϴ����ģ�1���ϴθ���

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
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void Redraw() override;
	CUI_DeclRedrawAndPresent;
	void OnTimer(UINT uTimerID) override;
	BOOL InitElem() override;
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;

	/// <summary>
	/// ��m_idxTopItem�ʼ���»滭���и��
	/// </summary>
	void DrawAllLrc_Top();

	/// <summary>
	/// �Ե�ǰ���Ϊ�м�һ��滭���и�ʣ��м����λ����m_yCenterItem����
	/// </summary>
	void DrawAllLrc_Center();

	/// <summary>
	/// ��������
	/// </summary>
	/// <param name="bImmdShow">�Ƿ�������ʾ</param>
	void RedrawSB(BOOL bImmdShow);

	/// <summary>
	/// ��һ����
	/// </summary>
	/// <param name="iIndex">����</param>
	/// <param name="y">�߶ȣ���Ҫʹ���ϴθ߶ȣ�����ΪINFINITY</param>
	/// <param name="bTop">ָ��"y"�����Ƿ�Ϊ���ߣ�TRUEΪ���ߣ�FALSEΪ�ױߣ���y==INFINITY����˲���������ΪTRUE</param>
	/// <param name="bClearBK"></param>
	/// <param name="bImmdShow">�Ƿ�������ʾ</param>
	/// <returns>�߶�</returns>
	float RedrawItem(int iIndex, float y, BOOL bTop, BOOL bClearBK, BOOL bImmdShow, BOOL bCenterLine = FALSE, int* yOut = NULL);

	/// <summary>
	/// ���в���
	/// </summary>
	/// <param name="pt">���Ե�</param>
	/// <returns>�ɹ����ظ��������ʧ�ܷ���-1</returns>
	int HitTest(POINT pt);

	/// <summary>
	/// ��ʼ����ֱ�л�����
	/// </summary>
	void InitVAnimation();

	void InitAnimation(int idxPrevCenter, int idxNewCenter);
};
// ������
class CUIProgBar :public CUIElem
{
private:
	ULONGLONG m_ullPos = 0ull;		// λ��
	ULONGLONG m_ullMax = 0ull;		// ���λ��

	ULONGLONG m_ullTempPos = 0ull;		// ��ʱλ��
	BOOL m_bShowTempMark = FALSE;	// �Ƿ�������ʱλ��

	BOOL m_bLBtnDown = FALSE;	// ����Ƿ���

	ID2D1SolidColorBrush* m_pBrNormal = NULL;// ���Ȼ�ˢ
	ID2D1SolidColorBrush* m_pBrBK = NULL;// ������ˢ
	ID2D1SolidColorBrush* m_pBrTempMark = NULL;// ��ʱ���Ȼ�ˢ
public:
	CUIProgBar();
	~CUIProgBar();
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void Redraw() override;
	BOOL InitElem() override;
	void OnTimer(UINT uTimerID) override;
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;
	CUI_DeclRedrawAndPresent;

	/// <summary>
	/// ��λ��
	/// </summary>
	/// <param name="ullPos">λ��</param>
	PNInline void SetPos(ULONGLONG ullPos)
	{
		if (ullPos > m_ullMax)
			ullPos = m_ullMax;
		m_ullPos = ullPos;
	}

	/// <summary>
	/// ȡλ��
	/// </summary>
	/// <returns>����λ��</returns>
	PNInline ULONGLONG GetPos() { return m_ullPos; }

	/// <summary>
	/// �����ֵ
	/// </summary>
	/// <param name="ullMax">���ֵ</param>
	PNInline void SetMax(ULONGLONG ullMax) { m_ullMax = ullMax; }

	/// <summary>
	/// ȡ���ֵ
	/// </summary>
	/// <returns>�������ֵ</returns>
	PNInline ULONGLONG GetMax() { return m_ullMax; }

	/// <summary>
	/// ������ʱ����
	/// </summary>
	/// <param name="b">�Ƿ�����</param>
	PNInline void EnableTempPosMark(BOOL b) { m_bShowTempMark = b; }

	/// <summary>
	/// ����ʱ����λ��
	/// </summary>
	/// <param name="ullPos">λ��</param>
	PNInline void SetTempPosMark(ULONGLONG ullPos) { m_ullTempPos = ullPos; }

	/// <summary>
	/// ���в���
	/// </summary>
	/// <param name="pt">��Ա���������</param>
	/// <param name="pullPos">���ؽ�����λ��</param>
	/// <returns>���Գɹ����ط��㣬���򷵻���</returns>
	int HitTest(POINT pt, ULONGLONG* pullPos);
};
// ��������ť״̬
enum UITOOLBARBTNSTATE
{
	TBBS_NORMAL = 0x00000000,
	TBBS_CHECK = 0x00000001
};
// ��������ť����
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
// ������
class CUIToolBar :public CUIElem
{
private:
	BOOL m_bLBtnDown = FALSE;	// ����Ƿ���
	BOOL m_bInToolBar = FALSE;	// ����Ƿ��ڹ�������

	D2D1_RECT_F m_rcFTimeLabel = {};	// ʱ���ǩ����
	RECT m_rcTimeLabel = {};	// ʱ���ǩ����
	WCHAR m_szTime[BUFSIZE_TOOLBATTIME] = { L"00:00/00:00" };// ʱ���ǩ����
	UINT32 m_cchTime = 0;		// ʱ���ǩ�ı�����

	UITOOLBARBTNINDEX m_idxHot = TBBI_INVALID;// �ȵ�����
	UITOOLBARBTNINDEX m_idxLastHot = TBBI_INVALID;// �ϴ��ȵ�����
	UITOOLBARBTNINDEX m_idxPush = TBBI_INVALID;// ��������
	UITOOLBARBTNINDEX m_idxLastHover = TBBI_INVALID;// �ϴ���ͣ����

	UINT m_uBtnsCheckState = 0;		// ��ť���״̬��ʹ�� 1<<���� ����λ��־

	HWND m_hToolTip = NULL;		// ������ʾ���ھ��
	TTTOOLINFOW m_ti = {};		// ���ߵ��Խṹ

	ID2D1SolidColorBrush* m_pBrText = NULL;// �ı���ˢ
	ID2D1SolidColorBrush* m_pBrBtnHot = NULL;// �ȵ㱳����ˢ
	ID2D1SolidColorBrush* m_pBrBtnPushed = NULL;// ���±�����ˢ
	ID2D1SolidColorBrush* m_pBrBtnChecked = NULL;// ��鱳����ˢ
	IDWriteTextFormat* m_pTfTime = NULL;
public:
	CUIToolBar();
	~CUIToolBar();
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void Redraw() override;
	void OnTimer(UINT uTimerID) override;
	LRESULT OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam) override;
	BOOL InitElem() override;
	CUI_DeclRedrawAndPresent;

	/// <summary>
	/// �ػ�ʱ���ǩ
	/// </summary>
	void RedrawTimeInfo();

	/// <summary>
	/// ���в���
	/// </summary>
	/// <param name="pt">���Ե�</param>
	/// <returns>��ť����</returns>
	UITOOLBARBTNINDEX HitTest(POINT pt);

	/// <summary>
	/// ִ�а�ť����
	/// </summary>
	/// <param name="idx">����</param>
	void DoCmd(UITOOLBARBTNINDEX idx);
};
// ������Ϣ
class CUIInfo :public CUIElem
{
private:
	ID2D1SolidColorBrush* m_pBrBigTip = NULL;
	ID2D1SolidColorBrush* m_pBrSmallTip = NULL;
	IDWriteTextFormat* m_pTfTitle = NULL;
	IDWriteTextFormat* m_pTfTip = NULL;

	void UpdateTF();
public:
	CUIInfo();
	~CUIInfo();
	void Redraw() override;
	BOOL InitElem() override;
};