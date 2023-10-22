#include "CPlayer.h"
#include "CApp.h"
#include "COptionsMgr.h"

void CPlayer::Play(int idx)
{
	Stop();
	auto& Item = m_List.At(idx);

	m_Bass.Open(Item.rsFile.Data());
	if (!m_Bass.GetHStream())
	{
		CApp::ShowError(NULL, m_Bass.GetError(), CApp::ErrSrc::Bass, L"≤•∑≈ ß∞‹");
		return;
	}
	m_rsCurrFile = Item.rsFile;
	m_idxCurrFile = idx;
	m_Bass.TempoCreate();
	m_Bass.Play();
	ApplyPrevEffect();

	m_ullLength = m_Bass.GetLength() * 1000.;
	
	Utils::GetMusicInfo(m_rsCurrFile.Data(), m_MusicInfo);
	Utils::ParseLrc(m_rsCurrFile.Data(), 0u, m_vLrc, m_vLrcLabel, COptionsMgr::GetInst().iLrcFileEncoding);
	if (!m_vLrc.size())
		Utils::ParseLrc(m_MusicInfo.rsLrc.Data(), m_MusicInfo.rsLrc.ByteSize(), m_vLrc, m_vLrcLabel, Utils::LrcEncoding::UTF16LE);
}

void CPlayer::Stop()
{
	m_Bass.Stop();
	m_Bass.Close();

	m_idxCurrFile = -1;
	m_idxCurrLrc = -1;
}

void CPlayer::PlayNext()
{
	if (m_idxCurrFile < 0)
		return;
	int idx = m_idxCurrFile + 1;
	if (idx > m_List.GetCount())
		idx = 0;
	Play(idx);
}