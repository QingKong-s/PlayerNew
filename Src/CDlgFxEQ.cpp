#include "CDlgFxEQ.h"

void CDlgFxEQ::UpdateCtrl()
{
	const auto& Param = App->GetPlayer().GetEffectMgr().EQ;
	m_TBBandWidth.SetPos((int)(Param[0].fBandwidth * 10.f));
	EckCounter(ARRAYSIZE(m_TB), i)
		m_TB[i].SetPos((int)(Param[i].fGain * 100.f));
	m_CBBPresetEQ.SetCurSel(App->GetPlayer().GetEffectMgr().idxPresetEQ);
}

LRESULT CDlgFxEQ::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
					Player.SetFx(FXI_PARAMEQ);
				else
					Player.RemoveFx(FXI_PARAMEQ);
			}
			return 0;
			case IDC_BT_RESET:
			{
				FxMgr.ResetParam(FXI_PARAMEQ);
				FxMgr.ApplyParam(FXI_PARAMEQ);
				UpdateCtrl();
			}
			return 0;
			}
		}
		return 0;

		case CBN_SELCHANGE:
		{
			if (LOWORD(wParam) == IDC_CB_EQ)
			{
				const int idxCurr = m_CBBPresetEQ.GetCurSel();
				FxMgr.idxPresetEQ = idxCurr;
				EckCounter(ARRAYSIZE(FxMgr.EQ), i)
					FxMgr.EQ[i].fGain = CEffectMgr::EQSetting[idxCurr].Setting[i];
				FxMgr.ApplyParam(FXI_PARAMEQ);
				UpdateCtrl();
			}
		}
		return 0;
		}
	}
	break;

	case WM_VSCROLL:
	{
		auto& FxMgr = App->GetPlayer().GetEffectMgr();
		const int iID = GetDlgCtrlID((HWND)lParam);
		switch (iID)
		{
		case IDC_TB_BANDWIDTH:
		{
			for (auto& e : FxMgr.EQ)
				e.fBandwidth = m_TBBandWidth.GetPos() / 10.f;
			FxMgr.ApplyParam(FXI_PARAMEQ);
		}
		return 0;
		case IDC_TB1: case IDC_TB2: case IDC_TB3: case IDC_TB4: case IDC_TB5:
		case IDC_TB6: case IDC_TB7: case IDC_TB8: case IDC_TB9: case IDC_TB10:
		{
			const static std::unordered_map<int, int> s_map
			{
				{IDC_TB1,0},{IDC_TB2,1},{IDC_TB3,2},{IDC_TB4,3},{IDC_TB5,4},
				{IDC_TB6,5},{IDC_TB7,6},{IDC_TB8,7},{IDC_TB9,8},{IDC_TB10,9},
			};

			const int idx = s_map.at(iID);
			FxMgr.EQ[idx].fGain = m_TB[idx].GetPos() / 100.f;
			FxMgr.ApplyParam(FXI_PARAMEQ);
		}
		return 0;
		}
	}
	break;

	case WM_NCPAINT:
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CDlgFxEQ::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_CBEnable.AttachNew(GetDlgItem(hDlg, IDC_CB_ENABLE));
	m_BTReset.AttachNew(GetDlgItem(hDlg, IDC_BT_RESET));

	m_TBBandWidth.AttachNew(GetDlgItem(hDlg, IDC_TB_BANDWIDTH));
	m_TBBandWidth.SetRange32(10, 360, FALSE);// 缩小10倍

	constexpr static int c_idTB[]
	{
		IDC_TB1,IDC_TB2,IDC_TB3,IDC_TB4,IDC_TB5,
		IDC_TB6,IDC_TB7,IDC_TB8,IDC_TB9,IDC_TB10
	};
	static_assert(ARRAYSIZE(m_TB) == ARRAYSIZE(c_idTB));

	EckCounter(ARRAYSIZE(m_TB), i)
	{
		m_TB[i].AttachNew(GetDlgItem(hDlg, c_idTB[i]));
		m_TB[i].SetRange32(-1500, 1500);// 缩小100倍
	}

	m_CBBPresetEQ.AttachNew(GetDlgItem(hDlg, IDC_CB_EQ));
	for (const auto& e : CEffectMgr::EQSetting)
		m_CBBPresetEQ.AddString(e.pszText);

	m_CBEnable.SetCheckState(App->GetPlayer().GetEffectMgr().IsValid(FXI_PARAMEQ));
	UpdateCtrl();

	SetFocus(m_CBEnable.HWnd);
	return FALSE;
}