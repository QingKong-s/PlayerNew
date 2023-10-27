#pragma once
#include "CApp.h"

struct FONTOPTIONS
{
	eck::CRefStrW rsFontName;
	int iWeight;
	float fFontSize;
};

enum class RepeatMode
{
	AllLoop,
	All,
	Radom,
	SingleLoop,
	Single,
};

class COptionsMgr
{
public:
	FONTOPTIONS LrcFont{};

	RepeatMode iRepeatMode{};

	Utils::LrcEncoding iLrcFileEncoding{};

	std::vector<eck::CRefStrW> vListPath{};


	static COptionsMgr& GetInst()
	{
		static COptionsMgr om;
		return om;
	}
};