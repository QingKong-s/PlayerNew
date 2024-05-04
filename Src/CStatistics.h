#pragma once
#include "Utils.h"
#include "sqlite3.h"

static constexpr WCHAR c_pszStatFileExt[]{ L".PNStat" };

class CStatistics
{
private:
	sqlite3* m_pDb{};

	eck::CRefStrW m_rsTableSong{};
	eck::CRefStrW m_rsTableArtist{};

	eck::CRefStrW m_rsLastError{};
public:
	~CStatistics()
	{
		sqlite3_close(m_pDb);
	}

	int LoadDataBase(PCWSTR pszFileName);

	int InitTable(int iYear = 0, int iMonth = 0);

	int IncPlayCount(const eck::CRefStrW& rsSong);

	int IncLoopCount(const eck::CRefStrW& rsSong);

	int IncPlayTime(const eck::CRefStrW& rsSong, int iSeconds);

	int IncPlayCount(const std::vector<eck::CRefStrW>& vArtist);

	int IncLoopCount(const std::vector<eck::CRefStrW>& vArtist);

	int IncPlayTime(const std::vector<eck::CRefStrW>& vArtist, int iSeconds);

	EckInline const auto& GetLastErrorMsg() const { return m_rsLastError; }
};