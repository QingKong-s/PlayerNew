#include "CWndBK.h"
#include "CWndMain.h"
#include "CDlgAbout.h"
#include "CDlgOptions.h"

#include "eck\CLinearLayout.h"

BOOL CWndBK::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
	m_uMsgCUIButton = RegisterWindowMessageW(L"PlayerNew.Message.CUIButtonClick");
	m_iDpi = eck::GetDpi(hWnd);
	eck::UpdateDpiSize(m_Ds, m_iDpi);
	eck::UpdateDpiSizeF(m_DsF, m_iDpi);

	const auto pDC = GetD2D().GetDC();

	pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBrWhite);
	pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.5f), &m_pBrWhite2);

	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Prev], &m_pBmpIcon[ICIDX_Prev]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Triangle], &m_pBmpIcon[ICIDX_Play]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Block], &m_pBmpIcon[ICIDX_Stop]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Next], &m_pBmpIcon[ICIDX_Next]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Lrc], &m_pBmpIcon[ICIDX_Lrc]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Circle], &m_pBmpIcon[ICIDX_RMAllLoop]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_ArrowRight3], &m_pBmpIcon[ICIDX_RMAll]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_ArrowCross], &m_pBmpIcon[ICIDX_RMRadom]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_CircleOne], &m_pBmpIcon[ICIDX_RMSingleLoop]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_ArrowRight], &m_pBmpIcon[ICIDX_RMSingle]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_PlayOpt], &m_pBmpIcon[ICIDX_PlayOpt]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_PlayList], &m_pBmpIcon[ICIDX_PlayList]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Gear], &m_pBmpIcon[ICIDX_Options]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Info], &m_pBmpIcon[ICIDX_About]);
	pDC->CreateBitmapFromWicBitmap(App->GetWicRes()[IIDX_Pause], &m_pBmpIcon[ICIDX_Pause]);

	SetupElem();
	return TRUE;
}

