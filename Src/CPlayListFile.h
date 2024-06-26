﻿#pragma once
#include "Utils.h"
#include "eck\CFile.h"

#include <functional>

#include <lmcons.h>

//////////////////////////////////旧版本的播放列表文件//////////////////////////////////
struct LISTFILEHEADER_0	// 播放列表文件头
{
	CHAR cHeader[4];	// 文件起始标记，ASCII字符串"QKPL"
	int iCount;			// 项目数
	DWORD dwVer;		// 存档文件版本，QKLFVER_常量（见下）
	DWORD dwReserved;	// 保留，必须为0
};

constexpr inline int
QKLFVER_1 = 0,
QKLFVER_2 = 1;

struct LISTFILEITEM_0	// 播放列表文件项目头
{
	UINT uFlags;		// 项目标志
	DWORD dwReserved1;	// 保留，必须为0
	DWORD dwReserved2;	// 保留，必须为0
};

enum
{
	QKLIF_INVALID = 0x00000001,	// 项目无效
	QKLIF_IGNORED = 0x00000002,	// 忽略
	QKLIF_BOOKMARK = 0x00000004,// 有书签
	QKLIF_DUMMY1 = 0x00000008,	// 
	QKLIF_TIME = 0x00000010,	// 有时长字符串
	QKLIF_DUMMY2 = 0x00000020,	// 
};

/*
* LISTFILEHEADER_0
* {
*	LISTFILEITEM_0
*	名称\0
*	文件名\0
*	[仅当具有QKLIF_BOOKMARK时] :
*		书签颜色 : COLORREF
*		书签名称\0
*		书签备注\0
* 	[仅版本2] :
*		时间\0
* }
*/

//////////////////////////////////新版本的播放列表文件//////////////////////////////////
constexpr inline int
PNLFVER_0 = 0,
PNBMVER_0 = 0;
struct LISTFILEHEADER_1	// 播放列表文件头
{
	CHAR chHeader[4];	// 文件起始标记，ASCII字符串"PNPL"
	int iVer;			// 存档文件版本，PNLFVER_常量
	UINT ocbBookMark;	// 书签信息偏移
	int cItems;			// 项目数
	int cchCreator;		// 创建者署名长度
	// WCHAR szCreator[];
};

struct PLUPUREDATA// 结构稳定，不能修改
{
	UINT uSecTime{};		// 【文件】时长
	UINT uSecPlayed{};		// 【统计】播放总时间
	UINT cPlayed{};			// 【统计】播放次数
	UINT cLoop{};			// 【统计】循环次数
	ULONGLONG ftLastPlayed{};	// 【统计】上次播放时间
	ULONGLONG ftModified{};	// 【文件】修改时间
	ULONGLONG ftCreated{};	// 【文件】创建时间
	USHORT usYear{};		// 【元数据】年代
	USHORT usBitrate{};		// 【元数据】比特率
	BYTE byRating{};		// 【元数据】分级
	BYTE bIgnore : 1{};		// 项目被忽略
	BYTE bBookmark : 1{};	// 项目含书签
	BYTE bNeedUpdated : 1{};// 信息需要更新
};

struct LISTFILEITEM_1	// 播放列表文件项目头
{
	// 下列长度均不包含结尾NULL
	int cchName;	// 名称长度
	int cchFile;	// 文件名长度
	int cchTitle;	// 标题长度
	int cchArtist;	// 艺术家长度
	int cchAlbum;	// 专辑名长度
	int cchGenre;	// 流派长度

	PLUPUREDATA s;
	// WCHAR szName[];
	// WCHAR szFile[];
	// WCHAR szTitle[];
	// WCHAR szArtist[];
	// WCHAR szAlbum[];
	// WCHAR szGenre[];
};

struct BOOKMARKHEADER	// 书签信息头
{
	int iVer;			// 书签信息版本，PNBMVER_常量
	int cBookmarks;		// 书签数
};

struct BOOKMARKITEM		// 书签信息项目头
{
	int idxItem;		// 书签所属项目的索引，取值范围为0到cItems，将显示在指定索引之前
	COLORREF cr;		// 书签颜色
	int cchName;		// 书签名称长度，不包括结尾的\0
	// WCHAR szName[];
};

/*
* LISTFILEHEADER_1
* {
* 	LISTFILEITEM_1
* }
*/

/*
* BOOKMARKHEADER
* {
* 	BOOKMARKITEM
* }
*/
////////////////////////////////////////////////////////////////////

class CPlayListFileReader
{
private:
	eck::CMappingFile m_File{};
	LISTFILEHEADER_0* m_pHeader0 = NULL;
	LISTFILEHEADER_1* m_pHeader1 = NULL;
	eck::CRefStrW m_rsCreator{};
public:
	using FItemProcessor = std::function<BOOL(const LISTFILEITEM_1* pItem,
		PCWSTR pszName, PCWSTR pszFile, PCWSTR pszTitle,
		PCWSTR pszArtist, PCWSTR pszAlbum, PCWSTR pszGenre)>;
	using FBookmarkProcessor = std::function<BOOL(const BOOKMARKITEM* pItem, PCWSTR pszName)>;
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CPlayListFileReader)
public:
	CPlayListFileReader(PCWSTR pszFile)
	{
		Open(pszFile);
	}

	BOOL Open(PCWSTR pszFile);

	int GetItemCount();

	void For(const FItemProcessor& fnProcessor);

	void ForBookmark(const FBookmarkProcessor& fnProcessor);

	const auto& GetCreatorName() const { return m_rsCreator; }
};


class CPlayListFileWriter
{
private:
	eck::CFile m_File{};
	LISTFILEHEADER_1 m_Header{ {'P','N','P','L'},PNLFVER_0 };
	BOOKMARKHEADER m_BmHeader{ PNBMVER_0 };
	eck::CRefStrW m_rsCreator{};
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CPlayListFileWriter)
public:
	CPlayListFileWriter(PCWSTR pszFile)
	{
		Open(pszFile);
	}

	BOOL Open(PCWSTR pszFile);

	void PushBack(const LISTFILEITEM_1& Item,
		PCWSTR pszName, PCWSTR pszFile, PCWSTR pszTitle,
		PCWSTR pszArtist, PCWSTR pszAlbum, PCWSTR pszGenre);

	void BeginBookMark();

	void PushBackBookmark(const BOOKMARKITEM& Item, PCWSTR pszName);

	BOOL End();

	void SetCreatorName(PCWSTR pszName) { m_rsCreator = pszName; }

	BOOL SetCreatorNameAsCurrUser()
	{ 
		m_rsCreator.ReSize(UNLEN);
		DWORD cch = m_rsCreator.Size() + 1;
		if (!GetUserNameW(m_rsCreator.Data(), &cch))
		{
			m_rsCreator.Clear();
			return FALSE;
		}
		m_rsCreator.ReSize(cch - 1);
		return TRUE;
	}
};