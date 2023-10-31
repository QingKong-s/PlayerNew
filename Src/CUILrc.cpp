#include "CWndBK.h"

CUILrc::CUILrc()
{
    m_uType = UIET_LRC;
    m_uFlags = UIEF_WANTTIMEREVENT;
}

CUILrc::~CUILrc()
{
    SAFE_RELEASE(m_pBrTextNormal);
    SAFE_RELEASE(m_pBrTextHighlight);
    SAFE_RELEASE(m_pTextFormat);
}

BOOL CUILrc::InitElem()
{
    SAFE_RELEASE(m_pBrTextNormal);
    SAFE_RELEASE(m_pBrTextHighlight);
    SAFE_RELEASE(m_pTextFormat);

    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrTextNormal);
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pBrTextHighlight);

    auto& Font = COptionsMgr::GetInst().LrcFont;
    App->m_pDwFactory->CreateTextFormat(Font.rsFontName.Data(), NULL,
        (DWRITE_FONT_WEIGHT)Font.iWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        Font.fFontSize, L"zh-cn", &m_pTextFormat);
    return TRUE;
}

void CUILrc::Redraw()
{
    auto pDC = m_pBK->m_pDC;
    //BOOL bSwitchLrc;

    //PCWSTR pszText = NULL;
    //if (!g_hStream)
    //{
    //    pszText = L"晴空的音乐播放器 - VC++/Win32";
    //    g_iLrcState = LRCSTATE_STOP;
    //    g_iCurrLrcIndex = -2;
    //    //m_IsDraw[2] = FALSE;
    //}
    //else if (!g_Lrc->iCount)
    //{
    //    pszText = L"无歌词";
    //    g_iLrcState = LRCSTATE_NOLRC;
    //    g_iCurrLrcIndex = -2;
    //    //m_IsDraw[2] = FALSE;
    //}

    //if (pszText)
    //{
    //    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcFLrc, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcFLrc);
    //    m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    //    m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    //    pDC->DrawTextW(pszText, lstrlenW(pszText), m_pTextFormat, &m_rcF, m_pBrTextNormal, D2D1_DRAW_TEXT_OPTIONS_CLIP);
    //    BkDbg_DrawElemFrame();
    //    return;
    //}

    ////m_idxCurr = -1;
    ////g_iCurrLrcIndex = -2;

    //int cLrc = g_Lrc->iCount;
    //float fTempTime;



    ////if (m_iLrcSBPos != -1)
    ////    m_idxCurr = m_iLrcSBPos;
    ////if (m_iLrcFixedIndex != -1)
    ////    m_idxCurr = m_iLrcFixedIndex;
    ////// 索引查找完毕
    //if (m_idxCurr != m_idxLastHighlight)
    //{
    //    m_idxLastHighlight = m_idxCurr;
    //    //m_IsDraw[2] = TRUE;
    //    bSwitchLrc = TRUE;
    //}
    //else
    //    bSwitchLrc = FALSE;
    //++m_iDrawingID;
    //if (!m_iDrawingID)
    //    ++m_iDrawingID;
    //g_iLrcState = LRCSTATE_NORMAL;

    //bSwitchLrc = TRUE;


    ////GS.bLrcAnimation = FALSE;
    //if (bSwitchLrc)
    //    if (GS.bLrcAnimation)
    //    {
    //        //if (!m_bShowSB && m_iLastLrcIndex[0] != -1)
    //        //{

    //        //}
    //    }
    //    else
    //    {
    //        auto p = (LRCDATA*)QKAGet(g_Lrc, m_idxCurr);
    //        m_yCenterItem = (m_rcFLrc.bottom + m_rcFLrc.top - p->cyTotal / 2) / 2.f;
    //        //UI_VEDrawLrc(m_rcLrcShow.top + m_cyLrcShow / 2, bImmdShow, bIndependlyDrawing);
    //        DrawAllLrc_Center();
    //    }
    ////m_iLastLrcIndex[0] = m_idxCurr;
    ////m_iLastLrcIndex[1] = g_iCurrLrcIndex;
    ////m_IsDraw[2] = FALSE;

    //LrcWnd_DrawLrc();
    BkDbg_DrawElemFrame();
}

