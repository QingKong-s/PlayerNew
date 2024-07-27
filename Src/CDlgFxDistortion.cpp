#include "pch.h"
#include "CDlgFxDistortion.h"

void CDlgFxDistortion::UpdateCtrl()
{
	const auto& Param = App->GetPlayer().GetEffectMgr().Distortion;
	m_TBGain.SetPos((int)(Param.fGain * 10.f));
	m_TBEdge.SetPos((int)(Param.fEdge));
	m_TBCenterFreq.SetPos((int)(Param.fPostEQCenterFrequency));
	m_TBBandWidth.SetPos((int)(Param.fPostEQBandwidth));
	m_TBPreLowPassCutOff.SetPos((int)(Param.fPreLowpassCutoff));
}

LRESULT CDlgFxDistortion::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
					Player.SetFx(FXI_DISTORTION);
				else
					Player.RemoveFx(FXI_DISTORTION);
			}
			return 0;
			case IDC_BT_RESET:
			{
				FxMgr.ResetParam(FXI_DISTORTION);
				FxMgr.ApplyParam(FXI_DISTORTION);
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
			FxMgr.Distortion.fGain = m_TBGain.GetPos() / 10.f;
			FxMgr.ApplyParam(FXI_DISTORTION);
			return 0;
		case IDC_TB_EDGE:
			FxMgr.Distortion.fEdge = (float)m_TBEdge.GetPos();
			FxMgr.ApplyParam(FXI_DISTORTION);
			return 0;
		case IDC_TB_CENTERFREQUENCY:
			FxMgr.Distortion.fPostEQCenterFrequency = (float)m_TBCenterFreq.GetPos();
			FxMgr.ApplyParam(FXI_DISTORTION);
			return 0;
		case IDC_TB_BANDWIDTH:
			FxMgr.Distortion.fPostEQBandwidth = (float)m_TBBandWidth.GetPos();
			FxMgr.ApplyParam(FXI_DISTORTION);
			return 0;
		case IDC_TB_PRELOWPASSCUTOFF:
			FxMgr.Distortion.fPreLowpassCutoff = (float)m_TBPreLowPassCutOff.GetPos();
			FxMgr.ApplyParam(FXI_DISTORTION);
			return 0;
		}
	}
	break;

	case WM_NCPAINT:
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CDlgFxDistortion::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_CBEnable.AttachNew(GetDlgItem(hDlg, IDC_CB_ENABLE));
	m_BTReset.AttachNew(GetDlgItem(hDlg, IDC_BT_RESET));

	m_TBGain.AttachNew(GetDlgItem(hDlg, IDC_TB_GAIN));
	m_TBEdge.AttachNew(GetDlgItem(hDlg, IDC_TB_EDGE));
	m_TBCenterFreq.AttachNew(GetDlgItem(hDlg, IDC_TB_CENTERFREQUENCY));
	m_TBBandWidth.AttachNew(GetDlgItem(hDlg, IDC_TB_BANDWIDTH));
	m_TBPreLowPassCutOff.AttachNew(GetDlgItem(hDlg, IDC_TB_PRELOWPASSCUTOFF));

	m_TBGain.SetRange32(-600, 0, FALSE);// 缩小10倍
	m_TBEdge.SetRange32(0, 100, FALSE);
	m_TBCenterFreq.SetRange32(100, 8000, FALSE);
	m_TBBandWidth.SetRange32(100, 8000, FALSE);
	m_TBPreLowPassCutOff.SetRange32(100, 8000, FALSE);

	m_CBEnable.SetCheckState(App->GetPlayer().GetEffectMgr().IsValid(FXI_DISTORTION));
	UpdateCtrl();

	SetFocus(m_CBEnable.HWnd);
	return FALSE;
}