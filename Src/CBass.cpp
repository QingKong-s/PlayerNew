#include "pch.h"

static const std::unordered_map<int, PCWSTR> c_BassErrorMap
{
	{0,L"BASS_OK"},
	{1,L"BASS_ERROR_MEM"},
	{2,L"BASS_ERROR_FILEOPEN"},
	{3,L"BASS_ERROR_DRIVER"},
	{4,L"BASS_ERROR_BUFLOST"},
	{5,L"BASS_ERROR_HANDLE"},
	{6,L"BASS_ERROR_FORMAT"},
	{7,L"BASS_ERROR_POSITION"},
	{8,L"BASS_ERROR_INIT"},
	{9,L"BASS_ERROR_START"},
	{10,L"BASS_ERROR_SSL"},
	{14,L"BASS_ERROR_ALREADY"},
	{17,L"BASS_ERROR_NOTAUDIO"},
	{18,L"BASS_ERROR_NOCHAN"},
	{19,L"BASS_ERROR_ILLTYPE"},
	{20,L"BASS_ERROR_ILLPARAM"},
	{21,L"BASS_ERROR_NO3D"},
	{22,L"BASS_ERROR_NOEAX"},
	{23,L"BASS_ERROR_DEVICE"},
	{24,L"BASS_ERROR_NOPLAY"},
	{25,L"BASS_ERROR_FREQ"},
	{27,L"BASS_ERROR_NOTFILE"},
	{29,L"BASS_ERROR_NOHW"},
	{31,L"BASS_ERROR_EMPTY"},
	{32,L"BASS_ERROR_NONET"},
	{33,L"BASS_ERROR_CREATE"},
	{34,L"BASS_ERROR_NOFX"},
	{37,L"BASS_ERROR_NOTAVAIL"},
	{38,L"BASS_ERROR_DECODE"},
	{39,L"BASS_ERROR_DX"},
	{40,L"BASS_ERROR_TIMEOUT"},
	{41,L"BASS_ERROR_FILEFORM"},
	{42,L"BASS_ERROR_SPEAKER"},
	{43,L"BASS_ERROR_VERSION"},
	{44,L"BASS_ERROR_CODEC"},
	{45,L"BASS_ERROR_ENDED"},
	{46,L"BASS_ERROR_BUSY"},
	{47,L"BASS_ERROR_UNSTREAMABLE"},
	{-1,L"BASS_ERROR_UNKNOWN"},
};

CBass::~CBass()
{
	Close();
}

DWORD CBass::Open(PCWSTR pszFile, DWORD dwFlagsHS, DWORD dwFlagsHM, DWORD dwFlagsHMIDI)
{
	Close();
	constexpr DWORD dwCommFlags = BASS_SAMPLE_FLOAT | BASS_UNICODE;
	DWORD h = BASS_StreamCreateFile(FALSE, pszFile, 0, 0, dwCommFlags | dwFlagsHS);

	m_eMusicType = MusicType::Invalid;
	if (!h && BASS_ErrorGetCode() == BASS_ERROR_FILEFORM)
	{
		h = BASS_MusicLoad(FALSE, pszFile, 0, 0, dwCommFlags | dwFlagsHM, 0);
		if (!h && BASS_ErrorGetCode() == BASS_ERROR_FILEFORM)
		{
			h = BASS_MIDI_StreamCreateFile(FALSE, pszFile, 0, 0, dwCommFlags | dwFlagsHMIDI, 1);
			if (h)
				m_eMusicType = MusicType::Midi;
		}
		else
			m_eMusicType = MusicType::Mod;
	}
	else
		m_eMusicType = MusicType::Normal;

	m_hStream = h;
	if (h)
		m_fDefSpeed = GetAttr(BASS_ATTRIB_FREQ);
	else
		m_fDefSpeed = 0.f;
	return h;
}

void CBass::Close()
{
	if (!m_hStream)
		return;
	switch (m_eMusicType)
	{
	case MusicType::Normal:
	case MusicType::Midi:
		BASS_StreamFree(m_hStream);
		break;
	case MusicType::Mod:
		BASS_MusicFree(m_hStream);
		break;
	default:
		EckDbgBreak();
		break;
	}
	m_hStream = 0;
}

int CBass::GetError(PCWSTR* ppszErr)
{
	int ii = BASS_ErrorGetCode();
	if (ppszErr)
	{
		auto it = c_BassErrorMap.find(ii);
		if (it == c_BassErrorMap.end())
			*ppszErr = c_BassErrorMap.at(-1);
		else
			*ppszErr = it->second;
	}
	return ii;
}

PCWSTR CBass::GetErrorMsg(int iErrCode)
{
	auto it = c_BassErrorMap.find(iErrCode);
	if (it == c_BassErrorMap.end())
		return c_BassErrorMap.at(-1);
	else
		return it->second;
}
