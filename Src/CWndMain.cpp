#include <format>

#include "CWndMain.h"
#include "DragDrop.h"
#include "resource.h"



class CDropTargetList :public CDropTarget
{
private:
	CWndMain& m_WndMain;
public:
	CDropTargetList(CWndMain& x) :m_WndMain{ x }
	{}
	// IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		const static QITAB qit[]
		{
			QITABENT(CDropTargetList, IDropTarget),
			{}
		};

		return QISearch(this, qit, riid, ppvObject);
	}
	// IDropTarget
	HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		if (!pdwEffect)
			return E_INVALIDARG;
		*pdwEffect = DROPEFFECT_NONE;
		m_WndMain.m_DragDropInfo.bValid = FALSE;
		FORMATETC fe
		{
			App->GetListDragClipFormat(),
			NULL,
			DVASPECT_CONTENT,
			-1,
			TYMED_HGLOBAL
		};

		if (SUCCEEDED(pDataObj->QueryGetData(&fe)))
		{
			if (eck::IsBitSet(grfKeyState, MK_CONTROL))
			{
				m_WndMain.m_DragDropInfo.bValid = TRUE;
				*pdwEffect = DROPEFFECT_COPY;
			}
			else if (!App->GetPlayer().GetList().IsSorting())
			{
				m_WndMain.m_DragDropInfo.bValid = TRUE;
				*pdwEffect = DROPEFFECT_MOVE;
			}
		}
		else
		{
			fe.cfFormat = CF_HDROP;
			if (SUCCEEDED(pDataObj->QueryGetData(&fe)))
			{
				m_WndMain.m_DragDropInfo.bValid = TRUE;
				*pdwEffect = DROPEFFECT_COPY;
			}
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		if (!pdwEffect)
			return E_INVALIDARG;
		if (!m_WndMain.m_DragDropInfo.bValid ||
			App->GetPlayer().IsSearching() ||
			App->GetPlayer().GetList().IsSorting())
		{
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}
		*pdwEffect = DROPEFFECT_COPY;
		POINT pt0{ pt.x,pt.y };
		const auto& LV = m_WndMain.m_List.m_LVList;
		const BOOL bOnLV = (WindowFromPoint(pt0) == LV.GetHWND());// 光标是否在列表视图上
		const int cItems = LV.GetItemCount();

		LVINSERTMARK lim;
		lim.cbSize = sizeof(LVINSERTMARK);
		lim.dwReserved = 0;

		if (bOnLV)
		{
			ScreenToClient(LV.GetHWND(), &pt0);
			LVHITTESTINFO lvhti;
			lvhti.pt = { 0,pt0.y };// x设为0执行命中测试
			LV.HitTest(&lvhti);

			if (lvhti.iItem != -1)
			{
				lim.iItem = lvhti.iItem;
				RECT rcItem;
				LV.GetItemRect(lvhti.iItem, &rcItem);
				if (pt0.y > rcItem.top + (rcItem.bottom - rcItem.top) / 2)// 是否落在表项的下半部分
					lim.dwFlags = LVIM_AFTER;
				else
					lim.dwFlags = 0;
			}
			else if (eck::IsBitSet(lvhti.flags, LVHT_NOWHERE))// 落在空白处，在最后插入
			{
				lim.iItem = cItems - 1;
				lim.dwFlags = LVIM_AFTER;
			}
			LV.SetInsertMark(&lim);
		}
		else
		{
			lim.iItem = cItems - 1;
			lim.dwFlags = LVIM_AFTER;
			LV.SetInsertMark(&lim);
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE DragLeave(void)
	{
		m_WndMain.m_List.m_LVList.SetInsertMark(-1);
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		if (!pdwEffect)
			return E_INVALIDARG;
		*pdwEffect = DROPEFFECT_NONE;
		if (!m_WndMain.m_DragDropInfo.bValid ||
			App->GetPlayer().IsSearching())
			return S_OK;

		POINT pt0{ pt.x,pt.y };
		const auto& LV = m_WndMain.m_List.m_LVList;
		const BOOL bOnLV = (WindowFromPoint(pt0) == LV.GetHWND());// 光标是否在列表视图上
		
		LVHITTESTINFO lvhti;
		if (bOnLV)
		{
			ScreenToClient(LV.GetHWND(), &pt0);
			lvhti.pt = { 0,pt0.y };
			LV.HitTest(&lvhti);
		}
		else
			lvhti.iItem = -1;

		LV.SetInsertMark(-1);

		PCVOID pData;

		const LISTDRAGPARAMHEADER* pHeader;

		const DROPFILES* pdf;
		LISTFILEITEM_1 Info{};
		Info.s.bNeedUpdated = TRUE;
		PCWSTR pszTemp;
		PCSTR pszTempA;
		eck::CRefStrW rsTemp{};

		FORMATETC fe
		{
			App->GetListDragClipFormat(),
			NULL,
			DVASPECT_CONTENT,
			-1,
			TYMED_HGLOBAL
		};

		STGMEDIUM sm{};
		sm.tymed = TYMED_HGLOBAL;

		auto& Player = App->GetPlayer();
		if (Player.IsSearching())
			return E_UNEXPECTED;
		else if (SUCCEEDED(pDataObj->GetData(&fe, &sm)))// 取自定义拖放信息
		{
			if (eck::IsBitSet(grfKeyState, MK_CONTROL))// 复制项目而不是移动
			{
				ReleaseStgMedium(&sm);
				goto TryHDrop;
			}

			pData = GlobalLock(sm.hGlobal);
			if (!pData)
			{
				ReleaseStgMedium(&sm);
				return E_UNEXPECTED;
			}

			eck::CMemReader r(pData, GlobalSize(sm.hGlobal));
			r.SkipPointer(pHeader);
			if (pHeader->iVer != LDPH_VER_1 || pHeader->cItems <= 0)// 验证合法性
			{
				GlobalUnlock(sm.hGlobal);
				ReleaseStgMedium(&sm);
				return E_UNEXPECTED;
			}
			if (pHeader->dwPID != GetCurrentProcessId())// 不是自进程拖放，转向处理HDROP
			{
				GlobalUnlock(sm.hGlobal);
				ReleaseStgMedium(&sm);
				goto TryHDrop;
			}
			
			if (Player.GetList().IsSorting())// 如果是自进程拖放，但当前处在排序状态且Ctrl键未按下，则拒绝拖放
			{
				*pdwEffect = DROPEFFECT_NONE;
				GlobalUnlock(sm.hGlobal);
				ReleaseStgMedium(&sm);
				return E_UNEXPECTED;
			}

			if (lvhti.iItem >= 0)// 修正索引
			{
				RECT rcItem;
				LV.GetItemRect(lvhti.iItem, &rcItem);
				if (pt0.y > rcItem.top + (rcItem.bottom - rcItem.top) / 2)
					++lvhti.iItem;
			}

			const int idxFirst = Player.MoveItems(lvhti.iItem, (const int*)r.m_pMem, pHeader->cItems);// 移动项目
			LV.SetItemState(-1, 0, LVIS_SELECTED);
			EckCounter(pHeader->cItems, i)
				LV.SetItemState(idxFirst + i, LVIS_SELECTED, LVIS_SELECTED);// 选中移动的项目
			LV.Redraw();
			GlobalUnlock(sm.hGlobal);
			ReleaseStgMedium(&sm);
			*pdwEffect = DROPEFFECT_MOVE;
			return S_OK;
		}
		else
		{
		TryHDrop:
			fe.cfFormat = CF_HDROP;
			if (SUCCEEDED(pDataObj->GetData(&fe, &sm)))// 取HDROP
			{
				pData = GlobalLock(sm.hGlobal);
				if (!pData)
				{
					ReleaseStgMedium(&sm);
					return E_UNEXPECTED;
				}

				eck::CMemReader r(pData, GlobalSize(sm.hGlobal));
				r.SkipPointer(pdf);

				const BOOL bSort = Player.BeginAddOperation();
				if (pdf->fWide)
				{
					pszTemp = (PCWSTR)r.m_pMem;
					while (*pszTemp != L'\0')
					{
						Info.cchFile = (int)wcslen(pszTemp);
						Player.Insert(lvhti.iItem, Info, NULL, pszTemp, 
							NULL, NULL, NULL, NULL);
						r += eck::Cch2Cb(Info.cchFile);
						pszTemp = (PCWSTR)r.m_pMem;
					}
				}
				else
				{
					pszTempA = (PCSTR)r.m_pMem;
					while (*pszTempA != '\0')
					{
						const int cchA = (int)strlen(pszTempA);
						rsTemp = eck::StrX2W(pszTempA, cchA);
						Info.cchFile = rsTemp.Size();
						Player.Insert(lvhti.iItem, Info, NULL, rsTemp.Data(),
							NULL, NULL, NULL, NULL);
						r += (UINT)(cchA + 1);
						pszTempA = (PCSTR)r.m_pMem;
					}
				}
				Player.EndAddOperation(bSort);
				LV.SetItemCount(Player.GetList().GetCount());

				GlobalUnlock(sm.hGlobal);
				ReleaseStgMedium(&sm);
				*pdwEffect = DROPEFFECT_COPY;
				return S_OK;
			}
		}
		return E_UNEXPECTED;
	}
};


void CWndMain::UpdateDpi(int iDpi)
{
	m_iDpi = iDpi;
	eck::UpdateDpiSize(m_Ds, iDpi);
}

void CWndMain::InitBK()
{
	//auto pa = new CUIAlbum;
	//pa->Create(NULL, Dui::DES_VISIBLE, 0,
	//	10, 10, 400, 500, NULL, &m_BK);

	//auto p = new CUIInfo;
	//m_BK.AddElem(p);
	//p->InitElem();
	//rc = { 50,10,800,300 };
	//p->SetElemRect(&rc);

	//auto pbtn = new CUIButton;
	//m_BK.AddElem(pbtn);
	//pbtn->InitElem();
	//rc = { 10,220,100,400 };
	//pbtn->SetElemRect(&rc);
	//pbtn->SetImg(m_BK.m_pBmpIcon[CWndBK::ICIDX_Play]);
	//pbtn->SetImgSize(m_BK.m_Ds.cxIcon, m_BK.m_Ds.cyIcon);

	//auto prb = new CUIRoundButton;
	//m_BK.AddElem(prb);
	//prb->InitElem();
	//rc = { 140,410,300,580 };
	//prb->SetElemRect(&rc);
	//prb->SetImg(m_BK.m_pBmpIcon[CWndBK::ICIDX_Options]);
	//prb->SetImgSize(m_BK.m_Ds.cxIcon, m_BK.m_Ds.cyIcon);

	/*auto plrc = new CUILrc{};
	plrc->Create(NULL, Dui::DES_VISIBLE, 0,
		0, 0, 800, 730, NULL, &m_BK);

	auto ppb = new CUIProgressBar{};
	ppb->Create(NULL, Dui::DES_VISIBLE, 0,
		70, 730, 800, 70, NULL, &m_BK);

	auto ppc = new CUIPlayingCtrl{};
	ppc->Create(NULL, Dui::DES_VISIBLE, 0,
		70, 800, 800, 80, NULL, &m_BK);*/

	//auto pw = new CUIWaves{};
	//pw->Create(NULL, Dui::DES_VISIBLE, 0,
	//	400, 730, 500, 140, NULL, &m_BK);
	//pw->SetLineWidth((int)(2.f * 1.5f));

	//auto pspe = new CUISpe;
	//m_BK.AddElem(pspe);
	//pspe->InitElem();
	//rc = { 400,730,900,900 };
	//pspe->SetElemRect(&rc);
	//pspe->SetGapWidth(1.5f);
	//pspe->SetCount(40);

	//auto pspe2 = new CUISpe2;
	//m_BK.AddElem(pspe2);
	//pspe2->InitElem();
	//rc = { 100,120,800,600 };
	//pspe2->SetElemRect(&rc);
	//pspe2->SetSampleCount(60);

	//auto pra = new CUIAlbumRotating;
	//m_BK.AddElem(pra);
	//pra->InitElem();
	//rc = { 100,120,600,800 };
	//pra->SetElemRect(&rc);

	//auto p2 = new Dui::c;
	//m_BK.AddElem(p2);
	//p2->InitElem();
	//rc = { 90,780,800,800 };
	//p2->SetElemRect(&rc);
	//p2->SetMax(10000ull);

	//auto p3 = new CUIToolBar;
	//m_BK.AddElem(p3);
	//p3->InitElem();
	//p3->SetElemRect({ 70,650,800,700 });

	m_BK.Redraw();
}

void CWndMain::OnSize(HWND hWnd, UINT uState, int cx, int cy)
{
	HDWP hDwp = BeginDeferWindowPos(3);
	hDwp = DeferWindowPos(hDwp, m_BK.GetHWND(), NULL,
		0,
		0,
		cx - m_cxList - m_Ds.cxSeparateBar,
		cy, SWP_NOZORDER | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_SPB.GetHWND(), NULL,
		cx - m_cxList - m_Ds.cxSeparateBar,
		0,
		m_Ds.cxSeparateBar,
		cy, SWP_NOZORDER | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_List.GetHWND(), NULL,
		cx - m_cxList,
		0,
		m_cxList,
		cy, SWP_NOZORDER | SWP_NOACTIVATE);
	EndDeferWindowPos(hDwp);
	m_TbGhost.InvalidateLivePreviewCache();
}

BOOL CWndMain::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
	eck::GetThreadCtx()->UpdateDefColor();
	BOOL bOpaqueBlend;
	DwmGetColorizationColor(&m_argbDwm, &bOpaqueBlend);
	m_crDwm = eck::ARGBToD2dColorF(m_argbDwm);
	if (m_bDarkColor = eck::ShouldAppUseDarkMode())
	{
		App->InvertIconColor();
		eck::GetThreadCtx()->SetNcDarkModeForAllTopWnd(TRUE);
	}

	App->GetPlayer().SetPlayingCtrlCallBack([this](PLAYINGCTRLTYPE uType, INT_PTR i1, INT_PTR i2)
		{
			switch (uType)
			{
			case PCT_PLAYNEW:
			{
				if (m_Lrc.IsValid())
				{
					m_Lrc.InvalidateCache();
					m_Lrc.Draw();
				}
				DwmInvalidateIconicBitmaps(m_TbGhost.HWnd);
				m_TbGhost.InvalidateLivePreviewCache();
				m_TbGhost.InvalidateThumbnailCache();
				m_TbGhost.SetIconicThumbnail();
				if (i1 >= 0)
					m_List.m_LVList.RedrawItem((int)i1);
				auto& Player = App->GetPlayer();
				m_List.m_LVList.RedrawItem(Player.GetCurrFile());
				UpdateTaskbarPlayBtn();
				SetText(Player.GetList().At(Player.GetCurrFile()).rsName.Data());
				if (m_pTbList)
					m_pTbList->SetProgressState(HWnd, TBPF_NORMAL);
			}
			break;
			case PCT_PLAY_OR_PAUSE:
			{
				if (m_Lrc.IsValid() && m_Lrc.IsBkVisible())
					m_Lrc.Draw();
				UpdateTaskbarPlayBtn();
				if (m_pTbList)
					m_pTbList->SetProgressState(HWnd,
						(App->GetPlayer().IsPlaying()) ? TBPF_NORMAL : TBPF_PAUSED);
			}
			break;
			case PCT_STOP:
				m_List.m_LVList.RedrawItem((int)i1);
				m_pTbList->SetProgressValue(HWnd, 0ull, 0ull);
				break;
			case PCT_REMOVE_LATER_PLAY:
				m_List.m_LVList.RedrawItem((int)i1);
				break;
			}
			m_BK.OnPlayingControl(uType);
		});
	UpdateDpi(eck::GetDpi(hWnd));
	RECT rc;
	GetClientRect(hWnd, &rc);
	m_cxList = rc.right * 37 / 100;

	m_BK.Create(NULL, WS_CHILD, 0, 0, 0, 0, 0, hWnd, IDC_BK);
	m_List.Create(L"列表", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 0, 0, 0, hWnd, IDC_LIST);
	m_SPB.Create(NULL, WS_CHILD, 0, 0, 0, 0, 0, hWnd, IDC_SPB);

	m_BK.Show(SW_SHOWNOACTIVATE);
	m_List.Show(SW_SHOWNOACTIVATE);
	m_SPB.Show(SW_SHOWNOACTIVATE);

	m_pDropTarget = new CDropTargetList(*this);
	const auto hr = RegisterDragDrop(hWnd, m_pDropTarget);
	if (FAILED(hr))
		CApp::ShowError(hWnd, hr, CApp::ErrSrc::HResult, L"注册拖放目标失败");
	
	InitBK();
	m_BK.SendMsg(PNWM_DWMCOLORCHANGED, 0, 0);
	return TRUE;
}

