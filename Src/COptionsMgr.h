#pragma once
#include <Windows.h>
#include <dwrite.h>

#include "Utils.h"

#include "eck\CRefStr.h"
#include "eck\WndHelper.h"



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

class COptionsMgr
{
public:
	FONTOPTIONS LrcFont{};

	RepeatMode iRepeatMode{};

	Utils::LrcEncoding iLrcFileEncoding{};

	std::vector<eck::CRefStrW> vListPath{};

	float cyLrcPadding{};

	FONTOPTIONS DtLrcFontMain{};
	FONTOPTIONS DtLrcFontTranslation{};
	BOOL DtLrcBorder = TRUE;
	ARGB DtLrcBorderColor{};
	int DtLrcAlign[2]{};
	SIZE DtLrcMinSize{};
	BOOL DtLrcEnableShadow{ 1 };
	float DtLrcShadowOffset{ 1.5f };

	BOOL ProgShowCoverLivePreview{ 1 };


	//static COptionsMgr& GetInst()
	//{
	//	static COptionsMgr om;
	//	return om;
	//}

	static RepeatMode NextRepeatMode(RepeatMode i)
	{
		auto ii = (int)i;
		++ii;
		if (ii >= (int)RepeatMode::P_End)
			ii = (int)RepeatMode::P_Begin;
		return (RepeatMode)ii;
	}
};