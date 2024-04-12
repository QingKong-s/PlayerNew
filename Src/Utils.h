#pragma once
#include "eck\Utility.h"
#include "eck\CFile.h"
#include "eck\CRefStr.h"
#include "eck\CMemWalker.h"
#include "eck\CException.h"
#include "eck\SystemHelper.h"

#include <algorithm>

#include <Shlwapi.h>
#include <wincodec.h>

using eck::PCVOID;
using eck::PCBYTE;
using eck::BITBOOL;
using eck::SafeRelease;

#define PNInline __forceinline

#define UTILS_NAMESPACE_BEGIN namespace Utils {
#define UTILS_NAMESPACE_END }

UTILS_NAMESPACE_BEGIN
#pragma region MusicInfo
enum :UINT
{
    MIF_TITLE = 1u << 0,
    MIF_ARTIST = 1u << 1,
    MIF_ALBUM = 1u << 2,
    MIF_COMMENT = 1u << 3,
    MIF_LRC = 1u << 4,
    MIF_COVER = 1u << 5,
    MIF_GENRE = 1u << 6,

    MIF_ALL = MIF_TITLE | MIF_ARTIST | MIF_ALBUM | MIF_COMMENT | MIF_LRC | MIF_COVER | MIF_GENRE
};

struct MUSICINFO
{
    UINT uFlags{ MIF_ALL };
	eck::CRefStrW rsTitle{};
	eck::CRefStrW rsArtist{};
	eck::CRefStrW rsAlbum{};
	eck::CRefStrW rsComment{};
	eck::CRefStrW rsLrc{};
    eck::CRefBin rbCover{};
    eck::CRefStrW rsGenre{};
};


struct ID3v2_Header		// ID3v2标签头
{
    CHAR Header[3];		// "ID3"
    BYTE Ver;			// 版本号
    BYTE Revision;		// 副版本号
    BYTE Flags;			// 标志
    BYTE Size[4];		// 标签大小，28位数据，每个字节最高位不使用，包括标签头的10个字节和所有的标签帧
};

struct ID3v2_ExtHeader  // ID3v2扩展头
{
    BYTE ExtHeaderSize[4];  // 扩展头大小
    BYTE Flags[2];          // 标志
    BYTE PaddingSize[4];    // 空白大小
};

struct ID3v2_FrameHeader// ID3v2帧头
{
    CHAR ID[4];			// 帧标识
    BYTE Size[4];		// 帧内容的大小，32位数据，不包括帧头
    BYTE Flags[2];		// 存放标志
};

struct FLAC_Header      // Flac头
{
    BYTE by;
    BYTE bySize[3];
};

static_assert(alignof(ID3v2_Header) == 1);
static_assert(alignof(ID3v2_ExtHeader) == 1);
static_assert(alignof(ID3v2_FrameHeader) == 1);
static_assert(alignof(FLAC_Header) == 1);


// ID3v2头标志
enum :UINT
{
    ID3V2HF_UNSYNCHRONIZATION = 1u << 7,            // 不同步
    ID3V2HF_EXTENDED_HEADER = 1u << 6,              // 含扩展头
    ID3V2HF_EXPERIMENTAL = 1u << 5,                 // 实验性标签
    // ----Only ID3v2.4----
    ID3V2HF_FOOTER = 1u << 4,                       // 含页脚
};

// ID3v2扩展头标志
enum :UINT
{
    // ----Only ID3v2.3----
    ID3V23EH_CRC_DATA_PRESENT = 1u << 7,            // 含CRC数据
    // ----Only ID3v2.4----
    ID3V24EH_UPDATE = 1u << 6,                      // 更新标志
	ID3V24EH_CRC_DATA_PRESENT = 1u << 5,            // 含CRC数据
	ID3V24EH_RESTRICTIONS = 1u << 4,                // 限制标签尺寸
};

// ID3v2帧标志
enum :UINT
{
    // ----状态----
	ID3V2FF_TAG_ALTER_PRESERVATION = 1u << 15,      // 标签修改后应丢弃
	ID3V2FF_FILE_ALTER_PRESERVATION = 1u << 14,     // 文件修改后应丢弃
	ID3V2FF_READ_ONLY = 1u << 13,                   // 只读
    // ----格式----
	ID3V2FF_HAS_GROUP_IDENTITY = 1u << 6,           // 含组标志（1B）
	ID3V2FF_COMPRESSION = 1u << 3,                  // 已压缩（zlib）
	ID3V2FF_ENCRYPTION = 1u << 2,                   // 已加密（1B，指示加密方式）
	ID3V2FF_UNSYNCHRONIZATION = 1u << 1,            // 不同步
	ID3V2FF_HAS_DATA_LENGTH_INDICATOR = 1u << 0,    // 含长度指示（4B，同步安全整数）
};

