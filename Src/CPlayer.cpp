﻿#include "CPlayer.h"
#include "CApp.h"
#include "COptionsMgr.h"
#include "CWndBK.h"

HRESULT CPlayer::CreateWicBmpCover()
{
	SAFE_RELEASE(m_pWicCoverBmp);
	HRESULT hr = CApp::WICCreateBitmap(m_MusicInfo.pCoverData, &m_pWicCoverBmp);
	if (FAILED(hr))
	{
		hr = CApp::WICCreateBitmap((eck::GetRunningPath() + LR"(\Img\DefCover.png)").Data(), &m_pWicCoverBmp);
	}
	return hr;
}

CPlayer::~CPlayer()
{
	SAFE_RELEASE(m_pWicCoverBmp);
}

PlayOpErr CPlayer::Play(int idx)
{
	EckAssert(idx >= 0 && idx < m_List.GetCount());
	Stop();
	auto& Item = m_List.At(idx);

	m_rsCurrFile = Item.rsFile;
	m_idxCurrFile = idx;
	Utils::GetMusicInfo(m_rsCurrFile.Data(), m_MusicInfo);
	CreateWicBmpCover();

	m_Bass.Open(Item.rsFile.Data());
	if (!m_Bass.GetHStream())
		return PlayOpErr::BassError;
	m_bHasActiveFile = TRUE;
	m_Bass.TempoCreate();
	m_Bass.Play();
	ApplyPrevEffect();

	m_ullLength = (ULONGLONG)(m_Bass.GetLength() * 1000.);

	Utils::ParseLrc(m_rsCurrFile.Data(), 0u, m_vLrc, m_vLrcLabel, COptionsMgr::GetInst().iLrcFileEncoding);
	if (!m_vLrc.size())
		Utils::ParseLrc(m_MusicInfo.rsLrc.Data(), m_MusicInfo.rsLrc.ByteSize(), m_vLrc, m_vLrcLabel, Utils::LrcEncoding::UTF16LE);
	m_pWndBK->OnPlayingControl(PCT_PLAY);
	return PlayOpErr::Ok;
}

void CPlayer::Stop(BOOL bNoGap)
{
	m_Bass.Stop();
	m_Bass.Close();

	m_idxCurrFile = -1;
	m_idxCurrLrc = -1;

	if (!bNoGap)
	{
		m_bHasActiveFile = FALSE;
		m_pWndBK->OnPlayingControl(PCT_STOP);
	}
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

	m_List.m_SortParam.uCurrSortFlags = uFlags;
	m_List.m_SortParam.idxBegin = idxBegin;
	m_List.m_SortParam.idxEnd = idxEnd;
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
	{
		m_idxCurrFileBeforeFirstSorting = m_idxCurrFile;
		m_idxLaterPlayBeforeFirstSorting = m_idxLaterPlay;
	}

	int idxRealCurr = -1;
	if (m_idxCurrFile >= 0)
		if (m_List.IsSorting())
			idxRealCurr = m_List.At(m_idxCurrFile).idxSortMapping;
		else
			idxRealCurr = m_idxCurrFile;
	int idxRealLaterPlay = -1;
	if (m_idxLaterPlay >= 0)
		if (m_List.IsSorting())
			idxRealLaterPlay = m_List.At(m_idxLaterPlay).idxSortMapping;
		else
			idxRealLaterPlay = m_idxLaterPlay;

	const int cElem = idxEnd - idxBegin + 1;
	int* pidxSortMapping = (int*)_malloca(cElem * sizeof(int));
	EckAssert(pidxSortMapping);
	for (int i = idxBegin; i <= idxEnd; ++i)
		pidxSortMapping[i - idxBegin] = i;

	std::sort(std::execution::par_unseq, pidxSortMapping, pidxSortMapping + cItems, pfnSort[CPlayList::Sf2FnIdx(uFlags)]);

	using ItList = std::vector<PLAYLISTUNIT>::iterator;

	ItList itBegin = vList.begin() + idxBegin;
	ItList itEnd = ((idxEnd == (int)vList.size() - 1) ? vList.end() : vList.begin() + idxEnd);

	for (int i = idxBegin; i <= idxEnd; ++i)
		vList[i].idxSortMapping = pidxSortMapping[i - idxBegin];

	if (idxRealCurr >= 0)
	{
		auto pNewPos = std::find(std::execution::par_unseq, pidxSortMapping, pidxSortMapping + cElem, idxRealCurr);
		if (pNewPos != pidxSortMapping + cElem)
			m_idxCurrFile = (int)(pNewPos - pidxSortMapping);
	}
	if (idxRealLaterPlay >= 0)
	{
		auto pNewPos = std::find(std::execution::par_unseq, pidxSortMapping, pidxSortMapping + cElem, idxRealLaterPlay);
		if (pNewPos != pidxSortMapping + cElem)
			m_idxLaterPlay = (int)(pNewPos - pidxSortMapping);
	}

	_freea(pidxSortMapping);
	m_List.m_bSort = TRUE;
}

