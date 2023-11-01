#pragma once
#include <Windows.h>

#include <unordered_map>

#include "bass.h"
#include "bass_fx.h"
#include "bassmidi.h"

#include "Utils.h"

class CBass
{
private:
	enum class MusicType
	{
		Invalid,
		Normal,
		Mod,
		Midi
	};

	MusicType m_eMusicType = MusicType::Invalid;
	DWORD m_hStream = NULL;

public:
	static int GetError(PCWSTR* ppszErr = NULL);

	static PCWSTR GetErrorMsg(int iErrCode);

	static void Init(int iDevice = -1, DWORD dwFreq = 44100, DWORD dwFlags = 0, HWND hWnd = NULL)
	{
		BASS_Init(iDevice, dwFreq, dwFlags, hWnd, NULL);
	}

	static void Free()
	{
		BASS_Free();
	}

	~CBass();

	DWORD Open(PCWSTR pszFile,
		DWORD dwFlagsHS = BASS_SAMPLE_FX | BASS_STREAM_DECODE,
		DWORD dwFlagsHM = BASS_SAMPLE_FX | BASS_MUSIC_PRESCAN | BASS_STREAM_DECODE,
		DWORD dwFlagsHMIDI = BASS_SAMPLE_FX | BASS_STREAM_DECODE);

	PNInline BOOL Play(BOOL bReset = FALSE) const
	{
		return BASS_ChannelPlay(m_hStream, bReset);
	}

	PNInline BOOL Pause() const
	{
		return BASS_ChannelPause(m_hStream);
	}

	PNInline BOOL Stop() const
	{
		return BASS_ChannelStop(m_hStream);
	}

	PNInline BOOL SetVolume(float fVolume) const
	{
		return BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_VOL, fVolume);
	}

	PNInline BOOL SetPan(float fPan) const
	{
		return BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_PAN, fPan);
	}

	PNInline BOOL SetPosition(double fTime) const
	{
		return BASS_ChannelSetPosition(m_hStream, BASS_ChannelSeconds2Bytes(m_hStream, fTime), BASS_POS_BYTE);
	}

	PNInline double GetPosition() const
	{
		return BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetPosition(m_hStream, BASS_POS_BYTE));
	}

	PNInline double GetLength() const
	{
		return BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetLength(m_hStream, BASS_POS_BYTE));
	}

	PNInline void Close();

	PNInline DWORD GetHStream() { return m_hStream; }

	PNInline DWORD GetLevel()
	{
		return BASS_ChannelGetLevel(m_hStream);
	}

	PNInline DWORD GetData(float* pBuf, DWORD cbBuf)
	{
		return BASS_ChannelGetData(m_hStream, pBuf, cbBuf);
	}

	PNInline DWORD TempoCreate(DWORD dwFlags = BASS_SAMPLE_FX | BASS_FX_FREESOURCE)
	{
		m_hStream = BASS_FX_TempoCreate(m_hStream, dwFlags);
		return m_hStream;
	}

	PNInline DWORD IsActive()
	{
		return BASS_ChannelIsActive(m_hStream);
	}
};