BOOL GetMusicInfo(PCWSTR pszFile, MUSICINFO& mi);
#pragma endregion

#pragma region Lrc
enum class LrcEncoding
{
    Auto,
    GB2312,
    UTF8,
    UTF16LE,
    UTF16BE
};
struct LRCINFO
{
	PWSTR pszLrc = NULL;// 歌词
    PWSTR pszTranslation = NULL;// 翻译，指向pszLrc的中间，可能为NULL
    int cchTotal = 0;
	int cchLrc = 0;
	float fTime = 0.f;
	float fDuration = 0.f;

	LRCINFO() = default;
	LRCINFO(PWSTR pszLrc_, PWSTR pszTranslation_, int cchTotal_, int cchLrc_, float fTime_, float fDuration_)
		:pszLrc(pszLrc_), pszTranslation(pszTranslation_), cchTotal(cchTotal_), cchLrc(cchLrc_)
		, fTime(fTime_), fDuration(fDuration_) {}

	LRCINFO(const LRCINFO& li)
	{
		memcpy(this, &li, sizeof(LRCINFO));
		pszLrc = (PWSTR)malloc(eck::Cch2Cb(cchTotal));
        EckAssert(pszLrc);
        pszTranslation = pszLrc + cchLrc;
        wcscpy(pszLrc, li.pszLrc);
	}

    LRCINFO(LRCINFO&& li) noexcept
    {
        memcpy(this, &li, sizeof(LRCINFO));
        ZeroMemory(&li, sizeof(LRCINFO));
    }

    LRCINFO& operator=(const LRCINFO& li)
    {
        free(pszLrc);
		memcpy(this, &li, sizeof(LRCINFO));
        pszLrc = (PWSTR)malloc(eck::Cch2Cb(cchTotal));
        EckAssert(pszLrc);
		pszTranslation = pszLrc + cchLrc;
		wcscpy(pszLrc, li.pszLrc);
		return *this;
	}

    LRCINFO& operator=(LRCINFO&& li) noexcept
    {
        free(pszLrc);
		memcpy(this, &li, sizeof(LRCINFO));
        ZeroMemory(&li, sizeof(LRCINFO));
		return *this;
	}

    ~LRCINFO()
    {
        free(pszLrc);
	}
};

struct LRCLABEL
{
    eck::CRefStrW rsKey{};
    eck::CRefStrW rsValue{};
};

/// <summary>
/// 解析LRC。
/// 支持压缩LRC，支持无换行LRC，支持歌词内嵌入中括号
/// </summary>
/// <param name="p">文件名或LRC字节流</param>
/// <param name="cbMem">若为0，则指示p为文件名，否则p为LRC字节流</param>
/// <param name="Result">解析结果</param>
/// <param name="Label">其他标签的解析结果</param>
/// <param name="iDefTextEncoding">默认文本编码</param>
/// <returns></returns>
BOOL ParseLrc(PCVOID p, SIZE_T cbMem, std::vector<LRCINFO>& Result,
    std::vector<LRCLABEL>& Label, LrcEncoding uTextEncoding = LrcEncoding::Auto, float fTotalTime = 0.f);
#pragma endregion

enum
{
    MBBID_1 = 10001,
    MBBID_2,
    MBBID_3
};

UINT MsgBox(
    PCWSTR pszMainInstruction,
    PCWSTR pszContent = NULL,
    PCWSTR pszWndTitle = NULL,
    UINT cButtons = 1,
    HICON hIcon = NULL,
    HWND hParent = NULL,
    UINT iDefButton = 1,
    BOOL bCenterPos = FALSE,
    PCWSTR pszCheckBoxTitle = NULL,
    PCWSTR pszButton1Title = NULL,
    PCWSTR pszButton2Title = NULL,
    PCWSTR pszButton3Title = NULL,
    BOOL* pbCheck = NULL);
UTILS_NAMESPACE_END