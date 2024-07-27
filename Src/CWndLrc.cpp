#include "pch.h"
#include "CWndLrc.h"

#include "COptionsMgr.h"
#include "CWndMain.h"

void CWndLrc::ReSizeRenderStuff(int cx, int cy)
{
	ReleaseTextCache();
	m_DC.Create(m_hWnd, cx, cy);

	constexpr D2D1_RENDER_TARGET_PROPERTIES DcRtProp
	{
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		{DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
		0.f,
		0.f,
		D2D1_RENDER_TARGET_USAGE_NONE,
		D2D1_FEATURE_LEVEL_DEFAULT
	};

	SafeRelease(m_pRT);
	HRESULT hr;
	if (FAILED(hr = App->m_pD2dFactory->CreateDCRenderTarget(&DcRtProp, &m_pRT)))
	{
		EckDbgBreak();
	}

	RECT rc{ 0,0,cx,cy };
	m_pRT->BindDC(m_DC.GetDC(), &rc);
	m_pRT->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

	UpdateSysColorBrush();
	UpdateTextBrush();
	SafeRelease(m_pBrBk);
	SafeRelease(m_pBrFrame);
	m_pRT->CreateSolidColorBrush(D2D1::ColorF(0x3F3F3F, 0.5), &m_pBrBk);
	m_pRT->CreateSolidColorBrush(D2D1::ColorF(0x97D2CB, 0.5), &m_pBrFrame);

	SafeRelease(m_pDC1);
	m_pRT->QueryInterface(&m_pDC1);

	m_BtnBox.PostCreateRenderTarget();
}

void CWndLrc::UpdateSysColorBrush()
{
	SafeRelease(m_pBrHot);
	SafeRelease(m_pBrPressed);

	m_pRT->CreateSolidColorBrush(
		eck::ColorrefToD2dColorF(GetSysColor(COLOR_HOTLIGHT), 0.5f), &m_pBrHot);
	m_pRT->CreateSolidColorBrush(
		eck::ColorrefToD2dColorF(GetSysColor(COLOR_HIGHLIGHT), 0.8f), &m_pBrPressed);
}

void CWndLrc::Draw()
{
	m_pRT->BeginDraw();
	m_pRT->Clear({});

	float y = m_DsF.Margin;
	if (m_bShowBk && !m_bLocked)
	{
		//--------画背景
		m_pRT->FillRectangle(D2D1::RectF(0, 0, (float)m_cxClient, (float)m_cyClient), m_pBrBk);
		m_pRT->DrawRectangle(D2D1::RectF(y, y, (float)m_cxClient - y, (float)m_cyClient - y), m_pBrFrame, y);
	}
	//--------画按钮
	m_BtnBox.Draw();

	y += ((float)m_Ds.cxyBtn + m_DsF.Margin);

	const auto idxCurr = App->GetPlayer().GetCurrLrc();
	const auto& vLrc = App->GetPlayer().GetLrc();

	const float m = m_DsF.Margin * 3;
	m_pRT->PushAxisAlignedClip({ m,m,m_cxClient - m,m_cyClient - m }, D2D1_ANTIALIAS_MODE_ALIASED);

	if (!App->GetPlayer().IsFileActive())
		DrawStaticLine(LRCIDX_APPNAME, y);
	else if (idxCurr >= 0)
	{
		float cy;
		if (idxCurr % 2)
		{
			cy = DrawLrcLine(idxCurr, y, FALSE);
			if (idxCurr + 1 < (int)vLrc.size())
				DrawLrcLine(idxCurr + 1, y + cy + m_DsF.TextPadding, TRUE);
		}
		else
		{
			int idx = idxCurr + 1;
			if (idx >= (int)vLrc.size())
				idx = idxCurr - 1;

			if (idx >= 0 && idx < (int)vLrc.size())
				cy = DrawLrcLine(idx, y, FALSE);
			else
				cy = 0.f;
			DrawLrcLine(idxCurr, y + cy + m_DsF.TextPadding, TRUE);
		}
	}
	else
		DrawStaticLine(LRCIDX_FILENAME, y);
	m_pRT->PopAxisAlignedClip();
	m_pRT->EndDraw();

	POINT pt{};
	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 0xFF;
	bf.AlphaFormat = AC_SRC_ALPHA;
	SIZE size{ m_cxClient,m_cyClient };
	UpdateLayeredWindow(m_hWnd, NULL, NULL, &size, m_DC.GetDC(), &pt, 0, &bf, ULW_ALPHA);
}

float CWndLrc::DrawLrcLine(int idxLrc, float y, BOOL bSecondLine)
{
	EckAssert(bSecondLine == 1 || bSecondLine == 0);
	const auto& Lrc = App->GetPlayer().GetLrc()[idxLrc];
	auto& Cache = m_TextCache[bSecondLine];
	const BOOL bHiLight = (idxLrc == App->GetPlayer().GetCurrLrc());

	const float cxMax = CalcMaxLrcWidth();
	const float xStart = CalcLrcMargin();

	if (Cache.idxLrc != idxLrc)// 更新缓存
	{
		float xDpi, yDpi;
		m_pDC1->GetDpi(&xDpi, &yDpi);

		const float fTolerance = D2D1::ComputeFlatteningTolerance(
			D2D1::Matrix3x2F::Identity(), xDpi, yDpi, 1.f);

		const float cxStroke = eck::DpiScaleF(App->GetOptionsMgr().DtLrcBorderWidth * 2.f, m_iDpi);

		float cyOld;

		Cache.idxLrc = idxLrc;
		SafeRelease(Cache.pLayout);
		SafeRelease(Cache.pLayoutTrans);

		constexpr WCHAR szEmpty[]{ L"♪♬♪♬♪" };
		constexpr int cchEmpty = (int)(ARRAYSIZE(szEmpty) - 1);
		Utils::LRCINFO FakeLrc{ (PWSTR)szEmpty,nullptr,cchEmpty,cchEmpty,Lrc.fTime,Lrc.fDuration };

		const auto& LrcNew = (Lrc.cchTotal == 0) ? FakeLrc : Lrc;
		//-----------重建文本布局
		DWRITE_TEXT_METRICS tm;
		const auto& om = App->GetOptionsMgr();
		m_pTfMain->SetTextAlignment((DWRITE_TEXT_ALIGNMENT)om.DtLrcAlign[bSecondLine]);
		App->m_pDwFactory->CreateTextLayout(LrcNew.pszLrc, LrcNew.cchLrc, m_pTfMain,
			cxMax, (float)m_cyClient, &Cache.pLayout);

		Cache.pLayout->GetMetrics(&tm);
		if (tm.width > cxMax)
		{
			Cache.pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			Cache.pLayout->GetMetrics(&tm);
			Cache.bTooLong = TRUE;
		}
		else
			Cache.bTooLong = FALSE;
		cyOld = Cache.size.height;
		Cache.size = { tm.width,tm.height };
		//-----------重建画刷
		if (!Cache.pBr || !eck::FloatEqual(cyOld, tm.height))
		{
			SafeRelease(Cache.pBr);
			ReCreateBrush(tm.height, 0.f, bHiLight, &Cache.pBr, NULL);
		}
		//-----------重建几何实现
		ID2D1PathGeometry1* pPath;
		eck::GetTextLayoutPathGeometry(Cache.pLayout, m_pRT, 0, 0, pPath);
		SafeRelease(Cache.pGrF);
		SafeRelease(Cache.pGrS);
		m_pDC1->CreateFilledGeometryRealization(pPath, fTolerance, &Cache.pGrF);
		m_pDC1->CreateStrokedGeometryRealization(pPath, fTolerance, 
			cxStroke, NULL, &Cache.pGrS);
		pPath->Release();

		if (LrcNew.pszTranslation)
		{
			//-----------重建文本布局
			m_pTfTranslation->SetTextAlignment((DWRITE_TEXT_ALIGNMENT)om.DtLrcAlign[bSecondLine]);
			App->m_pDwFactory->CreateTextLayout(LrcNew.pszTranslation, LrcNew.cchTotal - Lrc.cchLrc, m_pTfTranslation,
				cxMax, (float)m_cyClient, &Cache.pLayoutTrans);
			Cache.pLayoutTrans->GetMetrics(&tm);
			if (tm.width > cxMax)
			{
				Cache.pLayoutTrans->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				Cache.pLayoutTrans->GetMetrics(&tm);
				Cache.bTooLongTrans = TRUE;
			}
			else
				Cache.bTooLongTrans = FALSE;
			cyOld = Cache.size.height;
			Cache.sizeTrans = { tm.width,tm.height };
			//-----------重建画刷
			if (!Cache.pBrTrans || !eck::FloatEqual(cyOld, tm.height))
			{
				SafeRelease(Cache.pBrTrans);
				ReCreateBrush(0.f, tm.height, bHiLight, NULL, &Cache.pBrTrans);
			}
			//-----------重建几何实现
			ID2D1PathGeometry1* pPath;
			eck::GetTextLayoutPathGeometry(Cache.pLayoutTrans, m_pRT, 0, 0, pPath);

			SafeRelease(Cache.pGrFTrans);
			SafeRelease(Cache.pGrSTrans);
			m_pDC1->CreateFilledGeometryRealization(pPath, fTolerance, &Cache.pGrFTrans);
			m_pDC1->CreateStrokedGeometryRealization(pPath, fTolerance, 
				cxStroke, NULL, &Cache.pGrSTrans);
			pPath->Release();
		}

		ZeroMemory(&FakeLrc, sizeof(FakeLrc));
	}

	const float dTime = App->GetPlayer().GetPosF() - Lrc.fTime;

	const float cxMaxHalf = cxMax / 2.f;
	float dx;
	if (Cache.bTooLong)
	{
		dx = dTime * Cache.size.width / Lrc.fDuration;
		if (dx < cxMaxHalf)
			dx = 0.f;
		else if (dx > Cache.size.width - cxMaxHalf)
			dx = cxMax - Cache.size.width;
		else
			dx = cxMaxHalf - dx;
	}
	else
		dx = 0.f;
#pragma warning(push)
#pragma warning(disable:6387)// 可能为NULL
	D2D1_MATRIX_3X2_F mat0;
	m_pDC1->GetTransform(&mat0);
	m_pDC1->SetTransform(D2D1::Matrix3x2F::Translation(xStart + dx, y));
	m_pDC1->DrawGeometryRealization(Cache.pGrS, m_pBrTextBorder);
	m_pDC1->DrawGeometryRealization(Cache.pGrF, Cache.pBr);
	m_pDC1->SetTransform(mat0);

	float cy = Cache.size.height;
	if (Cache.pLayoutTrans)
	{
		const float yNew = y + cy;
		if (Cache.bTooLongTrans)
		{
			dx = dTime * Cache.sizeTrans.width / Lrc.fDuration;
			if (dx < cxMaxHalf)
				dx = 0;
			else if (dx > Cache.sizeTrans.width - cxMaxHalf)
				dx = cxMax - Cache.sizeTrans.width;
			else
				dx = cxMaxHalf - dx;
		}
		else
			dx = 0.f;

		m_pDC1->GetTransform(&mat0);
		m_pDC1->SetTransform(D2D1::Matrix3x2F::Translation(xStart + dx, yNew));
		m_pDC1->DrawGeometryRealization(Cache.pGrSTrans, m_pBrTextBorder);
		m_pDC1->DrawGeometryRealization(Cache.pGrFTrans, Cache.pBrTrans);
		m_pDC1->SetTransform(mat0);

		cy += Cache.sizeTrans.height;
	}
#pragma warning(pop)
	return cy;
}

void CWndLrc::DrawStaticLine(int idxFake, float y)
{
	EckAssert(idxFake == LRCIDX_APPNAME || idxFake == LRCIDX_FILENAME);
	auto& Cache = m_TextCache[0];
	if (Cache.idxLrc != idxFake)
	{
		float xDpi, yDpi;
		m_pDC1->GetDpi(&xDpi, &yDpi);

		const float fTolerance = D2D1::ComputeFlatteningTolerance(
			D2D1::Matrix3x2F::Identity(), xDpi, yDpi, 1.f);

		const float cxStroke = eck::DpiScaleF(App->GetOptionsMgr().DtLrcBorderWidth * 2.f, m_iDpi);

		Cache.idxLrc = idxFake;
		SafeRelease(Cache.pLayout);
		SafeRelease(Cache.pLayoutTrans);
		SafeRelease(Cache.pBrTrans);
		SafeRelease(Cache.pGrFTrans);
		SafeRelease(Cache.pGrSTrans);
		Cache.bScrolling = FALSE;
		Cache.bTooLong = Cache.bTooLongTrans = FALSE;

		constexpr static WCHAR c_szAppName[]{ L"PlayerNew - VC++/Win32" };
		PCWSTR pszText;
		int cchText;
		switch (idxFake)
		{
		case LRCIDX_APPNAME:
			pszText = c_szAppName;
			cchText = ARRAYSIZE(c_szAppName) - 1;
			break;
		case LRCIDX_FILENAME:
		{
			const auto& v = App->GetPlayer().GetList().GetList();
			pszText = v[App->GetPlayer().GetCurrFile()].rsName.Data();
			cchText = v[App->GetPlayer().GetCurrFile()].rsName.Size();
		}
		break;
		default:
			EckDbgBreak();
			return;
		}

		const float cxMax = CalcMaxLrcWidth();

		m_pTfMain->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		m_pTfMain->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
		App->m_pDwFactory->CreateTextLayout(pszText, cchText, m_pTfMain,
			cxMax, (float)m_cyClient, &Cache.pLayout);
		DWRITE_TEXT_METRICS tm;
		Cache.pLayout->GetMetrics(&tm);
		const float cyOld = Cache.size.height;
		Cache.size = { tm.width,tm.height };
		m_pTfMain->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		//-----------重建画刷
		if (!Cache.pBr || !eck::FloatEqual(cyOld, tm.height))
		{
			SafeRelease(Cache.pBr);
			ReCreateBrush(tm.height, 0.f, FALSE, &Cache.pBr, NULL);
		}
		//-----------重建几何实现
		ID2D1PathGeometry1* pPath;
		eck::GetTextLayoutPathGeometry(Cache.pLayout, m_pRT, 0, 0, pPath);
		SafeRelease(Cache.pGrF);
		SafeRelease(Cache.pGrS);
		m_pDC1->CreateFilledGeometryRealization(pPath, fTolerance, &Cache.pGrF);
		m_pDC1->CreateStrokedGeometryRealization(pPath, fTolerance,
			cxStroke, NULL, &Cache.pGrS);
		pPath->Release();
	}

#pragma warning(push)
#pragma warning(disable:6387)// 可能为NULL
	D2D1_MATRIX_3X2_F mat0;
	m_pDC1->GetTransform(&mat0);
	m_pDC1->SetTransform(D2D1::Matrix3x2F::Translation(CalcLrcMargin(), y));
	m_pDC1->DrawGeometryRealization(Cache.pGrS, m_pBrTextBorder);
	m_pDC1->DrawGeometryRealization(Cache.pGrF, Cache.pBr);
	m_pDC1->SetTransform(mat0);
#pragma warning(pop)
}

void CWndLrc::UpdateTextFormat()
{
	SafeRelease(m_pTfMain);
	SafeRelease(m_pTfTranslation);

	const auto& Font = App->GetOptionsMgr().DtLrcFontMain;
	m_pTfMain = FONTOPTIONS::CreateTextFormat(App->m_pDwFactory, Font, m_iDpi);

	const auto& Font1 = App->GetOptionsMgr().DtLrcFontTranslation;
	m_pTfTranslation = FONTOPTIONS::CreateTextFormat(App->m_pDwFactory, Font1, m_iDpi);
}

void CWndLrc::UpdateOptionRes()
{
	UpdateTextFormat();
	UpdateTextBrush();
}

void CWndLrc::UpdateDpi(int iDpi)
{
	m_iDpi = iDpi;
	eck::UpdateDpiSize(m_Ds, iDpi);
	eck::UpdateDpiSizeF(m_DsF, iDpi);
	UpdateTextFormat();
}

void CWndLrc::ReCreateBrush(float cyTextMain, float cyTextTrans, BOOL bHiLight,
	ID2D1LinearGradientBrush** ppBrMain, ID2D1LinearGradientBrush** ppBrTrans)
{
	D2D1_GRADIENT_STOP Stops[]{ {0.f},{1.f} };
	ID2D1GradientStopCollection* pStopCollection;
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES Prop
	{
		D2D1::Point2F(0, 0),
		D2D1::Point2F(0, 0),
	};
	ID2D1LinearGradientBrush* pBrush;
	if(ppBrMain)
	{
		const auto& Font = App->GetOptionsMgr().DtLrcFontMain;
		Stops[0].color = eck::ARGBToD2dColorF(bHiLight ? Font.argbHiLightGra[0] : Font.argbNormalGra[0]);
		Stops[1].color = eck::ARGBToD2dColorF(bHiLight ? Font.argbHiLightGra[1] : Font.argbNormalGra[1]);
		m_pRT->CreateGradientStopCollection(Stops, 2, &pStopCollection);
		EckAssert(pStopCollection);

		Prop.endPoint.y = cyTextMain;
		m_pRT->CreateLinearGradientBrush(&Prop, NULL, pStopCollection, &pBrush);
		pStopCollection->Release();
		*ppBrMain = pBrush;
	}
	if (ppBrTrans)
	{
		const auto& Font = App->GetOptionsMgr().DtLrcFontTranslation;

		Stops[0].color = eck::ARGBToD2dColorF(bHiLight ? Font.argbHiLightGra[0] : Font.argbNormalGra[0]);
		Stops[1].color = eck::ARGBToD2dColorF(bHiLight ? Font.argbHiLightGra[1] : Font.argbNormalGra[1]);
		m_pRT->CreateGradientStopCollection(Stops, 2, &pStopCollection);
		EckAssert(pStopCollection);

		Prop.endPoint.y = cyTextTrans;
		m_pRT->CreateLinearGradientBrush(&Prop, NULL, pStopCollection, &pBrush);
		pStopCollection->Release();
		*ppBrTrans = pBrush;
	}
}

void CWndLrc::ReleaseTextCache()
{
	for (auto& e : m_TextCache)
	{
		SafeRelease(e.pLayout);
		SafeRelease(e.pLayoutTrans);
		SafeRelease(e.pGrF);
		SafeRelease(e.pGrS);
		SafeRelease(e.pGrFTrans);
		SafeRelease(e.pGrSTrans);
		SafeRelease(e.pBr);
		SafeRelease(e.pBrTrans);
	}
	InvalidateCache();
}

LRESULT CWndLrc::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_BtnBox.MouseEvent(uMsg, wParam, lParam);
	switch (uMsg)
	{
	case WM_TIMER:
	{
		switch (wParam)
		{
		case IDT_MOUSELEAVE:
		{
			RECT rc;
			GetWindowRect(hWnd, &rc);
			POINT pt;
			GetCursorPos(&pt);
			if (!m_bNcLBtnDown && !PtInRect(&rc, pt))
			{
				m_bShowBk = FALSE;
				KillTimer(hWnd, IDT_MOUSELEAVE);
				Draw();
			}
		}
		return 0;

		case IDT_LOCK:
		{
			RECT rc;
			GetWindowRect(hWnd, &rc);
			POINT pt;
			GetCursorPos(&pt);
			BOOL b = PtInRect(&rc, pt);
			if (b != m_bDrawLockBtn)
			{
				m_bDrawLockBtn = b;
				Draw();
			}

			if (b)
			{
				ScreenToClient(hWnd, &pt);
				const int cxBtnTotal = m_Ds.cxBtnPadding * (CLrcBtnBox::c_cBtn - 1) + m_Ds.cxyBtn * CLrcBtnBox::c_cBtn;
				rc.left = (m_cxClient - cxBtnTotal) / 2;
				rc.top = (int)(m_DsF.Margin * 2);
				rc.bottom = rc.top + m_Ds.cxyBtn;

				rc.left += ((cxBtnTotal - m_Ds.cxyBtn) / 2);
				rc.right = rc.left + m_Ds.cxyBtn;
				if ((b = PtInRect(&rc, pt)) != m_bHoverLockBtn)
				{
					m_bHoverLockBtn = b;
					ModifyStyle(b ? 0u : WS_EX_TRANSPARENT, WS_EX_TRANSPARENT, GWL_EXSTYLE);
				}
			}
		}
		return 0;
		}
	}
	break;

	case WM_SIZE:
	{
		ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
		ReSizeRenderStuff(m_cxClient, m_cyClient);
		Draw();
	}
	break;

	case WM_MOUSEMOVE:
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = hWnd;
		TrackMouseEvent(&tme);
	}
	break;

	case WM_NCHITTEST:
	{
		POINT pt ECK_GET_PT_LPARAM(lParam);
		ScreenToClient(hWnd, &pt);

		const RECT rc{ 0,0,m_cxClient,m_cyClient };
		if (!m_bLocked && PtInRect(&rc, pt))
		{
			if (!m_bShowBk)
			{
				m_bShowBk = TRUE;
				SetTimer(hWnd, IDT_MOUSELEAVE, TE_MOUSELEAVE, NULL);
				Draw();
			}
		}

		if (m_BtnBox.HitTest(pt) >= 0)
			return HTCLIENT;
		else
		{
			if (!m_BtnBox.m_bLBtnDown && m_BtnBox.m_idxHot >= 0)
			{
				m_BtnBox.m_idxHot = -1;
				Draw();
			}
			const int cx = (int)(m_DsF.Margin * 2.f);
			const MARGINS m{ cx,cx,cx,cx };
			return eck::MsgOnNcHitTest(pt, m, m_cxClient, m_cyClient);
		}
	}
	break;

	case WM_GETMINMAXINFO:
	{
		const auto& om = App->GetOptionsMgr();
		auto p = (MINMAXINFO*)lParam;
		p->ptMinTrackSize = { eck::DpiScale(om.DtLrcMinSize.cx,m_iDpi),
			eck::DpiScale(om.DtLrcMinSize.cy,m_iDpi) };
	}
	return 0;

	case WM_NCLBUTTONDOWN:
	{
		m_bNcLBtnDown = TRUE;
		LRESULT lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
		m_bNcLBtnDown = FALSE;
		return lResult;
	}
	break;

	case WM_MOUSELEAVE:
	{
		if (!m_BtnBox.m_bLBtnDown && m_BtnBox.m_idxHot >= 0)
		{
			m_BtnBox.m_idxHot = -1;
			Draw();
		}
	}
	break;

	case WM_SYSCOLORCHANGE:
		UpdateSysColorBrush();
		break;

	case WM_DPICHANGED:
	{
		UpdateDpi(LOWORD(wParam));
		UpdateTextFormat();
		m_BtnBox.PostCreateRenderTarget();
		const RECT* const prc = (RECT*)lParam;
		SetWindowPos(hWnd, NULL, prc->left, prc->top, 
			prc->right - prc->left, prc->bottom - prc->top, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	return 0;

	case WM_CREATE:
	{
		RECT rc;
		GetClientRect(hWnd, &rc);

		m_cxClient = rc.right;
		m_cyClient = rc.bottom;

		UpdateDpi(eck::GetDpi(hWnd));

		SetTimer(hWnd, IDT_MOUSELEAVE, TE_MOUSELEAVE_FIRST, NULL);
	}
	return TRUE;

	case WM_DESTROY:
	{
		SafeRelease(m_pBrHot);
		SafeRelease(m_pBrPressed);
		SafeRelease(m_pBrBk);
		SafeRelease(m_pBrFrame);
		SafeRelease(m_pBrTextBorder);
		SafeRelease(m_pTfMain);
		SafeRelease(m_pTfTranslation);
		SafeRelease(m_pRT);
		SafeRelease(m_pDC1);
		m_DC.Destroy();
		ReleaseTextCache();

		auto& om = App->GetOptionsMgr();
		om.DtLrcWndPos.FromHWND(hWnd);
		om.DtLrcShow = FALSE;
	}
	break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

HWND CWndLrc::Create()
{
	const HWND hMain = App->GetMainWnd()->GetHWND();
	const auto& MinSize = App->GetOptionsMgr().DtLrcMinSize;
	auto& pos = App->GetOptionsMgr().DtLrcWndPos;
	if (pos.EnsureVisible() == eck::WPSEV_ERROR)
	{
		const int iDpi = eck::GetDpi(hMain);
		pos.cx = eck::DpiScale(MinSize.cx, iDpi);
		pos.cy = eck::DpiScale(MinSize.cy, iDpi);

		const auto hMon = MonitorFromWindow(hMain, MONITOR_DEFAULTTONULL);
		if (hMon)
		{
			eck::CWindowPosSetting posMain;
			posMain.FromHWND(hMain);
			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfoW(hMon, &mi);
			RECT rc, rcMain;
			GetWindowRect(hMain, &rcMain);
			eck::IntersectRect(rc, rcMain, mi.rcMonitor);
			if (eck::IsRectEmpty(rc))
				goto OrgPoint;
			pos.x = rc .left+ (rc.right- rc.left- pos.cx) / 2;
			pos.y = rc.bottom - pos.cy;
		}
		else
		{
		OrgPoint:
			pos.x = pos.y = 0;
		}
	}
	return Create(L"PlayerNew桌面歌词", 0, 0,
		pos.x, pos.y, pos.cx, pos.cy, NULL, NULL);
}

void CWndLrc::UpdateTextBrush()
{
	SafeRelease(m_pBrTextBorder);

	m_pRT->CreateSolidColorBrush(eck::ColorrefToD2dColorF(App->GetOptionsMgr().DtLrcBorderColor, 1.f),
		&m_pBrTextBorder);
}

void CWndLrc::DoCmd(int idx)
{
	switch (idx)
	{
	case IDLBT_PREV:
		App->GetPlayer().Prev();
		break;
	case IDLBT_PLAY:
		App->GetPlayer().PlayOrPause();
		break;
	case IDLBT_NEXT:
		App->GetPlayer().Next();
		break;
	case IDLBT_LOCK:
		Lock(!m_bLocked);
		break;
	case IDLBT_CLOSE:
		Destroy();
		break;
	}
}

void CWndLrc::Lock(BOOL bLock)
{
	if (m_bLocked == bLock)
		return;
	m_bShowBk = !bLock;
	m_bLocked = bLock;
	m_BtnBox.m_Btn[IDLBT_LOCK].bChecked = bLock;
	ModifyStyle(0u, WS_EX_TRANSPARENT, GWL_EXSTYLE);
	Draw();

	if (bLock)
		SetTimer(m_hWnd, IDT_LOCK, TE_LOCK, NULL);
	else
	{
		KillTimer(m_hWnd, IDT_LOCK);
		SetTimer(m_hWnd, IDT_MOUSELEAVE, TE_MOUSELEAVE, NULL);
	}
}




void CLrcBtnBox::PostCreateRenderTarget()
{
	const auto pRT = m_Wnd.m_pRT;
	SafeRelease(m_Btn[0].pBmp);
	pRT->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_PrevSolid], &m_Btn[0].pBmp);
	SafeRelease(m_Btn[1].pBmp);
	pRT->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_PauseSolid], &m_Btn[1].pBmp);
	SafeRelease(m_Btn[2].pBmp);
	pRT->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_NextSolid], &m_Btn[2].pBmp);
	SafeRelease(m_Btn[3].pBmp);
	pRT->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_LockSolid], &m_Btn[3].pBmp);
	SafeRelease(m_Btn[4].pBmp);
	pRT->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_CrossSolid], &m_Btn[4].pBmp);
	SafeRelease(m_pBmpPlay);
	pRT->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_TriangleSolid], &m_pBmpPlay);
}

