#include "CWndBK.h"
#include "CDlgAbout.h"

CUIPlayingCtrl::CUIPlayingCtrl()
{
    m_uType = UIET_PLAYINGCTRL;
    m_uFlags = UIEF_NONE;
}

void CUIPlayingCtrl::Redraw()
{
    auto pDC = m_pBK->m_pDC;
    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, &m_rcF);
    pDC->PushAxisAlignedClip(&m_rcF, D2D1_ANTIALIAS_MODE_ALIASED);
    for (auto pElem : m_vChildren)
        pElem->Redraw();
    pDC->PopAxisAlignedClip();

    BkDbg_DrawElemFrame();
}

BOOL CUIPlayingCtrl::InitElem()
{
    m_pBTOptions = new CUIButton(PCBTI_OPT);
    m_pBTPlayOpt = new CUIButton(PCBTI_PLAYOPT);
    m_pBTRepeatMode = new CUIButton(PCBTI_REPEATMODE);
    m_pBTPrev = new CUIButton(PCBTI_PREV);
    m_pBTPlay = new CUIRoundButton(PCBTI_PLAY);
    m_pBTNext = new CUIButton(PCBTI_NEXT);
    m_pBTStop = new CUIButton(PCBTI_STOP);
    m_pBTLrc = new CUIButton(PCBTI_LRC);
    m_pBTAbout = new CUIButton(PCBTI_ABOUT);
    m_pBTOptions->m_uStyle |= UIES_NOERASEBK;
    m_pBTPlayOpt->m_uStyle |= UIES_NOERASEBK;
    m_pBTRepeatMode->m_uStyle |= UIES_NOERASEBK;
    m_pBTPrev->m_uStyle |= UIES_NOERASEBK;
    m_pBTPlay->m_uStyle |= UIES_NOERASEBK;
    m_pBTNext->m_uStyle |= UIES_NOERASEBK;
    m_pBTStop->m_uStyle |= UIES_NOERASEBK;
    m_pBTLrc->m_uStyle |= UIES_NOERASEBK;
    m_pBTAbout->m_uStyle |= UIES_NOERASEBK;
    m_pBK->AddElem(m_pBTOptions);
    m_pBK->AddElem(m_pBTPlayOpt);
    m_pBK->AddElem(m_pBTRepeatMode);
    m_pBK->AddElem(m_pBTPrev);
    m_pBK->AddElem(m_pBTPlay);
    m_pBK->AddElem(m_pBTNext);
    m_pBK->AddElem(m_pBTStop);
    m_pBK->AddElem(m_pBTLrc);
    m_pBK->AddElem(m_pBTAbout);
    m_pBTOptions->SetParent(this);
    m_pBTPlayOpt->SetParent(this);
    m_pBTRepeatMode->SetParent(this);
    m_pBTPrev->SetParent(this);
    m_pBTPlay->SetParent(this);
    m_pBTNext->SetParent(this);
    m_pBTStop->SetParent(this);
    m_pBTLrc->SetParent(this);
    m_pBTAbout->SetParent(this);

    m_pBTOptions->InitElem();
    m_pBTPlayOpt->InitElem();
    m_pBTRepeatMode->InitElem();
    m_pBTPrev->InitElem();
    m_pBTPlay->InitElem();
    m_pBTNext->InitElem();
    m_pBTStop->InitElem();
    m_pBTLrc->InitElem();
    m_pBTAbout->InitElem();
    m_pBTOptions->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Options]);
    m_pBTPlayOpt->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_PlayOpt]);
    m_pBTRepeatMode->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_RMAllLoop]);
    m_pBTPrev->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Prev]);
    m_pBTPlay->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Play]);
    m_pBTNext->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Next]);
    m_pBTStop->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Stop]);
    m_pBTLrc->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Lrc]);
    m_pBTAbout->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_About]);

    for (auto pElem : m_vChildren)
        if (pElem->m_uType == UIET_BUTTON || pElem->m_uType == UIET_ROUNDBUTTON)
            ((CUIButton*)pElem)->SetImgSize(m_pBK->m_DsF.cxIcon, m_pBK->m_DsF.cyIcon);

    return TRUE;
}

