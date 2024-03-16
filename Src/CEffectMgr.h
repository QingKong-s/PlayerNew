#pragma once
#include "CBass.h"
#include "Utils.h"

enum
{
	FXI_CHORUS,		
	FXI_COMPRESSOR,	
	FXI_DISTORTION,	
	FXI_ECHO,		
	FXI_FLANGER,	
	FXI_GARGLE,		
	FXI_I3DL2REVERB,
	FXI_PARAMEQ,	
	FXI_REVERB,		
	FXI_ROTATE,		
/*	FXI_PHASER,
	FXI_AUTOWAH,*/	

	FXI_MAX
};

struct CEffectMgr
{
	constexpr static int BassIdx[FXI_MAX]
	{
		BASS_FX_DX8_CHORUS,
		BASS_FX_DX8_COMPRESSOR,
		BASS_FX_DX8_DISTORTION,
		BASS_FX_DX8_ECHO,
		BASS_FX_DX8_FLANGER,
		BASS_FX_DX8_GARGLE,
		BASS_FX_DX8_I3DL2REVERB,
		BASS_FX_DX8_PARAMEQ,
		BASS_FX_DX8_REVERB,
		BASS_FX_BFX_ROTATE,
		//BASS_FX_BFX_PHASER,
		//BASS_FX_BFX_AUTOWAH,
	};

	constexpr static PCWSTR FxName[FXI_MAX]
	{
		L"合唱",
		L"压缩",
		L"失真",
		L"回声",
		L"镶边",
		L"漱口",
		L"3D混响",
		L"均衡器",
		L"混响",
		L"环绕",
		//L"移相",
		//L"自动哇音",
	};

	constexpr static BASS_DX8_CHORUS DefChorus
	{
		50.f,
		10.f,
		25.f,
		1.1f,
		1,
		16.f,
		BASS_DX8_PHASE_90
	};

	constexpr static BASS_DX8_COMPRESSOR DefCompressor
	{
		0.f,
		10.f,
		200.f,
		-20.f,
		3.f,
		4.f
	};

	constexpr static BASS_DX8_DISTORTION DefDistortion
	{
		-18.f,
		15.f,
		2400.f,
		2400.f,
		8000.f
	};

	constexpr static BASS_DX8_ECHO DefEcho
	{
		50.f,
		50.f,
		500.f,
		500.f,
		FALSE
	};

	constexpr static BASS_DX8_FLANGER DefFlanger
	{
		50.f,
		100.f,
		-50.f,
		0.25f,
		1,
		2.0f,
		BASS_DX8_PHASE_ZERO
	};

	constexpr static BASS_DX8_GARGLE DefGargle
	{
		20,
		0,
	};

	constexpr static BASS_DX8_I3DL2REVERB DefI3DL2Reverb
	{
		-1000,
		-100,
		0.0f,
		1.49f,
		0.83f,
		-2602,
		0.007f,
		200,
		0.011f,
		100.f,
		100.f,
		5000.f
	};

	constexpr static BASS_DX8_REVERB DefReverb
	{
		0.f,
		0.f,
		1000.f,
		0.001f
	};

	struct EQSETTING
	{
		PCWSTR pszText;
		float Setting[10];
	};