void CWndBK::OnSize(HWND hWnd, UINT state, int cx, int cy)
{
	ECK_DUILOCKWND;
	m_cxClient = cx;
	m_cyClient = cy;
	m_rcfClient = eck::MakeD2DRcF({ 0,0,cx,cy });

	HRESULT hr;
	D2D1_BITMAP_PROPERTIES1 D2dBmpProp
	{
		{DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
		96,
		96,
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		NULL
	};

	SafeRelease(m_pBmpBKStatic);
	D2D_SIZE_U D2DSizeU = { (UINT32)cx, (UINT32)cy };
	D2dBmpProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
	if (FAILED(hr = GetD2D().GetDC()->CreateBitmap(D2DSizeU, NULL, 0, D2dBmpProp, &m_pBmpBKStatic)))
	{
		EckDbgPrint(L"ID2D1DeviceContext::CreateBitmap Error");
	}
	UpdateStaticBmp();

	const auto margin = m_Ds.UIMargin;
	m_pLayout->LoSetPosSize(margin, margin, m_cxClient - 2 * margin, m_cyClient - 2 * margin);
	m_pLayout->LoCommit();
}

void CWndBK::OnDestroy(HWND hWnd)
{
	m_pBrWhite->Release();
	m_pBrWhite2->Release();
	m_pBmpBKStatic->Release();
	SafeRelease(m_pBmpAlbum);
	for (auto pBmp : m_pBmpIcon)
		pBmp->Release();
	for (auto pElem : m_vAllElems)
		delete pElem;
	for (auto pLayout : m_vLayout)
		delete pLayout;
	m_pLayout = NULL;
	m_vAllElems.clear();
	m_vElemsWantTimer.clear();
}

void CWndBK::SetupElem()
{
	const auto margin2 = m_Ds.UIMarginInt;

	const auto pLayout = new eck::CLinearLayoutV{};
	m_vLayout.push_back(pLayout);

	{
		const auto pLoH = new eck::CLinearLayoutH{};
		m_vLayout.push_back(pLoH);
		{
			//const auto pLoAlbumSpe = new eck::CLinearLayoutV{};
			//m_vLayout.push_back(pLoAlbumSpe);

			const auto palbum = new CUIAlbum{};
			palbum->Create(NULL, Dui::DES_VISIBLE, 0,
				10, 10, 400, 500, NULL, this);
			m_vAllElems.push_back(palbum);
			//pLoAlbumSpe->Add(palbum, {}, eck::LLF_FIXHEIGHT | eck::LLF_FIXWIDTH);

			//const auto pspe = new CUISpe{};
			//pspe->Create(NULL, Dui::DES_VISIBLE, 0,
			//	0, 0, 400, 160, NULL, this);
			//pspe->SetCount(40);
			//pspe->SetGapWidth(eck::DpiScaleF(1.f, m_iDpi));
			//m_vAllElems.push_back(pspe);
			//m_vElemsWantTimer.push_back(pspe);
			//pLoAlbumSpe->Add(pspe, {}, eck::LLF_FIXHEIGHT | eck::LLF_FIXWIDTH);

			//pLoH->Add(pLoAlbumSpe, {}, eck::LLF_FIXHEIGHT | eck::LLF_FIXWIDTH);
			pLoH->Add(palbum, {}, eck::LLF_FIXHEIGHT | eck::LLF_FIXWIDTH);
		}

		const auto plrc = new CUILrc{};
		plrc->Create(NULL, Dui::DES_VISIBLE, 0,
			0, 0, 800, 730, NULL, this);
		m_vAllElems.push_back(plrc);
		pLoH->Add(plrc, { margin2 }, eck::LLF_FILLWIDTH | eck::LLF_FILLHEIGHT, 1);

		pLayout->Add(pLoH, { 0,0,0,margin2 }, eck::LLF_FILLWIDTH | eck::LLF_FILLHEIGHT, 1);
	}

	const auto ppb = new CUIProgressBar{};
	ppb->Create(NULL, Dui::DES_VISIBLE, 0,
		70, 730, 800, 40, NULL, this);
	m_vAllElems.push_back(ppb);
	pLayout->Add(ppb, { 0,0,0,margin2 }, eck::LLF_FIXHEIGHT | eck::LLF_FILLWIDTH);

	const auto ppc = new CUIPlayingCtrl{};
	ppc->Create(NULL, Dui::DES_VISIBLE, 0,
		70, 800, 800, 80, NULL, this);
	m_vAllElems.push_back(ppc);
	pLayout->Add(ppc, {}, eck::LLF_FIXHEIGHT | eck::LLF_FILLWIDTH);
	m_pLayout = pLayout;
}

LRESULT CWndBK::OnElemEvent(Dui::CElem* pElem, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case Dui::EE_COMMAND:
	{
		switch (pElem->GetID())
		{
		case IDE_BT_OPT:
		{
			auto pDlg{ std::make_unique<CDlgOptions>() };
			pDlg->DlgBox(HWnd);
		}
		break;
		case IDE_BT_PLAYOPT:
		{
			//if (m_pVolCtrl)
			//	break;
			//RECT rc = pElem->GetRect();
			//pElem->GetParentElem()->ElemToClient(rc);

			//auto pVol = new CUIVolTrackBar{};
			//m_pVolCtrl = pVol;
			//pVol->Create(NULL, Dui::DES_VISIBLE, 0,
			//	rc.left - 40, rc.top - 200, 400, 100, NULL, this);
			//pVol = 0;
			if (!m_DlgFx.IsValid())
				m_DlgFx.CreateDlg(HWnd, NULL);
		}
		break;
		case IDE_BT_REPEATMODE:
		{
			const auto pBtn = (Dui::CCircleButton*)pElem;
			App->GetOptionsMgr().PlayRepeatMode = COptionsMgr::NextRepeatMode(App->GetOptionsMgr().PlayRepeatMode);
			constexpr int idx[]
			{
				CWndBK::ICIDX_RMAllLoop,
				CWndBK::ICIDX_RMAll,
				CWndBK::ICIDX_RMRadom,
				CWndBK::ICIDX_RMSingleLoop,
				CWndBK::ICIDX_RMSingle,
			};
			auto iRM = (int)App->GetOptionsMgr().PlayRepeatMode;
			EckAssert(iRM >= 0 && iRM < ARRAYSIZE(idx));
			pBtn->SetImage(m_pBmpIcon[idx[iRM]]);
			pBtn->InvalidateRect();
		}
		break;
		case IDE_BT_PREV:
			App->GetPlayer().Prev();
			break;
		case IDE_BT_PLAY:
			App->GetPlayer().PlayOrPause();
			break;
		case IDE_BT_NEXT:
			App->GetPlayer().Next();
			break;
		case IDE_BT_STOP:
			App->GetPlayer().Stop();
			break;
		case IDE_BT_LRC:
		{
			auto& om = App->GetOptionsMgr();
			ECKBOOLNOT(om.DtLrcShow);
			App->GetMainWnd()->ShowLrc(om.DtLrcShow);
			break;
		}
		case IDE_BT_ABOUT:
		{
			CDlgAbout Dlg{};
			Dlg.DlgBox(GetHWND());
		}
		break;
		}
	}
	return 0;
	case Dui::TBE_POSCHANGED:
	{
		switch (pElem->GetID())
		{
		case IDE_TB_PROGRESS:
		{
			auto pTrackBar = (Dui::CTrackBar*)pElem;
			App->GetPlayer().GetBass().SetPosition(pTrackBar->GetPos());
		}
		return 0;
		case IDE_TB_VOL:
		{
			auto pTrackBar = (Dui::CTrackBar*)pElem;
			App->GetPlayer().GetBass().SetVolume(pTrackBar->GetPos() / 100.f);
		}
		return 0;
		}
	}
	return 0;
	}
	return __super::OnElemEvent(pElem, uMsg, wParam, lParam);
}

