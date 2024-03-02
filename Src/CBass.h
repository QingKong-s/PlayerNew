#pragma once
#include <Windows.h>

#include <unordered_map>
#include <format>

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
	float m_fDefSpeed = 0.f;
public:
	static int GetError(PCWSTR* ppszErr = NULL);

	static PCWSTR GetErrorMsg(int iErrCode);

	PNInline static BOOL Init(int iDevice = -1, DWORD dwFreq = 44100, DWORD dwFlags = 0, HWND hWnd = NULL)
	{
		return BASS_Init(iDevice, dwFreq, dwFlags, hWnd, NULL);
	}

	PNInline static BOOL Free()
	{
		return BASS_Free();
	}

	PNInline static DWORD GetVer()
	{
		return BASS_GetVersion();
	}

	PNInline static std::wstring VerToString(DWORD dw)
	{
		const WORD wHigh = HIWORD(dw);
		const WORD wLow = LOWORD(dw);
		return std::format(L"{}.{}.{}.{}", HIBYTE(wHigh), LOBYTE(wHigh), HIBYTE(wLow), LOBYTE(wLow));
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
		return SetAttr(BASS_ATTRIB_VOL, fVolume);
	}

	PNInline float GetVolume() const
	{
		return GetAttr(BASS_ATTRIB_VOL);
	}

	PNInline BOOL SetPan(float fPan) const
	{
		return SetAttr(BASS_ATTRIB_PAN, fPan);
	}

	PNInline BOOL SetSpeed(float fScale) const
	{
		return SetAttr(BASS_ATTRIB_FREQ, fScale * m_fDefSpeed);
	}

	PNInline BOOL SetPosition(double fTime) const
	{
		return BASS_ChannelSetPosition(m_hStream, BASS_ChannelSeconds2Bytes(m_hStream, fTime), BASS_POS_BYTE);
	}

	PNInline BOOL SetAttr(DWORD dwAttr, float f) const
	{
		return BASS_ChannelSetAttribute(m_hStream, dwAttr, f);
	}

	PNInline float GetAttr(DWORD dwAttr, BOOL* pb = NULL) const
	{
		float f;
		const auto b = BASS_ChannelGetAttribute(m_hStream, dwAttr, &f);
		if (pb)
			*pb = b;
		return f;
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

	PNInline DWORD GetHStream() const { return m_hStream; }

	PNInline DWORD GetLevel() const
	{
		return BASS_ChannelGetLevel(m_hStream);
	}

	PNInline DWORD GetData(float* pBuf, DWORD cbBuf) const
	{
		return BASS_ChannelGetData(m_hStream, pBuf, cbBuf);
	}

	PNInline DWORD TempoCreate(DWORD dwFlags = BASS_SAMPLE_FX | BASS_FX_FREESOURCE)
	{
		m_hStream = BASS_FX_TempoCreate(m_hStream, dwFlags);
		return m_hStream;
	}

	PNInline DWORD IsActive() const
	{
		return BASS_ChannelIsActive(m_hStream);
	}
};

