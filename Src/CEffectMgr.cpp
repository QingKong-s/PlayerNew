#include "pch.h"

BOOL CEffectMgr::ApplyParam(int idx) const
{
	EckAssert(idx >= 0 && idx < FXI_MAX);
	switch (idx)
	{
	case FXI_CHORUS:
		return BASS_FXSetParameters(GetHFx(idx), &Chorus);
	case FXI_COMPRESSOR:
		return BASS_FXSetParameters(GetHFx(idx), &Compressor);
	case FXI_DISTORTION:
		return BASS_FXSetParameters(GetHFx(idx), &Distortion);
	case FXI_ECHO:
		return BASS_FXSetParameters(GetHFx(idx), &Echo);
	case FXI_FLANGER:
		return BASS_FXSetParameters(GetHFx(idx), &Flanger);
	case FXI_GARGLE:
		return BASS_FXSetParameters(GetHFx(idx), &Gargle);
	case FXI_I3DL2REVERB:
		return BASS_FXSetParameters(GetHFx(idx), &I3DL2Reverb);
	case FXI_REVERB:
		return BASS_FXSetParameters(GetHFx(idx), &Reverb);
	case FXI_ROTATE:
		return BASS_FXSetParameters(GetHFx(idx), &Rotate);
	case FXI_PARAMEQ:
		EckCounter(ARRAYSIZE(EQ), i)
			BASS_FXSetParameters(hFxEQ[i], &EQ[i]);
		return TRUE;
	default:
		__assume(0);
	}
}

void CEffectMgr::ResetParam(int idx)
{
	EckAssert(idx >= 0 && idx < FXI_MAX);
	switch (idx)
	{
	case FXI_CHORUS:
		Chorus = DefChorus;
		break;
	case FXI_COMPRESSOR:
		Compressor = DefCompressor;
		break;
	case FXI_DISTORTION:
		Distortion = DefDistortion;
		break;
	case FXI_ECHO:
		Echo = DefEcho;
		break;
	case FXI_FLANGER:
		Flanger = DefFlanger;
		break;
	case FXI_GARGLE:
		Gargle = DefGargle;
		break;
	case FXI_I3DL2REVERB:
		I3DL2Reverb = DefI3DL2Reverb;
		break;
	case FXI_REVERB:
		Reverb = DefReverb;
		break;
	case FXI_ROTATE:
		Rotate = DefRotate;
		break;
	case FXI_PARAMEQ:
		memcpy(EQ, DefEQ, sizeof(EQ));
		break;
	default:
		__assume(0);
	}
}