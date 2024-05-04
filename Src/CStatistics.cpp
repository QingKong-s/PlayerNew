#include "CStatistics.h"

int CStatistics::LoadDataBase(PCWSTR pszFileName)
{
	if (m_pDb)
		sqlite3_close(m_pDb);
	const int iRet = sqlite3_open(eck::StrW2X(pszFileName, -1, CP_UTF8).Data(), &m_pDb);
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
#undef TABLE_FMT
	const auto rsSql =
		L"CREATE TABLE IF NOT EXISTS "_rs + m_rsTableSong +
		L" (Name		TEXT		PRIMARY KEY,"
		L"	PlayCount	INTEGER,"
		L"	PlayTime	INTEGER,"
		L"	LoopCount	INTEGER);"
		L"CREATE TABLE IF NOT EXISTS "_rs + m_rsTableArtist +
		L" (Name		TEXT		PRIMARY KEY,"
		L"	PlayCount	INTEGER,"
		L"	PlayTime	INTEGER,"
		L"	LoopCount	INTEGER);"
		;

	PSTR pszErr;
	const auto i = sqlite3_exec(m_pDb, eck::StrW2X(rsSql, CP_UTF8).Data(), NULL, NULL, &pszErr);
	if (pszErr)
		m_rsLastError = eck::StrX2W(pszErr, -1, CP_UTF8);
	else
		m_rsLastError.Clear();
	sqlite3_free(pszErr);
#ifdef _DEBUG
	if (i != SQLITE_OK)
		EckDbgPrintWithPos(m_rsLastError.Data());
#endif
	return i;
}

int CStatistics::IncPlayCount(const eck::CRefStrW& rsSong)
{
	using namespace eck::Literals;
	const auto rsSql =
		L"INSERT INTO "_rs + m_rsTableSong +
		L" VALUES(\"" + rsSong + L"\", 1, 0, 0)"
		L" ON CONFLICT(Name) DO UPDATE"
		L" SET PlayCount = PlayCount + 1"
		L" WHERE Name = \"" + rsSong + L"\"";

	PSTR pszErr;
	const auto i = sqlite3_exec(m_pDb, eck::StrW2X(rsSql, CP_UTF8).Data(), NULL, NULL, &pszErr);
	if (pszErr)
		m_rsLastError = eck::StrX2W(pszErr, -1, CP_UTF8);
	else
		m_rsLastError.Clear();
	sqlite3_free(pszErr);
#ifdef _DEBUG
	if (i != SQLITE_OK)
		EckDbgPrintWithPos(m_rsLastError.Data());
#endif
	return i;
}

int CStatistics::IncLoopCount(const eck::CRefStrW& rsSong)
{
	using namespace eck::Literals;
	const auto rsSql =
		L"UPDATE "_rs + m_rsTableSong +
		L" SET LoopCount = LoopCount + 1"
		L" WHERE Name = \"" + rsSong + L"\"";

	PSTR pszErr;
	const auto i = sqlite3_exec(m_pDb, eck::StrW2X(rsSql, CP_UTF8).Data(), NULL, NULL, &pszErr);
	if (pszErr)
		m_rsLastError = eck::StrX2W(pszErr, -1, CP_UTF8);
	else
		m_rsLastError.Clear();
	sqlite3_free(pszErr);
#ifdef _DEBUG
	if (i != SQLITE_OK)
		EckDbgPrintWithPos(m_rsLastError.Data());
#endif
	return i;
}

int CStatistics::IncPlayTime(const eck::CRefStrW& rsSong, int iSeconds)
{
	EckAssert(iSeconds >= 0);
	using namespace eck::Literals;
	const auto rsSql =
		L"UPDATE "_rs + m_rsTableSong +
		L" SET PlayTime = PlayTime + " + eck::ToStr(iSeconds) +
		L" WHERE Name = \"" + rsSong + L"\"";

	PSTR pszErr;
	const auto i = sqlite3_exec(m_pDb, eck::StrW2X(rsSql, CP_UTF8).Data(), NULL, NULL, &pszErr);
	if (pszErr)
		m_rsLastError = eck::StrX2W(pszErr, -1, CP_UTF8);
	else
		m_rsLastError.Clear();
	sqlite3_free(pszErr);
#ifdef _DEBUG
	if (i != SQLITE_OK)
		EckDbgPrintWithPos(m_rsLastError.Data());
#endif
	return i;
}

int CStatistics::IncPlayCount(const std::vector<eck::CRefStrW>& vArtist)
{
	using namespace eck::Literals;
	PSTR pszErr;
	for (const auto& e : vArtist)
	{
		const auto r = sqlite3_exec(m_pDb,
			eck::StrW2X(
				L"INSERT INTO "_rs + m_rsTableArtist +
				L" VALUES(\"" + e + L"\", 1, 0, 0)"
				L" ON CONFLICT(Name) DO UPDATE" +
				L" SET PlayCount = PlayCount + 1"
				L" WHERE Name = \"" + e + L"\"",
				CP_UTF8).Data(),
			NULL, NULL, &pszErr);
		if (r != SQLITE_OK)
		{
			if (pszErr)
				m_rsLastError = eck::StrX2W(pszErr, -1, CP_UTF8);
			else
				m_rsLastError.Clear();
			sqlite3_free(pszErr);
			EckDbgPrintWithPos(m_rsLastError.Data());
			return r;
		}
	}
	return SQLITE_OK;
}

int CStatistics::IncLoopCount(const std::vector<eck::CRefStrW>& vArtist)
{
	using namespace eck::Literals;
	PSTR pszErr;
	for (const auto& e : vArtist)
	{
		const auto r = sqlite3_exec(m_pDb,
			eck::StrW2X(
				L"UPDATE "_rs + m_rsTableArtist +
				L" SET LoopCount = LoopCount + 1"
				L" WHERE Name = \"" + e + L"\"",
				CP_UTF8).Data(),
			NULL, NULL, &pszErr);
		if (r != SQLITE_OK)
		{
			if (pszErr)
				m_rsLastError = eck::StrX2W(pszErr, -1, CP_UTF8);
			else
				m_rsLastError.Clear();
			sqlite3_free(pszErr);
			EckDbgPrintWithPos(m_rsLastError.Data());
			return r;
		}
	}
	return SQLITE_OK;
}

int CStatistics::IncPlayTime(const std::vector<eck::CRefStrW>& vArtist, int iSeconds)
{
	using namespace eck::Literals;
	PSTR pszErr;
	for (const auto& e : vArtist)
	{
		const auto i = sqlite3_exec(m_pDb,
			eck::StrW2X(
				L"UPDATE "_rs + m_rsTableArtist +
				L" SET PlayTime = PlayTime + " + eck::ToStr(iSeconds) +
				L" WHERE Name = \"" + e + L"\"",
				CP_UTF8).Data(),
			NULL, NULL, &pszErr);
		if (i != SQLITE_OK)
		{
			if (pszErr)
				m_rsLastError = eck::StrX2W(pszErr, -1, CP_UTF8);
			else
				m_rsLastError.Clear();
			sqlite3_free(pszErr);
			EckDbgPrintWithPos(m_rsLastError.Data());
			return i;
		}
	}
	return SQLITE_OK;
}