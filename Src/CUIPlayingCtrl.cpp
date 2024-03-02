#include "CWndBK.h"
#include "CDlgAbout.h"


LRESULT CUIPlayingCtrl::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
    {
        const auto pBk = GetBk();

        const int cxTotal = pBk->m_Ds.iPCBTGap * 8 + pBk->m_Ds.cxPCBT * 8 + pBk->m_Ds.cxPCBTBig;
        int xLeft = (GetViewWidth() - cxTotal) / 2;
        const int yTop = (GetViewHeight() - pBk->m_Ds.cyPCBT) / 2;
        const int cxBT = pBk->m_Ds.cxPCBT;
        const int iGap = pBk->m_Ds.iPCBTGap;
        RECT rc{ 0,yTop,0,yTop + pBk->m_Ds.cyPCBT };
        rc.left = xLeft;

        m_BTOptions.SetPos(xLeft, yTop);

        xLeft += (cxBT + iGap);
        m_BTPlayOpt.SetPos(xLeft, yTop);

        POINT pt{ xLeft, yTop - GetBk()->GetDpiSize().cyVolParent };
        ElemToClient(pt);
        m_VolTB.SetPos(pt.x, pt.y);

        xLeft += (cxBT + iGap);
        m_BTRepeatMode.SetPos(xLeft, yTop);

        xLeft += (cxBT + iGap);
        m_BTPrev.SetPos(xLeft, yTop);

        xLeft += (cxBT + iGap);
        m_BTPlay.SetPos(xLeft, (GetViewHeight() - pBk->m_Ds.cyPCBTBig) / 2);

        xLeft += (iGap + pBk->m_Ds.cxPCBTBig);
        m_BTNext.SetPos(xLeft, yTop);

        xLeft += (cxBT + iGap);
        m_BTStop.SetPos(xLeft, yTop);

        xLeft += (cxBT + iGap);
        m_BTLrc.SetPos(xLeft, yTop);

        xLeft += (cxBT + iGap);
        m_BTAbout.SetPos(xLeft, yTop);
    }
    return 0;

    case WM_NCHITTEST:
        return HTTRANSPARENT;

    case WM_PAINT:
    {
        Dui::ELEMPAINTSTRU ps;
        BeginPaint(ps, wParam, lParam);
        BkDbg_DrawElemFrame();
        EndPaint(ps);
    }
    return 0;

    case WM_CREATE:
    {
        const auto pBk = GetBk();

        constexpr auto uStyle = Dui::DES_VISIBLE | Dui::DES_TRANSPARENT;
        const int cxBT = pBk->m_Ds.cxPCBT;
        const int cyBT = pBk->m_Ds.cyPCBT;

        m_VolTB.Create(NULL, 0, 0, 
			0, 0, pBk->GetDpiSize().cxVolTrack, pBk->GetDpiSize().cyVolParent, NULL, GetWnd());

        m_BTOptions.Create(NULL, uStyle, 0,
			0, 0, cxBT, cyBT, this, GetWnd(), IDE_BT_OPT);
        m_BTPlayOpt.Create(NULL, uStyle, 0,
            0, 0, cxBT, cyBT, this, GetWnd(), IDE_BT_PLAYOPT);
        m_BTRepeatMode.Create(NULL, uStyle, 0,
            0, 0, cxBT, cyBT, this, GetWnd(), IDE_BT_REPEATMODE);
        m_BTPrev.Create(NULL, uStyle, 0,
            0, 0, cxBT, cyBT, this, GetWnd(), IDE_BT_PREV);
        m_BTPlay.Create(NULL, uStyle, 0,
            0, 0, pBk->m_Ds.cxPCBTBig, pBk->m_Ds.cyPCBTBig, this, GetWnd(), IDE_BT_PLAY);
        m_BTNext.Create(NULL, uStyle, 0,
            0, 0, cxBT, cyBT, this, GetWnd(), IDE_BT_NEXT);
        m_BTStop.Create(NULL, uStyle, 0,
            0, 0, cxBT, cyBT, this, GetWnd(), IDE_BT_STOP);
        m_BTLrc.Create(NULL, uStyle, 0,
            0, 0, cxBT, cyBT, this, GetWnd(), IDE_BT_LRC);
        m_BTAbout.Create(NULL, uStyle, 0,
            0, 0, cxBT, cyBT, this, GetWnd(), IDE_BT_ABOUT);

        m_BTOptions.SetImage(pBk->m_pBmpIcon[CWndBK::ICIDX_Options]);
        m_BTPlayOpt.SetImage(pBk->m_pBmpIcon[CWndBK::ICIDX_PlayOpt]);
        m_BTRepeatMode.SetImage(pBk->m_pBmpIcon[CWndBK::ICIDX_RMAllLoop]);
        m_BTPrev.SetImage(pBk->m_pBmpIcon[CWndBK::ICIDX_Prev]);
        m_BTPlay.SetImage(pBk->m_pBmpIcon[CWndBK::ICIDX_Play]);
        m_BTNext.SetImage(pBk->m_pBmpIcon[CWndBK::ICIDX_Next]);
		m_BTStop.SetImage(pBk->m_pBmpIcon[CWndBK::ICIDX_Stop]);
		m_BTLrc.SetImage(pBk->m_pBmpIcon[CWndBK::ICIDX_Lrc]);
		m_BTAbout.SetImage(pBk->m_pBmpIcon[CWndBK::ICIDX_About]);

		D2D1_SIZE_F sizeImg{ pBk->m_DsF.cxIcon,pBk->m_DsF.cyIcon };
		m_BTOptions.SetImageSize(sizeImg);
		m_BTPlayOpt.SetImageSize(sizeImg);
		m_BTRepeatMode.SetImageSize(sizeImg);
		m_BTPrev.SetImageSize(sizeImg);
		m_BTPlay.SetImageSize(sizeImg);
		m_BTNext.SetImageSize(sizeImg);
		m_BTStop.SetImageSize(sizeImg);
		m_BTLrc.SetImageSize(sizeImg);
		m_BTAbout.SetImageSize(sizeImg);

        auto cr = m_BTPlay.GetColorTheme()->Get();
        cr.crBkNormal = {};
        const auto pTheme = new Dui::CColorTheme(cr);
        m_BTOptions.SetColorTheme(pTheme);
        m_BTPlayOpt.SetColorTheme(pTheme);
        m_BTRepeatMode.SetColorTheme(pTheme);
        m_BTPrev.SetColorTheme(pTheme);
        m_BTNext.SetColorTheme(pTheme);
        m_BTStop.SetColorTheme(pTheme);
        m_BTLrc.SetColorTheme(pTheme);
        m_BTAbout.SetColorTheme(pTheme);
        pTheme->DeRef();
    }
	break;

    case WM_NOTIFY:
    {
        const auto pElem = (Dui::CElem*)wParam;
		if (pElem == &m_BTPlayOpt && ((Dui::DUINMHDR*)lParam)->uCode == Dui::EE_COMMAND)
        {
            if (!m_VolTB.IsVisible())
                m_VolTB.SetVisible(TRUE);
            m_VolTB.m_TrackBar.SetFocus();
            m_VolTB.m_TrackBar.SetPos(App->GetPlayer().GetBass().GetVolume() * 100.f);
            return TRUE;
        }
    }
    break;

    case UIEE_ONPLAYINGCTRL:
    {
        if (App->GetPlayer().IsPlaying())
            m_BTPlay.SetImage(GetBk()->m_pBmpIcon[CWndBK::ICIDX_Pause]);
        else
            m_BTPlay.SetImage(GetBk()->m_pBmpIcon[CWndBK::ICIDX_Play]);
        m_BTPlay.InvalidateRect();
    }
    break;
    }

    return __super::OnEvent(uMsg, wParam, lParam);
}