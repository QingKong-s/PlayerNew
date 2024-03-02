#include "CWndLrc.h"

#include "COptionsMgr.h"

class CTextRenderer : public IDWriteTextRenderer
{
private:
	ULONG m_uRef = 1;
public:
	ID2D1Brush* m_pBr = NULL;
	ID2D1SolidColorBrush* m_pBrOutLine = NULL;
	ID2D1RenderTarget* m_pRT = NULL;
	CWndLrc& m_Wnd;

	CTextRenderer(CWndLrc& Wnd, ID2D1RenderTarget* pRT, ID2D1Brush* pBr, ID2D1SolidColorBrush* pBrOutLine)
		:m_Wnd{ Wnd }, m_pRT{ pRT }, m_pBr{ pBr }, m_pBrOutLine{ pBrOutLine } {}

	STDMETHOD(DrawGlyphRun)(
		void* pClientDrawingContext,
		FLOAT xOrgBaseline,
		FLOAT yOrgBaseline,
		DWRITE_MEASURING_MODE MeasuringMode,
		DWRITE_GLYPH_RUN const* pGlyphRun,
		DWRITE_GLYPH_RUN_DESCRIPTION const* pGlyphRunDesc,
		IUnknown* pClientDrawingEffect)
	{
		const auto& om = App->GetOptionsMgr();

		ID2D1PathGeometry* pPathGeometry;
		ID2D1TransformedGeometry* pTransformedGeometry;
		ID2D1GeometrySink* pSink;
		HRESULT hr;
		hr = App->m_pD2dFactory->CreatePathGeometry(&pPathGeometry);
		if (!pPathGeometry)
			return hr;

		pPathGeometry->Open(&pSink);
		pGlyphRun->fontFace->GetGlyphRunOutline(pGlyphRun->fontEmSize, pGlyphRun->glyphIndices,
			pGlyphRun->glyphAdvances, pGlyphRun->glyphOffsets, pGlyphRun->glyphCount,
			pGlyphRun->isSideways, pGlyphRun->bidiLevel, pSink);
		pSink->Close();

		if (om.DtLrcEnableShadow != 0.f)
		{
			const float oxy = eck::DpiScaleF(om.DtLrcShadowOffset, (float)m_Wnd.GetDpiValue());

			App->m_pD2dFactory->CreateTransformedGeometry(
				pPathGeometry,
				D2D1::Matrix3x2F::Translation(xOrgBaseline + oxy, yOrgBaseline + oxy),
				&pTransformedGeometry);
			EckAssert(pTransformedGeometry);
			m_pRT->FillGeometry(pTransformedGeometry, m_pBrOutLine);
			pTransformedGeometry->Release();
		}

		hr = App->m_pD2dFactory->CreateTransformedGeometry(pPathGeometry,
			D2D1::Matrix3x2F::Translation(xOrgBaseline, yOrgBaseline), &pTransformedGeometry);
		if (!pTransformedGeometry)
			return hr;

		m_pRT->DrawGeometry(pTransformedGeometry, m_pBrOutLine,
			eck::DpiScaleF(1.5f, m_Wnd.GetDpiValue()));// 描边
		m_pRT->FillGeometry(pTransformedGeometry, m_pBr);// 填充
		pTransformedGeometry->Release();

		pPathGeometry->Release();
		pSink->Release();
		return S_OK;
	}

	STDMETHOD(DrawUnderline)(
		void* pClientDrawingContext,
		FLOAT xOrgBaseline,
		FLOAT yOrgBaseline,
		DWRITE_UNDERLINE const* pUnderline,
		IUnknown* pClientDrawingEffect)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DrawStrikethrough)(
		void* pClientDrawingContext,
		FLOAT xOrgBaseline,
		FLOAT yOrgBaseline,
		DWRITE_STRIKETHROUGH const* strikethrough,
		IUnknown* pClientDrawingEffect)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DrawInlineObject)(
		void* pClientDrawingContext,
		FLOAT xOrg,
		FLOAT yOrg,
		IDWriteInlineObject* pInlineObject,
		BOOL bSideways,
		BOOL bRightToLeft,
		IUnknown* pClientDrawingEffect)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(IsPixelSnappingDisabled)(void* pClientDrawingContext, BOOL* pbDisabled)
	{
		*pbDisabled = FALSE;
		return S_OK;
	}

	STDMETHOD(GetCurrentTransform)(void* pClientDrawingContext, DWRITE_MATRIX* pMatrix)
	{
		m_pRT->GetTransform((D2D1_MATRIX_3X2_F*)pMatrix);
		return S_OK;
	}

	STDMETHOD(GetPixelsPerDip)(void* pClientDrawingContext, FLOAT* pfPixelsPerDip)
	{
		float x, y;
		m_pRT->GetDpi(&x, &y);
		*pfPixelsPerDip = x / 96.f;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObj)
	{
		const static QITAB qit[]
		{
			QITABENT(CTextRenderer, IDWriteTextRenderer),
			QITABENT(CTextRenderer, IDWritePixelSnapping),
			{},
		};
		return QISearch(this, qit, iid, ppvObj);
	}

	ULONG STDMETHODCALLTYPE AddRef() { return ++m_uRef; }

	ULONG STDMETHODCALLTYPE Release()
	{
		--m_uRef;
		if (m_uRef)
			return m_uRef;
		delete this;
		return 0;
	}
};

