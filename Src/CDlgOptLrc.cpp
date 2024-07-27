#include "pch.h"
#include "CDlgOptLrc.h"

void CDlgOptLrc::DataToCtrl()
{
	auto& om = App->GetOptionsMgr();

	m_CBBLrcEncoding.SetCurSel((int)om.LrcFileEncoding);
	// TODO: 歌词目录

	m_CBLsAnimation.SetCheckState(om.ScLrcAnimation);
	m_CBLsAutoWrap.SetCheckState(om.ScLrcAutoWrap);
	m_CBLsShowTranslation.SetCheckState(om.ScLrcShowTranslation);
	m_CBLsSpaceLine.SetCheckState(om.ScLrcSpaceLine);
	m_EDLsSpaceLine.SetText(om.ScLrcSpaceLineText.Data());

	m_CPBLsNormal.SetColor(eck::ARGBToColorref(om.ScLrcFontMain.argbNormal));
	m_CPBLsHilight.SetColor(eck::ARGBToColorref(om.ScLrcFontMain.argbHiLight));
	m_EDLsLinePadding.SetText(std::to_wstring(om.ScLrcPaddingHeight).c_str());
	m_CBBLsAlign.SetCurSel(om.ScLrcAlign);

	m_FPLsMain.FromInfo(om.ScLrcFontMain);
	m_FPLsTranslation.FromInfo(om.ScLrcFontTranslation);


	m_CBDlBorder.SetCheckState(om.DtLrcBorder);
	m_CPBDlBorder.SetColor(eck::ARGBToColorref(om.DtLrcBorderColor));
	m_CBDlShadow.SetCheckState(om.DtLrcEnableShadow);
	m_EDDlShadowOffset.SetText(std::to_wstring(om.DtLrcShadowOffset).c_str());
	m_CBDlShowWhenStartup.SetCheckState(om.DtLrcShow);
	m_CBDlSpaceLine.SetCheckState(om.DtLrcSpaceLine);
	m_EDDlSpaceLine.SetText(om.DtLrcSpaceLineText.Data());
	m_CBDlShowTranslation.SetCheckState(om.DtLrcShowTranslation);

	m_FPDlMain.FromInfo(om.DtLrcFontMain);
	m_CPBDlMainNormal1.SetColor(eck::ARGBToColorref(om.DtLrcFontMain.argbNormalGra[0]));
	m_CPBDlMainNormal2.SetColor(eck::ARGBToColorref(om.DtLrcFontMain.argbNormalGra[1]));
	m_CPBDlMainHilight1.SetColor(eck::ARGBToColorref(om.DtLrcFontMain.argbHiLightGra[0]));
	m_CPBDlMainHilight2.SetColor(eck::ARGBToColorref(om.DtLrcFontMain.argbHiLightGra[1]));

	m_FPDlTranslation.FromInfo(om.DtLrcFontTranslation);
	m_CPBDlTranslationNormal1.SetColor(eck::ARGBToColorref(om.DtLrcFontTranslation.argbNormalGra[0]));
	m_CPBDlTranslationNormal2.SetColor(eck::ARGBToColorref(om.DtLrcFontTranslation.argbNormalGra[1]));
	m_CPBDlTranslationHilight1.SetColor(eck::ARGBToColorref(om.DtLrcFontTranslation.argbHiLightGra[0]));
	m_CPBDlTranslationHilight2.SetColor(eck::ARGBToColorref(om.DtLrcFontTranslation.argbHiLightGra[1]));

	m_TBDlAlpha.SetPos(om.DtLrcAlpha);

	m_CBBDlAlign1.SetCurSel(om.DtLrcAlign[0]);
	m_CBBDlAlign2.SetCurSel(om.DtLrcAlign[1]);

	m_CBBDlView.SetCurSel((int)om.DtLrcView);
}