ATOM CWndBK::RegisterWndClass()
{
	WNDCLASSEX wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DefWindowProcW;
	wcex.hInstance = App->GetHInstance();
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.lpszClassName = WCN_MAINBK;
	wcex.cbWndExtra = sizeof(void*);
	return RegisterClassExW(&wcex);
}

LRESULT CWndBK::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_TIMER:
	{
		switch (wParam)
		{
		case IDT_PGS:
		{
			auto& Player = App->GetPlayer();
			auto uTickRet = Player.Tick();

			auto& LrcWnd = App->GetMainWnd()->m_Lrc;
			if (LrcWnd.IsValid())
			{
				if (uTickRet & TKC_LRCPOSUPDATED)
				{
					LrcWnd.UpdateBrush();
					LrcWnd.Draw();
				}
				else if (LrcWnd.IsCacheLayoutTooLong())
					LrcWnd.Draw();
			}

			const auto pTL = App->GetMainWnd()->m_pTbList;
			if (pTL)
				pTL->SetProgressValue(App->GetMainWnd()->HWnd,
					Player.GetPos(), Player.GetLength());
		}
		break;
		}
		for (auto pElem : m_vElemsWantTimer)
			pElem->OnTimer((UINT)wParam);
	}
	return 0;

	case WM_LBUTTONDOWN:
		SetFocus(hWnd);
		break;

	case WM_SIZE:
		HANDLE_WM_SIZE(hWnd, wParam, lParam, OnSize);
		break;

	case PNWM_DWMCOLORCHANGED:
		BroadcastEvent(UIEE_DWMCOLORCHANGED, 0, 0);
		return 0;

	case PNWM_SETTINGCHANGED:
		BroadcastEvent(UIEE_ONSETTINGSCHANGE, 0, 0);
		return 0;

	case WM_CREATE:
	{
		const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
		HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
		return lResult;
	}

	case WM_DESTROY:
	{
		const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
		HANDLE_WM_DESTROY(hWnd, wParam, lParam, OnDestroy);
		return lResult;
	}

	}

	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

