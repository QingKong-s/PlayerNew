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

class CPlayer
{
private:
	CBass m_Bass{};
	float m_fPos = 0.f;
	ULONGLONG m_ullPos = 0ull;
	ULONGLONG m_ullLength = 0ull;

	CPlayList m_List{};
	int m_idxCurrFile = -1;
	int m_idxCurrFileBeforeFirstSorting = -1;
	int m_idxLaterPlay = -1;
	eck::CRefStrW m_rsCurrFile{};
	BOOL m_bHasActiveFile = FALSE;

	std::vector<Utils::LRCLABEL> m_vLrcLabel{};
	std::vector<Utils::LRCINFO> m_vLrc{};
	int m_idxCurrLrc = -1;

	Utils::MUSICINFO m_MusicInfo{};

	void ApplyPrevEffect()
	{

	}
public:
	PlayOpErr Play(int idx);

	void Stop();

	PlayOpErr Next();

	PlayOpErr Prev();

	PlayOpErr AutoNext();

	PNInline void SetLaterPlaying(int idxItem) { m_idxLaterPlay = idxItem; }

	PNInline int GetLaterPlaying() const { return m_idxLaterPlay; }

	PNInline int Insert(int idxPos, const LISTFILEITEM_1& Info, PCWSTR pszName, PCWSTR pszTime, PCWSTR pszFile)
	{
		const int idx = m_List.Insert(idxPos, Info, pszName, pszTime, pszFile);
		if (m_idxCurrFile >= idx)
			++m_idxCurrFile;
		if (m_idxLaterPlay >= idx)
			++m_idxLaterPlay;
		return idx;
	}

	PNInline int Delete(int idxItem)
	{
		const int cItems = m_List.Delete(idxItem);
		if (!cItems)
		{
			Stop();
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

	void Sort(SortFlags uFlags, int idxBegin = -1, int idxEnd = -1);

	PNInline void CancelSort()
	{
		m_List.m_bSort = FALSE;
		if (m_idxCurrFileBeforeFirstSorting >= 0)
		{
			m_idxCurrFile = m_idxCurrFileBeforeFirstSorting;
			m_idxCurrFileBeforeFirstSorting = -1;
		}
	}

	PNInline CBass& GetBass() { return m_Bass; }

	PNInline float GetPosF() const { return m_fPos; }

	PNInline ULONGLONG GetPos() const { return m_ullPos; }

	PNInline ULONGLONG GetLength() const { return m_ullLength; }

	PNInline int GetCurrFile() const { return m_idxCurrFile; }

	PNInline const eck::CRefStrW GetCurrFileName() const { return m_rsCurrFile; }

	PNInline int GetCurrLrc() const { return m_idxCurrLrc; }

	PNInline const auto& GetLrc() const { return m_vLrc; }

	PNInline const auto& GetLrcLabel() const { return m_vLrcLabel; }

	PNInline const auto& GetMusicInfo() const { return m_MusicInfo; }

	PNInline CPlayList& GetList() { return m_List; }

	PNInline BOOL IsFileActive() const { return m_bHasActiveFile; }
};