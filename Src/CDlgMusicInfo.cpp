#include "CDlgMusicInfo.h"

BOOL CDlgMusicInfo::OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam)
{
    m_Tab.AttachNew(GetDlgItem(hDlg, IDC_TAB));
    m_Tab.InsertItem(L"元数据");
    m_Tab.InsertItem(L"封面");
    m_Tab.InsertItem(L"其他");


    return TRUE;
}
