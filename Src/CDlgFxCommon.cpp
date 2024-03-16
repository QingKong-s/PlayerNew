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
	swprintf(sz, L"%d", (int)f);
	m_STPan.SetText(sz);

	f = Bass.GetAttr(BASS_ATTRIB_VOL) * 100.f;
	m_TBVol.SetPos((int)f);
	swprintf(sz, L"%d", (int)f);
	m_STVol.SetText(sz);

	f = Bass.GetAttr(BASS_ATTRIB_TEMPO_PITCH);
	m_TBPitch.SetPos((int)f * 10.f);
	swprintf(sz, L"%d", (int)f);
	m_STPitch.SetText(sz);
}

void CDlgFxCommon::UpdateDpi(int iDpi)
{
	eck::CStatic st{};

	auto pBmp = App->ScaleImageForButton(IIDX_Tempo, iDpi);
	DestroyIcon(m_hiTempo);
	m_hiTempo = eck::CreateHICON(pBmp);
	pBmp->Release();

	pBmp = App->ScaleImageForButton(IIDX_Speed, iDpi);
	DestroyIcon(m_hiSpeed);
	m_hiSpeed = eck::CreateHICON(pBmp);
	pBmp->Release();

	pBmp = App->ScaleImageForButton(IIDX_Balance, iDpi);
	DestroyIcon(m_hiPan);
	m_hiPan = eck::CreateHICON(pBmp);
	pBmp->Release();

	pBmp = App->ScaleImageForButton(IIDX_Speaker, iDpi);
	DestroyIcon(m_hiVol);
	m_hiVol = eck::CreateHICON(pBmp);
	pBmp->Release();

	pBmp = App->ScaleImageForButton(IIDX_Pitch, iDpi);
	DestroyIcon(m_hiPitch);
	m_hiPitch = eck::CreateHICON(pBmp);
	pBmp->Release();

	pBmp = App->ScaleImageForButton(IIDX_Silent, iDpi);
	DestroyIcon(m_hiSilent);
	m_hiSilent = eck::CreateHICON(pBmp);
	pBmp->Release();

	st.Attach(GetDlgItem(HWnd, IDC_ST_TEMPOTIP));
	st.ModifyStyle(SS_REALSIZEIMAGE | SS_ICON, SS_REALSIZEIMAGE | SS_ICON);
	st.SetIcon(m_hiTempo);
	(void)st.Detach();

	st.Attach(GetDlgItem(HWnd, IDC_ST_SPEEDTIP));
	st.ModifyStyle(SS_REALSIZEIMAGE | SS_ICON, SS_REALSIZEIMAGE | SS_ICON);
	st.SetIcon(m_hiSpeed);
	(void)st.Detach();

	st.Attach(GetDlgItem(HWnd, IDC_ST_PANTIP));
	st.ModifyStyle(SS_REALSIZEIMAGE | SS_ICON, SS_REALSIZEIMAGE | SS_ICON);
	st.SetIcon(m_hiPan);
	(void)st.Detach();

	m_BTVol.SetImage(App->GetOptionsMgr().PlaySilent ? m_hiSilent : m_hiVol, IMAGE_ICON);

	st.Attach(GetDlgItem(HWnd, IDC_ST_PITCHTIP));
	st.ModifyStyle(SS_REALSIZEIMAGE | SS_ICON, SS_REALSIZEIMAGE | SS_ICON);
	st.SetIcon(m_hiPitch);
	(void)st.Detach();
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
				Bass.SetAttr(BASS_ATTRIB_TEMPO_PITCH, 0.f);
				UpdateCtrl();
			}
			return 0;

			case IDC_BT_VOL:
			{
				auto& om = App->GetOptionsMgr();
				auto& Bass = App->GetPlayer().GetBass();
				ECKBOOLNOT(om.PlaySilent);
				if (om.PlaySilent)// TODO: 静音
				{
					m_BTVol.SetImage(m_hiSilent, IMAGE_ICON);
					//Bass.SetAttr(BASS_ATTRIB_VOL, 0.f);
				}
				else
				{
					m_BTVol.SetImage(m_hiVol, IMAGE_ICON);
				}
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
			swprintf(sz, L"%d", (int)f);
			m_STPan.SetText(sz);
			return 0;
		case IDC_TB_VOL:
			f = (float)m_TBVol.GetPos();
			Bass.SetAttr(BASS_ATTRIB_VOL, f / 100.f);
			swprintf(sz, L"%d", (int)f);
			m_STVol.SetText(sz);
			return 0;
		case IDC_TB_PITCH:
			f = m_TBPitch.GetPos() / 10.f;
			Bass.SetAttr(BASS_ATTRIB_TEMPO_PITCH, f);
			swprintf(sz, L"%d", (int)f);
			m_STPitch.SetText(sz);
			return 0;
		}
	}
	break;

	case WM_NCPAINT:
		return 0;

	case WM_SHOWWINDOW:
	{
		if (wParam)
			m_STVu.Resume();
		else
			m_STVu.Pause();
	}
	break;

	case WM_DPICHANGED_AFTERPARENT:
		UpdateDpi(eck::GetDpi(hWnd));
		break;

	case WM_DESTROY:
		DestroyIcon(m_hiTempo);
		DestroyIcon(m_hiSpeed);
		DestroyIcon(m_hiPan);
		DestroyIcon(m_hiVol);
		DestroyIcon(m_hiPitch);
		DestroyIcon(m_hiSilent);
		break;
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
	m_TBPitch.AttachNew(GetDlgItem(hDlg, IDC_TB_PITCH));

	m_STTempo.AttachNew(GetDlgItem(hDlg, IDC_ST_TEMPO));
	m_STSpeed.AttachNew(GetDlgItem(hDlg, IDC_ST_SPEED));
	m_STPan.AttachNew(GetDlgItem(hDlg, IDC_ST_PAN));
	m_STVol.AttachNew(GetDlgItem(hDlg, IDC_ST_VOL));
	m_STPitch.AttachNew(GetDlgItem(hDlg, IDC_ST_PITCH));
	m_STVu.AttachNew(GetDlgItem(hDlg, IDC_ST_VU));
	m_STVu.Init();

	m_BTVol.AttachNew(GetDlgItem(hDlg, IDC_BT_VOL));

	m_LBFxPriority.AttachNew(GetDlgItem(hDlg, IDC_LB_FXPRIORITY));

	UpdateDpi(eck::GetDpi(hDlg));

	m_TBTempo.SetRange32(-95, 5000);// 缩小100倍
	m_TBSpeed.SetRange32(0, 500);// 缩小100倍
	m_TBPan.SetRange32(-100, 100);// 缩小100倍
	m_TBVol.SetRange32(0, 200);// 缩小100倍
	m_TBPitch.SetRange32(-600, 600);// 缩小10倍

	m_LBFxPriority.SetRedraw(FALSE);
	EckCounter(FXI_MAX, i)
		m_LBFxPriority.AddString(CEffectMgr::Fxi2Name(i));
	m_LBFxPriority.SetRedraw(TRUE);

	UpdateCtrl();
	return FALSE;
}