void CWndBK::OnPlayingControl(PLAYINGCTRLTYPE uType)
{
	switch (uType)
	{
	case PCT_PLAYNEW:
	{
		ECK_DUILOCKWND;
		SafeRelease(m_pBmpAlbum);
		GetD2D().GetDC()->CreateBitmapFromWicBitmap(App->GetPlayer().GetWicBmpCover(), &m_pBmpAlbum);
		auto Size = m_pBmpAlbum->GetSize();
		m_cxAlbum = (int)Size.width;
		m_cyAlbum = (int)Size.height;
		UpdateStaticBmp();
		SetTimer(m_hWnd, IDT_PGS, 40, NULL);
	}
	break;
	case PCT_STOP:
		KillTimer(m_hWnd, IDT_PGS);
		break;
	}

	BroadcastEvent(UIEE_ONPLAYINGCTRL, uType, 0);
	Redraw();
}

void CWndBK::UpdateStaticBmp()
{
	ECK_DUILOCKWND;
	if (m_cxClient <= 0 || m_cyClient <= 0)
		return;
	IWICBitmap* pWICBitmapOrg = App->GetPlayer().GetWicBmpCover();// 原始WIC位图
	if (!pWICBitmapOrg)
		pWICBitmapOrg = App->GetWicRes()[IIDX_DefCover];
	////////////////////绘制静态位图
	const auto pDC = GetD2D().GetDC();
	pDC->BeginDraw();
	pDC->SetTarget(m_pBmpBKStatic);
	pDC->Clear(D2D1::ColorF(D2D1::ColorF::White));

	UINT cx0, cy0;
	float cyRgn/*截取区域高*/, cx/*缩放后图片宽*/, cy/*缩放后图片高*/;

	if (pWICBitmapOrg)
	{
		/*
		情况一，客户区宽为最大边
		cxClient   cyClient
		-------- = --------
		 cxPic      cyRgn
		情况二，客户区高为最大边
		cyClient   cxClient
		-------- = --------
		 cyPic      cxRgn
		*/
		////////////////////处理缩放与截取（无论怎么改变窗口大小，用来模糊的封面图都要居中充满整个窗口）
		D2D_POINT_2F pt;
		pWICBitmapOrg->GetSize(&cx0, &cy0);
		cyRgn = (float)m_cyClient / (float)m_cxClient * (float)cx0;
		if (cyRgn <= cy0)// 情况一
		{
			cx = (float)m_cxClient;
			cy = cx * cy0 / cx0;
			pt = { 0.f,(float)(m_cyClient - cy) / 2 };
		}
		else// 情况二
		{
			cy = (float)m_cyClient;
			cx = cx0 * cy / cy0;
			pt = { (float)(m_cxClient - cx) / 2,0.f };
		}
		////////////缩放
		IWICBitmap* pWicBmpScaled;
		eck::ScaleWicBitmap(pWICBitmapOrg, pWicBmpScaled, (int)cx, (int)cy,
			WICBitmapInterpolationModeNearestNeighbor);
		ID2D1Bitmap1* pBmpScaled;
		pDC->CreateBitmapFromWicBitmap(pWicBmpScaled, &pBmpScaled);
		pWicBmpScaled->Release();
		////////////模糊 
		ID2D1Effect* pEffect;
		pDC->CreateEffect(CLSID_D2D1GaussianBlur, &pEffect);
		EckAssert(pEffect);
		pEffect->SetInput(0, pBmpScaled);
		pEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 50.f);
		pEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
		pDC->DrawImage(pEffect, &pt);
		pEffect->Release();
		pBmpScaled->Release();
		////////////半透明遮罩
		m_pBrWhite2->SetColor(eck::ColorrefToD2dColorF(eck::GetThreadCtx()->crDefBkg, 0.6f));
		pDC->FillRectangle(m_rcfClient, m_pBrWhite2);
	}

	pDC->EndDraw();
	pDC->SetTarget(GetD2D().GetBitmap());

	SetBkgBitmap(m_pBmpBKStatic);
}

LRESULT CUIElem::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::OnEvent(uMsg, wParam, lParam);
}