BOOL CUILrc::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hWnd = m_pBK->m_hWnd;
    // switch (uMsg)
    // {
    // case WM_LBUTTONUP:
    // {
    //     if (m_bSBLBtnDown)
    //     {
    //         m_bSBLBtnDown = FALSE;
    //         ReleaseCapture();
    //     }
    // }
    // return 0;

    // case WM_MOUSEWHEEL:
    // {
    //     if (!g_Lrc->iCount)
    //         return 0;

    //     m_bShowSB = TRUE;
    //     m_iDelayTime = 5;
    //     if (m_uSBPos == -1)
    //         m_uSBPos = g_iCurrLrcIndex;

    //     int iDistance = -GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
    //     if (m_uSBPos + iDistance < 0)
    //         m_uSBPos = 0;
    //     else if (m_uSBPos + iDistance > m_uSBMax)
    //         m_uSBPos = m_uSBMax;
    //     else
    //         m_uSBPos = m_uSBPos + iDistance;
    //     //WndProc_LeftBK(hWnd, LEFTBKM_LRCSHOW_REDRAWSB, FALSE, TRUE);
    //     //m_IsDraw[2] = TRUE;
    //     //UI_VEProcLrcShowing();

    //     m_idxCenter = m_uSBPos;

    //     auto pDC = m_pBK->m_pDC;
    //     pDC->BeginDraw();
    //     m_ActiveItem.clear();
    //     //DrawAllLrc_Top();
    //     DrawAllLrc_Center();
    //     pDC->EndDraw();
    //     DXGI_PRESENT_PARAMETERS pp;
    //     pp.DirtyRectsCount = 1;
    //     pp.pDirtyRects = &m_rc;
    //     pp.pScrollOffset = NULL;
    //     pp.pScrollRect = NULL;
    //     m_pBK->m_pSwapChain->Present1(0, 0, &pp);
    //     SetTimer(hWnd, IDT_LRCSCROLL, TIMERELAPSE_LRCSCROLL, NULL);
    // }
    // return 0;

    // case WM_MOUSEMOVE:
    // {
    //     POINT pt = GET_PT_LPARAM(lParam);
    //     if (m_bSBLBtnDown)
    //     {
    //         m_iDelayTime = 5;

    //         m_rcThumb.top = pt.y - m_iSBCursorOffset;
    //         int iMaxPos = m_rcSB.top + m_cy - m_iThumbSize;
    //         if (m_rcThumb.top < m_rcSB.top)
    //             m_rcThumb.top = m_rcSB.top;
    //         else if (m_rcThumb.top > iMaxPos)
    //             m_rcThumb.top = iMaxPos;
    //         m_rcThumb.bottom = m_rcThumb.top + m_iThumbSize;
    //         m_rcFThumb = QKRect2D2DRcF(m_rcThumb);

    //         m_uSBPos = (m_rcThumb.top - m_rcSB.top) * m_uSBMax / m_cy;


    //         m_idxCenter = m_uSBPos;
    //         //-----------------------------------
    ////         LRCDATA* p;
    ////         float cy = 0.f;
    ////         for (int i = 0; i < g_Lrc->iCount; ++i)
    ////         {
    ////             p = (LRCDATA*)QKAGet(g_Lrc, i);
    ////             cy += (p->cyTotal);
    ////             if (cy > m_uSBPos)
    ////             {
    ////                 m_idxTopItem = i;
    ////                 m_fOffsetTopIndex = p->cyTotal - (cy - m_uSBPos);
    ////                 break;
    ////             }
    ////             cy += GS.iSCLrcLineGap;
    //         //}
    //         //-----------------------------------
    //         //Redraw(TRUE);

    //         auto pDC = m_pBK->m_pDC;
    //         pDC->BeginDraw();
    //         //DrawAllLrc_Top();
    //         DrawAllLrc_Center();
    //         pDC->EndDraw();
    //         DXGI_PRESENT_PARAMETERS pp;
    //         pp.DirtyRectsCount = 1;
    //         pp.pDirtyRects = &m_rc;
    //         pp.pScrollOffset = NULL;
    //         pp.pScrollRect = NULL;
    //         m_pBK->m_pSwapChain->Present1(0, 0, &pp);
    //     }
    //     else if (PtInRect(&m_rcLrc, pt))
    //     {
    //         int i = HitTest(pt);
    //         m_iDelayTime = 5;
    //         if (i != m_idxLastMouseHover)
    //         {
    //             m_idxMouseHover = i;
    //             if (i >= 0 && m_idxLastMouseHover >= 0)
    //             {
    //                 int a = 0;
    //             }

    //             if (!m_bVAnOngoing)
    //             {
    //                 if (m_idxLastMouseHover >= 0)
    //                     RedrawItem(m_idxLastMouseHover, INFINITY, TRUE, TRUE, TRUE);
    //                 if (i >= 0)
    //                     RedrawItem(i, INFINITY, TRUE, TRUE, TRUE);
    //                 //if (m_ActiveItem.find(m_idxLastMouseHover) != m_ActiveItem.end())
    //                 //	RedrawItem(m_idxLastMouseHover, INFINITY, TRUE, TRUE, TRUE);
    //                 //if (m_ActiveItem.find(i) != m_ActiveItem.end())
    //                 //	RedrawItem(i, INFINITY, TRUE, TRUE, TRUE);
    //             }
    //             m_idxLastMouseHover = i;
    //         }
    //     }
    //     else
    //     {
    //         if (m_idxLastMouseHover != -1)
    //         {
    //             m_idxMouseHover = -1;
    //             if (m_idxLastMouseHover >= 0 && !m_bVAnOngoing)
    //                 RedrawItem(m_idxLastMouseHover, INFINITY, TRUE, TRUE, TRUE);
    //             m_idxLastMouseHover = -1;
    //         }
    //     }
    // }
    // return 0;

    // case WM_MOUSELEAVE:
    // {
    //     if (m_idxLastMouseHover >= 0)
    //         RedrawItem(m_idxLastMouseHover, INFINITY, TRUE, TRUE, TRUE);
    //     m_idxLastMouseHover = -1;
    // }
    // return 0;

    // case WM_RBUTTONUP:
    // {
    //     //POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    //     //if (PtInRect(&m_rcLrcShow, pt))
    //     //{
    //     //	int i = HitTest_LrcShow(pt);
    //     //	if (i != iLastIndex)
    //     //	{
    //     //		iLastIndex = m_iLrcMouseHover = i;
    //     //		m_IsDraw[2] = TRUE;
    //     //		TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
    //     //	}
    //     //	UINT uFlags = (i == -1) ? MF_GRAYED : 0;
    //     //	HMENU hMenu = CreatePopupMenu();
    //     //	AppendMenuW(hMenu, uFlags, IDMI_LS_PLAY, L"从此处播放");
    //     //	AppendMenuW(hMenu, uFlags, IDMI_LS_COPY, L"复制歌词");
    //     //	ClientToScreen(hWnd, &pt);
    //     //	m_iLrcFixedIndex = m_iLrcCenter;
    //     //	int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
    //     //	DestroyMenu(hMenu);
    //     //	switch (iRet)
    //     //	{
    //     //	case IDMI_LS_PLAY:// 从此处播放
    //     //	{
    //     //		BASS_ChannelSetPosition(
    //     //			g_hStream,
    //     //			BASS_ChannelSeconds2Bytes(
    //     //				g_hStream,
    //     //				((LRCDATA*)QKAGet(g_Lrc, i))->fTime),
    //     //			BASS_POS_BYTE
    //     //		);
    //     //	}
    //     //	break;
    //     //	case IDMI_LS_COPY:// 复制歌词
    //     //	{
    //     //		if (OpenClipboard(hWnd))
    //     //		{
    //     //			PWSTR pszLrc = ((LRCDATA*)QKAGet(g_Lrc, i))->pszLrc;
    //     //			HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, (lstrlenW(pszLrc) + 1) * sizeof(WCHAR));
    //     //			void* pGlobal = GlobalLock(hGlobal);
    //     //			lstrcpyW((PWSTR)pGlobal, pszLrc);
    //     //			GlobalUnlock(hGlobal);
    //     //			EmptyClipboard();
    //     //			SetClipboardData(CF_UNICODETEXT, hGlobal);
    //     //			CloseClipboard();
    //     //		}
    //     //	}
    //     //	break;
    //     //	}
    //     //	m_iLrcFixedIndex = -1;
    //     //	m_IsDraw[2] = TRUE;
    //     //	TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
    //     //}
    // }
    // return 0;

    // case WM_KEYDOWN:
    // {
    //     if (wParam == VK_ESCAPE)
    //     {
    //         KillTimer(hWnd, IDT_LRCSCROLL);
    //         //m_iLrcSBPos = -1;
    //         //WndProc_LeftBK(hWnd, LEFTBKM_LRCSHOW_REDRAWSB, TRUE, TRUE);
    //         return 0;
    //     }
    // }
    // return 0;

    // case WM_LBUTTONDOWN:
    // {
    //     POINT pt = GET_PT_LPARAM(lParam);
    //     if (PtInRect(&m_rcSB, pt))
    //     {
    //         if (PtInRect(&m_rcThumb, pt))// 命中滑块
    //         {
    //             m_bSBLBtnDown = TRUE;
    //             m_iSBCursorOffset = pt.y - m_rcThumb.top;// 计算光标偏移
    //             SetCapture(hWnd);
    //         }
    //     }
    // }
    // return 0;
    // }
    // return 0;
