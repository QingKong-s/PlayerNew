#include "CPlayListFile.h"

BOOL CPlayListFile::Open(PCWSTR pszFile)
{
	auto p = m_File.Open(pszFile, FILE_MAP_READ, PAGE_READONLY, eck::CFile::ModeExisting, GENERIC_READ);
	if (!p)
		return FALSE;

	if (memcpy(p, "PNPL", 4) == 0)
	{
		m_pHeader0 = NULL;
		m_pHeader1 = (LISTFILEHEADER_1*)p;
		return TRUE;
	}
	else if (memcpy(p, "QKPL", 4) == 0)
	{
		m_pHeader0 = (LISTFILEHEADER_0*)p;
		m_pHeader1 = NULL;
		return TRUE;
	}
	else
		return FALSE;
}

int CPlayListFile::GetItemCount()
{
	if (m_pHeader1)
		return m_pHeader1->cItems;
	else if (m_pHeader0)
		return m_pHeader0->iCount;
	else
	{
		EckDbgBreak();
		return 0;
	}
}

void CPlayListFile::For(FItemProcessor fnProcessor)
{
	eck::CMemReader r{ NULL };
	PCWSTR pszName, pszFile, pszTime;
	if (m_pHeader1)
	{
#ifdef _DEBUG
		r.SetPtr(m_pHeader1, m_File.GetFile().GetSize32());
#else
		r.SetPtr(m_pHeader1, 0);
#endif
		r += sizeof(LISTFILEHEADER_1);
		const LISTFILEITEM_1* pItem;

		EckCounter(m_pHeader1->cItems, i)
		{
			r.SkipPointer(pItem);

			pszName = (PCWSTR)r.m_pMem;
			r += ((pItem->cchName + 1) * sizeof(WCHAR));
			pszFile = (PCWSTR)r.m_pMem;
			r += ((pItem->cchFile + 1) * sizeof(WCHAR));
			pszTime = (PCWSTR)r.m_pMem;
			r += ((pItem->cchTime + 1) * sizeof(WCHAR));

			fnProcessor(pItem, pszName, pszFile, pszTime);
		}
	}
	else if (m_pHeader0)
	{
#ifdef _DEBUG
		r.SetPtr(m_pHeader0, m_File.GetFile().GetSize32());
#else
		r.SetPtr(m_pHeader0, 0);
#endif
		r += sizeof(LISTFILEHEADER_0);
		LISTFILEITEM_1 Item1;
		const LISTFILEITEM_0* pItem;
		EckCounter(m_pHeader0->iCount, i)
		{
			r.SkipPointer(pItem);
			Item1.bIgnore = eck::IsBitSet(pItem->uFlags, QKLIF_IGNORED);
			Item1.bDelayPlaying = FALSE;

			pszName = (PCWSTR)r.m_pMem;
			Item1.cchName = (int)wcslen(pszName);
			r += ((Item1.cchName + 1) * sizeof(WCHAR));

			pszFile = (PCWSTR)r.m_pMem;
			Item1.cchFile = (int)wcslen(pszFile);
			r += ((Item1.cchFile + 1) * sizeof(WCHAR));

			if (eck::IsBitSet(pItem->uFlags, QKLIF_BOOKMARK))
			{
				r += sizeof(COLORREF);
				r += ((wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR));
				r += ((wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR));
			}

			if (m_pHeader0->dwVer == QKLFVER_2)
			{
				pszTime = (PCWSTR)r.m_pMem;
				Item1.cchTime = (int)wcslen(pszTime);
				r += ((Item1.cchTime + 1) * sizeof(WCHAR));
			}
			else
			{
				Item1.cchTime = 0;
				pszTime = NULL;
			}

			fnProcessor(&Item1, pszName, pszFile, pszTime);
		}
	}
	else
		EckDbgBreak();
}

void CPlayListFile::ForBookmark(FBookmarkProcessor fnProcessor)
{
	eck::CMemReader r{ NULL };
	PCWSTR pszName;
	if (m_pHeader1)
	{
#ifdef _DEBUG
		r.SetPtr(m_pHeader1, m_File.GetFile().GetSize32());
#else
		r.SetPtr(m_pHeader1, 0);
#endif
		r += m_pHeader1->ocbBookMark;
		const BOOKMARKHEADER* pHeader;
		const BOOKMARKITEM* pItem;
		r.SkipPointer(pHeader);
		EckCounter(pHeader->cBookmarks, i)
		{
			r.SkipPointer(pItem);
			pszName = (PCWSTR)r.m_pMem;
			r += ((pItem->cchName + 1) * sizeof(WCHAR));
			fnProcessor(pItem, pszName);
		}
	}
	else if (m_pHeader0)
	{
#ifdef _DEBUG
		r.SetPtr(m_pHeader0, m_File.GetFile().GetSize32());
#else
		r.SetPtr(m_pHeader0, 0);
#endif
		r += sizeof(LISTFILEHEADER_0);
		BOOKMARKITEM Item;
		const LISTFILEITEM_0* pItem;
		EckCounter(m_pHeader0->iCount, i)
		{
			r.SkipPointer(pItem);

			r += ((wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR));
			r += ((wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR));

			if (eck::IsBitSet(pItem->uFlags, QKLIF_BOOKMARK))
			{
				r >> Item.cr;

				pszName = (PCWSTR)r.m_pMem;
				Item.cchName = (int)wcslen(pszName);
				r += ((Item.cchName + 1) * sizeof(WCHAR));

				Item.idxItem = i;

				fnProcessor(&Item, pszName);

				r += ((wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR));
			}

			if (m_pHeader0->dwVer == QKLFVER_2)
				r += ((wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR));
		}
	}
	else
		EckDbgBreak();
}
