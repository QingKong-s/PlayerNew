#pragma once
#include "Utils.h"
#include "sqlite3.h"

static constexpr WCHAR c_pszStatFileExt[]{ L".PNStat" };

class CStatistics
{
private:
	sqlite3* m_pDb = NULL;

	eck::CRefStrW m_rsTableSong{};
	eck::CRefStrW m_rsTableArtist{};

	char* m_pszLastErrMsg = NULL;
public:
	~CStatistics()
	{
		sqlite3_free(m_pszLastErrMsg);
		sqlite3_close(m_pDb);
	}

	int LoadDataBase(PCWSTR pszFileName);

	int InitTable(int iYear = 0, int iMonth = 0);

	int IncPlayCount(const eck::CRefStrW& rsSong);

	int IncLoopCount(const eck::CRefStrW& rsSong)
	{
		using namespace eck::Literals;
		const auto rsSql =
			L"UPDATE "_rs +
			m_rsTableSong +
			L" SET LoopCount = LoopCount + 1 "
			L"WHERE Song = \"" +
			rsSong +
			L"\""
			;

		sqlite3_free(m_pszLastErrMsg);
		return sqlite3_exec(m_pDb, eck::StrW2X(rsSql, CP_UTF8).Data(), NULL, NULL, &m_pszLastErrMsg);
	}

	int IncPlayTime(const eck::CRefStrW& rsSong, int iSeconds)
	{
		EckAssert(iSeconds >= 0);
		using namespace eck::Literals;
		const auto rsSql =
			L"UPDATE "_rs +
			m_rsTableSong +
			L" SET PlayTime = PlayTime + " + 
			eck::ToStr(iSeconds) +
			L" WHERE Song = \"" +
			rsSong +
			L"\""
			;

		sqlite3_free(m_pszLastErrMsg);
		return sqlite3_exec(m_pDb, eck::StrW2X(rsSql, CP_UTF8).Data(), NULL, NULL, &m_pszLastErrMsg);
	}

	void IncPlayCount(const std::vector<eck::CRefStrW>& vArtist);

	void IncPlayTime(const std::vector<eck::CRefStrW>& vArtist, int iSeconds)
	{

	}

	eck::CRefStrW GetLastErrorMsg()
	{
		return eck::StrX2W(m_pszLastErrMsg, -1, CP_UTF8);
	}
};