return 0;
}

void CUILrc::OnTimer(UINT uTimerID)
{
    //switch (uTimerID)
    //{
    //case IDT_PGS:
    //{
    //    BOOL bSwitchLrc = FALSE;
    //    if (m_idxCurr != g_iCurrLrcIndex)// 进度已更新
    //    {
    //        m_idxLastHighlight = m_idxCurr;
    //        m_idxCurr = g_iCurrLrcIndex;
    //        if (!m_bShowSB)
    //            m_idxCenter = m_idxCurr;
    //        bSwitchLrc = TRUE;
    //    }

    //    m_ActiveItem.clear();
    //    if (bSwitchLrc)
    //    {
    //        InitVAnimation();
    //    }
    //    else
    //    {

    //    }

    //    if (bSwitchLrc)
    //    {
    //        m_pBK->m_PresentParam.DirtyRectsCount = 2;

    //        //Redraw();
    //    }
    //    else
    //    {
    //        m_pBK->m_PresentParam.DirtyRectsCount = 1;
    //    }
    //}
    //return;

    //case IDT_ANIMATION:
    //{
    //    if (m_LrcHScrollInfo.iIndex < 0 || m_idxCurr < 0)
    //    {
    //        KillTimer(m_pBK->m_hWnd, IDT_ANIMATION);
    //        return;
    //    }
    //    LRCDATA* p = (LRCDATA*)QKAGet(g_Lrc, m_idxCurr);
    //    float fLastTime = g_fTime - p->fTime;
    //    static int iLastx1 = 0, iLastx2 = 0;// 上次左边，如果跟上次一样就不要再画了
    //    int ii;
    //    BOOL bRedraw = FALSE;
    //    if (!m_bVAnOngoing)
    //    {
    //        if (m_LrcHScrollInfo.cx1 > 0)
    //        {
    //            if (fLastTime > m_LrcHScrollInfo.fNoScrollingTime1)
    //            {
    //                if (fLastTime < p->fDelay - m_LrcHScrollInfo.fNoScrollingTime1)
    //                {
    //                    ii = m_cxLrc / 2 - (int)(fLastTime * m_LrcHScrollInfo.cx1 / p->fDelay);
    //                    if (ii != iLastx1)
    //                    {
    //                        iLastx1 = m_LrcHScrollInfo.x1 = ii;
    //                        bRedraw = TRUE;
    //                    }
    //                }
    //                else
    //                {
    //                    ii = m_cxLrc - m_LrcHScrollInfo.cx1;
    //                    if (ii != iLastx1)
    //                    {
    //                        iLastx1 = m_LrcHScrollInfo.x1 = ii;
    //                        bRedraw = TRUE;
    //                    }
    //                }
    //            }
    //            else
    //            {
    //                if (iLastx1 != 0)
    //                {
    //                    iLastx1 = m_LrcHScrollInfo.x1 = 0;
    //                    bRedraw = TRUE;
    //                }
    //            }
    //        }

    //        if (m_LrcHScrollInfo.cx2 > 0)
    //        {
    //            if (fLastTime > m_LrcHScrollInfo.fNoScrollingTime2)
    //            {
    //                if (fLastTime < p->fDelay - m_LrcHScrollInfo.fNoScrollingTime2)
    //                {
    //                    ii = m_cxLrc / 2 - (int)(fLastTime * m_LrcHScrollInfo.cx2 / p->fDelay);
    //                    if (ii != iLastx2)
    //                    {
    //                        iLastx2 = m_LrcHScrollInfo.x2 = ii;
    //                        bRedraw = TRUE;
    //                    }
    //                }
    //                else
    //                {
    //                    ii = m_cxLrc - m_LrcHScrollInfo.cx2;
    //                    if (ii != iLastx2)
    //                    {
    //                        iLastx2 = m_LrcHScrollInfo.x2 = ii;
    //                        bRedraw = TRUE;
    //                    }
    //                }
    //            }
    //            else
    //            {
    //                if (iLastx2 != 0)
    //                {
    //                    iLastx2 = m_LrcHScrollInfo.x2 = 0;
    //                    bRedraw = TRUE;
    //                }
    //            }
    //        }
    //    }
    //    if (bRedraw)
    //        RedrawItem(m_idxCurr, INFINITY, TRUE, TRUE, TRUE);
    //}
    //return;

    //case IDT_ANIMATION2:
    //{
    //    if (m_bShowSB)
    //    {
    //        KillTimer(m_pBK->m_hWnd, IDT_ANIMATION2);
    //        m_bVAnOngoing = FALSE;
    //        return;
    //    }
    //    auto pDC = m_pBK->m_pDC;

    //    int iTop;
    //    static int iLastTop = 0x80000000;
    //    float fLastTime = g_fTime - m_LrcVScrollInfo.fTime;
    //    if (m_LrcVScrollInfo.bDirection)
    //    {
    //        iTop = m_LrcVScrollInfo.iSrcTop - fLastTime * m_LrcVScrollInfo.iDistance / m_LrcVScrollInfo.fDelay;
    //        if (iTop <= m_LrcVScrollInfo.iDestTop)
    //        {
    //            pDC->BeginDraw();
    //            m_yCenterItem = m_LrcVScrollInfo.iDestTop;
    //            DrawAllLrc_Center();
    //            KillTimer(m_pBK->m_hWnd, IDT_ANIMATION2);
    //            m_bVAnOngoing = FALSE;
    //            goto Ret;
    //        }
    //        else if (iTop != iLastTop)
    //        {
    //            iLastTop = iTop;
    //        }
    //        else
    //            return;
    //    }
    //    else
    //    {
    //        iTop = m_LrcVScrollInfo.iSrcTop + fLastTime * m_LrcVScrollInfo.iDistance / m_LrcVScrollInfo.fDelay;
    //        if (iTop >= m_LrcVScrollInfo.iDestTop)
    //        {
    //            pDC->BeginDraw();
    //            m_IsDraw[2] = TRUE;
    //            m_yCenterItem = m_LrcVScrollInfo.iDestTop;
    //            DrawAllLrc_Center();
    //            KillTimer(m_pBK->m_hWnd, IDT_ANIMATION2);
    //            m_bVAnOngoing = FALSE;
    //            goto Ret;
    //        }
    //        else if (iTop != iLastTop)
    //        {
    //            iLastTop = iTop;
    //        }
    //        else
    //            return;
    //    }
    //    pDC->BeginDraw();
    //    m_yCenterItem = iTop;
    //    DrawAllLrc_Center();
    //Ret:
    //    pDC->EndDraw();
    //    DXGI_PRESENT_PARAMETERS pp;
    //    pp.DirtyRectsCount = 1;
    //    pp.pDirtyRects = &m_rc;
    //    pp.pScrollOffset = NULL;
    //    pp.pScrollRect = NULL;
    //    m_pBK->m_pSwapChain->Present1(0, 0, &pp);
    //}
    //return;
    //}
}