void CWndMain::OnDpiChanged(HWND hWnd, int xDpi, int yDpi, RECT* pRect)
{
	UpdateDpi(xDpi);
}

void CWndMain::UpdateTaskbarPlayBtn()
{
	const auto pBmp = App->ScaleImageForButton(
		App->GetPlayer().IsPlaying() ? IIDX_PauseSolid : IIDX_TriangleSolid,
		m_iDpi);
	const auto hi = eck::CreateHICON(pBmp);
	pBmp->Release();

	THUMBBUTTON tb;
	tb.dwMask = THB_ICON | THB_TOOLTIP;
	tb.iId = IDTBB_PLAY;
	tb.hIcon = hi;
	wcscpy(tb.szTip, App->GetPlayer().IsPlaying() ? L"暂停" : L"播放");
	auto hr = m_pTbList->ThumbBarUpdateButtons(m_TbGhost.HWnd, 1, &tb);
	DestroyIcon(hi);
}

ATOM CWndMain::RegisterWndClass()
{
	WNDCLASSEX wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DefWindowProcW;
	wcex.hInstance = App->GetHInstance();
	wcex.hIcon = (HICON)LoadIconW(App->GetHInstance(), MAKEINTRESOURCEW(IDI_MAIN));
	wcex.hIconSm = (HICON)LoadIconW(App->GetHInstance(), MAKEINTRESOURCEW(IDI_MAIN));
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.lpszClassName = c_pszWndClassMain;
	wcex.cbWndExtra = sizeof(void*);
	return RegisterClassExW(&wcex);
}

