#include "CDlgFxCommon.h"

void CDlgFxCommon::UpdateCtrl()
{
	auto& Bass = App->GetPlayer().GetBass();
	WCHAR sz[32];

	auto f = Bass.GetAttr(BASS_ATTRIB_TEMPO) * 100.f;
	m_TBTempo.SetPos((int)f);
	swprintf(sz, L"%.0f%%", f);
	m_STTempo.SetText(sz);

	f = Bass.GetSpeed();
	m_TBSpeed.SetPos((int)(f * 100.f));
	swprintf(sz, L"x%.2f", f);
	m_STSpeed.SetText(sz);

	f = Bass.GetAttr(BASS_ATTRIB_PAN) * 100.f;
	m_TBPan.SetPos((int)f);
	swprintf(sz, L"%2d", (int)f);
	m_STPan.SetText(sz);

	f = Bass.GetAttr(BASS_ATTRIB_VOL) * 100.f;
	m_TBVol.SetPos((int)f);
	swprintf(sz, L"%3d", (int)f);
	m_STVol.SetText(sz);
}

LRESULT CDlgFxCommon::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
			case IDC_BT_RESET:
			{
				auto& Bass = App->GetPlayer().GetBass();
				Bass.SetAttr(BASS_ATTRIB_TEMPO, 1.f);
				Bass.SetSpeed(1.f);
				Bass.SetAttr(BASS_ATTRIB_PAN, 0.f);
				Bass.SetAttr(BASS_ATTRIB_VOL, 1.f);

				UpdateCtrl();
			}
			return 0;

			case IDC_BT_VOL:
			{

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
		auto& Bass = App->GetPlayer().GetBass();
		float f;
		WCHAR sz[32];
		switch (GetDlgCtrlID((HWND)lParam))
		{
		case IDC_TB_TEMPO:
			f = (float)m_TBTempo.GetPos();
			Bass.SetAttr(BASS_ATTRIB_TEMPO, f / 100.f);
			swprintf(sz, L"%.0f%%", f);
			m_STTempo.SetText(sz);
			return 0;
		case IDC_TB_SPEED:
			f = m_TBSpeed.GetPos() / 100.f;
			Bass.SetSpeed(f);
			swprintf(sz, L"x%.2f", f);
			m_STSpeed.SetText(sz);
			return 0;
		case IDC_TB_PAN:
			f = (float)m_TBPan.GetPos();
			Bass.SetAttr(BASS_ATTRIB_PAN, f / 100.f);
			swprintf(sz, L"%2d", (int)f);
			m_STPan.SetText(sz);
			return 0;
		case IDC_TB_VOL:
			f = (float)m_TBVol.GetPos();
			Bass.SetAttr(BASS_ATTRIB_VOL, f / 100.f);
			swprintf(sz, L"%3d", (int)f);
			m_STVol.SetText(sz);
			return 0;
		}
	}
	break;

	case WM_NCPAINT:
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CDlgFxCommon::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_BTReset.AttachNew(GetDlgItem(hDlg, IDC_BT_RESET));

	m_TBTempo.AttachNew(GetDlgItem(hDlg, IDC_TB_TEMPO));
	m_TBSpeed.AttachNew(GetDlgItem(hDlg, IDC_TB_SPEED));
	m_TBPan.AttachNew(GetDlgItem(hDlg, IDC_TB_PAN));
	m_TBVol.AttachNew(GetDlgItem(hDlg, IDC_TB_VOL));

	m_STTempo.AttachNew(GetDlgItem(hDlg, IDC_ST_TEMPO));
	m_STSpeed.AttachNew(GetDlgItem(hDlg, IDC_ST_SPEED));
	m_STPan.AttachNew(GetDlgItem(hDlg, IDC_ST_PAN));
	m_STVol.AttachNew(GetDlgItem(hDlg, IDC_ST_VOL));
	m_STVu.AttachNew(GetDlgItem(hDlg, IDC_ST_VU));

	m_BTVol.AttachNew(GetDlgItem(hDlg, IDC_BT_VOL));

	m_LBFxPriority.AttachNew(GetDlgItem(hDlg, IDC_LB_FXPRIORITY));

	m_TBTempo.SetRange32(-95, 5000);// 缩小100倍
	m_TBSpeed.SetRange32(0, 500);// 缩小100倍
	m_TBPan.SetRange32(-100, 100);// 缩小100倍
	m_TBVol.SetRange32(0, 200);// 缩小100倍

	m_LBFxPriority.SetRedraw(FALSE);
	EckCounter(FXI_MAX, i)
		m_LBFxPriority.AddString(CEffectMgr::Fxi2Name(i));
	m_LBFxPriority.SetRedraw(TRUE);

	UpdateCtrl();
	return FALSE;
}