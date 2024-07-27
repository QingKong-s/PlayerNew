#include "pch.h"
#include "CDlgFxRotate.h"

void CDlgFxRotate::UpdateCtrl()
{
	const auto& Param = App->GetPlayer().GetEffectMgr().Rotate;
	m_TBSpeed.SetPos((int)(Param.fRate * 100.f));
}

LRESULT CDlgFxRotate::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
					Player.SetFx(FXI_ROTATE);
				else
					Player.RemoveFx(FXI_ROTATE);
			}
			return 0;
			case IDC_BT_RESET:
			{
				FxMgr.ResetParam(FXI_ROTATE);
				FxMgr.ApplyParam(FXI_ROTATE);
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
		case IDC_TB_SPEED:
			FxMgr.Rotate.fRate = m_TBSpeed.GetPos() / 100.f;
			FxMgr.ApplyParam(FXI_ROTATE);
			return 0;
		}
	}
	break;

	case WM_NCPAINT:
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CDlgFxRotate::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_CBEnable.AttachNew(GetDlgItem(hDlg, IDC_CB_ENABLE));
	m_BTReset.AttachNew(GetDlgItem(hDlg, IDC_BT_RESET));

	m_TBSpeed.AttachNew(GetDlgItem(hDlg, IDC_TB_SPEED));

	m_TBSpeed.SetRange32(-200, 200);// 缩小100倍

	m_CBEnable.SetCheckState(App->GetPlayer().GetEffectMgr().IsValid(FXI_ROTATE));
	UpdateCtrl();

	SetFocus(m_CBEnable.HWnd);
	return FALSE;
}