#include "pch.h"
#include "CPlayList.h"
#include "CBass.h"

//int CPlayList::Insert(int idxPos, const LISTFILEITEM_1& Info, PCWSTR pszName, PCWSTR pszTime, PCWSTR pszFile)
//{
//	PLAYLISTUNIT Item{};
//	if (pszName)
//	{
//		Item.rsName.DupString(pszName, Info.cchName);
//	}
//	else
//	{
//		const PWSTR pTemp = (PWSTR)_malloca((Info.cchFile + 1) * sizeof(WCHAR));
//		EckAssert(pTemp);
//		wcscpy(pTemp, pszFile);
//
//		const auto pszFileName = PathFindFileNameW(pTemp);
//		PathRemoveExtensionW(pszFileName);
//		Item.rsName.DupString(pszFileName);
//
//		_freea(pTemp);
//	}
//
//	if (pszTime)
//	{
//		Item.rsTime.DupString(pszTime, Info.cchTime);
//	}
//
//	Item.rsFile.DupString(pszFile, Info.cchFile);
//
//	Item.bBookmark = Info.bBookmark;
//	Item.bIgnore = Info.bIgnore;
//
//	if (idxPos >= 0)
//		m_vList.insert(m_vList.begin() + idxPos, std::move(Item));
//	else
//	{
//		idxPos = (int)m_vList.size();
//		m_vList.push_back(std::move(Item));
//	}
//	return idxPos;
//}

void CPlayList::InsertBookmark(int idxItem, PCWSTR pszName, int cchName, COLORREF crColor)
{
	BOOKMAEKLISTUNIT Item;
	Item.rsName.ReSize(cchName);
	wcscpy(Item.rsName.Data(), pszName);

	Item.crColor = crColor;

	m_hmBookmark.insert(std::make_pair(idxItem, std::move(Item)));
	m_vList[idxItem].s.bBookmark = TRUE;
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
	if (it->s.bBookmark)
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
	EckAssert(it->s.bBookmark);
	it->s.bBookmark = FALSE;

	m_hmBookmark.erase(m_hmBookmark.find(idxItem));
	return 0;
}

void CPlayList::UpdateItemInfo(int idx)
{
	auto& e = AtAbs(idx);
	if (e.s.bNeedUpdated)
	{
		e.s.bNeedUpdated = FALSE;

		Utils::MUSICINFO mi{};
		Utils::GetMusicInfo(e.rsFile.Data(), mi);
		e.rsTitle = std::move(mi.rsTitle);
		e.rsArtist = std::move(mi.rsArtist);
		e.rsAlbum = std::move(mi.rsAlbum);
		e.rsGenre = std::move(mi.rsGenre);
		e.s.usYear = mi.stDate.wYear;

		CBass Bass{};
		Bass.Open(e.rsFile.Data(), BASS_STREAM_DECODE, BASS_STREAM_DECODE, BASS_STREAM_DECODE);
		e.s.uSecTime = (UINT)Bass.GetLength();
	}
}

BOOL CPlayList::Save(PCWSTR pszFile)
{
	CPlayListFileWriter ListFile{};
	if (!ListFile.Open(pszFile))
		return FALSE;

	LISTFILEITEM_1 Info{};
	EckCounter(GetCount(), i)
	{
		auto& x = AtAbs(i);
		Info.cchName = x.rsName.Size();
		Info.cchFile = x.rsFile.Size();
		Info.cchTitle = x.rsTitle.Size();
		Info.cchArtist = x.rsArtist.Size();
		Info.cchAlbum = x.rsAlbum.Size();
		Info.cchGenre = x.rsGenre.Size();

		Info.s = x.s;
		ListFile.PushBack(Info, x.rsName.Data(), x.rsFile.Data(),
			x.rsTitle.Data(), x.rsArtist.Data(), x.rsAlbum.Data(), x.rsGenre.Data());
	}
	ListFile.BeginBookMark();
	BOOKMARKITEM BmInfo{};
	for (const auto& x : GetBookmark())
	{
		BmInfo.idxItem = x.first;
		BmInfo.cr = x.second.crColor;
		BmInfo.cchName = x.second.rsName.Size();
		ListFile.PushBackBookmark(BmInfo, x.second.rsName.Data());
	}
	ListFile.End();
	return TRUE;
}

BOOL CPlayList::Load(PCWSTR pszFile)
{
	CPlayListFileReader ListFile{};
	if (!ListFile.Open(pszFile))
		return FALSE;

	m_ListInfo.rsCreator = ListFile.GetCreatorName();
	SetFileName(pszFile);

	Delete(-1);
	Reserve(ListFile.GetItemCount());

	ListFile.For([this](const LISTFILEITEM_1* pItem,
		PCWSTR pszName, PCWSTR pszFile, PCWSTR pszTitle,
		PCWSTR pszArtist, PCWSTR pszAlbum, PCWSTR pszGenre)->BOOL
		{
			Insert(-1,
				PLAYLISTUNIT{ pszName,pszFile,pszTitle,pszArtist,pszAlbum,pszGenre,pItem->s });
			return TRUE;
		});
	ListFile.ForBookmark([this](const BOOKMARKITEM* pItem, PCWSTR pszName)->BOOL
		{
			InsertBookmark(pItem->idxItem, pszName, pItem->cchName, pItem->cr);
			return TRUE;
		});
	return TRUE;
}