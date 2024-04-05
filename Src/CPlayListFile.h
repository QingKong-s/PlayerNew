#pragma once
#include "Utils.h"
#include "eck\CFile.h"

#include <functional>

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
*	opt : // 仅当具有QKLIF_BOOKMARK时
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
};

struct LISTFILEITEM_1	// 播放列表文件项目头
{
	int cchName;		// 名称长度，不包括结尾的\0
	int cchTime;		// 时间长度，不包括结尾的\0
	int cchFile;		// 文件名长度，不包括结尾的\0
	BITBOOL bIgnore : 1;
	BITBOOL bBookmark : 1;
	// WCHAR szName[];
	// WCHAR szFile[];
	// WCHAR szTime[];
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
public:
	using FItemProcessor = std::function<BOOL(const LISTFILEITEM_1* pItem, PCWSTR pszName, PCWSTR pszFile, PCWSTR pszTime)>;
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
};


class CPlayListFileWriter
{
private:
	eck::CFile m_File{};

	LISTFILEHEADER_1 m_Header{ {'P','N','P','L'},PNLFVER_0 };
	BOOKMARKHEADER m_BmHeader{ PNBMVER_0 };
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CPlayListFileWriter)
public:
	CPlayListFileWriter(PCWSTR pszFile)
	{
		Open(pszFile);
	}

	BOOL Open(PCWSTR pszFile);

	void PushBack(const LISTFILEITEM_1& Item, PCWSTR pszName, PCWSTR pszFile, PCWSTR pszTime);

	void BeginBookMark();

	void PushBackBookmark(const BOOKMARKITEM& Item, PCWSTR pszName);

	BOOL End();
};