void CDlgOptLrc::CtrlToData()
{
	auto& om = App->GetOptionsMgr();

	om.LrcFileEncoding = (Utils::LrcEncoding)m_CBBLrcEncoding.GetCurSel();
	// TODO: 歌词目录

	om.ScLrcAnimation = m_CBLsAnimation.GetCheckState();
	om.ScLrcAutoWrap = m_CBLsAutoWrap.GetCheckState();
	om.ScLrcShowTranslation = m_CBLsShowTranslation.GetCheckState();
	om.ScLrcSpaceLine = m_CBLsSpaceLine.GetCheckState();
	om.ScLrcSpaceLineText = m_EDLsSpaceLine.GetText();

	om.ScLrcFontMain.argbNormal = eck::ColorrefToARGB(m_CPBLsNormal.GetColor());
	om.ScLrcFontMain.argbHiLight = eck::ColorrefToARGB(m_CPBLsHilight.GetColor());
	om.ScLrcPaddingHeight = (float)_wtof(m_EDLsLinePadding.GetText().Data());
	om.ScLrcAlign = m_CBBLsAlign.GetCurSel();

	om.ScLrcFontMain.FromFontPicker(m_FPLsMain);
	om.ScLrcFontTranslation.FromFontPicker(m_FPLsTranslation);


	om.DtLrcBorder = m_CBDlBorder.GetCheckState();
	om.DtLrcBorderColor = eck::ColorrefToARGB(m_CPBDlBorder.GetColor());
	om.DtLrcEnableShadow = m_CBDlShadow.GetCheckState();
	om.DtLrcShadowOffset = (float)_wtof(m_EDDlShadowOffset.GetText().Data());
	om.DtLrcShow = m_CBDlShowWhenStartup.GetCheckState();
	om.DtLrcSpaceLine = m_CBDlSpaceLine.GetCheckState();
	om.DtLrcSpaceLineText = m_EDDlSpaceLine.GetText();
	om.DtLrcShowTranslation = m_CBDlShowTranslation.GetCheckState();

	om.DtLrcFontMain.FromFontPicker(m_FPDlMain);
	om.DtLrcFontMain.argbNormalGra[0] = eck::ColorrefToARGB(m_CPBDlMainNormal1.GetColor());
	om.DtLrcFontMain.argbNormalGra[1] = eck::ColorrefToARGB(m_CPBDlMainNormal2.GetColor());
	om.DtLrcFontMain.argbHiLightGra[0] = eck::ColorrefToARGB(m_CPBDlMainHilight1.GetColor());
	om.DtLrcFontMain.argbHiLightGra[1] = eck::ColorrefToARGB(m_CPBDlMainHilight2.GetColor());


	om.DtLrcFontTranslation.FromFontPicker(m_FPDlMain);
	om.DtLrcFontTranslation.argbNormalGra[0] = eck::ColorrefToARGB(m_CPBDlTranslationNormal1.GetColor());
	om.DtLrcFontTranslation.argbNormalGra[1] = eck::ColorrefToARGB(m_CPBDlTranslationNormal2.GetColor());
	om.DtLrcFontTranslation.argbHiLightGra[0] = eck::ColorrefToARGB(m_CPBDlTranslationHilight1.GetColor());
	om.DtLrcFontTranslation.argbHiLightGra[1] = eck::ColorrefToARGB(m_CPBDlTranslationHilight2.GetColor());

	om.DtLrcAlpha = m_TBDlAlpha.GetPos();

	om.DtLrcAlign[0] = m_CBBDlAlign1.GetCurSel();
	om.DtLrcAlign[1] = m_CBBDlAlign2.GetCurSel();

	om.DtLrcView = (DtLrcView)m_CBBDlView.GetCurSel();

}

