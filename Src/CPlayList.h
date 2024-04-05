#pragma once
#include "CPlayListFile.h"

#include <algorithm>
#include <vector>
#include <unordered_map>

#include <Shlwapi.h>

struct PLAYLISTUNIT
{
	eck::CRefStrW rsName;
	eck::CRefStrW rsTime;
	eck::CRefStrW rsFile;

	int idxSortMapping;

	BITBOOL bIgnore : 1;
	BITBOOL bBookmark : 1;
};

struct BOOKMAEKLISTUNIT
{
	eck::CRefStrW rsName;
	COLORREF crColor;
};

constexpr inline int SF_IDXPART = 17;
enum SortFlags :UINT
{
	PNSF_NONE = 0,
	// 低16位
	PNSF_NAME = 1,
	PNSF_TIME = 2,
	PNSF_FILE = 3,
	PNSF_REVERSE = 4,
	PNSF_CREATE_TIME = 5,
	PNSF_MODIFY_TIME = 6,
	// 高16位
	PNSF_ASCENDING = 1 << SF_IDXPART,
	PNSF_DESCENDING = 1 << (SF_IDXPART + 1),
	PNSF_DEFAULT = 1 << (SF_IDXPART + 2),
};

class CPlayList
{
	friend class CPlayer;
public:
	using TSortProc = std::function<bool(int idx1, int idx2)>;
private:
	std::vector<PLAYLISTUNIT> m_vList;
	std::unordered_map<int, BOOKMAEKLISTUNIT> m_hmBookmark;// 实际索引->书签项目

	BOOL m_bSort = FALSE;
	struct
	{
		SortFlags uCurrSortFlags = PNSF_NONE;
		int idxBegin = -1;
		int idxEnd = -1;
	}
	m_SortParam{};

	static PNInline size_t Sf2FnIdx(SortFlags u)
	{
		return LOWORD(u) - 1;
	}
public:
	int Insert(int idxPos, const LISTFILEITEM_1& Info, PCWSTR pszName, PCWSTR pszTime, PCWSTR pszFile);

	void InsertBookmark(int idxItem, PCWSTR pszName, int cchName, COLORREF crColor);

	int Delete(int idxItem);

	int DeleteBookmark(int idxItem);

	void Reserve(int cItems)
	{
		m_vList.reserve(cItems);
	}

	PNInline PLAYLISTUNIT& At(int idx)
	{
		if (m_bSort)
			return m_vList.at(m_vList.at(idx).idxSortMapping);
		else
			return m_vList.at(idx);
	}

	PNInline PLAYLISTUNIT& AtAbs(int idx)
	{
		return m_vList.at(idx);
	}

	PNInline int AbsIndex(int idx)
	{
		if (m_bSort)
			return At(idx).idxSortMapping;
		else
			return idx;
	}

	PNInline BOOKMAEKLISTUNIT& AtBookmark(int idx)
	{
		if (m_bSort)
			return m_hmBookmark.at(m_vList.at(idx).idxSortMapping);
		else
			return m_hmBookmark.at(idx);
	}

	PNInline int GetCount() const
	{
		return (int)m_vList.size();
	}

	PNInline int GetBookmarkCount() const
	{
		return (int)m_hmBookmark.size();
	}

	PNInline auto& GetList() { return m_vList; }

	PNInline auto& GetBookmark() { return m_hmBookmark; }

	PNInline BOOL IsSorting() { return m_bSort; }
};

