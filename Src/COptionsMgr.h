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


	static COptionsMgr& GetInst()
	{
		static COptionsMgr om;
		return om;
	}

	static RepeatMode NextRepeatMode(RepeatMode i)
	{
		auto ii = (int)i;
		++ii;
		if (ii >= (int)RepeatMode::P_End)
			ii = (int)RepeatMode::P_Begin;
		return (RepeatMode)ii;
	}
};