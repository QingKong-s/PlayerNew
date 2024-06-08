#pragma once
#include "CApp.h"
#include "CPlayList.h"
#include "CLVPlay.h"
#include "CDlgMusicInfo.h"

#include "eck\CLabel.h"
#include "eck\CEditExt.h"
#include "eck\CButton.h"
#include "eck\CToolBar.h"

#include <shlobj_core.h>

constexpr inline auto WCN_LIST = L"PlayerNew.WndClass.List";

#pragma pack(push, 4)
constexpr inline int LDPH_VER_1 = 0;
struct LISTDRAGPARAMHEADER
{
	int iVer;
	DWORD dwPID;
	int cItems;
	// int idxLV[];
};
#pragma pack(pop)

enum// 工具条按钮ID
{
	TBCID_BEGIN = 200,
	TBCID_LOCATE = TBCID_BEGIN,
	TBCID_ADD,
	TBCID_LOADLIST,
	TBCID_SAVELIST,
	TBCID_EMPTY,
	TBCID_MANAGE,

	TBBTCOUNT = 6
};

class CWndMain;
class CWndList : public eck::CForm
{
	friend class CDropTargetList;
	friend class CWndMain;
private:
	eck::CLabel m_LAListName{};
	eck::CEditExt m_EDSearch{};
	eck::CPushButton m_BTSearch{};
	eck::CToolBar m_TBManage{};
	CLVSearch m_LVSearch{*this};
	CLVPlay m_LVList{ *this,m_LVSearch };

	HIMAGELIST m_hIml = NULL;

	HFONT m_hFont = NULL;
	HFONT m_hFontListName = NULL;

	HMENU m_hMenuAdd = NULL;
	HMENU m_hMenuLV = NULL;
	HMENU m_hMenuManage = NULL;

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;

	HICON m_hiSearch = NULL;

	eck::CRefStrW m_rsCurrKeyword{};

	CWndMain& m_WndMain;

	CDlgMusicInfo* m_pDlgMusicInfo{ new CDlgMusicInfo{} };


	enum// 控件ID
	{
		IDC_LA_LIST_NAME = 100,
		IDC_ED_SEARCH,
		IDC_BT_SEARCH,
		IDC_LV_LIST,
		IDC_LV_SEARCHING,
		IDC_TB_MANAGE
	};

	enum// 菜单ID
	{
		IDMI_ADDFILE = 300,
		IDMI_ADDDIR,

		IDMI_PLAY,
		IDMI_DELETE_FROM_LIST,
		IDMI_DELETE_FROM_DISK,
		IDMI_OPEN_IN_EXPLORER,
		IDMI_RENAME,
		IDMI_INFO,
		IDMI_ADDBOOKMARK,
		IDMI_NEXTBOOKMARK,
		IDMI_PREVBOOKMARK,
		IDMI_IGNORE,
		IDMI_PLAYLATER,

		IDMI_SORT_DEF,
		IDMI_SORT_FILENAME,
		IDMI_SORT_NAME,
		IDMI_SORT_CREATE_TIME,
		IDMI_SORT_MODIFY_TIME,
		IDMI_SORT_REVERSE,
		IDMI_SORT_ASCENDING,
		IDMI_SORT_DESCENDING,
		IDMI_FIXSORT,
		
		IDMI_DETAIL,
		IDMI_BOOKMARK_MGR,
	};

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(iEdgePadding, 4)
		ECK_DS_ENTRY(cyListName, 24)
		ECK_DS_ENTRY(cySearch, 30)
		ECK_DS_ENTRY(cyTool, 32)
		ECK_DS_ENTRY(cxToolBtn, 60)
		ECK_DS_ENTRY(iGap, 4)
		ECK_DS_ENTRY(cxLVTextSpace, 4)
		ECK_DS_ENTRY(cyLVItem, c_cyLVItem)
		ECK_DS_ENTRY(cxColumn1, 280)
		ECK_DS_ENTRY(cxColumn2, 60)
		;
	ECK_DS_END_VAR(m_Ds);

	PNInline void UpdateDpiInit(int iDpi)
	{
		m_iDpi = iDpi;
		eck::UpdateDpiSize(m_Ds, iDpi);

		const int
			cxIcon = eck::DpiScale(c_cxBtnIcon, iDpi),
			cyIcon = eck::DpiScale(c_cyBtnIcon, iDpi);
		HIMAGELIST hIml = ImageList_Create(cxIcon, cyIcon, ILC_COLOR32, 0, 2);
		HICON hIcon;
		IWICBitmap* pBmp;

		constexpr static int c_idxIcon[]
		{
			IIDX_Locate,
			IIDX_Plus,
			IIDX_LoadFile,
			IIDX_Disk,
			IIDX_Cross,
			IIDX_ListManage
		};

		for (auto idx : c_idxIcon)
		{
			pBmp = App->ScaleImageForButton(idx, iDpi);
			hIcon = eck::CreateHICON(pBmp);
			ImageList_AddIcon(hIml, hIcon);
			pBmp->Release();
			DestroyIcon(hIcon);
		}

		m_TBManage.SetImageList(hIml);
		std::swap(m_hIml, hIml);
		ImageList_Destroy(hIml);

		m_TBManage.SetButtonSize(m_Ds.cxToolBtn, m_Ds.cyTool);
	}

	void UpdateDpi(int iDpi);

	void UpdateUISize();

	BOOL AutoSaveList();

	void ListSwitched();

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnSize(HWND hWnd, UINT state, int cx, int cy);

	void OnDestroy(HWND hWnd);

	void OnCmdLocate();

	void OnMenuAddFile();
	
	void OnMenuAddDir();

	void OnCmdLoadList();

	void OnCmdSaveList();

	void OnCmdEmpty();

	void OnCmdManage();

	void OnListLVNDbLClick(NMITEMACTIVATE* pnmia);

	BOOL OnListLVNRClick(NMITEMACTIVATE* pnmia);

	void OnMenuOpenInExplorer();

	void OnMenuDel(BOOL bDelFile);

	void OnMenuAddDelBookmark(int idx);

	void OnListLVNBeginDrag(NMLISTVIEW* pnmlv);

	void OnSearchLVNDbLClick(NMITEMACTIVATE* pnmia);

	BOOL OnSearchLVNRClick(NMITEMACTIVATE* pnmia);

	void OnSearchLVNBeginDrag(NMLISTVIEW* pnmlv);

	void OnENChange();

	BOOL OnListLVNEndLabelEdit(NMLVDISPINFOW* pnmlvdi);

	BOOL OnSearchLVNEndLabelEdit(NMLVDISPINFOW* pnmlvdi);
public:
	CWndList(CWndMain& Main) :m_WndMain(Main) {}

	~CWndList() { delete m_pDlgMusicInfo; }

	static ATOM RegisterWndClass();

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WCN_LIST, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, App->GetHInstance(), NULL);
	}

	void PlayListItem(int idx);

	void EnsureVisibleBookmark(int idx);

	PNInline const auto& GetDs() const { return m_Ds; }

	PNInline const auto& GetCurrSearchKeyword() const { return m_rsCurrKeyword; }
};