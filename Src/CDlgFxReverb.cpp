#include "pch.h"
#include "CDlgFxReverb.h"

void CDlgFxReverb::UpdateCtrl()
{
	const auto& Param = App->GetPlayer().GetEffectMgr().Reverb;
	m_TBGain.SetPos((int)(Param.fInGain * 10.f));
	m_TBMix.SetPos((int)(Param.fReverbMix * 10.f));
	m_TBTime.SetPos((int)(Param.fReverbTime * 1000.f));
	m_TBHighFreqRatio.SetPos((int)(Param.fHighFreqRTRatio * 1000.f));
}

LRESULT CDlgFxReverb::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
					Player.SetFx(FXI_REVERB);
				else
					Player.RemoveFx(FXI_REVERB);
			}
			return 0;
			case IDC_BT_RESET:
			{
				FxMgr.ResetParam(FXI_REVERB);
				FxMgr.ApplyParam(FXI_REVERB);
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
		case IDC_TB_GAIN:
			FxMgr.Reverb.fInGain = m_TBGain.GetPos() / 10.f;
			FxMgr.ApplyParam(FXI_REVERB);
			return 0;
		case IDC_TB_REVERBMIX:
			FxMgr.Reverb.fReverbMix = m_TBMix.GetPos() / 10.f;
			FxMgr.ApplyParam(FXI_REVERB);
			return 0;
		case IDC_TB_REVERBTIME:
			FxMgr.Reverb.fReverbTime = m_TBTime.GetPos() / 1000.f;
			FxMgr.ApplyParam(FXI_REVERB);
			return 0;
		case IDC_TB_HIGHFREQRTRATIO:
			FxMgr.Reverb.fHighFreqRTRatio = m_TBHighFreqRatio.GetPos() / 1000.f;
			FxMgr.ApplyParam(FXI_REVERB);
			return 0;
		}
	}
	break;

	case WM_NCPAINT:
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CDlgFxReverb::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_CBEnable.AttachNew(GetDlgItem(hDlg, IDC_CB_ENABLE));
	m_BTReset.AttachNew(GetDlgItem(hDlg, IDC_BT_RESET));

	m_TBGain.AttachNew(GetDlgItem(hDlg, IDC_TB_GAIN));
	m_TBMix.AttachNew(GetDlgItem(hDlg, IDC_TB_REVERBMIX));
	m_TBTime.AttachNew(GetDlgItem(hDlg, IDC_TB_REVERBTIME));
	m_TBHighFreqRatio.AttachNew(GetDlgItem(hDlg, IDC_TB_HIGHFREQRTRATIO));

	m_TBGain.SetRange32(-960, 0);// 缩小10倍
	m_TBMix.SetRange32(-960, 0);// 缩小10倍
	m_TBTime.SetRange32(1, 3000000);// 缩小1000倍
	m_TBHighFreqRatio.SetRange32(1, 999);// 缩小1000倍

	m_CBEnable.SetCheckState(App->GetPlayer().GetEffectMgr().IsValid(FXI_REVERB));
	UpdateCtrl();

	SetFocus(m_CBEnable.HWnd);
	return FALSE;
}