	constexpr static EQSETTING EQSetting[]
	{
		{L"无",			{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}},
		{L"ACG",		{ 4.f, 6.f, 3.f, 0.f, 0.f, 2.f, 5.f, 1.f, 1.f, 4.f}},
		{L"民谣",		{ 0.f, 3.f, 0.f, 0.f, 1.f, 4.f, 5.f, 3.f, 0.f, 2.f}},
		{L"低音",		{ 6.f, 4.f, 6.f, 2.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}},
		{L"低音&高音",	{ 6.f, 5.f, 6.f, 1.f, 0.f, 0.f, 1.f, 3.f, 4.f, 0.f}},
		{L"蓝调",		{ 2.f, 6.f, 4.f, 0.f,-2.f,-1.f, 2.f, 2.f, 1.f, 3.f}},
		{L"古风",		{ 4.f, 2.f, 2.f, 0.f,-1.f, 3.f, 4.f, 1.f, 1.f, 3.f}},
		{L"古典",		{ 4.f, 4.f, 3.f, 2.f,-1.f,-1.f, 0.f, 1.f, 3.f, 4.f}},
		{L"乡村",		{ 0.f, 2.f, 3.f, 0.f, 0.f, 2.f, 3.f, 1.f, 0.f, 0.f}},
		{L"舞曲",		{ 4.f, 5.f, 7.f, 0.f, 1.f, 3.f, 4.f, 4.f, 3.f, 0.f}},
		{L"慢歌",		{ 2.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,-1.f,-1.f}},
		{L"电音",		{ 5.f, 6.f, 5.f, 0.f,-1.f, 1.f, 0.f, 1.f, 4.f, 3.f}},
		{L"重金属",		{-2.f, 5.f, 4.f,-2.f,-2.f,-1.f, 2.f, 3.f, 1.f, 4.f}},
		{L"说唱",		{ 5.f, 5.f, 4.f, 0.f,-2.f, 1.f, 3.f, 0.f, 3.f, 4.f}},
		{L"爵士",		{ 3.f, 3.f, 1.f, 2.f,-1.f,-1.f, 0.f, 1.f, 2.f, 4.f}},
		{L"现场",		{ 5.f, 5.f, 6.f, 0.f,-1.f, 0.f, 3.f, 4.f, 6.f, 5.f}},
		{L"中音",		{-2.f,-3.f,-3.f, 0.f, 1.f, 4.f, 3.f, 2.f,-1.f,-2.f}},
		{L"流行",		{-1.f,-1.f, 0.f, 1.f, 4.f, 3.f, 1.f, 0.f,-1.f, 1.f}},
		{L"摇滚",		{ 4.f, 3.f, 3.f, 1.f, 0.f,-1.f, 0.f, 1.f, 2.f, 4.f}},
		{L"柔和",		{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,-2.f,-2.f, 0.f}},
		{L"柔和低音",		{ 3.f, 2.f, 1.f, 0.f, 0.f, 0.f, 0.f,-2.f,-2.f,-2.f}},
		{L"柔和高音",		{-3.f,-1.f, 0.f, 0.f, 0.f, 0.f, 0.f,-1.f, 3.f, 2.f}},
		{L"超重低音",		{ 6.f, 5.f, 8.f, 2.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}},
	};

	constexpr static float EQCenter[10]{ 31,62,125,250,500,1000,2000,4000,8000,16000 };

	constexpr static BASS_DX8_PARAMEQ DefEQ[10]
	{
		{EQCenter[0],12.f},
		{EQCenter[1],12.f},
		{EQCenter[2],12.f},
		{EQCenter[3],12.f},
		{EQCenter[4],12.f},
		{EQCenter[5],12.f},
		{EQCenter[6],12.f},
		{EQCenter[7],12.f},
		{EQCenter[8],12.f},
		{EQCenter[9],12.f},
	};

	constexpr static BASS_BFX_ROTATE DefRotate
	{
		0.f,
		-1
	};

	float fTempo{};
	int idxPresetEQ{};

	HFX hFx[FXI_MAX]{};

	BASS_DX8_CHORUS			Chorus{ DefChorus };
	BASS_DX8_COMPRESSOR		Compressor{ DefCompressor };
	BASS_DX8_DISTORTION		Distortion{ DefDistortion };
	BASS_DX8_ECHO			Echo{ DefEcho };
	BASS_DX8_FLANGER		Flanger{ DefFlanger };
	BASS_DX8_GARGLE			Gargle{ DefGargle };
	BASS_DX8_I3DL2REVERB	I3DL2Reverb{ DefI3DL2Reverb };
	BASS_DX8_REVERB			Reverb{ DefReverb };
	HFX						hFxEQ[10]{};
	BASS_DX8_PARAMEQ		EQ[10]{};
	BASS_BFX_ROTATE			Rotate{ DefRotate };

	int iPriority[FXI_MAX]{};


	PNInline constexpr static int Fxi2BassIdx(int idx)
	{
		return BassIdx[idx];
	}

	PNInline constexpr static PCWSTR Fxi2Name(int idx)
	{
		return FxName[idx];
	}

	CEffectMgr()
	{
		ResetParam(FXI_PARAMEQ);
	}

	BOOL ApplyParam(int idx) const;

	void ResetParam(int idx);

	PNInline HFX GetHFx(int idx) const
	{
		EckAssert(idx >= 0 && idx < FXI_MAX && idx != FXI_PARAMEQ);
		return hFx[idx];
	}

	PNInline HFX& GetHFx(int idx)
	{
		EckAssert(idx >= 0 && idx < FXI_MAX && idx != FXI_PARAMEQ);
		return hFx[idx];
	}

	PNInline HFX* GetHFxEQ() { return hFxEQ; }

	PNInline BOOL IsValid(int idx) const
	{
		if (idx == FXI_PARAMEQ)
		{
			for (auto e : hFxEQ)
				if (e)
					return TRUE;
			return FALSE;
		}
		else ECKLIKELY
			return GetHFx(idx) != 0;
	}

	PNInline int GetPriority(int idx) const { return iPriority[idx]; }

	PNInline void SetPriority(int idx, int i) { iPriority[idx] = i; }
};