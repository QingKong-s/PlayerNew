#if defined _WIN64
#pragma comment(lib,R"(..\BassLib\bass_x64.lib)")
#pragma comment(lib,R"(..\BassLib\bass_fx_x64.lib)")
#pragma comment(lib,R"(..\BassLib\bassmidi_x64.lib)")
#pragma comment(lib,R"(..\Sqlite\sqlite3_x64.lib)")
#elif defined _WIN32
#pragma comment(lib,R"(..\BassLib\bass.lib)")
#pragma comment(lib,R"(..\BassLib\bass_fx.lib)")
#pragma comment(lib,R"(..\BassLib\bassmidi.lib)")
#endif

#include "Entry.h"
#include "Resource.h"
#include "CApp.h"
#include "CWndMain.h"
#include "CWndLrc.h"
#include "CTbGhost.h"

#include "eck\Env.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pszCmdLine, _In_ int nCmdShow)
{
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	
	HRESULT hr = OleInitialize(NULL);
	if (FAILED(hr))
	{
		CApp::ShowError(NULL, hr, CApp::ErrSrc::HResult, L"OleInitialize调用失败");
		return 0;
	}

	auto iEckRet = eck::Init(hInstance);
	if (iEckRet != eck::InitStatus::Ok)
	{
		CApp::ShowError(NULL, (DWORD)iEckRet, CApp::ErrSrc::None,
			L"eck::Init调用失败", eck::InitStatusToString(iEckRet));
		return 0;
	}

	App = new CApp{};
	App->Init(hInstance);

	CWndMain::RegisterWndClass();
	CWndBK::RegisterWndClass();
	CWndList::RegisterWndClass();
	CWndLrc::RegisterWndClass();
	CTbGhost::RegisterWndClass();

	ChangeWindowMessageFilter(CWndMain::s_uMsgTaskbarButtonCreated, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_COMMAND, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_DWMSENDICONICTHUMBNAIL, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_DWMSENDICONICLIVEPREVIEWBITMAP, MSGFLT_ADD);

	auto pWnd = new CWndMain{};
	App->m_pWndMain = pWnd;

	auto& opt = App->GetOptionsMgr();
	opt.ListFilePath.push_back(L"D:\\test");
	opt.ListFilePath.push_back(L"D:\\test - 副本");
	opt.ScLrcPaddingHeight = 30.f;
	opt.ScLrcFontMain = { L"微软雅黑", 400, 26.f };
	opt.ScLrcFontTranslation = { L"微软雅黑", 400, 15.f };
	opt.ScLrcAlign = DWRITE_TEXT_ALIGNMENT_CENTER;

	opt.DtLrcFontMain = {L"微软雅黑", 400, 40.f };


	opt.DtLrcFontMain.argbNormalGra[0] = eck::ColorrefToARGB(eck::Colorref::White);
	opt.DtLrcFontMain.argbNormalGra[1] = eck::ColorrefToARGB(eck::Colorref::Gray);

	opt.DtLrcFontMain.argbHiLightGra[0] = eck::ColorrefToARGB(eck::Colorref::White);
	opt.DtLrcFontMain.argbHiLightGra[1] = eck::ColorrefToARGB(eck::Colorref::Red);

	opt.DtLrcFontTranslation = { L"微软雅黑", 400, 30.f };

	opt.DtLrcFontTranslation.argbNormalGra[0] = eck::ColorrefToARGB(eck::Colorref::White);
	opt.DtLrcFontTranslation.argbNormalGra[1] = eck::ColorrefToARGB(eck::Colorref::Black);

	opt.DtLrcFontTranslation.argbHiLightGra[0] = eck::ColorrefToARGB(eck::Colorref::White);
	opt.DtLrcFontTranslation.argbHiLightGra[1] = eck::ColorrefToARGB(eck::Colorref::Green);

	opt.DtLrcAlign[0] = DWRITE_TEXT_ALIGNMENT_LEADING;
	opt.DtLrcAlign[1] = DWRITE_TEXT_ALIGNMENT_TRAILING;

	opt.DtLrcMinSize = { 580,160 };
#ifdef _DEBUG
	if(Test())
		return 0;
#endif // _DEBUG
	constexpr DWORD dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | 
		WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
	constexpr DWORD dwExStyle = 0u;

	const int iDpi = eck::GetDpi(NULL);
	RECT rc{ 0,0,1000 * iDpi / 96,640 * iDpi / 96 };
	AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);
	pWnd->Create(L"", dwStyle, dwExStyle, 100, 100, rc.right, rc.bottom, NULL, 0);

	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		if (!eck::PreTranslateMessage(msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	delete pWnd;
	delete App;
	eck::ThreadUnInit();
	eck::UnInit();
	OleUninitialize();
	return (int)msg.wParam;
}

#ifdef _DEBUG
BOOL Test()
{
	/*std::vector<Utils::LRCINFO> v{};
	std::vector<Utils::LRCLABEL> v2{};
	Utils::ParseLrc(LR"()", 0, v, v2);*/


	return FALSE;
}
#endif