void CPlayer::Search(PCWSTR pszKeyWord)
{
	m_vSearchingResult.clear();
	m_bSearching = TRUE;
	const int cItems = m_List.GetCount();
	EckCounter(cItems, i)
	{
		auto& Item = m_List.At(i);
		if (wcsstr(Item.rsName.Data(), pszKeyWord))
			m_vSearchingResult.emplace_back(i);
	}
}

int CPlayer::MoveItems(int idxDst, const int* pidx, int cItems)
{
	auto& vItems = m_List.GetList();
	vItems.reserve(vItems.size() + cItems);

	int m = 0, n = 0;
	BOOL b[2]{};
	if (idxDst < 0 || idxDst >= (int)vItems.size())
	{
		idxDst = (int)vItems.size();
		EckCounter(cItems, i)
		{
			int idx = pidx[i];
			if (b[0])
				--m_idxCurrFile;
			else if (idx == m_idxCurrFile)
			{
				m_idxCurrFile = idxDst - 1;
				b[0] = TRUE;
			}
			if (b[1])
				--m_idxLaterPlay;
			else if (idx == m_idxLaterPlay)
			{
				m_idxLaterPlay = idxDst - 1;
				b[1] = TRUE;
			}
			idx -= n;
			vItems.emplace_back(std::move(vItems[idx]));
			vItems.erase(vItems.begin() + idx);
			++n;
		}
	}
	else
	{
		EckCounter(cItems, i)
		{
			int idx = pidx[i];
			if (idx >= idxDst)
			{
				if (idx == m_idxCurrFile)
					m_idxCurrFile = idxDst + m;
				if (idx == m_idxLaterPlay)
					m_idxLaterPlay = idxDst + m;

				vItems.insert(vItems.begin() + idxDst + m, std::move(vItems[idx]));
				vItems.erase(vItems.begin() + idx + 1);
				++m;
			}
			else
			{
				if (b[0])
					--m_idxCurrFile;
				else if (idx == m_idxCurrFile)
				{
					m_idxCurrFile = idxDst - 1;
					b[0] = TRUE;
				}
				if (b[1])
					--m_idxLaterPlay;
				else if (idx == m_idxLaterPlay)
				{
					m_idxLaterPlay = idxDst - 1;
					b[1] = TRUE;
				}
				idx -= n;
				vItems.insert(vItems.begin() + idxDst, std::move(vItems[idx]));
				vItems.erase(vItems.begin() + idx);
				++n;
			}
		}
	}
	return idxDst - n;
}

TICKCHANGING CPlayer::Tick()
{
	TICKCHANGING uRet = TKC_NONE;
	m_fPos = (float)m_Bass.GetPosition();
	m_ullPos = (ULONGLONG)(m_fPos * 1000.f);

	const int cLrc = (int)m_vLrc.size();
	if (cLrc)
	{
		if (m_idxCurrLrc >= 0)
		{
			if (m_idxCurrLrc + 1 < cLrc)
			{
				if (m_fPos >= m_vLrc[m_idxCurrLrc].fTime && m_fPos < m_vLrc[m_idxCurrLrc + 1].fTime)
					goto EndFindLrc;
			}
			else if (m_fPos >= m_vLrc[m_idxCurrLrc].fTime)
				goto EndFindLrc;
		}
		auto it = std::lower_bound(m_vLrc.begin(), m_vLrc.end(), m_fPos, [](const Utils::LRCINFO& Item, float fPos)->bool
			{
				return Item.fTime < fPos;
			});
		if (it == m_vLrc.end())
		{
			if (m_fPos < m_vLrc.front().fTime)
				m_idxCurrLrc = -1;
			else
				m_idxCurrLrc = -2;
		}
		else
			m_idxCurrLrc = (int)std::distance(m_vLrc.begin(), it);
		
		if (m_idxCurrLrc != m_idxLastLrc)
		{
			m_idxLastLrc = m_idxCurrLrc;
			uRet |= TKC_LRCPOSUPDATED;
		}
	}
EndFindLrc:

	return uRet;
}