void CWndLrc::ReSizeRenderStuff(int cx, int cy)
{
	InvalidateCache();
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

	eck::SafeRelease(m_pRT);
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
	eck::SafeRelease(m_pBrBk);
	eck::SafeRelease(m_pBrFrame);
	m_pRT->CreateSolidColorBrush(D2D1::ColorF(0x3F3F3F, 0.5), &m_pBrBk);
	m_pRT->CreateSolidColorBrush(D2D1::ColorF(0x97D2CB, 0.5), &m_pBrFrame);

	m_BtnBox.PostCreateRenderTarget();
}

void CWndLrc::UpdateSysColorBrush()
{
	eck::SafeRelease(m_pBrHot);
	eck::SafeRelease(m_pBrPressed);

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
		Cache.idxLrc = idxLrc;
		eck::SafeRelease(Cache.pLayout);
		eck::SafeRelease(Cache.pLayoutTrans);

		constexpr WCHAR szEmpty[]{ L"♪♬♪♬♪" };
		constexpr int cchEmpty = (int)(ARRAYSIZE(szEmpty) - 1);
		auto pszEmpty = (PWSTR)malloc(sizeof(szEmpty));
		EckCheckMem(pszEmpty);
		memcpy(pszEmpty, szEmpty, sizeof(szEmpty));
		Utils::LRCINFO FakeLrc{ pszEmpty,nullptr,cchEmpty,cchEmpty,Lrc.fTime,Lrc.fDuration };

		const auto& LrcNew = (Lrc.cchTotal == 0) ? FakeLrc : Lrc;

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
		Cache.size = { tm.width,tm.height };

		if (LrcNew.pszTranslation)
		{
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
			Cache.sizeTrans = { tm.width,tm.height };
		}
	}

	const auto& Font = App->GetOptionsMgr().DtLrcFontMain;
	D2D1_GRADIENT_STOP Stops[]
	{
		{ 0.f, eck::ARGBToD2dColorF(bHiLight ? Font.argbHiLightGra[0] : Font.argbNormalGra[0]) },
		{ 1.f, eck::ARGBToD2dColorF(bHiLight ? Font.argbHiLightGra[1] : Font.argbNormalGra[1]) },
	};
	ID2D1GradientStopCollection* pStopCollection;
	m_pRT->CreateGradientStopCollection(Stops, 2, &pStopCollection);
	EckAssert(pStopCollection);

	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES Prop
	{
		D2D1::Point2F(0, y),
		D2D1::Point2F(0, y + Cache.size.height),
	};

	ID2D1LinearGradientBrush* pBrush;
	m_pRT->CreateLinearGradientBrush(&Prop, NULL, pStopCollection, &pBrush);
	pStopCollection->Release();
	EckAssert(pBrush);

	auto pRenderer = new CTextRenderer(*this, m_pRT, pBrush, m_pBrTextBorder);

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

	Cache.pLayout->Draw(NULL, pRenderer, xStart + dx, y);

	pBrush->Release();

	float cy = Cache.size.height;
	if (Cache.pLayoutTrans)
	{
		const float yNew = y + cy;
		const auto& Font = App->GetOptionsMgr().DtLrcFontTranslation;

		Stops[0].color = eck::ARGBToD2dColorF(bHiLight ? Font.argbHiLightGra[0] : Font.argbNormalGra[0]);
		Stops[1].color = eck::ARGBToD2dColorF(bHiLight ? Font.argbHiLightGra[1] : Font.argbNormalGra[1]);
		m_pRT->CreateGradientStopCollection(Stops, 2, &pStopCollection);
		EckAssert(pStopCollection);

		Prop.startPoint.y += cy;
		Prop.endPoint.y = Prop.startPoint.y + Cache.sizeTrans.height;
		m_pRT->CreateLinearGradientBrush(&Prop, NULL, pStopCollection, &pBrush);
		pStopCollection->Release();
		EckAssert(pBrush);

		pRenderer->m_pBr = pBrush;

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
		Cache.pLayoutTrans->Draw(NULL, pRenderer, xStart + dx, yNew);
		pBrush->Release();

		cy += Cache.sizeTrans.height;
	}

