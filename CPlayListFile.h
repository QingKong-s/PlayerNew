#pragma once
#include "Utils.h"
#include "eck\CFile.h"

#include <functional>

//////////////////////////////////旧版本的播放列表文件//////////////////////////////////
#pragma pack(push)
#pragma pack(4)
struct LISTFILEHEADER_0	// 播放列表文件头
{
	CHAR cHeader[4];	// 文件起始标记，ASCII字符串"QKPL"
	int iCount;			// 项目数
	DWORD dwVer;		// 存档文件版本，QKLFVER_常量（见下）
	DWORD dwReserved;	// 保留，必须为0
};

#define QKLFVER_1				0// 这个版本的列表文件还没有记录时间的功能，但是作者已经把自己的列表排序好了，因为不想重排（懒）所以加了这个版本控制（留保留字段果然是明智的选择2333）
#define QKLFVER_2				1

struct LISTFILEITEM_0	// 播放列表文件项目头
{
	UINT uFlags;		// 项目标志
	DWORD dwReserved1;	// 保留，必须为0
	DWORD dwReserved2;	// 保留，必须为0
};

#define QKLIF_INVALID			0x00000001// 项目无效
#define QKLIF_IGNORED			0x00000002// 忽略
#define QKLIF_BOOKMARK			0x00000004// 有书签
#define QKLIF_DRAGMARK_CURRFILE	0x00000008// 仅在拖放时有效，现行播放标志，还原索引用
#define QKLIF_TIME				0x00000010// 仅在存档或读取文件时有效，已有时长字符串
#define QKLIF_DRAGMARK_PLLATER	0x00000020// 仅在拖放时有效，稍后播放标志，还原索引用
#pragma pack(pop)
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
#pragma pack(push, 4)
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
	BITBOOL bDelayPlaying : 1;
	// WCHAR szName[];
	// WCHAR szTime[];
	// WCHAR szFile[];
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
#pragma pack(pop)
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

class CPlayListFile
{
private:
	eck::CMappingFile m_File{};
	LISTFILEHEADER_0* m_pHeader0 = NULL;
	LISTFILEHEADER_1* m_pHeader1 = NULL;
public:
	using FItemProcessor = std::function<BOOL(const LISTFILEITEM_1* pItem, PCWSTR pszName, PCWSTR pszFile, PCWSTR pszTime)>;
	using FBookmarkProcessor = std::function<BOOL(const BOOKMARKITEM* pItem, PCWSTR pszName)>;

	CPlayListFile() = default;
	CPlayListFile(PCWSTR pszFile)
	{
		Open(pszFile);
	}

	CPlayListFile(const CPlayListFile&) = delete;
	CPlayListFile(CPlayListFile&&) = delete;
	CPlayListFile& operator=(const CPlayListFile&) = delete;
	CPlayListFile& operator=(CPlayListFile&&) = delete;
	~CPlayListFile() = default;

	BOOL Open(PCWSTR pszFile);

	int GetItemCount();

	void For(FItemProcessor fnProcessor);

	void ForBookmark(FBookmarkProcessor fnProcessor);
};