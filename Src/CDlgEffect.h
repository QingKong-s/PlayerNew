#pragma once
#include "CDlgFxCommon.h"
#include "CDlgFxEQ.h"
#include "CDlgChorus.h"
#include "CDlgFxCompression.h"
#include "CDlgFxDistortion.h"
#include "CDlgFxEcho.h"
#include "CDlgFxFlanger.h"
#include "CDlgFxGargle.h"
#include "CDlgFxI3DL2Reverb.h"
#include "CDlgFxReverb.h"
#include "CDlgFxRotate.h"

#include "eck\CTab.h"


class CDlgEffect final :public eck::CDialog
{
private:
	eck::CTab m_Tab{};

	CDlgFxCommon m_DlgComm{};
	CDlgFxEQ m_DlgEQ{};
	CDlgChorus m_DlgChorus{};
	CDlgFxCompression m_DlgComp{};
	CDlgFxDistortion m_DlgDistortion{};
	CDlgFxEcho m_DlgEcho{};
	CDlgFxFlanger m_DlgFlanger{};
	CDlgFxGargle m_DlgGargle{};
	CDlgFxI3DL2Reverb m_DlgI3DL2Reverb{};
	CDlgFxReverb m_DlgReverb{};
	CDlgFxRotate m_DlgRotate{};

	eck::CDialog* m_pDlg[13]
	{
		&m_DlgComm,
		&m_DlgEQ,
		&m_DlgChorus,
		&m_DlgComp,
		&m_DlgDistortion,
		&m_DlgEcho,
		&m_DlgFlanger,
		&m_DlgGargle,
		&m_DlgI3DL2Reverb,
		&m_DlgReverb,
		&m_DlgRotate
	};
public:
	HWND CreateDlg(HWND hParent, void* pParam) override
	{
		return IntCreateModelessDlg(App->GetHInstance(), MAKEINTRESOURCEW(IDD_EFFECT), hParent);
	}

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};