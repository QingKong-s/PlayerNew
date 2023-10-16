#pragma once
#include "eck\Utility.h"
#include "eck\CFile.h"

#include <algorithm>

#include <Shlwapi.h>
#include <wincodec.h>

using eck::PCVOID;
using eck::BITBOOL;

#define PNInline __forceinline

#define UTILS_NAMESPACE_BEGIN namespace Utils {
#define UTILS_NAMESPACE_END }

UTILS_NAMESPACE_BEGIN
#pragma region MusicInfo
struct MUSICINFO
{
	eck::CRefStrW rsTitle{};
	eck::CRefStrW rsArtist{};
	eck::CRefStrW rsAlbum{};
	eck::CRefStrW rsComment{};
	eck::CRefStrW rsLrc{};
	IStream* pCoverData = NULL;
};

#pragma pack (push)
#pragma pack (1)
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
#pragma pack (pop)

BOOL GetMusicInfo(PCWSTR pszFile, MUSICINFO& mi);
#pragma endregion

#pragma region Lrc
struct LRCINFO
{
	PWSTR pszLrc = NULL;// 歌词
    PWSTR pszTranslation = NULL;// 翻译，指向pszLrc的中间，可能为NULL
    float fTime = 0.f;
    int cchTotal = 0;
    int cchLrc = 0;

	LRCINFO(PWSTR pszLrc, PWSTR pszTranslation, float fTime, int cchTotal, int cchLrc)
		:pszLrc(pszLrc), pszTranslation(pszTranslation), fTime(fTime), cchTotal(cchTotal), cchLrc(cchLrc)
	{
	}

    LRCINFO(const LRCINFO& li)
    {
        memcpy(this, &li, sizeof(LRCINFO));
        pszLrc = (PWSTR)malloc(eck::Cch2Cb(cchTotal));
        pszTranslation = pszLrc + cchLrc;
        wcscpy(pszLrc, li.pszLrc);
	}

    LRCINFO(LRCINFO&& li) noexcept
    {
        memcpy(this, &li, sizeof(LRCINFO));
		li.pszLrc = NULL;
		li.pszTranslation = NULL;
    }

    LRCINFO& operator=(const LRCINFO& li)
    {
        free(pszLrc);
		memcpy(this, &li, sizeof(LRCINFO));
        pszLrc = (PWSTR)malloc(eck::Cch2Cb(cchTotal));
		pszTranslation = pszLrc + cchLrc;
		wcscpy(pszLrc, li.pszLrc);
		return *this;
	}

    LRCINFO& operator=(LRCINFO&& li) noexcept
    {
        free(pszLrc);
		memcpy(this, &li, sizeof(LRCINFO));
		li.pszLrc = NULL;
		li.pszTranslation = NULL;
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

    LRCLABEL(PCWSTR pszKey, PCWSTR pszValue)
        :rsKey(pszKey), rsValue(pszValue)
    {
	}
};

BOOL ParseLrc(PCVOID p, SIZE_T cbMem, std::vector<LRCINFO>& Result, std::vector<LRCLABEL>& Label, int iDefTextEncoding = 0);
#pragma endregion
UTILS_NAMESPACE_END