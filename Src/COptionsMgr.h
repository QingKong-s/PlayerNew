#pragma once
#include <Windows.h>
#include <dwrite.h>

#include "Utils.h"

#include "eck\CRefStr.h"
#include "eck\WndHelper.h"
#include "eck\CWindowPosSetting.h"
#include "eck\CFontPicker.h"


struct FONTOPTIONS
{
	eck::CRefStrW rsFontName{};
	int iWeight = 400;
	float fFontSize = 0.f;
	union
	{
		ARGB argbNormal;
		ARGB argbNormalGra[2]{};
	};
	union
	{
		ARGB argbHiLight;
		ARGB argbHiLightGra[2]{};
	};
	union
	{
		struct
		{
			eck::BITBOOL bItalic : 1;
			eck::BITBOOL bAutoWrap : 1;
		};
		UINT uFlags{};
	};

	operator eck::FONTPICKERINFO() const
	{
		return { rsFontName.Data(),(int)fFontSize,iWeight };
	}

	void FromFontPicker(eck::CFontPicker& fp)
	{
		eck::FONTPICKERINFO fpi{};
		rsFontName = fp.ToInfo(fpi);
		iWeight = fpi.iWeight;
		fFontSize = (float)fpi.iPointSize;
	}

	static IDWriteTextFormat* CreateTextFormat(IDWriteFactory* pDWriteFactory, 
		const FONTOPTIONS& fo, int iDpi, HRESULT* phr = NULL)
	{
		IDWriteTextFormat* pTextFormat;
		auto hr = pDWriteFactory->CreateTextFormat(
			fo.rsFontName.Data(),
			NULL,
			(DWRITE_FONT_WEIGHT)fo.iWeight,
			fo.bItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			eck::DpiScaleF(fo.fFontSize, iDpi),
			L"zh-cn",
			&pTextFormat
		);
		if (phr)
			*phr = hr;
		if (FAILED(hr))
			return NULL;

		pTextFormat->SetWordWrapping(fo.bAutoWrap ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
		return pTextFormat;
	}
};

enum class RepeatMode
{
	P_Begin = 0,
	AllLoop = P_Begin,
	All,
	Radom,
	SingleLoop,
	Single,
	P_End
};

enum class DtLrcView
{
	SingleLine,
	DoubleLine,
};

class COptionsMgr
{
public:
	RepeatMode PlayRepeatMode{};
	BOOL PlaySilent{};

	Utils::LrcEncoding		LrcFileEncoding{};
	std::vector<eck::CRefStrW> LrcPath{};

	BOOL					DtLrcBorder{ TRUE };
	ARGB					DtLrcBorderColor{};
	float					DtLrcBorderWidth{ 1.f };
	BOOL					DtLrcEnableShadow{ TRUE };
	float					DtLrcShadowOffset{ 1.5f };
	BOOL					DtLrcShow{ FALSE };
	BOOL					DtLrcSpaceLine{ FALSE };
	eck::CRefStrW			DtLrcSpaceLineText{};
	BOOL					DtLrcShowTranslation{ TRUE };
	FONTOPTIONS				DtLrcFontMain{};
	FONTOPTIONS				DtLrcFontTranslation{};
	int						DtLrcAlign[2]{};
	BYTE					DtLrcAlpha{ 255 };
	SIZE					DtLrcMinSize{};
	DtLrcView				DtLrcView{ DtLrcView::DoubleLine };
	eck::CWindowPosSetting	DtLrcWndPos{};

	BOOL					ScLrcAnimation{ TRUE };
	BOOL					ScLrcAutoWrap{ FALSE };
	BOOL					ScLrcShowTranslation{ TRUE };
	BOOL					ScLrcSpaceLine{ FALSE };
	eck::CRefStrW			ScLrcSpaceLineText{};
	FONTOPTIONS				ScLrcFontMain{};
	FONTOPTIONS				ScLrcFontTranslation{};// 不使用颜色信息
	int						ScLrcAlign{};
	float					ScLrcCurrFontScale{ 1.3f };
	float					ScLrcPaddingHeight{};


	BOOL ProgShowCoverLivePreview{ TRUE };

	std::vector<eck::CRefStrW> ListFilePath{};
	BOOL ListAutoSave{ TRUE };

	int FxPage{};

	int MiPage{};

	static RepeatMode NextRepeatMode(RepeatMode i)
	{
		auto ii = (int)i;
		++ii;
		if (ii >= (int)RepeatMode::P_End)
			ii = (int)RepeatMode::P_Begin;
		return (RepeatMode)ii;
	}
};