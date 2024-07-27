#include "pch.h"
#include "CDlgProgress.h"

BOOL CDlgProgress::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	m_PB.AttachNew(GetDlgItem(hDlg, IDC_PROGRESS1));

	std::thread Thr([this]
		{
			auto& List = App->GetPlayer().GetList();
			m_PB.PostMsg(PBM_SETRANGE32, 0, List.GetCount());

			EckCounter(List.GetCount(), i)
			{
				if (m_bShouldExit.load())
					break;
				m_PB.PostMsg(PBM_SETPOS, i, 0);
				
				List.UpdateItemInfo(i);
			}

			PostMsg(WM_CLOSE, 0, 0);
		});
	DuplicateHandle(GetCurrentProcess(), Thr.native_handle(),
		GetCurrentProcess(), &m_hThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
	Thr.detach();
	return FALSE;
}

void CDlgProgress::OnCancel(HWND hCtrl)
{
	m_bShouldExit.store(TRUE);
	EndDlg(0);
}

LRESULT CDlgProgress::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}
