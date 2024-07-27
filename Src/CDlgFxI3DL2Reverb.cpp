#include "pch.h"
#include "CDlgFxI3DL2Reverb.h"

void CDlgFxI3DL2Reverb::UpdateCtrl()
{
	const auto& Param = App->GetPlayer().GetEffectMgr().I3DL2Reverb;
	m_TBRoom.SetPos(Param.lRoom);
	m_TBRoomHF.SetPos(Param.lRoomHF);
	m_TBRoomRollOffFactor.SetPos((int)(Param.flRoomRolloffFactor * 10.f));
	m_TBDecayTime.SetPos((int)(Param.flDecayTime * 10.f));
	m_TBDecayHFRatio.SetPos((int)(Param.flDecayHFRatio * 100.f));
	m_TBReflections.SetPos(Param.lReflections);
	m_TBReflectionsDelay.SetPos((int)(Param.flReflectionsDelay * 1000.f));
	m_TBReverb.SetPos(Param.lReverb);
	m_TBReverbDelay.SetPos((int)(Param.flReverbDelay * 1000.f));
	m_TBDiffusion.SetPos((int)(Param.flDiffusion));
	m_TBDensity.SetPos((int)(Param.flDensity));
	m_TBHFReference.SetPos((int)(Param.flHFReference));
}

LRESULT CDlgFxI3DL2Reverb::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		auto& Player = App->GetPlayer();
		auto& FxMgr = Player.GetEffectMgr();
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
		{
			switch (LOWORD(wParam))
			{
			case IDC_CB_ENABLE:
			{
				if (m_CBEnable.GetCheckState())
					Player.SetFx(FXI_I3DL2REVERB);
				else
					Player.RemoveFx(FXI_I3DL2REVERB);
			}
			return 0;
			case IDC_BT_RESET:
			{
				FxMgr.ResetParam(FXI_I3DL2REVERB);
				FxMgr.ApplyParam(FXI_I3DL2REVERB);
				UpdateCtrl();
			}
			return 0;
			}
		}
		return 0;
		}
	}
	break;

	case WM_HSCROLL:
	{
		auto& FxMgr = App->GetPlayer().GetEffectMgr();
		switch (GetDlgCtrlID((HWND)lParam))
		{
		case IDC_TB_ROOM:
			FxMgr.I3DL2Reverb.lRoom = m_TBRoom.GetPos();
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		case IDC_TB_ROOMHF:
			FxMgr.I3DL2Reverb.lRoomHF = m_TBRoomHF.GetPos();
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		case IDC_TB_ROOMROLLOFFFACTOR:
			FxMgr.I3DL2Reverb.flRoomRolloffFactor = m_TBRoomRollOffFactor.GetPos() / 10.f;
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		case IDC_TB_DECAYTIME:
			FxMgr.I3DL2Reverb.flDecayTime = m_TBDecayTime.GetPos() / 10.f;
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		case IDC_TB_DECAYHFRADIO:
			FxMgr.I3DL2Reverb.flDecayHFRatio = m_TBDecayHFRatio.GetPos() / 100.f;
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		case IDC_TB_REFLECTIONS:
			FxMgr.I3DL2Reverb.lReflections = m_TBReflections.GetPos();
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		case IDC_TB_REFLECTIONSDELAY:
			FxMgr.I3DL2Reverb.flReflectionsDelay = m_TBReflectionsDelay.GetPos() / 1000.f;
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		case IDC_TB_REVERB:
			FxMgr.I3DL2Reverb.lReverb = m_TBReverb.GetPos();
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		case IDC_TB_REVERBDELAY:
			FxMgr.I3DL2Reverb.flReverbDelay = m_TBReverbDelay.GetPos() / 1000.f;
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		case IDC_TB_DIFFUSION:
			FxMgr.I3DL2Reverb.flDiffusion = (float)m_TBDiffusion.GetPos();
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		case IDC_TB_DENSITY:
			FxMgr.I3DL2Reverb.flDensity = (float)m_TBDensity.GetPos();
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		case IDC_TB_HFREFERENCE:
			FxMgr.I3DL2Reverb.flHFReference = (float)m_TBHFReference.GetPos();
			FxMgr.ApplyParam(FXI_I3DL2REVERB);
			return 0;
		}
	}
	break;

	case WM_NCPAINT:
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CDlgFxI3DL2Reverb::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_CBEnable.AttachNew(GetDlgItem(hDlg, IDC_CB_ENABLE));
	m_BTReset.AttachNew(GetDlgItem(hDlg, IDC_BT_RESET));

	m_TBRoom.AttachNew(GetDlgItem(hDlg, IDC_TB_ROOM));
	m_TBRoomHF.AttachNew(GetDlgItem(hDlg, IDC_TB_ROOMHF));
	m_TBRoomRollOffFactor.AttachNew(GetDlgItem(hDlg, IDC_TB_ROOMROLLOFFFACTOR));
	m_TBDecayTime.AttachNew(GetDlgItem(hDlg, IDC_TB_DECAYTIME));
	m_TBDecayHFRatio.AttachNew(GetDlgItem(hDlg, IDC_TB_DECAYHFRADIO));
	m_TBReflections.AttachNew(GetDlgItem(hDlg, IDC_TB_REFLECTIONS));
	m_TBReflectionsDelay.AttachNew(GetDlgItem(hDlg, IDC_TB_REFLECTIONSDELAY));
	m_TBReverb.AttachNew(GetDlgItem(hDlg, IDC_TB_REVERB));
	m_TBReverbDelay.AttachNew(GetDlgItem(hDlg, IDC_TB_REVERBDELAY));
	m_TBDiffusion.AttachNew(GetDlgItem(hDlg, IDC_TB_DIFFUSION));
	m_TBDensity.AttachNew(GetDlgItem(hDlg, IDC_TB_DENSITY));
	m_TBHFReference.AttachNew(GetDlgItem(hDlg, IDC_TB_HFREFERENCE));

	m_TBRoom.SetRange32(-10000, 0, FALSE);
	m_TBRoomHF.SetRange32(-10000, 0, FALSE);
	m_TBRoomRollOffFactor.SetRange32(0, 100, FALSE);// 缩小10倍
	m_TBDecayTime.SetRange32(1, 200, FALSE);// 缩小10倍
	m_TBDecayHFRatio.SetRange32(10, 200, FALSE);// 缩小100倍
	m_TBReflections.SetRange32(-10000, 1000, FALSE);
	m_TBReflectionsDelay.SetRange32(0, 300, FALSE);// 缩小1000倍
	m_TBReverb.SetRange32(-10000, 20000, FALSE);
	m_TBReverbDelay.SetRange32(0, 100, FALSE);// 缩小1000倍
	m_TBDiffusion.SetRange32(0, 100, FALSE);
	m_TBDensity.SetRange32(0, 100, FALSE);
	m_TBHFReference.SetRange32(20, 20000, FALSE);


	m_CBEnable.SetCheckState(App->GetPlayer().GetEffectMgr().IsValid(FXI_I3DL2REVERB));
	UpdateCtrl();

	SetFocus(m_CBEnable.HWnd);
	return FALSE;
}