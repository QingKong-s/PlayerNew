#include "CWndBK.h"

void CUIInfo::UpdateTextFormat()
{
    SAFE_RELEASE(m_pTfTitle);
    SAFE_RELEASE(m_pTfTip);
    App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        m_pBK->Dpi(18.f), L"zh-cn", &m_pTfTitle);
    App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        m_pBK->Dpi(12.f), L"zh-cn", &m_pTfTip);

    DWRITE_TRIMMING DWTrimming
    {
        DWRITE_TRIMMING_GRANULARITY_CHARACTER,// 按字符裁剪
        0,
        0
    };
    IDWriteInlineObject* pDWInlineObj;// 省略号裁剪内联对象
    App->m_pDwFactory->CreateEllipsisTrimmingSign(m_pTfTitle, &pDWInlineObj);// 创建省略号裁剪
    if (!pDWInlineObj)
        return;
    ///////////置文本格式
    m_pTfTitle->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_pTfTitle->SetTrimming(&DWTrimming, pDWInlineObj);// 置溢出裁剪
    m_pTfTitle->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 不换行
    m_pTfTip->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_pTfTip->SetTrimming(&DWTrimming, pDWInlineObj);// 置溢出裁剪
    m_pTfTip->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 不换行
    pDWInlineObj->Release();
}

CUIInfo::CUIInfo()
{
    m_uType = UIET_INFO;
    m_uFlags = UIEF_STATIC;
}

void CUIInfo::Redraw()
{
    auto pDC = m_pBK->m_pDC;
    ////////////////////画顶部提示信息
    PCWSTR psz;
    if (App->GetPlayer().IsFileActive())
    {
        const auto& Item = App->GetPlayer().GetList().At(App->GetPlayer().GetCurrFile());
        psz = Item.rsName.Data();
    }
    else
        psz = L"未播放";
    ///////////画大标题
    D2D1_RECT_F rcF{ m_rcF.left,m_rcF.top,m_rcF.right,m_rcF.top + m_pBK->m_DsF.cyTopTitle };
    pDC->DrawTextW(psz, lstrlenW(psz), m_pTfTitle, &rcF, m_pBrBigTip);
    ///////////画其他信息
    constexpr PCWSTR pszTip[]
    {
        L"标题：",
        L"艺术家：",
        L"专辑：",
        L"备注："
    };

    const auto& MusicInfo = App->GetPlayer().GetMusicInfo();
    const PCWSTR pszTip2[]
    {
        MusicInfo.rsTitle.Data(),
        MusicInfo.rsArtist.Data(),
        MusicInfo.rsAlbum.Data(),
        MusicInfo.rsComment.Data()
    };
    const UINT32 cchTip2[]
    {
        (UINT32)MusicInfo.rsTitle.Size(),
        (UINT32)MusicInfo.rsArtist.Size(),
        (UINT32)MusicInfo.rsAlbum.Size(),
        (UINT32)MusicInfo.rsComment.Size()
    };

    const auto cxTopTip = m_pBK->m_DsF.cxTopTip;
    const auto cyTopTip = m_pBK->m_DsF.cyTopTip;
    rcF.right = rcF.left + cxTopTip;
    rcF.top += (cyTopTip + m_pBK->m_DsF.sizeTopTipGap);
    rcF.bottom = rcF.top + cyTopTip;

    int cxElem = m_rc.right - m_rc.left;
    for (int i = 0; i < ARRAYSIZE(pszTip); ++i)
    {
        pDC->DrawTextW(pszTip[i], (UINT32)wcslen(pszTip[i]), m_pTfTip, &rcF, m_pBrSmallTip);
        if (pszTip2[i])
        {
            rcF.left += cxTopTip;
            rcF.right = m_rcF.right;
            pDC->DrawTextW(pszTip2[i], cchTip2[i], m_pTfTip, &rcF, m_pBrSmallTip);
            rcF.left = m_rcF.left;
            rcF.right = rcF.left + cxTopTip;
        }
        rcF.top += cyTopTip;
        rcF.bottom = rcF.top + cyTopTip;
    }
    BkDbg_DrawElemFrame();
}

BOOL CUIInfo::InitElem()
{
    SAFE_RELEASE(m_pBrBigTip);
    SAFE_RELEASE(m_pBrSmallTip);
    auto pDC = m_pBK->m_pDC;
    pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrBigTip);
    pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBrSmallTip);
    UpdateTextFormat();
    return TRUE;
}

CUIInfo::~CUIInfo()
{
    SAFE_RELEASE(m_pBrBigTip);
    SAFE_RELEASE(m_pBrSmallTip);
    SAFE_RELEASE(m_pTfTitle);
    SAFE_RELEASE(m_pTfTip);
}