LRESULT CWndMain::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == s_uMsgTaskbarButtonCreated)
	{
		if (m_pTbList)
			SetupTaskbarStuff();
		return 0;
	}

	switch (uMsg)
	{
	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, OnSize);

	case WM_NOTIFY:
	{
		const auto pnmhdr = (NMHDR*)lParam;
		if (pnmhdr->hwndFrom == m_SPB.HWnd && pnmhdr->code == eck::NM_SPB_DRAGGED)
		{
			const auto p = (eck::NMSPBDRAGGED*)lParam;
			RECT rc;
			GetClientRect(hWnd, &rc);
			m_cxList = rc.right - p->xyPos - m_Ds.cxSeparateBar;
			OnSize(hWnd, 0, rc.right, rc.bottom);
		}
	}
	return 0;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDTBB_PREV:
			App->GetPlayer().Prev();
			return 0;
		case IDTBB_PLAY:
			App->GetPlayer().PlayOrPause();
			return 0;
		case IDTBB_NEXT:
			App->GetPlayer().Next();
			return 0;
		}
	}
	break;

	case PNWM_CHANNELENDED:
		App->GetPlayer().AutoNext();
		return 0;

	case WM_SETTINGCHANGE:
		if (eck::IsColorSchemeChangeMessage(lParam))
		{
			const auto ptc = eck::GetThreadCtx();
			ptc->UpdateDefColor();
			m_TbGhost.InvalidateLivePreviewCache();
			ptc->SetNcDarkModeForAllTopWnd(eck::ShouldAppUseDarkMode());
			ptc->SendThemeChangedToAllTopWindow();
			App->InvertIconColor();
		}
		break;
	case WM_SYSCOLORCHANGE:
		eck::GetThreadCtx()->UpdateDefColor();
		m_TbGhost.InvalidateLivePreviewCache();
		eck::BroadcastChildrenMessage(hWnd, uMsg, wParam, lParam);
		break;

	case WM_SETTEXT:
		m_TbGhost.SetText((PCWSTR)lParam);
		break;

	case WM_DPICHANGED:
		return ECK_HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, OnDpiChanged);

	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
		
	case WM_DWMCOLORIZATIONCOLORCHANGED:
	{
		m_argbDwm = (ARGB)wParam;
		m_crDwm = eck::ARGBToD2dColorF(m_argbDwm);
		const BOOL bDark = eck::IsColorLightArgb(m_argbDwm);
		if (m_bDarkColor != bDark)
			App->InvertIconColor();
		m_bDarkColor = bDark;
		m_BK.SendMsg(PNWM_DWMCOLORCHANGED, 0, 0);
	}
	return 0;

	case WM_DESTROY:
		m_TbGhost.Destroy();
		SafeRelease(m_pTbList);
		SafeRelease(m_pDropTarget);
		if (m_Lrc.IsValid())
			m_Lrc.Destroy();
		RevokeDragDrop(m_hWnd);
		PostQuitMessage(0);
		return 0;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

