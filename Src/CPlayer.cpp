﻿#include "pch.h"

#include "CDlgProgress.h"

HRESULT CPlayer::CreateWicBmpCover()
{
	HRESULT hr = E_FAIL;
	SafeRelease(m_pWicCoverBmp);
	const auto p = m_MusicInfo.GetMainCover();
	if (p)
	{
		if (p->bLink)
			hr = CApp::WICCreateBitmap(std::get<1>(p->varPic).Data(), &m_pWicCoverBmp);
		else
		{
			auto pStream = new eck::CStreamView(std::get<0>(p->varPic));
			hr = CApp::WICCreateBitmap(pStream, &m_pWicCoverBmp);
			pStream->Release();
		}
	}
	if (FAILED(hr))
		hr = CApp::WICCreateBitmap((eck::GetRunningPath() + LR"(\Img\DefCover.png)").Data(), &m_pWicCoverBmp);
	return hr;
}

CPlayer::~CPlayer()
{
	SafeRelease(m_pWicCoverBmp);
}

void CPlayer::ShowPlayErr(HWND hWnd, PlayOpErr uErr)
{
	int iBassErr;
	auto pszErrMsg = CPlayer::GetErrMsgToShow(uErr, &iBassErr);
	std::wstring sDetail;
	if (iBassErr != BASS_OK)
	{
		sDetail = std::format(L"{}\nBass错误代码：0x{:08X}。", pszErrMsg, iBassErr);
		pszErrMsg = sDetail.c_str();
	}
	CApp::ShowError(hWnd, (DWORD)uErr, CApp::ErrSrc::None, L"播放出错", pszErrMsg);
}

PlayOpErr CPlayer::Play(int idx)
{
	EckAssert(idx >= 0 && idx < m_List.GetCount());
	Stop();
	auto& Item = m_List.At(idx);

	m_Bass.Open(Item.rsFile.Data());
	if (!m_Bass.GetHStream())
		return PlayOpErr::BassError;
	m_Bass.SetSync(BASS_SYNC_END | BASS_SYNC_ONETIME, 0,
		[](DWORD, DWORD, DWORD, PVOID)
		{
			App->GetMainWnd()->PostMsg(PNWM_CHANNELENDED, 0, 0);
		});
	m_bHasActiveFile = TRUE;
	m_rsCurrFile = Item.rsFile;
	const auto idxPrev = m_idxCurrFile;
	m_idxCurrFile = idx;
	m_Bass.TempoCreate();
	m_Bass.Play(TRUE);
	EckAssert(!m_ullCurrTickBegin);
	m_ullCurrTickBegin = GetTickCount64();
	ApplyPrevEffect();

	m_ullLength = (ULONGLONG)(m_Bass.GetLength() * 1000.);

	Utils::GetMusicInfo(m_rsCurrFile.Data(), m_MusicInfo);

	if (!m_MusicInfo.rsTitle.IsEmpty())
		m_Stat.IncPlayCount(m_MusicInfo.rsTitle);
	m_vArtist.clear();
	if (!m_MusicInfo.rsArtist.IsEmpty())
	{
		eck::SplitStrWithMultiChar(m_MusicInfo.rsArtist.Data(), c_szArtistSplitter, m_vArtist);
		for (auto& e : m_vArtist)
			e.RLTrim();
		m_Stat.IncPlayCount(m_vArtist);
	}
	++Item.s.cPlayed;
	SYSTEMTIME st;
	GetLocalTime(&st);
	eck::SystemTimeToULongLong(st, Item.s.ftLastPlayed);

	CreateWicBmpCover();
	m_vLrc.clear();
	Utils::ParseLrc(m_rsCurrFile.Data(), 0u, m_vLrc, m_vLrcLabel,
		App->GetOptionsMgr().LrcFileEncoding, (float)m_Bass.GetLength());
	if (!m_vLrc.size())
		Utils::ParseLrc(m_MusicInfo.rsLrc.Data(), m_MusicInfo.rsLrc.ByteSize(), m_vLrc, m_vLrcLabel,
			Utils::LrcEncoding::UTF16LE, (float)m_Bass.GetLength());
	m_fnPayingCtrl(PCT_PLAYNEW, idxPrev, 0);
	return PlayOpErr::Ok;
}

