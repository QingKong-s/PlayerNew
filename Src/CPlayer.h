#pragma once
#include "CBass.h"
#include "CPlayList.h"
#include "Utils.h"

#include "eck\CRefStr.h"

#include <execution>

enum class PlayOpErr
{
	Ok,
	NoValidItem,
	LoopTerminated,
	NoCurrPlaying,
	BassError,
};

enum TICKCHANGING
{
	TKC_NONE = 0,
	TKC_LRCPOSUPDATED = (1u << 1),
};

PNInline constexpr TICKCHANGING operator|=(TICKCHANGING& x1, TICKCHANGING x2)
{
	return x1 = (TICKCHANGING)(x1 | x2);
}

// 播放控制类型
enum PLAYINGCTRLTYPE
{
	PCT_PLAYNEW,			// (idxOld, 0)
	PCT_PLAY_OR_PAUSE,
	PCT_STOP,				// (idxOld, 0)
	PCT_REMOVE_LATER_PLAY	// (idxLaterPlay, 0)
};


class CPlayer
{
public:
	using FOnPlayingControl = std::function<void(PLAYINGCTRLTYPE uType, INT_PTR i1, INT_PTR i2)>;
private:
	CBass m_Bass{};
	float m_fPos = 0.f;
	ULONGLONG m_ullPos = 0ull;
	ULONGLONG m_ullLength = 0ull;

	CPlayList m_List{};
	int m_idxCurrFile = -1;
	int m_idxCurrFileBeforeFirstSorting = -1;
	int m_idxLaterPlay = -1;
	int m_idxLaterPlayBeforeFirstSorting = -1;
	eck::CRefStrW m_rsCurrFile{};
	BOOL m_bHasActiveFile = FALSE;

	BOOL m_bSearching = FALSE;
	std::vector<int> m_vSearchingResult{};// 搜索结果，存储LV索引

	std::vector<Utils::LRCLABEL> m_vLrcLabel{};
	std::vector<Utils::LRCINFO> m_vLrc{};
	int m_idxCurrLrc = -1;
	int m_idxLastLrc = -1;

	Utils::MUSICINFO m_MusicInfo{};
	IWICBitmap* m_pWicCoverBmp = NULL;

	FOnPlayingControl m_fnPayingCtrl{};

	void ApplyPrevEffect()
	{

	}

	HRESULT CreateWicBmpCover();
public:
	~CPlayer();

	static constexpr PNInline PCWSTR GetErrMsg(PlayOpErr uErrCode)
	{
		constexpr PCWSTR c_szErr[]
		{
			L"操作成功。",
			L"没有有效的项目。",
			L"设定的循环已结束。",
			L"没有正在播放的项目。",
			L"Bass错误。",
		};
		return c_szErr[(int)uErrCode];
	}

	static PNInline PCWSTR GetErrMsgToShow(PlayOpErr uErrCode, int* piBassErr = NULL)
	{
		switch (uErrCode)
		{
		case PlayOpErr::Ok:
		case PlayOpErr::LoopTerminated:
			return NULL;
		case PlayOpErr::BassError:
		{
			const int iErr = CBass::GetError();
			if (piBassErr)
				*piBassErr = iErr;
			return CBass::GetErrorMsg(iErr);
		}
		default:
			return GetErrMsg(uErrCode);
		}
	}

	static void ShowPlayErr(HWND hWnd, PlayOpErr uErr);

	PlayOpErr Play(int idx);

	void Stop(BOOL bNoGap = FALSE);

	PlayOpErr Next();

	PlayOpErr Prev();

	PlayOpErr AutoNext();

	PlayOpErr PlayOrPause();

	/// <summary>
	/// 置稍后播放项
	/// </summary>
	/// <param name="idxItem">LV索引</param>
	PNInline void SetLaterPlaying(int idxItem) { m_idxLaterPlay = idxItem; }

	/// <summary>
	/// 取稍后播放项
	/// </summary>
	/// <returns>LV索引</returns>
	PNInline int GetLaterPlaying() const { return m_idxLaterPlay; }

	/// <summary>
	/// 插入项目
	/// </summary>
	/// <param name="idxPos">插入位置，LV索引</param>
	/// <param name="Info"></param>
	/// <param name="pszName"></param>
	/// <param name="pszTime"></param>
	/// <param name="pszFile"></param>
	/// <returns>新项的LV索引</returns>
	PNInline int Insert(int idxPos, const LISTFILEITEM_1& Info, PCWSTR pszName, PCWSTR pszTime, PCWSTR pszFile)
	{
		const int idx = m_List.Insert(idxPos, Info, pszName, pszTime, pszFile);
		if (m_idxCurrFile >= idx)
			++m_idxCurrFile;
		if (m_idxLaterPlay >= idx)
			++m_idxLaterPlay;
		return idx;
	}

