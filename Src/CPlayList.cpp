#include "CPlayList.h"

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

	Item.bBookmark = Info.bBookmark;
	Item.bIgnore = Info.bIgnore;

	if (idxPos >= 0)
		m_vList.insert(m_vList.begin() + idxPos, std::move(Item));
	else
	{
		idxPos = (int)m_vList.size();
		m_vList.push_back(std::move(Item));
	}
	return idxPos;
}

void CPlayList::InsertBookmark(int idxItem, PCWSTR pszName, int cchName, COLORREF crColor)
{
	//EckAssert(!m_vList[idxItem].bBookmark);

	BOOKMAEKLISTUNIT Item;
	Item.rsName.ReSizeAbs(cchName);
	wcscpy(Item.rsName.Data(), pszName);

	Item.crColor = crColor;

	m_hmBookmark.insert(std::make_pair(idxItem, std::move(Item)));
	m_vList[idxItem].bBookmark = TRUE;
}

int CPlayList::Delete(int idxItem)
{
	if (idxItem < 0)
	{
		m_vList.clear();
		m_hmBookmark.clear();
		return 0;
	}
	EckAssert(idxItem < (int)m_vList.size());

	auto it = m_vList.begin() + idxItem;
	if (it->bBookmark)
		m_hmBookmark.erase(m_hmBookmark.find(idxItem));

	m_vList.erase(m_vList.begin() + idxItem);

	return (int)m_vList.size();
}

int CPlayList::DeleteBookmark(int idxItem)
{
	if (idxItem < 0)
	{
		m_hmBookmark.clear();
		return 0;
	}
	EckAssert(idxItem < (int)m_vList.size());

	auto it = m_vList.begin() + idxItem;
	EckAssert(it->bBookmark);
	it->bBookmark = FALSE;

	m_hmBookmark.erase(m_hmBookmark.find(idxItem));
	return 0;
}