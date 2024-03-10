#include "CDlgChorus.h"

void CDlgChorus::UpdateCtrl()
{
    const auto& Param = App->GetPlayer().GetEffectMgr().Chorus;

    m_CBEnable.SetCheckState(App->GetPlayer().GetEffectMgr().IsValid(FXI_CHORUS));
    m_TBWetDryMix.SetPos((int)(Param.fWetDryMix * 10.f));
    m_TBDepth.SetPos((int)(Param.fDepth * 10.f));
    m_TBFeedback.SetPos((int)(Param.fFeedback * 10.f));
    m_TBFreq.SetPos((int)(Param.fFrequency * 10.f));

    if (Param.lWaveform == 1)
    {
        m_RBPhaseSine.SetCheckState(1);
        m_RBPhaseTriangle.SetCheckState(0);
    }
    else
    {
        m_RBPhaseSine.SetCheckState(0);
        m_RBPhaseTriangle.SetCheckState(1);
    }

    m_TBDelay.SetPos((int)(Param.fDelay * 10.f));
    m_CBBPhase.SetCurSel(Param.lPhase);
}

LRESULT CDlgChorus::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    Player.SetFx(FXI_CHORUS);
                else
                    Player.RemoveFx(FXI_CHORUS);
			}
            return 0;
            case IDC_BT_RESET:
            {
                FxMgr.ResetParam(FXI_CHORUS);
                FxMgr.ApplyParam(FXI_CHORUS);
                UpdateCtrl();
            }
            return 0;

            case IDC_RB_SINEWAVE:
            case IDC_RB_TRIANGLEWAVE:
                FxMgr.Chorus.lWaveform = !!m_RBPhaseSine.GetCheckState();
                FxMgr.ApplyParam(FXI_CHORUS);
                return 0;
			}
		}
		return 0;
		case CBN_SELCHANGE:
		{
			if (LOWORD(wParam) == IDC_CB_LFOPHASE)
			{
				FxMgr.Chorus.lPhase = m_CBBPhase.GetCurSel();
                FxMgr.ApplyParam(FXI_CHORUS);
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
            FxMgr.Chorus.fWetDryMix = m_TBWetDryMix.GetPos() / 10.f;
            FxMgr.ApplyParam(FXI_CHORUS);
            return 0;
        case IDC_TB_LFODEPTH:
            FxMgr.Chorus.fDepth = m_TBDepth.GetPos() / 10.f;
            FxMgr.ApplyParam(FXI_CHORUS);
            return 0;
        case IDC_TB_FEEDBACK:
            FxMgr.Chorus.fFeedback = m_TBFeedback.GetPos() / 10.f;
            FxMgr.ApplyParam(FXI_CHORUS);
            return 0;
        case IDC_TB_LFOFREQUENCY:
            FxMgr.Chorus.fFrequency = m_TBFreq.GetPos() / 10.f;
            FxMgr.ApplyParam(FXI_CHORUS);
            return 0;
        case IDC_TB_DELAY:
            FxMgr.Chorus.fDelay = m_TBDelay.GetPos() / 10.f;
            FxMgr.ApplyParam(FXI_CHORUS);
            return 0;
        }
    }
    break;

    case WM_NCPAINT:
        return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CDlgChorus::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
    m_CBEnable.AttachNew(GetDlgItem(hDlg, IDC_CB_ENABLE));
    m_BTReset.AttachNew(GetDlgItem(hDlg, IDC_BT_RESET));

    m_TBWetDryMix.AttachNew(GetDlgItem(hDlg, IDC_TB_WETDRYMIX));
    m_TBDepth.AttachNew(GetDlgItem(hDlg, IDC_TB_LFODEPTH));
    m_TBFeedback.AttachNew(GetDlgItem(hDlg, IDC_TB_FEEDBACK));
    m_TBFreq.AttachNew(GetDlgItem(hDlg, IDC_TB_LFOFREQUENCY));
    m_RBPhaseSine.AttachNew(GetDlgItem(hDlg, IDC_RB_SINEWAVE));
    m_RBPhaseTriangle.AttachNew(GetDlgItem(hDlg, IDC_RB_TRIANGLEWAVE));
    m_TBDelay.AttachNew(GetDlgItem(hDlg, IDC_TB_DELAY));
    m_CBBPhase.AttachNew(GetDlgItem(hDlg, IDC_CB_LFOPHASE));

    m_CBEnable.SetCheckState(App->GetPlayer().GetEffectMgr().IsValid(FXI_CHORUS));
    m_TBWetDryMix.SetRange32(0, 1000);// 缩小10倍
    m_TBDepth.SetRange32(0, 1000);// 缩小10倍
    m_TBFeedback.SetRange32(-990, 990);// 缩小10倍
    m_TBFreq.SetRange32(0, 100);// 缩小10倍
    m_TBDelay.SetRange32(0, 400);// 缩小10倍

    // same with index
    m_CBBPhase.AddString(L"-180");
    m_CBBPhase.AddString(L"-90");
    m_CBBPhase.AddString(L"0");
    m_CBBPhase.AddString(L"90");
    m_CBBPhase.AddString(L"180");

    UpdateCtrl();

    SetFocus(m_CBEnable.HWnd);
    return FALSE;
}