BOOL CDlgOptLrc::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	eck::CWnd* const pWnd[]{ &m_CBBLrcEncoding,&m_LBLrcPath,&m_CBLsAnimation,&m_CBLsAutoWrap,
		&m_CBLsShowTranslation,&m_CBLsSpaceLine,&m_EDLsSpaceLine,&m_CPBLsNormal,&m_CPBLsHilight,&m_EDLsLinePadding,
		&m_CBBLsAlign,&m_FPLsMain,&m_FPLsTranslation,&m_CBDlBorder,&m_CPBDlBorder,&m_CBDlShadow,
		&m_EDDlShadowOffset,&m_CBDlShowWhenStartup,&m_CBDlSpaceLine,&m_EDDlSpaceLine,&m_CBDlShowTranslation,
		&m_FPDlMain,&m_CPBDlMainNormal1,&m_CPBDlMainNormal2,&m_CPBDlMainHilight1,&m_CPBDlMainHilight2,
		&m_FPDlTranslation,&m_CPBDlTranslationNormal1,&m_CPBDlTranslationNormal2,&m_CPBDlTranslationHilight1,
		&m_CPBDlTranslationHilight2,&m_TBDlAlpha,&m_CBBDlAlign1,&m_CBBDlAlign2,&m_CBBDlView };
	const int iId[]{ IDC_CB_DEFTEXTCODE,IDC_LV_LRCPATH,IDC_CB_SCLRCANIMATION,IDC_CB_SCLRCAUTOWRAP,
		IDC_CB_SCLRCTRANSLATION,IDC_CB_SCLRCSPACELINE,IDC_ED_SCLRCSPACELINE,IDC_ST_SCLRCCLR1,IDC_ST_SCLRCCLR2,IDC_ED_SCLRCLINEGAP,
		IDC_CB_SCLRCALIGN,IDC_ED_SCLRCFONT,IDC_ED_SCLRCFONT2,IDC_CB_DTLRCBORDER,IDC_ST_DTLRCBORDERCLR,IDC_CB_DTLRCSHANDOW,
		IDC_ED_DTLRCSHADOWOFFSET,IDC_CB_DTSHOWWHENSTARTUP,IDC_CB_DTLRCSPACELINE,IDC_ED_DTLRCSPACELINE,IDC_CB_DTLRCTRANSLATION,
		IDC_ED_DTLRCFONT,IDC_ST_DTLRCCLR1,IDC_ST_DTLRCCLR2,IDC_ST_DTLRCCLRHIGHLIGHT1,IDC_ST_DTLRCCLRHIGHLIGHT2,
		IDC_ED_DTLRCFONT2,IDC_ST_DTLRCCLR3,IDC_ST_DTLRCCLR4,IDC_ST_DTLRCCLRHIGHLIGHT3,
		IDC_ST_DTLRCCLRHIGHLIGHT4,IDC_TB_DTLRCALPHA,IDC_CB_DTLRCALIGN1,IDC_CB_DTLRCALIGN2,IDC_CB_DTLINE };
	static_assert(ARRAYSIZE(pWnd) == ARRAYSIZE(iId));
	eck::AttachDlgItems(hDlg, ARRAYSIZE(pWnd), pWnd, iId);

	m_CBBLrcEncoding.AddString(L"自动判断");
	m_CBBLrcEncoding.AddString(L"GB2312");
	m_CBBLrcEncoding.AddString(L"UTF-8");
	m_CBBLrcEncoding.AddString(L"UTF-16LE");
	m_CBBLrcEncoding.AddString(L"UTF-16BE");

	eck::CComboBox* const pCBAlign[]{ &m_CBBLsAlign, &m_CBBDlAlign1,&m_CBBDlAlign2 };
	for (auto pCtrl : pCBAlign)
	{
		pCtrl->AddString(L"左对齐");
		pCtrl->AddString(L"右对齐");
		pCtrl->AddString(L"居中对齐");
	}

	m_CBBDlView.AddString(L"单行");
	m_CBBDlView.AddString(L"双行");

	m_TBDlAlpha.SetRange32(0, 255);

	DataToCtrl();
	return 0;
}

LRESULT CDlgOptLrc::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case PNWM_OPTDLG_APPLY:
		CtrlToData();
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}