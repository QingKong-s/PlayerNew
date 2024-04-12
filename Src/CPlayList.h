#pragma once
#include "CPlayListFile.h"

#include <algorithm>
#include <vector>
#include <unordered_map>

#include <Shlwapi.h>

struct PLAYLISTUNIT
{
	eck::CRefStrW rsName{};		// 名称
	eck::CRefStrW rsFile{};		// 文件路径

	eck::CRefStrW rsTitle{};	// 标题
	eck::CRefStrW rsArtist{};	// 艺术家
	eck::CRefStrW rsAlbum{};	// 唱片集
	eck::CRefStrW rsGenre{};	// 流派

	PLUPUREDATA s{};
	int idxSortMapping{ -1 };	// 【排序时用】映射到的项
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
	EckInline int Insert(int idxPos, const PLAYLISTUNIT& e)
	{
		if (idxPos < 0)
		{
			m_vList.emplace_back(e);
			idxPos = (int)m_vList.size() - 1;
		}
		else
			m_vList.emplace(m_vList.begin() + idxPos, e);
		if (e.rsName.IsEmpty())
			m_vList[idxPos].rsName = eck::GetFileNameFromPath(e.rsFile.Data(), e.rsFile.Size());
		return idxPos;
	}

	EckInline int Insert(int idxPos, PLAYLISTUNIT&& e)
	{
		if (idxPos < 0)
		{
			m_vList.emplace_back(std::move(e));
			idxPos = (int)m_vList.size() - 1;
		}
		else
			m_vList.emplace(m_vList.begin() + idxPos, std::move(e));
		if (m_vList[idxPos].rsName.IsEmpty())
			m_vList[idxPos].rsName = eck::GetFileNameFromPath(m_vList[idxPos].rsFile.Data(), m_vList[idxPos].rsFile.Size());
		return idxPos;
	}

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

	void UpdateItemInfo(int idx);
};