void CWndMain::SetupTaskbarStuff()
{
	m_pTbList->UnregisterTab(m_TbGhost.HWnd);

	m_pTbList->RegisterTab(m_TbGhost.HWnd, HWnd);
#pragma warning(suppress: 6387)// 可能为NULL
	m_pTbList->SetTabOrder(m_TbGhost.HWnd, NULL);

	HICON hi[3];
	auto pBmp = App->ScaleImageForButton(IIDX_PrevSolid, m_iDpi);
	hi[0] = eck::CreateHICON(pBmp);
	pBmp->Release();

	pBmp = App->ScaleImageForButton(IIDX_PauseSolid, m_iDpi);
	hi[1] = eck::CreateHICON(pBmp);
	pBmp->Release();

	pBmp = App->ScaleImageForButton(IIDX_NextSolid, m_iDpi);
	hi[2] = eck::CreateHICON(pBmp);
	pBmp->Release();

	THUMBBUTTON tb[3]{};
	constexpr auto dwMask = THB_ICON | THB_TOOLTIP;
	tb[0].dwMask = dwMask;
	tb[0].hIcon = hi[0];
	tb[0].iId = IDTBB_PREV;
	wcscpy(tb[0].szTip, L"上一曲");

	tb[1].dwMask = dwMask;
	tb[1].hIcon = hi[1];
	tb[1].iId = IDTBB_PLAY;
	wcscpy(tb[1].szTip, L"播放");

	tb[2].dwMask = dwMask;
	tb[2].hIcon = hi[2];
	tb[2].iId = IDTBB_NEXT;
	wcscpy(tb[2].szTip, L"下一曲");

	m_pTbList->ThumbBarAddButtons(m_TbGhost.HWnd, ARRAYSIZE(tb), tb);
	DestroyIcon(hi[0]);
	DestroyIcon(hi[1]);
	DestroyIcon(hi[2]);
}

void CWndMain::SettingChanged()
{
	m_List.SendMsg(PNWM_SETTINGCHANGED, 0, 0);
	m_BK.SendMsg(PNWM_SETTINGCHANGED, 0, 0);
	if (m_Lrc.IsValid())
		m_Lrc.SendMsg(PNWM_SETTINGCHANGED, 0, 0);
}