#pragma once
#include "CBass.h"
#include "CPlayList.h"
#include "Utils.h"

#include "eck\CRefStr.h"

class CPlayer
{
private:
	CBass m_Bass{};
	float m_fPos = 0.f;
	ULONGLONG m_ullPos = 0ull;
	ULONGLONG m_ullLength = 0ull;

	CPlayList m_List{};
	int m_idxCurrFile = -1;

	eck::CRefStrW m_rsCurrFile{};

	std::vector<Utils::LRCLABEL> m_vLrcLabel{};
	std::vector<Utils::LRCINFO> m_vLrc{};
	int m_idxCurrLrc = -1;

	Utils::MUSICINFO m_MusicInfo{};

	BOOL m_bHasActiveFile = FALSE;

	void ApplyPrevEffect()
	{

	}
public:
	BOOL Play(int idx);

	void Stop();

	void PlayNext();

	void OnItemInserted(int idxPos, int cItems);

	void OnItemDeleted(int idxItem, int cItems);

	PNInline CBass& GetBass() { return m_Bass; }

	PNInline float GetPosF() { return m_fPos; }

	PNInline ULONGLONG GetPos() { return m_ullPos; }

	PNInline ULONGLONG GetLength() { return m_ullLength; }

	PNInline int GetCurrFile() { return m_idxCurrFile; }

	PNInline const eck::CRefStrW GetCurrFileName() { return m_rsCurrFile; }

	PNInline int GetCurrLrc() { return m_idxCurrLrc; }

	PNInline const std::vector<Utils::LRCINFO>& GetLrc() { return m_vLrc; }

	PNInline const std::vector<Utils::LRCLABEL>& GetLrcLabel() { return m_vLrcLabel; }

	PNInline const Utils::MUSICINFO& GetMusicInfo() { return m_MusicInfo; }

	PNInline CPlayList& GetList() { return m_List; }

	PNInline BOOL IsFileActive() { return m_bHasActiveFile; }
};