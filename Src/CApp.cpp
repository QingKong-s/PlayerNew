﻿#include "pch.h"

CApp* App;

HRESULT CApp::WICCreateBitmap(IWICBitmapDecoder* pDecoder, IWICBitmap** ppBitmap)
{
	HRESULT hr;
	IWICBitmapFrameDecode* pFrameDecoder;
	IWICFormatConverter* pConverter;
	hr = App->m_pWicFactory->CreateFormatConverter(&pConverter);
	if (FAILED(hr))
	{
		EckDbgBreak();
		return hr;
	}

	hr = pDecoder->GetFrame(0, &pFrameDecoder);
	if (FAILED(hr))
	{
		EckDbgBreak();
		return hr;
	}

	hr = pConverter->Initialize(pFrameDecoder, GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);
	if (FAILED(hr))
	{
		EckDbgBreak();
		return hr;
	}

	hr = App->m_pWicFactory->CreateBitmapFromSource(pConverter, WICBitmapNoCache, ppBitmap);
	if (FAILED(hr))
	{
		EckDbgBreak();
		return hr;
	}

	pFrameDecoder->Release();
	pDecoder->Release();
	pConverter->Release();
	return S_OK;
}

void CApp::LoadRes()
{
	HRESULT hr;
	eck::CRefStrW rs = eck::GetRunningPath();
	const int cchPath = rs.Size();
	rs.ReSize(rs.Size() + MAX_PATH);
	EckCounter(ARRAYSIZE(c_szResFile), i)
	{
		wcscpy(rs.Data() + cchPath, c_szResFile[i]);
		if (FAILED(hr = WICCreateBitmap(rs.Data(), &m_pWicRes[i])))
			ShowError(NULL, hr, ErrSrc::HResult, L"加载资源时出错", std::format(LR"("{}"加载失败)", rs.Data()).c_str());
	}
}

CApp::~CApp()
{
	for (auto& p : m_pWicRes)
		SafeRelease(p);
	//m_pD2dFactory->Release();
	//m_pDwFactory->Release();
	//m_pWicFactory->Release();
	//m_pD2dDevice->Release();
	//m_pDxgiDevice->Release();
	//m_pDxgiFactory->Release();
}

void CApp::Init(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
	CBass::Init();

	m_cfListDrag = (CLIPFORMAT)RegisterClipboardFormatW(L"PlayerNew.CBFmt.ListDrag");
	EckAssert(m_cfListDrag);

	m_pD2dFactory = eck::g_pD2dFactory;
	m_pDwFactory = eck::g_pDwFactory;
	m_pWicFactory = eck::g_pWicFactory;
	m_pD2dDevice = eck::g_pD2dDevice;
	m_pDxgiDevice = eck::g_pDxgiDevice;
	m_pDxgiFactory = eck::g_pDxgiFactory;

	LoadRes();
}

void CApp::InvertIconColor()
{
	constexpr D2D1_RENDER_TARGET_PROPERTIES Prop
	{
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		{ DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED },
		96.f,96.f,
		D2D1_RENDER_TARGET_USAGE_NONE,
		D2D1_FEATURE_LEVEL_DEFAULT
	};
	ID2D1RenderTarget* pRT;
	ID2D1Bitmap* pD2dBitmap;

	ID2D1DeviceContext* pDC;
	ID2D1Effect* pEffect;
	IWICBitmap* pNewBitmap;

	UINT cx, cy;

	for (auto& pBmp : m_pWicRes)
	{
		pBmp->GetSize(&cx, &cy);
		App->m_pWicFactory->CreateBitmap(cx, cy, GUID_WICPixelFormat32bppPBGRA,
			WICBitmapCacheOnLoad, &pNewBitmap);
		App->m_pD2dFactory->CreateWicBitmapRenderTarget(pNewBitmap, &Prop, &pRT);
		pRT->CreateBitmapFromWicBitmap(pBmp, &pD2dBitmap);

		pRT->QueryInterface(&pDC);
		pDC->CreateEffect(CLSID_D2D1ColorMatrix, &pEffect);
		EckAssert(pEffect);
		pEffect->SetInput(0, pD2dBitmap);
		auto mat = D2D1::Matrix5x4F(-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0.999f, 0.999f, 0.999f, 0);
		pEffect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, mat);

		pDC->BeginDraw();
		pDC->Clear({});
		pDC->DrawImage(pEffect);
		pDC->EndDraw();

		std::swap(pBmp, pNewBitmap);

		pEffect->Release();
		pD2dBitmap->Release();
		pDC->Release();
		pRT->Release();
		pNewBitmap->Release();
	}
}

HRESULT CApp::WICCreateBitmap(PCWSTR pszFile, IWICBitmap** ppWICBitmap)
{
	*ppWICBitmap = NULL;
	IWICBitmapDecoder* pDecoder;
	HRESULT hr = App->m_pWicFactory->CreateDecoderFromFilename(pszFile, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
	if (FAILED(hr))
		return hr;
	return WICCreateBitmap(pDecoder, ppWICBitmap);
}

HRESULT CApp::WICCreateBitmap(IStream* pStream, IWICBitmap** ppWICBitmap)
{
	*ppWICBitmap = NULL;
	IWICBitmapDecoder* pDecoder;
	HRESULT hr = App->m_pWicFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnDemand, &pDecoder);
	if (FAILED(hr))
		return hr;
	return WICCreateBitmap(pDecoder, ppWICBitmap);
}

void CApp::ShowError(HWND hWnd, EckOpt(DWORD, dwErrCode), ErrSrc uSrc, PCWSTR pszInfo, PCWSTR pszDetail, PCWSTR pszTitle)
{
	TASKDIALOGCONFIG tdc{ sizeof(TASKDIALOGCONFIG) };
	tdc.hwndParent = hWnd;
	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
	tdc.dwCommonButtons = TDCBF_OK_BUTTON;
	tdc.pszMainIcon = TD_ERROR_ICON;

	tdc.pszWindowTitle = pszTitle ? pszTitle : L"错误";

	tdc.pszMainInstruction = pszInfo ? pszInfo : L"发生错误";

	std::wstring sContent;
	if (pszDetail)
		sContent = std::wstring(pszDetail) + L"\n";
	constexpr PCWSTR c_pszErrInfoFmt = L"错误代码：0x{:08X}\n\n错误信息：\n{}";
	switch (uSrc)
	{
	case CApp::ErrSrc::Win32:
		if (!dwErrCode.has_value())
			dwErrCode = GetLastError();
		[[fallthrough]];
	case CApp::ErrSrc::HResult:
	{
		PWSTR pszErrMsg = NULL;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
			dwErrCode.value(), 0, (PWSTR)&pszErrMsg, 0, NULL);
		sContent = std::format(c_pszErrInfoFmt, dwErrCode.value(), pszErrMsg);
		LocalFree(pszErrMsg);
	}
	break;
	case CApp::ErrSrc::Bass:
		if (!dwErrCode.has_value())
			dwErrCode = CBass::GetError();
		PCWSTR pszErrMsg = CBass::GetErrorMsg(dwErrCode.value());
		sContent = std::format(c_pszErrInfoFmt, dwErrCode.value(), pszErrMsg);
		break;
	}
	tdc.pszContent = sContent.c_str();

	eck::TASKDIALOGCTX Ctx{ &tdc };
	eck::CTaskDialog{}.DlgBox(hWnd, &Ctx);
}