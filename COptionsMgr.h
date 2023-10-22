#pragma once
#include "CApp.h"

struct FONTOPTIONS
{
	eck::CRefStrW rsFontName;
	int iWeight;
	int iFontSize;
};

enum class RepeatMode
{
	AllLoop,
	Radom,
	Single,
	SingleLoop,
	All
};

class COptionsMgr
{
public:
	FONTOPTIONS LrcFont{};

	RepeatMode iRepeatMode;

	Utils::LrcEncoding iLrcFileEncoding;

	std::vector<eck::CRefStrW> vListPath{};


	static COptionsMgr& GetInst()
	{
		static COptionsMgr om;
		return om;
	}
};