#ifdef _DEBUG
	EckAssert(pRenderer->Release() == 0);
#else
	pRenderer->Release();
#endif // _DEBUG

	return cy;
}

void CWndLrc::DrawStaticLine(int idxFake, float y)
{
	EckAssert(idxFake == LRCIDX_APPNAME || idxFake == LRCIDX_FILENAME);
	auto& Cache = m_TextCache[0];
	if (Cache.idxLrc != idxFake)
	{
		Cache.idxLrc = idxFake;
		SafeRelease(Cache.pLayout);
		SafeRelease(Cache.pLayoutTrans);
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
			return;
		}

		const float cxMax = CalcMaxLrcWidth();

		m_pTfMain->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		m_pTfMain->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
		App->m_pDwFactory->CreateTextLayout(pszText, cchText, m_pTfMain,
			cxMax, (float)m_cyClient, &Cache.pLayout);
		DWRITE_TEXT_METRICS tm;
		Cache.pLayout->GetMetrics(&tm);
		Cache.size = { tm.width,tm.height };
		m_pTfMain->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	}

	const auto& Font = App->GetOptionsMgr().DtLrcFontMain;

	D2D1_GRADIENT_STOP Stops[]
	{
		{ 0.f, eck::ARGBToD2dColorF(Font.argbNormalGra[0]) },
		{ 1.f, eck::ARGBToD2dColorF(Font.argbNormalGra[1]) },
	};
	ID2D1GradientStopCollection* pStopCollection;
	m_pRT->CreateGradientStopCollection(Stops, 2, &pStopCollection);
	EckAssert(pStopCollection);

	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES Prop
	{
		D2D1::Point2F(0, y),
		D2D1::Point2F(0, y + Cache.size.height),
	};

	ID2D1LinearGradientBrush* pBrush;
	m_pRT->CreateLinearGradientBrush(&Prop, NULL, pStopCollection, &pBrush);
	pStopCollection->Release();
	EckAssert(pBrush);

	auto pRenderer = new CTextRenderer(*this, m_pRT, pBrush, m_pBrTextBorder);
	Cache.pLayout->Draw(NULL, pRenderer, CalcLrcMargin(), y);
	pBrush->Release();

#ifdef _DEBUG
	EckAssert(pRenderer->Release() == 0);
#else
	pRenderer->Release();
#endif // _DEBUG
}

void CWndLrc::UpdateTextFormat()
{
	eck::SafeRelease(m_pTfMain);
	eck::SafeRelease(m_pTfTranslation);

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
		eck::SafeRelease(m_pBrHot);
		eck::SafeRelease(m_pBrPressed);
		eck::SafeRelease(m_pBrBk);
		eck::SafeRelease(m_pBrFrame);
		eck::SafeRelease(m_pBrTextBorder);
		eck::SafeRelease(m_pTfMain);
		eck::SafeRelease(m_pTfTranslation);
		eck::SafeRelease(m_pRT);
		m_DC.Destroy();
		eck::SafeRelease(m_TextCache[0].pLayout);
		eck::SafeRelease(m_TextCache[0].pLayoutTrans);
		eck::SafeRelease(m_TextCache[1].pLayout);
		eck::SafeRelease(m_TextCache[1].pLayoutTrans);
		InvalidateCache();
	}
	break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

void CWndLrc::UpdateTextBrush()
{
	eck::SafeRelease(m_pBrTextBorder);

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
		ShowWnd(FALSE);
		break;
	}
}

void CWndLrc::ShowWnd(BOOL bShow)
{
	if (m_bShow == bShow)
		return;
	m_bShow = bShow;
	Show(bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
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
	eck::SafeRelease(m_Btn[0].pBmp);
	pRT->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_PrevSolid], &m_Btn[0].pBmp);
	eck::SafeRelease(m_Btn[1].pBmp);
	pRT->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_PauseSolid], &m_Btn[1].pBmp);
	eck::SafeRelease(m_Btn[2].pBmp);
	pRT->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_NextSolid], &m_Btn[2].pBmp);
	eck::SafeRelease(m_Btn[3].pBmp);
	pRT->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_LockSolid], &m_Btn[3].pBmp);
	eck::SafeRelease(m_Btn[4].pBmp);
	pRT->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_CrossSolid], &m_Btn[4].pBmp);
	eck::SafeRelease(m_pBmpPlay);
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
	rc.left = (m_Wnd.m_cxClient - cxBtnTotal) / 2;
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