void CUILrc::InitVAnimation()
{
    //if (m_idxLastHighlight < 0)
    //{
    //    KillTimer(m_pBK->m_hWnd, IDT_ANIMATION);
    //    return;
    //}
    //auto p1 = (LRCDATA*)QKAGet(g_Lrc, m_idxCurr);
    //auto p2 = (LRCDATA*)QKAGet(g_Lrc, m_idxLastHighlight);

    //int iHeight = p1->cyTotal,
    //    iHeight2 = p2->cyTotal;

    //int iTop = m_rc.top + (m_cy - iHeight2) / 2;// 上一句顶边
    //m_LrcVScrollInfo.fDelay = 0.1f;
    //m_LrcVScrollInfo.fTime = g_fTime;
    //float ff;
    //if (m_idxCurr > m_idxLastHighlight)// 下一句在上一句的下方
    //{
    //    m_LrcVScrollInfo.bDirection = TRUE;
    //    //m_LrcVScrollInfo.iDestTop = m_rc.top + m_cyHalf - iHeight / 2;
    //    //m_LrcVScrollInfo.iSrcTop = m_rc.top + m_cyHalf + iHeight2 / 2 + GS.iSCLrcLineGap ;
    //    m_LrcVScrollInfo.iDestTop = m_rc.top + m_cyHalf - iHeight / 2;
    //    m_LrcVScrollInfo.iSrcTop = m_rc.top + m_cyHalf + iHeight / 2 + GS.iSCLrcLineGap;
    //    m_LrcVScrollInfo.iDistance = m_LrcVScrollInfo.iSrcTop - m_LrcVScrollInfo.iDestTop;
    //    ff = p1->fTime - p2->fTime;
    //    if (m_LrcVScrollInfo.fDelay > ff)
    //        m_LrcVScrollInfo.fDelay = ff / 2;
    //}
    //else// 下一句在上一句的上方
    //{
    //    m_LrcVScrollInfo.bDirection = FALSE;
    //    m_LrcVScrollInfo.iDestTop = m_rc.top + m_cyHalf - iHeight / 2;
    //    m_LrcVScrollInfo.iSrcTop = m_rc.top + m_cyHalf - iHeight - iHeight2 / 2 - GS.iSCLrcLineGap;
    //    m_LrcVScrollInfo.iDistance = m_LrcVScrollInfo.iDestTop - m_LrcVScrollInfo.iSrcTop;
    //    ff = p2->fTime - p1->fTime;
    //    if (m_LrcVScrollInfo.fDelay > ff)
    //        m_LrcVScrollInfo.fDelay = ff / 2;
    //}
    //KillTimer(m_pBK->m_hWnd, IDT_ANIMATION2);
    //SetTimer(m_pBK->m_hWnd, IDT_ANIMATION2, TIMERELAPSE_ANIMATION2, NULL);
    //m_bVAnOngoing = TRUE;
    ////m_iLastLrcIndex[0] = m_idxCurr;
    ////m_iLastLrcIndex[1] = g_iCurrLrcIndex;
    ////m_IsDraw[2] = FALSE;
    //return;
}

void CUILrc::InitAnimation(int idxPrevCenter, int idxNewCenter)
{
    /*m_idxAnStart = m_idxTopItem;
    auto p = (LRCDATA*)QKAGet(g_Lrc, m_idxAnStart);

    float y = p->y;
    int idxTop = m_idxAnStart;
    while (y < m_cy)
    {
        ++idxTop;
        if (idxTop == g_Lrc->iCount)
            break;
        y += ((LRCDATA*)QKAGet(g_Lrc, idxTop))->cyTotal;
    }
    m_idxAnEnd = idxTop;
    KillTimer(m_pBK->m_hWnd, IDT_ANIMATION);
    SetTimer(m_pBK->m_hWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, NULL);*/
}

void CUILrc::DrawAllLrc_Top()
{
    /* auto pDC = m_pBK->m_pDC;
     pDC->PushAxisAlignedClip(&m_rcF, D2D1_ANTIALIAS_MODE_ALIASED);
     pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcFLrc, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcFLrc);
     float y = -m_fOffsetTopIndex + m_rcFLrc.top;
     for (int i = m_idxTopItem; i < g_Lrc->iCount; ++i)
     {
         y += (RedrawItem(i, y, TRUE, FALSE, FALSE) + GS.iSCLrcLineGap);
         if (y > m_rcF.bottom)
             break;
     }

     RedrawSB(FALSE);
     pDC->PopAxisAlignedClip();*/
}

void CUILrc::DrawAllLrc_Center()
{
    //auto pDC = m_pBK->m_pDC;
    //pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);// 刷背景
    //pDC->PushAxisAlignedClip(&m_rcF, D2D1_ANTIALIAS_MODE_ALIASED);

    //m_ullLastDrawingTime = GetTickCount64();
    //float cy = RedrawItem(m_idxCenter, m_yCenterItem, TRUE, FALSE, FALSE);

    //// 初始化成上面一项的底边
    //float fBottom = m_yCenterItem - (float)GS.iSCLrcLineGap;
    //// 初始化成下面一项的顶边
    //float fTop = m_yCenterItem + ((float)GS.iSCLrcLineGap + cy);
    //int i = m_idxCenter;
    //////////////////////////////////////////////////向上画
    //while (fBottom > m_rcF.top)
    //{
    //    if (i - 1 < 0)
    //        break;
    //    --i;
    //    cy = RedrawItem(i, fBottom, FALSE, FALSE, FALSE);
    //    fBottom -= (GS.iSCLrcLineGap + cy);
    //}
    //i = m_idxCenter;
    //////////////////////////////////////////////////向下画
    //while (fTop < m_rcF.bottom)
    //{
    //    if (i + 1 >= g_Lrc->iCount)
    //        break;
    //    ++i;
    //    cy = RedrawItem(i, fTop, TRUE, TRUE, FALSE);
    //    fTop += (GS.iSCLrcLineGap + cy);
    //}

    //pDC->PopAxisAlignedClip();

    //RedrawSB(FALSE);
}

