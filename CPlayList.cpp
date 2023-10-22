#include "CPlayList.h"

void CPlayList::SortInternal(int idxBegin, int idxEnd, const FSortProc& fnProc)
{
	EckAssert(idxBegin >= 0 && idxBegin < (int)m_PlayList.size());
	EckAssert(idxEnd >= 0 && idxEnd <= (int)m_PlayList.size());
	EckAssert(idxBegin <= idxEnd);

	int cItems = idxEnd - idxBegin + 1;
	if (cItems <= 1)
		return;

	int* pidxSortMapping = (int*)_malloca((idxEnd - idxBegin + 1) * sizeof(int));
	EckAssert(pidxSortMapping);
	for (int i = idxBegin; i <= idxEnd; ++i)
		pidxSortMapping[i - idxBegin] = (m_PlayList[i].idxSortMapping < 0 ? i : m_PlayList[i].idxSortMapping);

	std::sort(pidxSortMapping, pidxSortMapping + cItems, [&fnProc = fnProc, this](int idx1, int idx2)->bool
		{
			return fnProc(m_PlayList[idx1], m_PlayList[idx2]);
		});

	for (int i = idxBegin; i <= idxEnd; ++i)
	{
		m_PlayList[i].idxSortMapping = pidxSortMapping[i - idxBegin];
	}

	_freea(pidxSortMapping);
}

int CPlayList::Insert(int idxPos, const LISTFILEITEM_1& Info, PCWSTR pszName, PCWSTR pszTime, PCWSTR pszFile)
{
	PLAYLISTUNIT Item;
	if (pszName)
	{
		Item.rsName.ReSizeAbs(Info.cchName);
		wcsncpy(Item.rsName.Data(), pszName, Info.cchName);
	}
	else
	{
		PWSTR pTemp = (PWSTR)_malloca((Info.cchFile + 1) * sizeof(WCHAR));
		EckAssert(pTemp);

		wcscpy(pTemp, pszFile);

		auto pszFileName = PathFindFileNameW(pTemp);
		PathRemoveExtensionW(pszFileName);

		int cchName = (int)wcslen(pszFileName);
		Item.rsName.ReSizeAbs(cchName);
		wcscpy(Item.rsName.Data(), pszFileName);

		_freea(pTemp);
	}

	if (pszTime)
	{
		Item.rsTime.ReSizeAbs(Info.cchTime);
		wcsncpy(Item.rsTime.Data(), pszTime, Info.cchTime);
	}

	Item.rsFile.ReSizeAbs(Info.cchFile);
	wcsncpy(Item.rsFile.Data(), pszFile, Info.cchFile);

	Item.bDelayPlaying = Info.bDelayPlaying;
	Item.bBookmark = Info.bBookmark;
	Item.bIgnore = Info.bIgnore;

	if (idxPos >= 0)
		m_PlayList.insert(m_PlayList.begin() + idxPos, std::move(Item));
	else
	{
		idxPos = (int)m_PlayList.size();
		m_PlayList.push_back(std::move(Item));
	}
	return idxPos;
}

void CPlayList::InsertBookmark(int idxItem, PCWSTR pszName, int cchName, COLORREF crColor)
{
	EckAssert(m_PlayList[idxItem].bBookmark);

	BOOKMAEKLISTUNIT Item;
	Item.rsName.ReSizeAbs(cchName);
	wcscpy(Item.rsName.Data(), pszName);

	Item.crColor = crColor;

	m_BookmarkList.insert(std::make_pair(idxItem, std::move(Item)));
}

int CPlayList::Delete(int idxItem)
{
	if (idxItem < 0)
	{
		m_PlayList.clear();
		return 0;
	}

	EckAssert(idxItem < (int)m_PlayList.size());

	auto it = m_PlayList.begin() + idxItem;
	if (it->bBookmark)
		m_BookmarkList.erase(m_BookmarkList.find(idxItem));

	m_PlayList.erase(m_PlayList.begin() + idxItem);

	return (int)m_PlayList.size();
}

int CPlayList::DeleteBookmark(int idxItem)
{
	if (idxItem < 0)
	{
		m_BookmarkList.clear();
		return 0;
	}
	EckAssert(idxItem < (int)m_PlayList.size());

	auto it = m_PlayList.begin() + idxItem;
	EckAssert(it->bBookmark);
	it->bBookmark = FALSE;

	m_BookmarkList.erase(m_BookmarkList.find(idxItem));
	return 0;
}

int CPlayList::Sort(SortFlags uFlags, int idxBegin, int idxEnd)
{
	if(m_PlayList.empty())
		return 0;
	PFSortProc fnSort[]
	{
		[](const PLAYLISTUNIT& Item1, const PLAYLISTUNIT& Item2)->bool
		{
			return Item1.rsName < Item2.rsName;
		},
		[](const PLAYLISTUNIT& Item1, const PLAYLISTUNIT& Item2)->bool
		{
			return Item1.rsTime < Item2.rsTime;
		},
		[](const PLAYLISTUNIT& Item1, const PLAYLISTUNIT& Item2)->bool
		{
			return Item1.rsFile < Item2.rsFile;
		}
	};

	if (idxBegin < 0 || idxEnd < 0)
	{
		idxBegin = 0;
		idxEnd = (int)m_PlayList.size() - 1;
	}

	SortInternal(idxBegin, idxEnd, fnSort[SF2FNIDX(uFlags)]);
}
