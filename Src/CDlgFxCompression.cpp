#include "CDlgFxCompression.h"

void CDlgFxCompression::UpdateCtrl()
{
	const auto& Param = App->GetPlayer().GetEffectMgr().Compressor;
	m_TBGain.SetPos((int)(Param.fGain * 10.f));
	m_TBAttack.SetPos((int)(Param.fAttack * 100.f));
	m_TBRelease.SetPos((int)(Param.fRelease));
	m_TBThreshold.SetPos((int)(Param.fThreshold * 10.f));
	m_TBRatio.SetPos((int)(Param.fRatio));
	m_TBPreDelay.SetPos((int)(Param.fPredelay * 100.f));
}

LRESULT CDlgFxCompression::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
					Player.SetFx(FXI_COMPRESSOR);
				else
					Player.RemoveFx(FXI_COMPRESSOR);
			}
			return 0;
			case IDC_BT_RESET:
			{
				FxMgr.ResetParam(FXI_COMPRESSOR);
				FxMgr.ApplyParam(FXI_COMPRESSOR);
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
			FxMgr.Compressor.fGain = m_TBGain.GetPos() / 10.f;
			FxMgr.ApplyParam(FXI_COMPRESSOR);
			return 0;
		case IDC_TB_ATTACK:
			FxMgr.Compressor.fAttack = m_TBAttack.GetPos() / 100.f;
			FxMgr.ApplyParam(FXI_COMPRESSOR);
			return 0;
		case IDC_TB_RELEASE:
			FxMgr.Compressor.fRelease = (float)m_TBRelease.GetPos();
			FxMgr.ApplyParam(FXI_COMPRESSOR);
			return 0;
		case IDC_TB_THRESHOLD:
			FxMgr.Compressor.fThreshold = m_TBThreshold.GetPos() / 10.f;
			FxMgr.ApplyParam(FXI_COMPRESSOR);
			return 0;
		case IDC_TB_RATIO:
			FxMgr.Compressor.fRatio = (float)m_TBRatio.GetPos();
			FxMgr.ApplyParam(FXI_COMPRESSOR);
			return 0;
		case IDC_TB_PREDELAY:
			FxMgr.Compressor.fPredelay = m_TBPreDelay.GetPos() / 100.f;
			FxMgr.ApplyParam(FXI_COMPRESSOR);
			return 0;
		}
	}
	break;

	case WM_NCPAINT:
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CDlgFxCompression::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_CBEnable.AttachNew(GetDlgItem(hDlg, IDC_CB_ENABLE));
	m_BTReset.AttachNew(GetDlgItem(hDlg, IDC_BT_RESET));

	m_TBGain.AttachNew(GetDlgItem(hDlg, IDC_TB_GAIN));
	m_TBAttack.AttachNew(GetDlgItem(hDlg, IDC_TB_ATTACK));
	m_TBRelease.AttachNew(GetDlgItem(hDlg, IDC_TB_RELEASE));
	m_TBThreshold.AttachNew(GetDlgItem(hDlg, IDC_TB_THRESHOLD));
	m_TBRatio.AttachNew(GetDlgItem(hDlg, IDC_TB_RATIO));
	m_TBPreDelay.AttachNew(GetDlgItem(hDlg, IDC_TB_PREDELAY));

	m_TBGain.SetRange32(-600, 600);// 缩小10倍
	m_TBAttack.SetRange32(1, 50000);// 缩小100倍
	m_TBRelease.SetRange32(50, 3000);
	m_TBThreshold.SetRange32(-600, 0);// 缩小10倍
	m_TBRatio.SetRange32(1, 100);
	m_TBPreDelay.SetRange32(0, 400);// 缩小100倍

	m_CBEnable.SetCheckState(App->GetPlayer().GetEffectMgr().IsValid(FXI_COMPRESSOR));
	UpdateCtrl();

	SetFocus(m_CBEnable.HWnd);
	return FALSE;
}