#pragma once
#include "CApp.h"
#include "resource.h"

#include "eck\CDialog.h"
#include "eck\CComboBox.h"
#include "eck\CListBox.h"
#include "eck\CButton.h"
#include "eck\CEdit.h"
#include "eck\CColorPickBlock.h"
#include "eck\CFontPicker.h"
#include "eck\CTrackBar.h"

class CDlgOptLrc :public eck::CDialog
{
private:
	eck::CComboBox			m_CBBLrcEncoding{};
	eck::CListBox			m_LBLrcPath{};


	eck::CCheckButton		m_CBLsAnimation{};
	eck::CCheckButton		m_CBLsAutoWrap{};
	eck::CCheckButton		m_CBLsShowTranslation{};
	eck::CCheckButton		m_CBLsSpaceLine{};
	eck::CEdit				m_EDLsSpaceLine{};

	eck::CColorPickBlock	m_CPBLsNormal{};
	eck::CColorPickBlock	m_CPBLsHilight{};
	eck::CEdit				m_EDLsLinePadding{};
	eck::CComboBox			m_CBBLsAlign{};
	eck::CFontPicker		m_FPLsMain{};
	eck::CFontPicker		m_FPLsTranslation{};


	eck::CCheckButton		m_CBDlBorder{};
	eck::CColorPickBlock	m_CPBDlBorder{};
	eck::CCheckButton		m_CBDlShadow{};
	eck::CEdit				m_EDDlShadowOffset{};
	eck::CCheckButton		m_CBDlShowWhenStartup{};
	eck::CCheckButton		m_CBDlSpaceLine{};
	eck::CEdit				m_EDDlSpaceLine{};
	eck::CCheckButton		m_CBDlShowTranslation{};

	eck::CFontPicker		m_FPDlMain{};
	eck::CColorPickBlock	m_CPBDlMainNormal1{};
	eck::CColorPickBlock	m_CPBDlMainNormal2{};
	eck::CColorPickBlock	m_CPBDlMainHilight1{};
	eck::CColorPickBlock	m_CPBDlMainHilight2{};
	eck::CFontPicker		m_FPDlTranslation{};
	eck::CColorPickBlock	m_CPBDlTranslationNormal1{};
	eck::CColorPickBlock	m_CPBDlTranslationNormal2{};
	eck::CColorPickBlock	m_CPBDlTranslationHilight1{};
	eck::CColorPickBlock	m_CPBDlTranslationHilight2{};

	eck::CTrackBar			m_TBDlAlpha{};
	eck::CComboBox			m_CBBDlAlign1{};
	eck::CComboBox			m_CBBDlAlign2{};
	eck::CComboBox			m_CBBDlView{};

	void DataToCtrl();

	void CtrlToData();
public:
	PNInline HWND CreateDlg(HWND hParent, void* pData = NULL) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_OPTLRC), hParent);
	}

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};