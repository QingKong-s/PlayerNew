#include "CStatistics.h"

int CStatistics::LoadDataBase(PCWSTR pszFileName)
{
	if (m_pDb)
		sqlite3_close(m_pDb);
	const int iRet = sqlite3_open16(pszFileName, &m_pDb);
	if (iRet != SQLITE_OK)
	{
		sqlite3_close(m_pDb);
		m_pDb = NULL;
	}
	return iRet;
}

int CStatistics::InitTable(int iYear, int iMonth)
{
	using namespace eck::Literals;
#define TABLE_FMT L"Year_%04hu_Month_%02hu"
	SYSTEMTIME st;
	if (!iYear || !iMonth)
		GetLocalTime(&st);
	else
		st.wYear = iYear, st.wMonth = iMonth;
	m_rsTableSong.Format(TABLE_FMT L"_0", st.wYear, st.wMonth);
	m_rsTableArtist.Format(TABLE_FMT L"_1", st.wYear, st.wMonth);

	const auto rsSql =
		L"CREATE TABLE IF NOT EXISTS "_rs + m_rsTableSong +
		L" (Song		TEXT		PRIMARY KEY,"
		L"	PlayCount	INTEGER,"
		L"	PlayTime	INTEGER,"
		L"	LoopCount	INTEGER);"
		L"CREATE TABLE IF NOT EXISTS "_rs + m_rsTableArtist +
		L" (Name		TEXT		PRIMARY KEY,"
		L"	PlayCount	INTEGER,"
		L"	LoopCount	INTEGER);"
		;

	sqlite3_free(m_pszLastErrMsg);
	return sqlite3_exec(m_pDb, eck::StrW2X(rsSql, CP_UTF8).Data(), NULL, NULL, &m_pszLastErrMsg);
}

int CStatistics::IncPlayCount(const eck::CRefStrW& rsSong)
{
	using namespace eck::Literals;
	const auto rsSql =
		L"INSERT INTO "_rs + m_rsTableSong +
		L" VALUES(\"" + rsSong + L"\", 1, 0, 0) "
		L"ON CONFLICT(Song) DO UPDATE "
		L"SET PlayCount = PlayCount + 1 "
		L"WHERE Song = \"" + rsSong + L"\""
		;

	sqlite3_free(m_pszLastErrMsg);
	return sqlite3_exec(m_pDb, eck::StrW2X(rsSql, CP_UTF8).Data(), NULL, NULL, &m_pszLastErrMsg);
}

void CStatistics::IncPlayCount(const std::vector<eck::CRefStrW>& vArtist)
{
	using namespace eck::Literals;

	std::vector<eck::CRefStrA> vSqlU8(vArtist.size());

	EckCounter(vArtist.size(), i)
	{
		auto a = L"INSERT INTO "_rs + m_rsTableArtist +
			L" VALUES(\"" + vArtist[i] + L"\", 1, 0) "
			L"ON CONFLICT(Name) DO UPDATE " +
			L"SET PlayCount = PlayCount + 1 "
			L"WHERE Name = \"" + vArtist[i] + L"\"";
		sqlite3_exec(m_pDb,
			eck::StrW2X(
				L"INSERT INTO "_rs + m_rsTableArtist +
				L" VALUES(\"" + vArtist[i] + L"\", 1, 0) "
				L"ON CONFLICT(Name) DO UPDATE " +
				L"SET PlayCount = PlayCount + 1 "
				L"WHERE Name = \"" + vArtist[i] + L"\"",
				CP_UTF8).Data(),
			NULL, NULL, NULL);
	}
}