int CLrcBtnBox::HitTest(POINT pt)
{
	RECT rc;
	const int cxBtnTotal = m_Wnd.m_Ds.cxBtnPadding * (c_cBtn - 1) + m_Wnd.m_Ds.cxyBtn * c_cBtn;
	rc.left = (m_Wnd.m_cxClient - cxBtnTotal) / 2;
	rc.top = (int)(m_Wnd.m_DsF.Margin * 2);
	rc.right = rc.left + m_Wnd.m_Ds.cxyBtn;
	rc.bottom = rc.top + m_Wnd.m_Ds.cxyBtn;

	if (m_Wnd.m_bDrawLockBtn && !m_Wnd.m_bShowBk)
	{
		rc.left += ((cxBtnTotal - m_Wnd.m_Ds.cxyBtn) / 2);
		rc.right = rc.left + m_Wnd.m_Ds.cxyBtn;
		if (PtInRect(&rc, pt))
			return IDLBT_LOCK;
	}
	else if (m_Wnd.m_bShowBk)
	{
		EckCounter(c_cBtn, i)
		{
			if (PtInRect(&rc, pt))
				return i;
			const int cx = m_Wnd.m_Ds.cxBtnPadding + m_Wnd.m_Ds.cxyBtn;
			rc.left += cx;
			rc.right += cx;
		}
	}

	return IDLBT_NONE;
}

