#pragma once
#include "CPlayListFile.h"

#include <unordered_set>
#include <algorithm>
#include <vector>

#include <Shlwapi.h>

struct PLAYLISTUNIT
{
	eck::CRefStrW rsName;
	eck::CRefStrW rsTime;
	eck::CRefStrW rsFile;

	int idxSortMapping;

	BITBOOL bIgnore : 1;
	BITBOOL bDelayPlaying : 1;
	BITBOOL bBookmark : 1;
};

struct BOOKMAEKLISTUNIT
{
	eck::CRefStrW rsName;
	COLORREF crColor;
};

class CPlayList
{
public:
	using FSortProc = std::function<bool(const PLAYLISTUNIT& Item1, const PLAYLISTUNIT& Item2)>;
	using PFSortProc = bool(*)(const PLAYLISTUNIT& Item1, const PLAYLISTUNIT& Item2);

	constexpr static int SF_IDXPART = 17;
	enum SortFlags :UINT
	{
		// µÍ16Î»
		SF_NAME = 1,
		SF_TIME = 2,
		SF_FILE = 3,
		SF_REVERSE = 4,
		// ¸ß16Î»
		SF_ASCENDING = 1 << SF_IDXPART,
		SF_DESCENDING = 1 << (SF_IDXPART + 1),
		SF_DEFAULT = 1 << (SF_IDXPART + 2),
	};
private:
	std::vector<PLAYLISTUNIT> m_PlayList;
	std::unordered_map<int, BOOKMAEKLISTUNIT> m_BookmarkList;

	void SortInternal(int idxBegin, int idxEnd, const FSortProc& fnProc);

	static PNInline size_t SF2FNIDX(SortFlags u)
	{
		return LOWORD(u) - 1;
	}
public:

	int Insert(int idxPos, PCWSTR pszName, int cchName, PCWSTR pszTime, int cchTime, PCWSTR pszFile, int cchFile);

	void InsertBookmark(int idxItem, PCWSTR pszName, int cchName, COLORREF crColor);

	int Delete(int idxItem);

	int DeleteBookmark(int idxItem);

	int Sort(SortFlags uFlags, int idxBegin = -1, int idxEnd = -1);
};