void CPlayer::Stop(BOOL bNoGap)
{
	if (m_idxCurrFile >= 0)
	{
		EckAssert(IsPlaying() ? m_ullCurrTickBegin : !m_ullCurrTickBegin);
		if (m_ullCurrTickBegin)
		{
			const auto uDelta = (UINT)(GetTickCount64() - m_ullCurrTickBegin) / 1000u;
			m_List.At(m_idxCurrFile).s.uSecPlayed += uDelta;
			m_Stat.IncPlayTime(m_MusicInfo.rsTitle, uDelta);
			m_Stat.IncPlayTime(m_vArtist, uDelta);
		}
	}
	m_ullCurrTickBegin = 0ull;
	m_Bass.Stop();
	m_Bass.Close();

	const auto idxOld = m_idxCurrFile;
	m_idxCurrFile = -1;
	m_idxCurrLrc = -1;

	if (!bNoGap)
	{
		m_bHasActiveFile = FALSE;
		m_fnPayingCtrl(PCT_STOP, idxOld, 0);
	}
}

PlayOpErr CPlayer::Next()
{
	if (m_idxCurrFile < 0)
		return PlayOpErr::NoCurrPlaying;

	if (m_idxLaterPlay >= 0)
	{
		Play(m_idxLaterPlay);
		const auto idxOld = m_idxLaterPlay;
		m_idxLaterPlay = -1;
		m_fnPayingCtrl(PCT_REMOVE_LATER_PLAY, idxOld, 0);
		return PlayOpErr::Ok;
	}

	int idx = m_idxCurrFile;
	const int cItems = m_List.GetCount();
	EckCounter(cItems, i)
	{
		++idx;
		if (idx >= cItems)
			idx = 0;

		if (!m_List.At(idx).s.bIgnore)
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

		if (!m_List.At(idx).s.bIgnore)
		{
			Play(idx);
			return PlayOpErr::Ok;
		}
	}

	return PlayOpErr::NoValidItem;
}

PlayOpErr CPlayer::AutoNext()
{
	switch (App->GetOptionsMgr().PlayRepeatMode)
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
	{
		auto& e = m_List.At(m_idxCurrFile);
		++e.s.cLoop;
		m_Stat.IncLoopCount(m_MusicInfo.rsTitle);
		m_Stat.IncLoopCount(m_vArtist);
		m_Bass.Play(TRUE);
	}
	return PlayOpErr::Ok;
	case RepeatMode::Single:
		Stop();
		return PlayOpErr::LoopTerminated;
	}
	return PlayOpErr::Ok;
}

