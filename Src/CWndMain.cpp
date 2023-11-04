#include <format>

#include "CWndMain.h"
#include "DragDrop.h"



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

				const BOOL bSort = Player.BeginSortProtect();
				if (pdf->fWide)
				{
					pszTemp = (PCWSTR)r.m_pMem;
					while (*pszTemp != L'\0')
					{
						Info.cchFile = (int)wcslen(pszTemp);
						Player.Insert(lvhti.iItem, Info, NULL, NULL, pszTemp);
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
						rsTemp = eck::StrX2WRs(pszTempA, cchA);
						Info.cchFile = rsTemp.Size();
						Player.Insert(lvhti.iItem, Info, NULL, NULL, rsTemp.Data());
						r += (cchA + 1);
						pszTempA = (PCSTR)r.m_pMem;
					}
				}
				Player.EndSortProtect(bSort);
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


constexpr PCWSTR c_pszWndClassMain = L"PlayerNew.WndClass.Main";

void CWndMain::UpdateDpi(int iDpi)
{
	m_iDpi = iDpi;
	eck::UpdateDpiSize(m_Ds, iDpi);
}

void CWndMain::InitBK()
{
	RECT rc;
	auto p1 = new CUIAlbum;
	m_BK.AddElem(p1);
	rc = { 30,150,800,700 };
	p1->SetElemRect(&rc);

	auto p = new CUIInfo;
	m_BK.AddElem(p);
	p->InitElem();
	rc = { 50,10,800,300 };
	p->SetElemRect(&rc);

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

	auto ppc = new CUIPlayingCtrl;
	m_BK.AddElem(ppc);
	ppc->InitElem();
	rc = { 70,760,800,900 };
	ppc->SetElemRect(&rc);

	//auto pw = new CUIWaves;
	//m_BK.AddElem(pw);
	//pw->InitElem();
	//pw->SetElemRect({ 80,50,500,200 });
	//pw->SetLineWidth((int)(2.f * 1.5f));

	//auto pspe = new CUISpe;
	//m_BK.AddElem(pspe);
	//pspe->InitElem();
	//rc = { 400,120,900,600 };
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

	auto p2 = new CUIProgBar;
	m_BK.AddElem(p2);
	p2->InitElem();
	rc = { 90,710,800,750 };
	p2->SetElemRect(&rc);
	p2->SetMax(10000ull);

	//auto p3 = new CUIToolBar;
	//m_BK.AddElem(p3);
	//p3->InitElem();
	//p3->SetElemRect({ 70,650,800,700 });


	//eck::CRTCreateThread([](void* pParam)->UINT
	//	{
	//		while (TRUE)
	//		{
	//			PostMessageW((HWND)pParam, 114514, 101, 0);
	//			Sleep(20);
	//		}
	//		return 0;
	//	}, m_BK);

	m_BK.Redraw();
}

LRESULT CWndMain::WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CWndMain*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, p->OnSize);
	case SPBM_POSCHANGE:
	{
		p->m_xSeparateBar = (int)lParam;
		RECT rc;
		GetClientRect(hWnd, &rc);
		p->OnSize(hWnd, 0, rc.right, rc.bottom);
	}
	return 0;
	case WM_DPICHANGED:
		return HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, p->OnDpiChanged);
	case WM_NCCREATE:
		p = (CWndMain*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		break;
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, p->OnCreate);
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void CWndMain::OnSize(HWND hWnd, UINT uState, int cx, int cy)
{
	HDWP hDwp = BeginDeferWindowPos(3);
	hDwp = DeferWindowPos(hDwp, m_BK.GetHWND(), NULL,
		0,
		0,
		m_xSeparateBar,
		cy, SWP_NOZORDER | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_SPB.GetHWND(), NULL,
		m_xSeparateBar,
		0,
		m_Ds.cxSeparateBar,
		cy, SWP_NOZORDER | SWP_NOACTIVATE);
	hDwp = DeferWindowPos(hDwp, m_List.GetHWND(), NULL,
		m_xSeparateBar + m_Ds.cxSeparateBar,
		0,
		cx - m_Ds.cxSeparateBar - m_xSeparateBar,
		cy, SWP_NOZORDER | SWP_NOACTIVATE);
	EndDeferWindowPos(hDwp);
}