void CUILrc::RedrawSB(BOOL bImmdShow)
{
    auto pDC = m_pBK->m_pDC;
    if (bImmdShow)
        pDC->BeginDraw();
    pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcFSB, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcFSB);
    pDC->FillRectangle(&m_rcFThumb, m_pBrTextNormal);
    BkDbg_DrawElemFrame();
    if (bImmdShow)
    {
        pDC->EndDraw();
        DXGI_PRESENT_PARAMETERS pp;
        pp.DirtyRectsCount = 1;
        pp.pDirtyRects = &m_rcSB;
        pp.pScrollRect = NULL;
        pp.pScrollOffset = NULL;
        m_pBK->m_pSwapChain->Present1(0, 0, &pp);
    }
}

float CUILrc::RedrawItem(int iIndex, float y, BOOL bTop, BOOL bClearBK, BOOL bImmdShow, BOOL bCenterLine, int* yOut)
{
    //auto pDC = m_pBK->m_pDC;
    //HWND hWnd = m_pBK->m_hWnd;

    //BOOL bCurr = (iIndex == g_iCurrLrcIndex);

    //LRCDATA* p = (LRCDATA*)QKAGet(g_Lrc, iIndex);
    ////if (!p->cchTotal)
    ////    p = &GS.DTLrcSpaceLine;// 无歌词，内容转向到空行替代

    //IDWriteTextLayout* pDWTextLayout1;
    //IDWriteTextLayout* pDWTextLayout2;
    //ID2D1SolidColorBrush* pD2DBrush;

    //if (bCurr)
    //    pD2DBrush = m_pBrTextHighlight;
    //else
    //    pD2DBrush = m_pBrTextNormal;

    //D2D_RECT_F D2DRcF1, D2DRcF2;

    //if (y == INFINITY && !bCenterLine)
    //{
    //    //if (p->iDrawID != m_iDrawingID)
    //    if (p->ullLastDrawingTime != m_ullLastDrawingTime)
    //        return -1;
    //    y = p->iLastTop;
    //    bTop = TRUE;
    //}

    //int cy1, cy2;
    //int cx1, cx2;

    //DWRITE_TEXT_METRICS Metrics1, Metrics2;

    //if (GS.bForceTwoLines)// 是否禁止换行
    //{
    //    m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 禁止自动换行
    //    if (!p->pszLrc2)// 只有一行
    //    {
    //        m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

    //        if (bCurr)
    //        {
    //            if (iIndex != m_LrcHScrollInfo.iIndex)
    //            {
    //                g_pDWFactory->CreateTextLayout(p->pszLrc, p->cchLrc, m_pTextFormat, m_cxLrcF, m_cyLrcF, &pDWTextLayout1);
    //                pDWTextLayout1->GetMetrics(&m_LrcHScrollInfo.Metrics1);
    //                pDWTextLayout1->Release();

    //                cx1 = (int)m_LrcHScrollInfo.Metrics1.width;
    //                cy1 = (int)m_LrcHScrollInfo.Metrics1.height;

    //                m_LrcHScrollInfo.iIndex = iIndex;
    //                if (!m_LrcHScrollInfo.bWndSizeChangedFlag)
    //                    m_LrcHScrollInfo.x1 = m_LrcHScrollInfo.x2 = 0;
    //                else
    //                    m_LrcHScrollInfo.bWndSizeChangedFlag = FALSE;

    //                KillTimer(g_hMainWnd, IDT_ANIMATION);
    //                if (cx1 > m_cxLrc)
    //                {
    //                    m_LrcHScrollInfo.cx1 = cx1;// 超长了，需要后续滚动
    //                    m_LrcHScrollInfo.fNoScrollingTime1 = m_cxLrc * p->fDelay / m_LrcHScrollInfo.cx1 / 2;
    //                    SetTimer(g_hMainWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, NULL);
    //                }
    //                else
    //                {
    //                    m_LrcHScrollInfo.cx1 = -1;
    //                    m_LrcHScrollInfo.x1 = m_LrcHScrollInfo.x2 = 0;
    //                }
    //            }
    //            else// 横向滚动就別测高了
    //            {
    //                cx1 = (int)m_LrcHScrollInfo.Metrics1.width;
    //                cy1 = (int)m_LrcHScrollInfo.Metrics1.height;
    //            }
    //        }
    //        else
    //        {
    //            g_pDWFactory->CreateTextLayout(p->pszLrc, p->cchLrc, m_pTextFormat, m_cxLrcF, m_cyLrcF, &pDWTextLayout1);
    //            pDWTextLayout1->GetMetrics(&Metrics1);
    //            pDWTextLayout1->Release();
    //            cx1 = (int)Metrics1.width;
    //            cy1 = (int)Metrics1.height;
    //        }

    //        if (cx1 > m_cxLrc)
    //            m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    //        else
    //            m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    //        p->cy = cy1;

    //        if (bCenterLine)
    //        {
    //            y -= p->cy / 2;
    //            bTop = TRUE;
    //            *yOut = y;
    //        }

    //        if (bTop)
    //            D2DRcF2 = { m_rcF.left,(float)y,m_rcF.right,(float)(y + cy1) };
    //        else
    //            D2DRcF2 = { m_rcF.left,(float)(y - p->cy),m_rcF.right,(float)y };
    //        D2DRcF1 = D2DRcF2;


    //        if (bImmdShow)
    //        {
    //            pDC->BeginDraw();
    //            if (D2DRcF2.top < m_rc.top)
    //                D2DRcF2.top = m_rcF.top;
    //            if (D2DRcF2.bottom > m_rc.bottom)
    //                D2DRcF2.bottom = m_rcF.bottom;
    //            pDC->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    //            pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);// 画背景
    //        }

    //        if (bCurr)
    //            D2DRcF1.left += m_LrcHScrollInfo.x1;

    //        p->rcItem = { (LONG)D2DRcF1.left,(LONG)D2DRcF1.top,(LONG)D2DRcF1.right,(LONG)D2DRcF1.bottom };
    //        p->iLastTop = (int)D2DRcF1.top;
    //        pDC->DrawTextW(p->pszLrc, p->cchLrc, m_pTextFormat, &D2DRcF1, pD2DBrush);

    //    }
    //    else// 有两行
    //    {
    //        m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

    //        if (bCurr)
    //        {
    //            if (iIndex != m_LrcHScrollInfo.iIndex)
    //            {
    //                g_pDWFactory->CreateTextLayout(p->pszLrc, p->cchLrc, m_pTextFormat, m_cxLrcF, m_cyLrcF, &pDWTextLayout1);// 创建文本布局
    //                g_pDWFactory->CreateTextLayout(p->pszLrc2, p->cchLrc2, m_pTextFormat, m_cxLrcF, m_cyLrcF, &pDWTextLayout2);// 创建文本布局
    //                pDWTextLayout1->GetMetrics(&m_LrcHScrollInfo.Metrics1);
    //                pDWTextLayout2->GetMetrics(&m_LrcHScrollInfo.Metrics2);
    //                pDWTextLayout1->Release();
    //                pDWTextLayout2->Release();

    //                cx1 = (int)m_LrcHScrollInfo.Metrics1.width;
    //                cx2 = (int)m_LrcHScrollInfo.Metrics2.width;
    //                cy1 = (int)m_LrcHScrollInfo.Metrics1.height;
    //                cy2 = (int)m_LrcHScrollInfo.Metrics2.height;

    //                m_LrcHScrollInfo.iIndex = iIndex;
    //                if (!m_LrcHScrollInfo.bWndSizeChangedFlag)
    //                    m_LrcHScrollInfo.x1 = m_LrcHScrollInfo.x2 = 0;
    //                else
    //                    m_LrcHScrollInfo.bWndSizeChangedFlag = FALSE;

    //                KillTimer(g_hMainWnd, IDT_ANIMATION);
    //                if (cx1 > m_cxLrc)
    //                {
    //                    m_LrcHScrollInfo.cx1 = cx1;// 超长了，需要后续滚动
    //                    m_LrcHScrollInfo.fNoScrollingTime1 = m_cxLrc * p->fDelay / m_LrcHScrollInfo.cx1 / 2;
    //                    SetTimer(g_hMainWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, NULL);
    //                }
    //                else
    //                {
    //                    m_LrcHScrollInfo.cx1 = -1;
    //                    m_LrcHScrollInfo.x1 = 0;
    //                }

    //                if (cx2 > m_cxLrc)
    //                {
    //                    m_LrcHScrollInfo.cx2 = cx2;// 超长了，需要后续滚动
    //                    m_LrcHScrollInfo.fNoScrollingTime2 = m_cxLrc * p->fDelay / m_LrcHScrollInfo.cx2 / 2;
    //                    SetTimer(g_hMainWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, NULL);
    //                }
    //                else
    //                {
    //                    m_LrcHScrollInfo.cx2 = -1;
    //                    m_LrcHScrollInfo.x2 = 0;
    //                }
    //            }
    //            else// 横向滚动就別测高了
    //            {
    //                cx1 = (int)m_LrcHScrollInfo.Metrics1.width;
    //                cx2 = (int)m_LrcHScrollInfo.Metrics2.width;
    //                cy1 = (int)m_LrcHScrollInfo.Metrics1.height;
    //                cy2 = (int)m_LrcHScrollInfo.Metrics2.height;
    //            }
    //        }
    //        else
    //        {
    //            g_pDWFactory->CreateTextLayout(p->pszLrc, p->cchLrc, m_pTextFormat, m_cxLrcF, m_cyLrcF, &pDWTextLayout1);// 创建文本布局
    //            g_pDWFactory->CreateTextLayout(p->pszLrc2, p->cchLrc2, m_pTextFormat, m_cxLrcF, m_cyLrcF, &pDWTextLayout2);// 创建文本布局
    //            pDWTextLayout1->GetMetrics(&Metrics1);
    //            pDWTextLayout2->GetMetrics(&Metrics2);
    //            pDWTextLayout1->Release();
    //            pDWTextLayout2->Release();
    //            cx1 = (int)Metrics1.width;
    //            cx2 = (int)Metrics2.width;
    //            cy1 = (int)Metrics1.height;
    //            cy2 = (int)Metrics2.height;
    //        }

    //        p->cy = cy1 + cy2;

    //        if (bCenterLine)
    //        {
    //            y -= p->cy / 2;
    //            bTop = TRUE;
    //            *yOut = y;
    //        }

    //        if (bTop)
    //        {
    //            D2DRcF2 = { m_rcF.left,(float)y,m_rcF.right,(float)(y + p->cy) };

    //            if (bImmdShow)
    //            {
    //                pDC->BeginDraw();
    //                D2DRcF1 = D2DRcF2;
    //                if (D2DRcF2.top < m_rc.top)
    //                    D2DRcF2.top = m_rcF.top;
    //                if (D2DRcF2.bottom > m_rc.bottom)
    //                    D2DRcF2.bottom = m_rcF.bottom;
    //                pDC->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    //                pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);
    //            }

    //            D2DRcF1.top = (FLOAT)y;
    //            p->iLastTop = (int)D2DRcF1.top;
    //            D2DRcF1.bottom = D2DRcF1.top + cy1;
    //            if (bCurr)
    //                D2DRcF1.left = (FLOAT)(m_rc.left + m_LrcHScrollInfo.x1);
    //            else
    //                D2DRcF1.left = m_rcF.left;
    //            D2DRcF1.right = m_rcF.right;
    //            if (cx1 > m_cxLrc)
    //                m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    //            else
    //                m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    //            pDC->DrawTextW(p->pszLrc, p->cchLrc, m_pTextFormat, &D2DRcF1, pD2DBrush);

    //            D2DRcF1.top = D2DRcF1.bottom;
    //            D2DRcF1.bottom += cy2;
    //            if (bCurr)
    //                D2DRcF1.left = (FLOAT)(m_rc.left + m_LrcHScrollInfo.x2);

    //            D2DRcF1.right = m_rcF.right;
    //            if (cx2 > m_cxLrc)
    //                m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    //            else
    //                m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    //            pDC->DrawTextW(p->pszLrc2, p->cchLrc2, m_pTextFormat, &D2DRcF1, pD2DBrush);
    //            D2DRcF1.top = (FLOAT)y;
    //            D2DRcF1.bottom = D2DRcF1.top + p->cy;
    //        }
    //        else
    //        {
    //            D2DRcF2 = { m_rcF.left,(float)(y - p->cy),m_rcF.right,(float)y };

    //            if (bImmdShow)
    //            {
    //                pDC->BeginDraw();
    //                D2DRcF1 = D2DRcF2;
    //                if (D2DRcF2.top < m_rc.top)
    //                    D2DRcF2.top = m_rcF.top;
    //                if (D2DRcF2.bottom > m_rc.bottom)
    //                    D2DRcF2.bottom = m_rcF.bottom;
    //                pDC->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    //                pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);
    //            }

    //            D2DRcF1.bottom = (FLOAT)y;
    //            D2DRcF1.top = D2DRcF1.bottom - cy2;
    //            if (bCurr)
    //                D2DRcF1.left = (FLOAT)(m_rc.left + m_LrcHScrollInfo.x2);
    //            else
    //                D2DRcF1.left = m_rcF.left;
    //            D2DRcF1.right = m_rcF.right;
    //            if (cx2 > m_cxLrc)
    //                m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    //            else
    //                m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    //            pDC->DrawTextW(p->pszLrc2, p->cchLrc2, m_pTextFormat, &D2DRcF1, pD2DBrush);

    //            D2DRcF1.bottom = D2DRcF1.top;
    //            D2DRcF1.top -= cy1;
    //            p->iLastTop = (int)D2DRcF1.top;
    //            if (bCurr)
    //                D2DRcF1.left = (FLOAT)(m_rc.left + m_LrcHScrollInfo.x1);
    //            D2DRcF1.right = m_rcF.right;
    //            if (cx1 > m_cxLrc)
    //                m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    //            else
    //                m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    //            pDC->DrawTextW(p->pszLrc, p->cchLrc, m_pTextFormat, &D2DRcF1, pD2DBrush);
    //            D2DRcF1.bottom = (FLOAT)y;
    //            D2DRcF1.top = D2DRcF1.bottom - p->cy;
    //        }
    //        p->rcItem = { (LONG)D2DRcF1.left,(LONG)D2DRcF1.top,(LONG)D2DRcF1.right,(LONG)D2DRcF1.bottom };
    //    }
    //}
    //else
    //{
    //    m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    //    m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);// 自动换行
    //    //uStrLen1 = lstrlenW(p->pszLrc);
    //    g_pDWFactory->CreateTextLayout(p->pszLrc, p->cchTotal, m_pTextFormat, m_cxLrcF, m_cyLrcF, &pDWTextLayout1);
    //    pDWTextLayout1->GetMetrics(&Metrics1);
    //    pDWTextLayout1->Release();
    //    cy1 = (int)Metrics1.height;
    //    p->cy = cy1;

    //    if (bCenterLine)
    //    {
    //        y -= p->cy / 2;
    //        bTop = TRUE;
    //        *yOut = y;
    //    }

    //    if (bTop)
    //        D2DRcF2 = { m_rcF.left,(float)y,m_rcF.right,(float)(y + p->cy) };
    //    else
    //        D2DRcF2 = { m_rcF.left,(float)(y - p->cy),m_rcF.right,(float)y };

    //    p->iLastTop = (int)D2DRcF2.top;

    //    D2DRcF1 = D2DRcF2;

    //    if (bImmdShow)
    //    {
    //        pDC->BeginDraw();
    //        if (D2DRcF2.top < m_rc.top)
    //            D2DRcF2.top = m_rcF.top;
    //        if (D2DRcF2.bottom > m_rc.bottom)
    //            D2DRcF2.bottom = m_rcF.bottom;
    //        pDC->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    //        pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);
    //    }
    //    pDC->DrawTextW(p->pszLrc, p->cchLrc, m_pTextFormat, &D2DRcF1, pD2DBrush);
    //    p->rcItem = { (LONG)D2DRcF1.left,(LONG)D2DRcF1.top,(LONG)D2DRcF1.right,(LONG)D2DRcF1.bottom };
    //}
    //if (m_idxMouseHover == iIndex)
    //{
    //    pDC->CreateSolidColorBrush(D2D1::ColorF(QKColorrefToHex(QKCOLOR_CYANDEEPER), 0.1f), &pD2DBrush);
    //    pDC->FillRectangle(D2DRcF1, pD2DBrush);
    //    pD2DBrush->Release();
    //}

    //BkDbg_DrawElemFrame();
    //if (bImmdShow)
    //{
    //    pDC->PopAxisAlignedClip();
    //    pDC->EndDraw();
    //    RECT rc = { m_rcLrc.left,y,m_rcLrc.right,y + p->cyTotal };
    //    DXGI_PRESENT_PARAMETERS pp;
    //    pp.DirtyRectsCount = 1;
    //    pp.pDirtyRects = &rc;
    //    pp.pScrollOffset = NULL;
    //    pp.pScrollRect = NULL;
    //    m_pBK->m_pSwapChain->Present1(0, 0, &pp);
    //}

    //p->iDrawID = m_iDrawingID;// 已经绘制标记
    //p->ullLastDrawingTime = m_ullLastDrawingTime;
    //return p->cy;
return 0;
}

