﻿#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"D2d1.lib")
#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"dxguid.lib")
#if defined _WIN64
#pragma comment(lib,R"(.\BassLib\bass_x64.lib)")
#pragma comment(lib,R"(.\BassLib\bass_fx_x64.lib)")
#pragma comment(lib,R"(.\BassLib\bassmidi_x64.lib)")
#elif defined _WIN32
#pragma comment(lib,R"(.\BassLib\bass.lib)")
#pragma comment(lib,R"(.\BassLib\bass_fx.lib)")
#pragma comment(lib,R"(.\BassLib\bassmidi.lib)")
#endif

#include "Entry.h"
#include "Resource.h"
#include "CApp.h"
#include "CWndMain.h"

#include "eck\Env.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pszCmdLine, _In_ int nCmdShow)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		CApp::ShowError(NULL, hr, CApp::ErrSrc::HResult, L"CoInitialize调用失败");
		return 0;
	}

	App = new CApp;
	App->Init(hInstance);

	CWndMain::RegisterWndClass();
	CWndMain Wnd{};

#ifdef _DEBUG
	if(Test())
		return 0;
#endif // _DEBUG
	const DWORD dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE | WS_OVERLAPPEDWINDOW;
	const DWORD dwExStyle = 0u;

	const int iDpi = GetDpiForSystem();
	RECT rc{ 0,0,512 * iDpi / 96,768 * iDpi / 96 };
	AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);
	Wnd.Create(L"", dwStyle, dwExStyle, 100, 100, rc.right, rc.bottom, NULL, 0);

	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	delete App;
	CoUninitialize();
	return (int)msg.wParam;
}

#ifdef _DEBUG
BOOL Test()
{
	std::vector<Utils::LRCINFO> v{};
	std::vector<Utils::LRCLABEL> v2{};
	Utils::ParseLrc(LR"(D:\@重要文件\@音乐\'Fuwa Fuwa Time' #6 'Gakuen Sai!' Version - 桜高軽音部.lrc)", 0, v, v2);
	return FALSE;
}
#endif