	/// <summary>
	/// 删除项目
	/// </summary>
	/// <param name="idxItem">LV索引，-1则删除所有项目</param>
	/// <returns>剩余项目数</returns>
	PNInline int Delete(int idxItem)
	{
		const int cItems = m_List.Delete(idxItem);
		if (!cItems)
		{
			Stop();
			m_List.m_bSort = FALSE;
			m_idxLaterPlay = -1;
			return cItems;
		}

		if (idxItem == m_idxCurrFile)
			Stop();
		else if (idxItem < m_idxCurrFile)
			--m_idxCurrFile;

		if (idxItem == m_idxLaterPlay)
			m_idxLaterPlay = -1;
		else if (idxItem < m_idxLaterPlay)
			--m_idxLaterPlay;
		return cItems;
	}

	/// <summary>
	/// 排序
	/// </summary>
	/// <param name="uFlags">标志，PNSF_常量</param>
	/// <param name="idxBegin">范围起始LV索引</param>
	/// <param name="idxEnd">范围结束LV索引</param>
	void Sort(SortFlags uFlags, int idxBegin = -1, int idxEnd = -1);

	PNInline BOOL BeginSortProtect()
	{
		const BOOL b = m_List.IsSorting();
		m_List.m_bSort = FALSE;
		return b;
	}

	PNInline void EndSortProtect(BOOL bSort)
	{
		m_List.m_bSort = bSort;
		if (bSort)
			Sort(m_List.m_SortParam.uCurrSortFlags, m_List.m_SortParam.idxBegin, m_List.m_SortParam.idxEnd);
	}

	/// <summary>
	/// 恢复默认排序
	/// </summary>
	PNInline void CancelSort()
	{
		m_List.m_bSort = FALSE;
		if (m_idxCurrFileBeforeFirstSorting >= 0)
		{
			m_idxCurrFile = m_idxCurrFileBeforeFirstSorting;
			m_idxCurrFileBeforeFirstSorting = -1;
		}
		if (m_idxLaterPlayBeforeFirstSorting >= 0)
		{
			m_idxLaterPlay = m_idxLaterPlayBeforeFirstSorting;
			m_idxLaterPlayBeforeFirstSorting = -1;
		}
	}

	void Search(PCWSTR pszKeyWord);

	PNInline void CancelSearch()
	{
		m_vSearchingResult.clear();
		m_bSearching = FALSE;
	}

	PNInline int GetSearchingResultCount()
	{
		return (int)m_vSearchingResult.size();
	}

	PNInline PLAYLISTUNIT& AtSearching(int idx)
	{
		return m_List.At(m_vSearchingResult[idx]);
	}

	PNInline int AtSearchingIndex(int idx)
	{
		return m_vSearchingResult[idx];
	}

	PNInline BOOL IsSearching() { return m_bSearching; }

	PNInline const auto& GetSearchingResult() { return m_vSearchingResult; }

	/// <summary>
	/// 移动项目。
	/// 搜索或排序时不应交换项目，因此本函数使用LV索引
	/// </summary>
	/// <param name="idxDst">移动到的位置，将在此位置之前插入，-1表示插入到末尾</param>
	/// <param name="pidx">要移动的项目，按从小到大排列</param>
	/// <param name="cItems">要移动的项目数</param>
	/// <returns>第一个新项的索引</returns>
	int MoveItems(int idxDst, const int* pidx, int cItems);

	PNInline CBass& GetBass() { return m_Bass; }

	PNInline float GetPosF() const { return m_fPos; }

	PNInline ULONGLONG GetPos() const { return m_ullPos; }

	PNInline ULONGLONG GetLength() const { return m_ullLength; }

	/// <summary>
	/// 取现行播放项
	/// </summary>
	/// <returns>LV索引</returns>
	PNInline int GetCurrFile() const { return m_idxCurrFile; }

	PNInline const eck::CRefStrW& GetCurrFileName() const { return m_rsCurrFile; }

	PNInline int GetCurrLrc() const { return m_idxCurrLrc; }

	PNInline const auto& GetLrc() const { return m_vLrc; }

	PNInline const auto& GetLrcLabel() const { return m_vLrcLabel; }

	PNInline const auto& GetMusicInfo() const { return m_MusicInfo; }

	PNInline CPlayList& GetList() { return m_List; }

	PNInline BOOL IsFileActive() const { return m_bHasActiveFile; }

	PNInline IWICBitmap* GetWicBmpCover() const { return m_pWicCoverBmp; }

	PNInline void SetPlayingCtrlCallBack(FOnPlayingControl fn) { m_fnPayingCtrl = fn; }

	TICKCHANGING Tick();

	PNInline BOOL IsPlaying() { return GetBass().IsActive() == BASS_ACTIVE_PLAYING; }
};