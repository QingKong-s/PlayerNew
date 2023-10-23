#include "CPlayer.h"
#include "CApp.h"
#include "COptionsMgr.h"

BOOL CPlayer::Play(int idx)
{
	EckAssert(idx >= 0 && idx < m_List.GetCount());
	Stop();
	auto& Item = m_List.At(idx);

	m_Bass.Open(Item.rsFile.Data());
	if (!m_Bass.GetHStream())
	{
		CApp::ShowError(NULL, m_Bass.GetError(), CApp::ErrSrc::Bass, L"播放失败");
		return FALSE;
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
	return FALSE;
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

void CPlayer::OnItemInserted(int idxPos, int cItems)
{
	if (m_idxCurrFile >= idxPos)
		m_idxCurrFile += cItems;
}

void CPlayer::OnItemDeleted(int idxItem, int cItems)
{
	if (idxItem == m_idxCurrFile)
		Stop();
	else if (idxItem < m_idxCurrFile)
	{
		m_idxCurrFile -= cItems;
	}
}