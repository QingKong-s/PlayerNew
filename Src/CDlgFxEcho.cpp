#include "CDlgFxEcho.h"

void CDlgFxEcho::UpdateCtrl()
{
	const auto& Param = App->GetPlayer().GetEffectMgr().Echo;
	m_TBWetDryMix.SetPos((int)(Param.fWetDryMix));
	m_TBFeedback.SetPos((int)(Param.fFeedback));
	m_TBLDelay.SetPos((int)(Param.fLeftDelay));
	m_TBRDelay.SetPos((int)(Param.fRightDelay));
	m_CBPanDelay.SetCheckState(Param.lPanDelay);
}

LRESULT CDlgFxEcho::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
					Player.SetFx(FXI_ECHO);
				else
					Player.RemoveFx(FXI_ECHO);
			}
			return 0;
			case IDC_BT_RESET:
			{
				FxMgr.ResetParam(FXI_ECHO);
				FxMgr.ApplyParam(FXI_ECHO);
				UpdateCtrl();
			}
			return 0;

			case IDC_CB_PANDELAY:
			{
				FxMgr.Echo.lPanDelay = m_CBPanDelay.GetCheckState();
				FxMgr.ApplyParam(FXI_ECHO);
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
		case IDC_TB_WETDRYMIX:
			FxMgr.Echo.fWetDryMix = (float)m_TBWetDryMix.GetPos();
			FxMgr.ApplyParam(FXI_ECHO);
			return 0;
		case IDC_TB_FEEDBACK:
			FxMgr.Echo.fFeedback = (float)m_TBFeedback.GetPos();
			FxMgr.ApplyParam(FXI_ECHO);
			return 0;
		case IDC_TB_LEFTDELAY:
			FxMgr.Echo.fLeftDelay = (float)m_TBLDelay.GetPos();
			FxMgr.ApplyParam(FXI_ECHO);
			return 0;
		case IDC_TB_RIGHTDELAY:
			FxMgr.Echo.fRightDelay = (float)m_TBRDelay.GetPos();
			FxMgr.ApplyParam(FXI_ECHO);
			return 0;
		}
	}
	break;

	case WM_NCPAINT:
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CDlgFxEcho::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_CBEnable.AttachNew(GetDlgItem(hDlg, IDC_CB_ENABLE));
	m_BTReset.AttachNew(GetDlgItem(hDlg, IDC_BT_RESET));

	m_TBWetDryMix.AttachNew(GetDlgItem(hDlg, IDC_TB_WETDRYMIX));
	m_TBFeedback.AttachNew(GetDlgItem(hDlg, IDC_TB_FEEDBACK));
	m_TBLDelay.AttachNew(GetDlgItem(hDlg, IDC_TB_LEFTDELAY));
	m_TBRDelay.AttachNew(GetDlgItem(hDlg, IDC_TB_RIGHTDELAY));
	m_CBPanDelay.AttachNew(GetDlgItem(hDlg, IDC_CB_PANDELAY));

	m_TBWetDryMix.SetRange32(0, 100, FALSE);
	m_TBFeedback.SetRange32(0, 100, FALSE);
	m_TBLDelay.SetRange32(1, 2000, FALSE);
	m_TBRDelay.SetRange32(1, 2000, FALSE);

	m_CBEnable.SetCheckState(App->GetPlayer().GetEffectMgr().IsValid(FXI_ECHO));
	UpdateCtrl();

	SetFocus(m_CBEnable.HWnd);
	return FALSE;
}