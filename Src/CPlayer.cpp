#include "CPlayer.h"
#include "CApp.h"
#include "COptionsMgr.h"

PlayOpErr CPlayer::Play(int idx)
{
	EckAssert(idx >= 0 && idx < m_List.GetCount());
	Stop();
	auto& Item = m_List.At(idx);

	m_Bass.Open(Item.rsFile.Data());
	if (!m_Bass.GetHStream())
		return PlayOpErr::BassError;
	m_rsCurrFile = Item.rsFile;
	m_idxCurrFile = idx;
	m_Bass.TempoCreate();
	m_Bass.Play();
	ApplyPrevEffect();

	m_ullLength = (ULONGLONG)(m_Bass.GetLength() * 1000.);

	Utils::GetMusicInfo(m_rsCurrFile.Data(), m_MusicInfo);
	Utils::ParseLrc(m_rsCurrFile.Data(), 0u, m_vLrc, m_vLrcLabel, COptionsMgr::GetInst().iLrcFileEncoding);
	if (!m_vLrc.size())
		Utils::ParseLrc(m_MusicInfo.rsLrc.Data(), m_MusicInfo.rsLrc.ByteSize(), m_vLrc, m_vLrcLabel, Utils::LrcEncoding::UTF16LE);
	return PlayOpErr::Ok;
}

void CPlayer::Stop()
{
	m_Bass.Stop();
	m_Bass.Close();

	m_idxCurrFile = -1;
	m_idxCurrLrc = -1;
}

PlayOpErr CPlayer::Next()
{
	if (m_idxCurrFile < 0)
		return PlayOpErr::NoCurrPlaying;

	if (m_idxLaterPlay >= 0)
	{
		Play(m_idxLaterPlay);
		m_idxLaterPlay = -1;
		return PlayOpErr::Ok;
	}

	int idx = m_idxCurrFile;
	const int cItems = m_List.GetCount();
	EckCounter(cItems, i)
	{
		++idx;
		if (idx >= cItems)
			idx = 0;

		if (!m_List.At(idx).bIgnore)
		{
			Play(idx);
			return PlayOpErr::Ok;
		}
	}

	return PlayOpErr::NoValidItem;
}

PlayOpErr CPlayer::Prev()
{
	if (m_idxCurrFile < 0)
		return PlayOpErr::NoCurrPlaying;

	int idx = m_idxCurrFile;
	const int cItems = m_List.GetCount();
	EckCounter(cItems, i)
	{
		--idx;
		if (idx < 0)
			idx = cItems - 1;

		if (!m_List.At(idx).bIgnore)
		{
			Play(idx);
			return PlayOpErr::Ok;
		}
	}

	return PlayOpErr::NoValidItem;
}

PlayOpErr CPlayer::AutoNext()
{
	switch (COptionsMgr::GetInst().iRepeatMode)
	{
	case RepeatMode::AllLoop:
		return Next();
	case RepeatMode::All:
		if (m_idxCurrFile == m_List.GetCount() - 1)
		{
			Stop();
			return PlayOpErr::LoopTerminated;
		}
		else
			return Next();
	case RepeatMode::Radom:
		break;
	case RepeatMode::SingleLoop:
		m_Bass.Play(TRUE);
		return PlayOpErr::Ok;
	case RepeatMode::Single:
		Stop();
		return PlayOpErr::LoopTerminated;
	}
	return PlayOpErr::Ok;
}

void CPlayer::Sort(SortFlags uFlags, int idxBegin, int idxEnd)
{
	auto& vList = m_List.m_vList;
	const CPlayList::TSortProc pfnSort[]
	{
		[&vList](int idx1, int idx2)->bool
		{
			return StrCmpLogicalW(vList[idx1].rsName.Data() , vList[idx2].rsName.Data()) < 0;
		},
		[&vList](int idx1, int idx2)->bool
		{
			return StrCmpLogicalW(vList[idx1].rsTime.Data() , vList[idx2].rsTime.Data()) < 0;
		},
		[&vList](int idx1, int idx2)->bool
		{
			return StrCmpLogicalW(vList[idx1].rsFile.Data() , vList[idx2].rsFile.Data()) < 0;
		}
	};

	if (idxBegin < 0 || idxEnd < 0)
	{
		idxBegin = 0;
		idxEnd = (int)vList.size() - 1;
	}

	EckAssert(idxBegin >= 0 && idxBegin < (int)vList.size());
	EckAssert(idxEnd >= 0 && idxEnd <= (int)vList.size());
	EckAssert(idxBegin <= idxEnd);

	int cItems = idxEnd - idxBegin + 1;
	if (cItems <= 1)
		return;
	if (!m_List.m_bSort)
		m_idxCurrFileBeforeFirstSorting = m_idxCurrFile;

	int idxRealCurr = -1;
	if (m_idxCurrFile >= 0)
		if (m_List.IsSorting())
			idxRealCurr = m_List.At(m_idxCurrFile).idxSortMapping;
		else
			idxRealCurr = m_idxCurrFile;

	const int cElem = idxEnd - idxBegin + 1;
	int* pidxSortMapping = (int*)_malloca(cElem * sizeof(int));
	EckAssert(pidxSortMapping);
	for (int i = idxBegin; i <= idxEnd; ++i)
		pidxSortMapping[i - idxBegin] = (vList[i].idxSortMapping < 0 ? i : vList[i].idxSortMapping);

	std::sort(pidxSortMapping, pidxSortMapping + cItems, pfnSort[CPlayList::Sf2FnIdx(uFlags)]);

	using ItList = std::vector<PLAYLISTUNIT>::iterator;

	ItList itBegin = vList.begin() + idxBegin;
	ItList itEnd = ((idxEnd == (int)vList.size() - 1) ? vList.end() : vList.begin() + idxEnd);

	for (int i = idxBegin; i <= idxEnd; ++i)
		vList[i].idxSortMapping = pidxSortMapping[i - idxBegin];

	auto pNewPos = std::find(std::execution::par_unseq, pidxSortMapping, pidxSortMapping + cElem, idxRealCurr);
	if (pNewPos != pidxSortMapping + cElem)
		m_idxCurrFile = (int)(pNewPos - pidxSortMapping);

	_freea(pidxSortMapping);
	m_List.m_bSort = TRUE;
}