void CLrcBtnBox::Draw()
{
	const auto pRT = m_Wnd.m_pRT;
	ID2D1DeviceContext* pDC;
	pRT->QueryInterface(IID_PPV_ARGS(&pDC));

	D2D1_RECT_F rc;
	const int cxBtnTotal = m_Wnd.m_Ds.cxBtnPadding * (c_cBtn - 1) + m_Wnd.m_Ds.cxyBtn * c_cBtn;
	rc.left = (float)((m_Wnd.m_cxClient - cxBtnTotal) / 2);
	rc.top = m_Wnd.m_DsF.Margin * 3;
	rc.right = rc.left + m_Wnd.m_DsF.cxyBtn;
	rc.bottom = rc.top + m_Wnd.m_DsF.cxyBtn;
	eck::InflateRect(rc, -m_Wnd.m_DsF.cxBtnPadding, -m_Wnd.m_DsF.cxBtnPadding);

	ID2D1SolidColorBrush* pBrush = NULL;
	if (m_Wnd.m_bLocked && m_Wnd.m_bDrawLockBtn && !m_Wnd.m_bShowBk)
	{
		rc.left += ((cxBtnTotal - m_Wnd.m_DsF.cxyBtn) / 2);
		rc.right += ((cxBtnTotal - m_Wnd.m_DsF.cxyBtn) / 2);
		const auto& e = m_Btn[IDLBT_LOCK];
		if (e.bChecked)
			pBrush = m_Wnd.m_pBrPressed;
		else if (IDLBT_LOCK == m_idxHot)
			if (m_bLBtnDown)
				pBrush = m_Wnd.m_pBrPressed;
			else
				pBrush = m_Wnd.m_pBrHot;
		if (pBrush)
		{
			eck::InflateRect(rc, m_Wnd.m_DsF.cxBtnPadding, m_Wnd.m_DsF.cxBtnPadding);
			pRT->FillRectangle(&rc, pBrush);
			eck::InflateRect(rc, -m_Wnd.m_DsF.cxBtnPadding, -m_Wnd.m_DsF.cxBtnPadding);
		}

		pDC->DrawBitmap(e.pBmp, rc, 1.f, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
	}
	else if (m_Wnd.m_bShowBk)
	{
		EckCounter(c_cBtn, i)
		{
			if (m_Btn[i].bChecked)
				pBrush = m_Wnd.m_pBrPressed;
			else if (i == m_idxHot)
				if (m_bLBtnDown)
					pBrush = m_Wnd.m_pBrPressed;
				else
					pBrush = m_Wnd.m_pBrHot;

			if (pBrush)
			{
				eck::InflateRect(rc, m_Wnd.m_DsF.cxBtnPadding, m_Wnd.m_DsF.cxBtnPadding);
				pRT->FillRectangle(&rc, pBrush);
				eck::InflateRect(rc, -m_Wnd.m_DsF.cxBtnPadding, -m_Wnd.m_DsF.cxBtnPadding);
			}

			if (i == IDLBT_PLAY)
			{
				if (App->GetPlayer().IsPlaying())
					pDC->DrawBitmap(m_Btn[i].pBmp, rc, 1.f, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
				else
					pDC->DrawBitmap(m_pBmpPlay, rc, 1.f, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
			}
			else
				pDC->DrawBitmap(m_Btn[i].pBmp, rc, 1.f, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);

			const float cx = m_Wnd.m_DsF.cxBtnPadding + m_Wnd.m_DsF.cxyBtn;
			rc.left += cx;
			rc.right += cx;
			pBrush = NULL;
		}
	}
	pDC->Release();
}

void CLrcBtnBox::MouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	{
		m_bLBtnDown = TRUE;
		SetCapture(m_Wnd.HWnd);
		if (HitTest(ECK_GET_PT_LPARAM(lParam)) >= 0)
			m_Wnd.Draw();
	}
	break;

	case WM_LBUTTONUP:
	{
		if (!m_bLBtnDown)
			break;
		m_bLBtnDown = FALSE;
		ReleaseCapture();
		if (int idx; (idx = HitTest(ECK_GET_PT_LPARAM(lParam))) >= 0)
		{
			m_Wnd.DoCmd(idx);
			if (m_Wnd.IsValid())
				m_Wnd.Draw();
		}
	}
	break;

	case WM_MOUSEMOVE:
	{
		if (!m_bLBtnDown)
		{
			const int idx = HitTest(ECK_GET_PT_LPARAM(lParam));
			if (idx != m_idxHot)
			{
				m_idxHot = idx;
				m_Wnd.Draw();
			}
		}
	}
	break;

	case WM_LBUTTONDBLCLK:
		PostMessageW(m_Wnd.HWnd, WM_LBUTTONDOWN, wParam, lParam);
		break;
	}
}