BOOL CWndMain::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
	App->GetPlayer().SetPlayingCtrlCallBack([this](PLAYINGCTRLTYPE uType, INT_PTR i1, INT_PTR i2)
		{
			switch (uType)
			{
			case PCT_PLAY:
				if (i1 >= 0)
					m_List.m_LVList.RedrawItem((int)i1);
				m_List.m_LVList.RedrawItem(App->GetPlayer().GetCurrFile());
				break;
			case PCT_STOP:
				m_List.m_LVList.RedrawItem((int)i1);
				break;
			case PCT_REMOVE_LATER_PLAY:
				m_List.m_LVList.RedrawItem((int)i1);
				break;
			}
			m_BK.OnPlayingControl(uType);
		});
	UpdateDpi(eck::GetDpi(hWnd));
	m_BK.Create(NULL, WS_CHILD, 0, 0, 0, 0, 0, hWnd, IDC_BK);
	m_List.Create(L"列表", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 0, 0, 0, hWnd, IDC_LIST);
	m_SPB.Create(NULL, WS_CHILD, 0, 0, 0, 0, 0, hWnd, IDC_SPB);
	m_SPB.SetNotifyMsg(SPBM_POSCHANGE);

	RECT rc;
	GetClientRect(hWnd, &rc);

	m_xSeparateBar = rc.right * 2 / 3;
	OnSize(hWnd, 0, rc.right, rc.bottom);

	m_BK.Show(SW_SHOWNOACTIVATE);
	m_List.Show(SW_SHOWNOACTIVATE);
	m_SPB.Show(SW_SHOWNOACTIVATE);

	m_pDropTarget = new CDropTargetList(*this);
	HRESULT hr = RegisterDragDrop(hWnd, m_pDropTarget);
	if (FAILED(hr))
		CApp::ShowError(hWnd, hr, CApp::ErrSrc::HResult, L"注册拖放目标失败");
	
	InitBK();
	return TRUE;
	WIN32_FIND_DATAW wfd;
	HANDLE hFind = FindFirstFileW(LR"(D:\@重要文件\@音乐\*.mp3)", &wfd);
	int i = 0;
	do
	{
		++i;
		using namespace std::literals;
		if (i < 600)
			continue;
		auto p = new Utils::MUSICINFO{};
		Utils::GetMusicInfo((LR"(D:\@重要文件\@音乐\)"s + wfd.cFileName).c_str(), *p);
		m_vItem.emplace_back(p);
		m_vGroup.emplace_back(p->rsAlbum);
		m_vGroup.back().Items.emplace_back(p);
		m_vGroup.back().Items.emplace_back(p);
		m_vGroup.back().Items.emplace_back(p);
		
		if (i > 680)
			break;
	} while (FindNextFileW(hFind, &wfd));
	FindClose(hFind);

	

	//m_vStr.resize(100000);
	//EckCounter(m_vStr.size(), i)
	//{
	//	m_vStr[i] = std::format(L"第{}行", i).c_str();
	//}
	m_Sl.SetItemHeight(60);
	m_Sl.SetDispInfoProc([this](SLGETDISPINFO* p)
		{
			if (p->bItem)
			{
				if (p->Item.idxGroup < 0)
				{
					p->Item.pszText = m_vStr[p->Item.idxItem].Data();
					p->Item.cchText = m_vStr[p->Item.idxItem].Size();
				}
				else
				{
					auto& rs = m_vGroup[p->Item.idxGroup].Items[p->Item.idxItem]->rsTitle;
					p->Item.pszText = rs.Data();
					p->Item.cchText = rs.Size();
				}
			}
			else
			{
				p->Group.pszText = m_vGroup[p->Group.idxItem].rs.Data();
				p->Group.cchText = m_vGroup[p->Group.idxItem].rs.Size();
				if (m_vGroup[p->Group.idxItem].pBmp)
				{
					p->Group.pBmp = m_vGroup[p->Group.idxItem].pBmp;
				}
				else
				{
					IWICBitmap* pWicBmp;
					CApp::WICCreateBitmap(m_vGroup[p->Group.idxItem].Items[0]->pCoverData, &pWicBmp);
					if (!pWicBmp)
						pWicBmp = App->GetWicRes()[IIDX_DefCover];
					p->Group.pDC->CreateBitmapFromWicBitmap(pWicBmp, &p->Group.pBmp);
					m_vGroup[p->Group.idxItem].pBmp = p->Group.pBmp;
				}
			}
			return 0;
		});
	m_Sl.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, 0, 1100, 900, hWnd, 0);
	m_Sl.SetGroupCount(m_vGroup.size());
	EckCounter(m_vGroup.size(), i)
	{
		m_Sl.SetGroupItemCount(i, m_vGroup[i].Items.size());
	}
	m_Sl.ReCalc(0);
	m_Sl.Redraw();
	//m_Sl.SetItemCount((int)m_vStr.size());
	return TRUE;
}

void CWndMain::OnDestroy()
{
	RevokeDragDrop(m_hWnd);
	m_pDropTarget->Release();
}

void CWndMain::OnDpiChanged(HWND hWnd, int xDpi, int yDpi, RECT* pRect)
{
	UpdateDpi(xDpi);
}

ATOM CWndMain::RegisterWndClass()
{
	WNDCLASSEX wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc_Main;
	wcex.hInstance = App->GetHInstance();
	wcex.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.lpszClassName = c_pszWndClassMain;
	wcex.cbWndExtra = sizeof(void*);
	return RegisterClassExW(&wcex);
}

HWND CWndMain::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	m_hWnd = CreateWindowExW(dwExStyle, c_pszWndClassMain, pszText, dwStyle,
		x, y, cx, cy, NULL, NULL, App->GetHInstance(), this);
	return m_hWnd;
}