int CUILrc::HitTest(POINT pt)
{
    //if (!g_Lrc->iCount || !g_hStream)
    //    return -1;

    //LRCDATA* p = (LRCDATA*)QKAGet(g_Lrc, m_idxCenter);
    //int i = m_idxCenter;
    //if (pt.y >= p->rcItem.top)// 落在下半部分（包括中间一项）
    //{
    //    while (p->iDrawID == m_iDrawingID)
    //    {
    //        if (PtInRect(&p->rcItem, pt))
    //            return i;
    //        ++i;
    //        if (i >= g_Lrc->iCount)
    //            break;
    //        p = (LRCDATA*)QKAGet(g_Lrc, i);
    //    }
    //}
    //else// 落在上半部分
    //{
    //    --i;// 中间一项就不判断了，跳过
    //    if (i < 0)
    //        return -1;
    //    p = (LRCDATA*)QKAGet(g_Lrc, i);
    //    while (p->iDrawID == m_iDrawingID)
    //    {
    //        if (PtInRect(&p->rcItem, pt))
    //            return i;
    //        --i;
    //        if (i < 0)
    //            break;
    //        p = (LRCDATA*)QKAGet(g_Lrc, i);
    //    }
    //}

    return -1;
}

LRESULT CUILrc::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
    auto lResult = DefElemEventProc(uEvent, wParam, lParam);
    //    switch (uEvent)
    //    {
    //    case UIEE_SETRECT:
    //    {
    //        m_rcLrc = m_rc;
    //        m_rcLrc.right -= GC.DS_CXLRCTB;
    //        m_cxLrc = m_rcLrc.right - m_rcLrc.left;
    //        m_cyLrc = m_rcLrc.bottom - m_rcLrc.top;
    //
    //        m_rcFLrc = QKRect2D2DRcF(m_rcLrc);
    //        m_cxLrcF = m_rcFLrc.right - m_rcFLrc.left;
    //        m_cyLrcF = m_rcFLrc.bottom - m_rcFLrc.top;
    //
    //        m_rcSB = m_rc;
    //        m_rcSB.left = m_rcLrc.right;
    //        m_rcFSB = QKRect2D2DRcF(m_rcSB);
    //
    //
    //        int cxSB = m_rcSB.right - m_rcSB.left;
    //        m_rcThumb.left = m_rcSB.left;
    //        m_rcThumb.right = m_rcSB.right;
    //        if (m_uSBMax)
    //        {
    //            m_iThumbSize = m_cy * m_cy / m_uSBMax;
    //            m_rcThumb.top = m_rcSB.top + m_uSBPos * (m_cy - m_iThumbSize) / m_uSBMax;
    //            m_rcThumb.bottom = m_rcThumb.top + m_iThumbSize;
    //
    //            m_rcFThumb = QKRect2D2DRcF(m_rcThumb);
    //        }
    //    }
    //    break;
    //
    //    case UIEE_ONPLAYINGCTRL:
    //    {
    //
    //        // break;
    //
    //
    //        DWRITE_TEXT_METRICS Metrics;
    //        LRCDATA* p;
    //        IDWriteTextLayout* pTextLayout;
    //#define COUNT_LRCITEMHEIGHTGROUP    4
    //        if (m_pTextFormat)
    //        {
    //
    //            //GS.bForceTwoLines = FALSE;
    //            float cyTotal = 0.f;
    //            if (GS.bForceTwoLines)
    //            {
    //                m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    //                m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    //                for (int i = 0; i < g_Lrc->iCount; ++i)
    //                {
    //                    p = (LRCDATA*)QKAGet(g_Lrc, i);
    //                    g_pDWFactory->CreateTextLayout(p->pszLrc, p->cchLrc, m_pTextFormat, m_cxLrcF, m_cyLrcF, &p->pTextLayout1);
    //                    p->pTextLayout1->GetMetrics(&Metrics);
    //
    //                    p->cx1 = Metrics.width;
    //                    p->cy1 = Metrics.height;
    //                    if (p->pszLrc2)
    //                    {
    //                        g_pDWFactory->CreateTextLayout(p->pszLrc2, p->cchLrc2, m_pTextFormat, m_cxLrcF, m_cyLrcF, &p->pTextLayout2);
    //                        p->pTextLayout2->GetMetrics(&Metrics);
    //
    //                        p->cx2 = Metrics.width;
    //                        p->cy2 = Metrics.height;
    //                        p->cyTotal = p->cy1 + p->cy2;
    //                    }
    //                    else
    //                    {
    //                        p->cx2 = 0.f;
    //                        p->cy2 = 0.f;
    //                        p->cyTotal = p->cy1;
    //                    }
    //                    cyTotal += p->cyTotal;
    //                }
    //            }
    //            else
    //            {
    //                m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    //                m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
    //                for (int i = 0; i < g_Lrc->iCount; ++i)
    //                {
    //                    p = (LRCDATA*)QKAGet(g_Lrc, i);
    //                    g_pDWFactory->CreateTextLayout(p->pszLrc, p->cchTotal, m_pTextFormat, m_cxLrcF, m_cyLrcF, &p->pTextLayout1);
    //                    p->pTextLayout1->GetMetrics(&Metrics);
    //
    //                    p->cx1 = Metrics.width;
    //                    p->cy1 = Metrics.height;
    //                    p->cx2 = 0.f;
    //                    p->cy2 = 0.f;
    //                    p->cyTotal = p->cy1;
    //                    cyTotal += p->cyTotal;
    //                }
    //            }
    //
    //            m_uSBMax = (int)cyTotal + GS.iSCLrcLineGap * (g_Lrc->iCount - 1);
    //            m_uSBPos = 0;
    //
    //            int cxSB = m_rcSB.right - m_rcSB.left;
    //            //m_rcThumb.left = m_rcSB.left + (cxSB * 3 / 4);
    //            //m_rcThumb.right = m_rcThumb.left + (cxSB / 4);
    //            if (m_uSBMax)
    //            {
    //                m_iThumbSize = m_cy * m_cy / m_uSBMax;
    //                m_rcThumb.top = m_rcSB.top + m_uSBPos * (m_cy - m_iThumbSize) / m_uSBMax;
    //                m_rcThumb.bottom = m_rcThumb.top + m_iThumbSize;
    //
    //                m_rcFThumb = QKRect2D2DRcF(m_rcThumb);
    //            }
    //            //    m_ItemHeightTree.DestroyTree();
    //
    //            //    QKARRAY aGroupCount = QKACreate(0, 30);
    //            //    int cLrc = g_Lrc->iCount;
    //            //    while (TRUE)
    //            //    {
    //            //        if (cLrc % COUNT_LRCITEMHEIGHTGROUP)
    //            //            cLrc = cLrc / COUNT_LRCITEMHEIGHTGROUP + 1;
    //            //        else
    //            //            cLrc = cLrc / COUNT_LRCITEMHEIGHTGROUP;
    //
    //            //        if (cLrc)
    //            //            QKAAddValue(aGroupCount, cLrc);
    //            //        else
    //            //            break;
    //            //    }
    //            //    int cGourp;
    //            //    LRCITEMHEIGHTDATA* pNodeData;
    //            //    QKTREENODE* pParentNode = NULL;
    //            //    for (int i = aGroupCount->iCount; i >= 0; ++i)
    //            //    {
    //            //        cGourp = QKAGetValue(aGroupCount, i);
    //            //        for (int j = 0; j < cGourp; ++j)
    //            //        {
    //            //            pNodeData = new LRCITEMHEIGHTDATA;
    //            //            m_ItemHeightTree.InsertNode(pParentNode);
    //            //        }
    //            //    }
    //
    //            //    int idx = 0;
    //            //    float cyGroup = 0.f;
    //            //    while (TRUE)
    //            //    {
    //            //        for (int i = idx; i < idx + COUNT_LRCITEMHEIGHTGROUP; ++i)
    //            //        {
    //            //            p = (LRCDATA*)QKAGet(g_Lrc, i);
    //            //            cyGroup += p->cyTotal;
    //
    //
    //            //            if (idx >= g_Lrc->iCount)
    //            //                goto EndBuildTree;
    //            //        }
    //            //    }
    //
    //            //EndBuildTree:
    //
    //            m_uSBMax = g_Lrc->iCount - 1;
    //            m_uSBPos = 0;
    //        }
    //    }
    //    break;
    //    }
    return lResult;
}