PlayOpErr CPlayer::PlayOrPause()
{
	auto iAct = m_Bass.IsActive();
	BOOL b = FALSE;
	switch (iAct)
	{
	case BASS_ACTIVE_PAUSED:
	{
		b = m_Bass.Play();
		EckAssert(!m_ullCurrTickBegin);
		m_ullCurrTickBegin = GetTickCount64();
	}
	break;
	case BASS_ACTIVE_PLAYING:
	{
		b = m_Bass.Pause();
		EckAssert(m_ullCurrTickBegin);
		const auto uDelta = (UINT)(GetTickCount64() - m_ullCurrTickBegin) / 1000u;
		m_List.At(m_idxCurrFile).s.uSecPlayed += uDelta;
		m_Stat.IncPlayTime(m_MusicInfo.rsTitle, uDelta);
		m_Stat.IncPlayTime(m_vArtist, uDelta);
		m_ullCurrTickBegin = 0ull;
	}
	break;
	default:
		return PlayOpErr::NoCurrPlaying;
	}
	m_fnPayingCtrl(PCT_PLAY_OR_PAUSE, 0, 0);
	if (b)
		return PlayOpErr::Ok;
	else
		return PlayOpErr::BassError;
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
		//[&vList](int idx1, int idx2)->bool
		//{
		//	return StrCmpLogicalW(vList[idx1].rsTime.Data() , vList[idx2].rsTime.Data()) < 0;
		//},
		[&vList](int idx1, int idx2)->bool
		{
			return StrCmpLogicalW(vList[idx1].rsFile.Data() , vList[idx2].rsFile.Data()) < 0;
		},
		{},

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

	int* pidxSortMapping = (int*)_malloca(cItems * sizeof(int));
	EckCheckMem(pidxSortMapping);
	for (int i = idxBegin; i <= idxEnd; ++i)
		pidxSortMapping[i - idxBegin] = i;

	std::sort(std::execution::par_unseq,
		pidxSortMapping, pidxSortMapping + cItems,
		pfnSort[CPlayList::Sf2FnIdx(uFlags)]);

	using ItList = std::vector<PLAYLISTUNIT>::iterator;

	ItList itBegin = vList.begin() + idxBegin;
	ItList itEnd = ((idxEnd == (int)vList.size() - 1) ? vList.end() : vList.begin() + idxEnd);

	for (int i = idxBegin; i <= idxEnd; ++i)
		vList[i].idxSortMapping = pidxSortMapping[i - idxBegin];

	if (idxRealCurr >= 0)
	{
		auto pNewPos = std::find(std::execution::par_unseq, pidxSortMapping, pidxSortMapping + cItems, idxRealCurr);
		if (pNewPos != pidxSortMapping + cItems)
			m_idxCurrFile = (int)(pNewPos - pidxSortMapping);
	}
	if (idxRealLaterPlay >= 0)
	{
		auto pNewPos = std::find(std::execution::par_unseq, pidxSortMapping, pidxSortMapping + cItems, idxRealLaterPlay);
		if (pNewPos != pidxSortMapping + cItems)
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
		if (StrStrIW(Item.rsName.Data(), pszKeyWord))
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
		//if (m_idxCurrLrc >= 0)
		//{
		//	if (m_idxCurrLrc + 1 < cLrc)
		//	{
		//		if (m_fPos >= m_vLrc[m_idxCurrLrc].fTime && m_fPos < m_vLrc[m_idxCurrLrc + 1].fTime)
		//		{
		//			++m_idxCurrLrc;
		//			goto EndFindLrc;
		//		}
		//	}
		//	else if (m_fPos >= m_vLrc[m_idxCurrLrc].fTime)
		//		goto EndFindLrc;
		//}
		auto it = std::lower_bound(m_vLrc.begin(), m_vLrc.end(), m_fPos,
			[](const Utils::LRCINFO& Item, float fPos)->bool
			{
				return Item.fTime < fPos;
			});
		if (it == m_vLrc.end())
			m_idxCurrLrc = (int)m_vLrc.size() - 1;
		else if (it == m_vLrc.begin())
			m_idxCurrLrc = -1;
		else
			m_idxCurrLrc = (int)std::distance(m_vLrc.begin(), it - 1);
		EckAssert(m_idxCurrLrc >= -1 && m_idxCurrLrc < (int)m_vLrc.size());

		if (m_idxCurrLrc != m_idxLastLrc)
		{
			m_idxLastLrc = m_idxCurrLrc;
			uRet |= TKC_LRCPOSUPDATED;
		}
	}

	return uRet;
}

HFX CPlayer::SetFx(int idx)
{
	if (idx == FXI_PARAMEQ)
	{
		const auto hFx = m_FxMgr.GetHFxEQ();
		EckCounter(ARRAYSIZE(CEffectMgr::EQ), i)
		{
			if (!hFx[i])
				hFx[i] = m_Bass.SetFx(
					CEffectMgr::Fxi2BassIdx(FXI_PARAMEQ),
					m_FxMgr.GetPriority(FXI_PARAMEQ));
		}
	}
	else
	{
		const auto hFx = m_FxMgr.GetHFx(idx);
		if (!hFx)
			return m_FxMgr.GetHFx(idx) = m_Bass.SetFx(
				CEffectMgr::Fxi2BassIdx(idx), m_FxMgr.GetPriority(idx));
	}
	return 0;
}

BOOL CPlayer::RemoveFx(int idx)
{
	if (idx == FXI_PARAMEQ)
	{
		const auto hFx = m_FxMgr.GetHFxEQ();
		EckCounter(ARRAYSIZE(CEffectMgr::EQ), i)
		{
			if (hFx[i])
			{
				m_Bass.RemoveFx(hFx[i]);
				hFx[i] = NULL;
			}
		}
		return TRUE;
	}
	else
	{
		const auto hFx = m_FxMgr.GetHFx(idx);
		if (hFx)
		{
			const auto b = m_Bass.RemoveFx(hFx);
			m_FxMgr.GetHFx(idx) = NULL;
			return b;
		}
		return TRUE;
	}
}

void CPlayer::EndAddOperation(BOOL bSort)
{
	if (m_cItemNeedUpdated > 10)
	{
		CDlgProgress Dlg{};
		Dlg.DlgBox(App->GetMainWnd()->HWnd);
	}
	else
	{
		EckCounter(GetList().GetCount(), i)
			GetList().UpdateItemInfo(i);
	}
	EndSortProtect(bSort);
}