void CUvBlock::PaintContent(HDC hDC)
{
	const RECT rc{ 0,0,m_cxClient,m_cyClient };
	eck::FillGradientRect(
		hDC,
		rc,
		RGB(0, 255 * 7 / 10, 0),
		RGB(255 * 7 / 10, 0, 0),
		FALSE);// 暗色背景

	if (!App->GetPlayer().IsFileActive())
		return;
	auto& Bass = App->GetPlayer().GetBass();

	m_Time[0].uTime += TE_TIMER;
	if (m_Time[0].uTime >= 500)
	{
		m_Time[0].b = TRUE;
		m_Time[0].uTime = 0;
	}
	m_Time[1].uTime += TE_TIMER;
	if (m_Time[1].uTime >= 500)
	{
		m_Time[1].b = TRUE;
		m_Time[1].uTime = 0;
	}

	int cxRight, cxLeft;
	HRGN hRgn1, hRgn2, hRgn3;

	int iStep = 10, iStep2 = 15;
	const DWORD dwLevel = Bass.GetLevel();
	cxRight = m_cxClient * HIWORD(dwLevel) / 32768;// 右声道
	if (cxRight > m_cxRightOld)
		m_cxRightOld = cxRight;
	else
		m_cxRightOld -= iStep;
	//------------
	if (m_cxRightOld < 3)
		m_cxRightOld = 3;
	//------------
	if (m_Time[0].b)
		m_Time[0].y -= iStep2;
	//------------
	if (m_cxRightOld > m_Time[0].y)
	{
		m_Time[0].y = m_cxRightOld;
		m_Time[0].b = FALSE;
		m_Time[0].uTime = 0;
	}
	//------------
	if (m_Time[0].y < 3)
		m_Time[0].y = 3;
	hRgn1 = CreateRectRgn(0, 0, m_cxRightOld, m_cyClient / 2);// 上区域
	hRgn2 = CreateRectRgn(m_Time[0].y - 3, 0, m_Time[0].y, m_cyClient / 2);// 峰值
	CombineRgn(hRgn1, hRgn1, hRgn2, RGN_OR);// 合并上区域和上峰值
	DeleteObject(hRgn2);

	cxLeft = m_cxClient * LOWORD(dwLevel) / 32768;// 左声道
	if (cxLeft > m_cxLeftOld)
		m_cxLeftOld = cxLeft;
	else
		m_cxLeftOld -= iStep;
	//------------
	if (m_cxLeftOld < 3)
		m_cxLeftOld = 3;
	//------------
	if (m_Time[1].b)
		m_Time[1].y -= iStep2;
	//------------
	if (m_cxLeftOld > m_Time[1].y)
	{
		m_Time[1].y = m_cxLeftOld;
		m_Time[1].b = FALSE;
		m_Time[1].uTime = 0;
	}
	//------------
	if (m_Time[1].y < 3)
		m_Time[1].y = 3;
	hRgn2 = CreateRectRgn(0, m_cyClient / 2, m_cxLeftOld, m_cyClient);//下区域
	hRgn3 = CreateRectRgn(m_Time[1].y - 3, m_cyClient / 2, m_Time[1].y, m_cyClient);//峰值
	CombineRgn(hRgn2, hRgn2, hRgn3, RGN_OR);// 下区域和下峰值
	DeleteObject(hRgn3);

	CombineRgn(hRgn1, hRgn1, hRgn2, RGN_OR);// 合并上下
	DeleteObject(hRgn2);
	SelectClipRgn(hDC, hRgn1);
	DeleteObject(hRgn1);

	eck::FillGradientRect(hDC, rc, eck::Colorref::Green, eck::Colorref::Red, FALSE);// 前景
	SelectClipRgn(hDC, NULL);//清除剪辑区
}

LRESULT CUvBlock::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
		m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		eck::BitBltPs(&ps, m_DC.GetDC());
		EndPaint(hWnd, &ps);
	}
	return 0;
	case WM_TIMER:
	{
		PaintContent(m_DC.GetDC());
		const HDC hDC = GetDC(hWnd);
		BitBlt(hDC, 0, 0, m_cxClient, m_cyClient, m_DC.GetDC(), 0, 0, SRCCOPY);
		ReleaseDC(hWnd, hDC);
	}
	return 0;
	}

	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}