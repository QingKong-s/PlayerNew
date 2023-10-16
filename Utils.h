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
struct ID3v2_Header		// ID3v2��ǩͷ
{
    CHAR Header[3];		// "ID3"
    BYTE Ver;			// �汾��
    BYTE Revision;		// ���汾��
    BYTE Flags;			// ��־
    BYTE Size[4];		// ��ǩ��С��28λ���ݣ�ÿ���ֽ����λ��ʹ�ã�������ǩͷ��10���ֽں����еı�ǩ֡
};

struct ID3v2_ExtHeader  // ID3v2��չͷ
{
    BYTE ExtHeaderSize[4];  // ��չͷ��С
    BYTE Flags[2];          // ��־
    BYTE PaddingSize[4];    // �հ״�С
};

struct ID3v2_FrameHeader// ID3v2֡ͷ
{
    CHAR ID[4];			// ֡��ʶ
    BYTE Size[4];		// ֡���ݵĴ�С��32λ���ݣ�������֡ͷ
    BYTE Flags[2];		// ��ű�־
};

struct FLAC_Header      // Flacͷ
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
	PWSTR pszLrc = NULL;// ���
    PWSTR pszTranslation = NULL;// ���룬ָ��pszLrc���м䣬����ΪNULL
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