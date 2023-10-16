#include "CApp.h"
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

CApp::~CApp()
{
	m_pD2dFactory->Release();
	m_pDwFactory->Release();
	m_pWicFactory->Release();
	m_pD2dDevice->Release();
	m_pDxgiDevice->Release();
	m_pDxgiFactory->Release();
}

void CApp::Init(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
	eck::Init(hInstance);

	HRESULT hr;
#ifndef NDEBUG
	D2D1_FACTORY_OPTIONS D2DFactoryOptions;
	D2DFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &D2DFactoryOptions, (void**)&m_pD2dFactory);
#else
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, IID_PPV_ARGS(&m_pD2dFactory));
#endif // !NDEBUG
	if (FAILED(hr))
	{
	}
	//////////////创建DWrite工厂
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_pDwFactory);
	if (FAILED(hr))
	{
	}
	//////////////创建DXGI工厂
	ID3D11Device* pD3DDevice;
	hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifndef NDEBUG
		| D3D11_CREATE_DEVICE_DEBUG
#endif // !NDEBUG
		, NULL, 0, D3D11_SDK_VERSION, &pD3DDevice, NULL, NULL);
	if (FAILED(hr))
	{
	}
	pD3DDevice->QueryInterface(IID_PPV_ARGS(&m_pDxgiDevice));
	pD3DDevice->Release();

	IDXGIAdapter* pDXGIAdapter;
	m_pDxgiDevice->GetAdapter(&pDXGIAdapter);

	pDXGIAdapter->GetParent(IID_PPV_ARGS(&m_pDxgiFactory));
	pDXGIAdapter->Release();
	//////////////创建DXGI设备
	hr = m_pD2dFactory->CreateDevice(m_pDxgiDevice, &m_pD2dDevice);
	if (FAILED(hr))
	{
	}
	//////////////创建WIC工厂
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWicFactory));
	if (FAILED(hr))
	{
	}
	
}

HRESULT CApp::WICCreateBitmap(PWSTR pszFile, IWICBitmap** ppWICBitmap)
{
	*ppWICBitmap = NULL;
	IWICBitmapDecoder* pDecoder;
	HRESULT hr = App->m_pWicFactory->CreateDecoderFromFilename(pszFile, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
	if (FAILED(hr))
	{
		EckDbgBreak();
		return hr;
	}
	return WICCreateBitmap(pDecoder, ppWICBitmap);
}

HRESULT CApp::WICCreateBitmap(IStream* pStream, IWICBitmap** ppWICBitmap)
{
	*ppWICBitmap = NULL;
	IWICBitmapDecoder* pDecoder;
	HRESULT hr = App->m_pWicFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnDemand, &pDecoder);
	if (FAILED(hr))
	{
		EckDbgBreak();
		return hr;
	}
	return WICCreateBitmap(pDecoder, ppWICBitmap);
}

void CApp::ShowError(HWND hWnd, EckOpt(DWORD, dwErrCode), ErrSrc uSrc, PCWSTR pszInfo, PCWSTR pszTitle)
{
	TASKDIALOGCONFIG tdc{ sizeof(TASKDIALOGCONFIG) };
	tdc.hwndParent = hWnd;
	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
	tdc.dwCommonButtons = TDCBF_OK_BUTTON;
	tdc.pszMainIcon = TD_ERROR_ICON;

	tdc.pszWindowTitle = pszTitle ? pszTitle : L"错误";

	tdc.pszMainInstruction = pszInfo ? pszInfo : L"发生错误";

	std::wstring sContent;
	constexpr PCWSTR c_pszErrInfoFmt = L"错误代码：0x{:08X}\n\n错误信息：\n{}";
	switch (uSrc)
	{
	case CApp::ErrSrc::Win32:
		if (!dwErrCode.has_value())
			dwErrCode = GetLastError();
		// fall through
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

	int iTemp;
	TaskDialogIndirect(&tdc, &iTemp, &iTemp, &iTemp);
}
