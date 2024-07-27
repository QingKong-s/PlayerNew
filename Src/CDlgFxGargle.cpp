#include "pch.h"
#include "CDlgFxGargle.h"

void CDlgFxGargle::UpdateCtrl()
{
    const auto& Param = App->GetPlayer().GetEffectMgr().Gargle;

    m_CBEnable.SetCheckState(App->GetPlayer().GetEffectMgr().IsValid(FXI_GARGLE));
    m_TBRate.SetPos((int)(Param.dwRateHz));

    if (Param.dwWaveShape == 1)
    {
        m_RBPhaseSine.SetCheckState(1);
        m_RBPhaseQuad.SetCheckState(0);
    }
    else
    {
        m_RBPhaseSine.SetCheckState(0);
        m_RBPhaseQuad.SetCheckState(1);
    }
}

LRESULT CDlgFxGargle::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    Player.SetFx(FXI_GARGLE);
                else
                    Player.RemoveFx(FXI_GARGLE);
            }
            return 0;
            case IDC_BT_RESET:
            {
                FxMgr.ResetParam(FXI_GARGLE);
                FxMgr.ApplyParam(FXI_GARGLE);
                UpdateCtrl();
            }
            return 0;

            case IDC_RB_SINEWAVE:
            case IDC_RB_TRIANGLEWAVE:
                FxMgr.Gargle.dwWaveShape = !!m_RBPhaseSine.GetCheckState();
                FxMgr.ApplyParam(FXI_GARGLE);
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
        case IDC_TB_RATEHZ:
            FxMgr.Gargle.dwRateHz = m_TBRate.GetPos();
            FxMgr.ApplyParam(FXI_GARGLE);
            return 0;
        }
    }
    break;

    case WM_NCPAINT:
        return 0;
    }
    return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CDlgFxGargle::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
    m_CBEnable.AttachNew(GetDlgItem(hDlg, IDC_CB_ENABLE));
    m_BTReset.AttachNew(GetDlgItem(hDlg, IDC_BT_RESET));

    m_TBRate.AttachNew(GetDlgItem(hDlg, IDC_TB_RATEHZ));
    m_RBPhaseSine.AttachNew(GetDlgItem(hDlg, IDC_RB_SINEWAVE));
    m_RBPhaseQuad.AttachNew(GetDlgItem(hDlg, IDC_RB_QUADWAVE));

    m_CBEnable.SetCheckState(App->GetPlayer().GetEffectMgr().IsValid(FXI_GARGLE));
    m_TBRate.SetRange32(0, 1000);

    UpdateCtrl();

    SetFocus(m_CBEnable.HWnd);
    return FALSE;
}