LRESULT CUIPlayingCtrl::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    auto lResult = DefElemEventProc(uEvent, wParam, lParam);
    switch (uEvent)
    {
    case UIEE_SETRECT:
    {
        const int cxTotal = m_pBK->m_Ds.iPCBTGap * 8 + m_pBK->m_Ds.cxPCBT * 8 + m_pBK->m_Ds.cxPCBTBig;
        int xLeft = (m_cx - cxTotal) / 2;
        const int yTop = (m_cy - m_pBK->m_Ds.cyPCBT) / 2;
        const int cxBT = m_pBK->m_Ds.cxPCBT;
        const int iGap = m_pBK->m_Ds.iPCBTGap;
        RECT rc{ 0,yTop,0,yTop + m_pBK->m_Ds.cyPCBT };
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTOptions->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTPlayOpt->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTRepeatMode->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTPrev->SetElemRect(&rc);

        xLeft += (cxBT + iGap);
        const int xBigBT = xLeft;
        xLeft += (iGap + m_pBK->m_Ds.cxPCBTBig);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTNext->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTStop->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTLrc->SetElemRect(&rc);
        xLeft += (cxBT + iGap);
        rc.left = xLeft;
        rc.right = rc.left + cxBT;
        m_pBTAbout->SetElemRect(&rc);

        rc.left = xBigBT;
        rc.top = (m_cy - m_pBK->m_Ds.cyPCBTBig) / 2;
        rc.right = rc.left + m_pBK->m_Ds.cxPCBTBig;
        rc.bottom = rc.top + m_pBK->m_Ds.cyPCBTBig;
        m_pBTPlay->SetElemRect(&rc);
    }
    break;
    case UIEE_CHILDREDRAW:
    {
        auto pDC = m_pBK->m_pDC;
        auto pElem = (CUIElem*)wParam;
        pDC->BeginDraw();
        pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &pElem->m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, &pElem->m_rcF);
        pElem->Redraw();
        pDC->EndDraw();
    }
    break;
    case UIEE_ONPLAYINGCTRL:
        if (wParam == PCT_PLAY)
            m_pBTPlay->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Pause]);
        break;
    }

    return lResult;
}

BOOL CUIPlayingCtrl::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DispatchEvent(uMsg, wParam, lParam);
    if (uMsg == m_pBK->m_uMsgCUIButton)
    {
        auto pBtn = (CUIButton*)lParam;
        switch (pBtn->GetID())
        {
        case PCBTI_PLAY:
            if (App->GetPlayer().IsPlaying())
                pBtn->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Pause]);
            else
                pBtn->SetImg(m_pBK->m_pBmpIcon[CWndBK::ICIDX_Play]);
            pBtn->CUIElem::Redraw(TRUE);
            break;
        case PCBTI_REPEATMODE:
        {
            constexpr int idx[]
            {
                CWndBK::ICIDX_RMAllLoop,
                CWndBK::ICIDX_RMAll,
                CWndBK::ICIDX_RMRadom,
                CWndBK::ICIDX_RMSingleLoop,
                CWndBK::ICIDX_RMSingle,
            };
            auto iRM = (int)App->GetOptionsMgr().iRepeatMode;
            EckAssert(iRM >= 0 && iRM < ARRAYSIZE(idx));
            pBtn->SetImg(m_pBK->m_pBmpIcon[idx[iRM]]);
            pBtn->CUIElem::Redraw(TRUE);
        }
        break;
        case PCBTI_ABOUT:
        {
            CDlgAbout Dlg{};
            Dlg.DlgBox(m_pBK->GetHWND());
        }
        break;
        }
    }
    return FALSE;
}