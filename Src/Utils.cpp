#include "Utils.h"

UTILS_NAMESPACE_BEGIN
/// <summary>
/// 同步安全整数到32位小端整数
/// </summary>
/// <param name="p">输入字节流</param>
/// <returns>转换结果</returns>
PNInline DWORD SynchSafeIntToDWORD(PCBYTE p)
{
	return ((p[0] & 0x7F) << 21) | ((p[1] & 0x7F) << 14) | ((p[2] & 0x7F) << 7) | (p[3] & 0x7F);
}

BOOL IsTextUTF8(char* str, ULONGLONG length)
{
	int i;
	DWORD nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
	UCHAR chr;
	BOOL bAllAscii = TRUE; //如果全部都是ASCII, 说明不是UTF-8
	for (i = 0; i < length; i++)
	{
		chr = *(str + i);
		if ((chr & 0x80) != 0) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
			bAllAscii = FALSE;
		if (nBytes == 0) //如果不是ASCII码,应该是多字节符,计算字节数
		{
			if (chr >= 0x80)
			{
				if (chr >= 0xFC && chr <= 0xFD)
					nBytes = 6;
				else if (chr >= 0xF8)
					nBytes = 5;
				else if (chr >= 0xF0)
					nBytes = 4;
				else if (chr >= 0xE0)
					nBytes = 3;
				else if (chr >= 0xC0)
					nBytes = 2;
				else
				{
					return FALSE;
				}
				nBytes--;
			}
		}
		else //多字节符的非首字节,应为 10xxxxxx
		{
			if ((chr & 0xC0) != 0x80)
			{
				return FALSE;
			}
			nBytes--;
		}
	}
	if (nBytes > 0) //违返规则
	{
		return FALSE;
	}
	if (bAllAscii) //如果全部都是ASCII, 说明不是UTF-8
	{
		return FALSE;
	}
	return TRUE;
}

/// <summary>
/// [解析ID3v2辅助函数]按指定编码处理文本
/// </summary>
/// <param name="pStream">字节流指针；未指定iTextEncoding时指向整个文本帧，指定iTextEncoding时指向字符串</param>
/// <param name="iLength">长度；未指定iTextEncoding时表示整个文本帧长度（包括1B的编码标记，不含结尾NULL），指定iTextEncoding时表示字符串长度（不含结尾NULL）</param>
/// <param name="iTextEncoding">自定义文本编码；-1（缺省）指示处理的是文本帧</param>
/// <returns>返回处理完毕的文本</returns>
eck::CRefStrW GetMP3ID3v2_ProcString(PCBYTE pStream, int cb, int iTextEncoding = -1)
{
	int iType = 0, cchBuf;
	if (iTextEncoding == -1)
	{
		memcpy(&iType, pStream, 1);
		++pStream;// 跳过文本编码标志
		--cb;
	}
	else
		iType = iTextEncoding;

	eck::CRefStrW rsResult{};

	switch (iType)
	{
	case 0:// ISO-8859-1，即Latin-1（拉丁语-1）
		cchBuf = MultiByteToWideChar(CP_ACP, 0, (PCCH)pStream, cb, NULL, 0);
		if (cchBuf == 0)
			return {};
		rsResult.ReSize(cchBuf);
		MultiByteToWideChar(CP_ACP, 0, (PCCH)pStream, cb, rsResult.Data(), cchBuf);
		break;
	case 1:// UTF-16LE
		if (*(PWSTR)pStream == L'\xFEFF')// 跳BOM（要不是算出来哈希值不一样我可能还真发现不了这个BOM的问题.....）
		{
			pStream += sizeof(WCHAR);
			cb -= sizeof(WCHAR);
		}
		cchBuf = cb / sizeof(WCHAR);
		rsResult.ReSize(cchBuf);
		wcsncpy(rsResult.Data(), (PWSTR)pStream, cchBuf);
		break;
	case 2:// UTF-16BE
		if (*(PWSTR)pStream == L'\xFFFE')// 跳BOM
		{
			pStream += sizeof(WCHAR);
			cb -= sizeof(WCHAR);
		}
		cchBuf = cb / sizeof(WCHAR);
		rsResult.ReSize(cchBuf);
		LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
			(PCWSTR)pStream, cchBuf, rsResult.Data(), cchBuf, NULL, NULL, 0);// 反转字节序
		break;
	case 3:// UTF-8
		cchBuf = MultiByteToWideChar(CP_UTF8, 0, (PCCH)pStream, cb, NULL, 0);
		if (cchBuf == 0)
			return {};
		rsResult.ReSize(cchBuf);
		MultiByteToWideChar(CP_UTF8, 0, (PCCH)pStream, cb, rsResult.Data(), cchBuf);
		break;
	default:
		EckDbgBreak();
		break;
	}

	return rsResult;
}

BOOL GetMusicInfo(PCWSTR pszFile, MUSICINFO& mi)
{
	eck::CFile File;
	if (File.Open(pszFile, eck::FCD_ONLYEXISTING, GENERIC_READ, FILE_SHARE_READ) == INVALID_HANDLE_VALUE)
	{
		EckDbgPrintFormatMessage(GetLastError());
		return FALSE;
	}
	DWORD cbFile = File.GetSize32();

	BYTE by[4];
	File >> by;// 读文件头
	if (memcmp(by, "ID3", 3) == 0)// ID3v2
	{
		if (cbFile < sizeof(ID3v2_Header))
			return FALSE;

		eck::CMappingFile2 mf(File);
		eck::CMemReader r(mf.Create(), cbFile);

		ID3v2_Header* pHeader;
		r.SkipPointer(pHeader);
		DWORD cbTotal = SynchSafeIntToDWORD(pHeader->Size);// 28位数据，包括标签头和扩展头
		if (cbTotal > cbFile)
			return FALSE;

		PCVOID pEnd = r.Data() + cbTotal;

		auto pExtHeader = (const ID3v2_ExtHeader*)r.Data();

		if (pHeader->Ver == 3)// 2.3
		{
			if (pHeader->Flags & 0x20)// 有扩展头
				r += (4 + eck::ReverseInteger(*(DWORD*)pExtHeader->ExtHeaderSize));
		}
		else if (pHeader->Ver == 4)// 2.4
		{
			if (pHeader->Flags & 0x20)// 有扩展头
				r += SynchSafeIntToDWORD(pExtHeader->ExtHeaderSize);
			// 2.4里变成了同步安全整数，而且这个尺寸包含了记录尺寸的四个字节
		}

		DWORD cbUnit;
		ID3v2_FrameHeader* pFrame;
		while (r < pEnd)
		{
			r.SkipPointer(pFrame);

			if (pHeader->Ver == 3)
				cbUnit = eck::ReverseInteger(*(DWORD*)pFrame->Size);// 2.3：32位数据，不包括帧头（偏4字节）
			else if (pHeader->Ver == 4)
				cbUnit = SynchSafeIntToDWORD(pFrame->Size);// 2.4：28位数据（同步安全整数）

			if (memcmp(pFrame->ID, "TIT2", 4) == 0)// 标题
			{
				mi.rsTitle = GetMP3ID3v2_ProcString(r, cbUnit);
				r += cbUnit;
			}
			else if (memcmp(pFrame->ID, "TPE1", 4) == 0)// 作者
			{
				mi.rsArtist = GetMP3ID3v2_ProcString(r, cbUnit);
				r += cbUnit;
			}
			else if (memcmp(pFrame->ID, "TALB", 4) == 0)// 专辑
			{
				mi.rsAlbum = GetMP3ID3v2_ProcString(r, cbUnit);
				r += cbUnit;
			}
			else if (memcmp(pFrame->ID, "USLT", 4) == 0)// 不同步歌词
			{
				/*
				<帧头>（帧标识为USLT）
				文本编码						$xx
				自然语言代码					$xx xx xx
				内容描述						<字符串> $00 (00)
				歌词							<字符串>
				*/
				DWORD cb = cbUnit;

				BYTE byEncodeingType;
				r >> byEncodeingType;// 读文本编码

				CHAR byLangCode[3];
				r >> byLangCode;// 读自然语言代码

				int t;
				if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1或UTF-8
					t = (int)strlen((PCSTR)r.m_pMem) + 1;
				else// UTF-16LE或UTF-16BE
					t = ((int)wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR);
				r += t;// 跳过内容描述

				cb -= (t + 4);

				mi.rsLrc = GetMP3ID3v2_ProcString(r, cb, byEncodeingType);
				r += cb;
			}
			else if (memcmp(pFrame->ID, "COMM", 4) == 0)// 备注
			{
				/*
				<帧头>（帧标识为COMM）
				文本编码						$xx
				自然语言代码					$xx xx xx
				备注摘要						<字符串> $00 (00)
				备注							<字符串>
				*/
				DWORD cb = cbUnit;

				BYTE byEncodeingType;
				r >> byEncodeingType;// 读文本编码

				CHAR byLangCode[3];
				r >> byLangCode;// 读自然语言代码

				int t;
				if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1或UTF-8
					t = (int)strlen((PCSTR)pFrame) + 1;
				else// UTF-16LE或UTF-16BE
					t = ((int)wcslen((PCWSTR)pFrame) + 1) * sizeof(WCHAR);
				r += t;// 跳过备注摘要

				cb -= (t + 4);
				// 此时pFrame指向备注字符串
				mi.rsComment = GetMP3ID3v2_ProcString(r, cb, byEncodeingType);
				r += cb;
			}
			else if (memcmp(pFrame->ID, "APIC", 4) == 0)// 图片
			{
				/*
				<帧头>（帧标识为APIC）
				文本编码                        $xx
				MIME 类型                       <ASCII字符串>$00（如'image/bmp'）
				图片类型                        $xx
				描述                            <字符串>$00(00)
				<图片数据>
				*/
				DWORD cb = cbUnit;

				BYTE byEncodeingType;
				r >> byEncodeingType;// 读文本编码

				int t;
				t = (int)strlen((PCSTR)r.m_pMem);
				r += (t + 2);// 跳过MIME类型字符串和图片类型

				cb -= (t + 3);

				if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1或UTF-8
					t = (int)strlen((PCSTR)r.m_pMem) + 1;
				else// UTF-16LE或UTF-16BE
					t = ((int)wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR);

				r += t;
				cb -= t;// 跳过描述字符串和结尾NULL

				mi.pCoverData = SHCreateMemStream(r, cb);// 创建流对象
				r += cb;
			}
			else
				r += cbUnit;
		}
	}
	else if (memcmp(by, "fLaC", 4) == 0)// Flac
	{
		FLAC_Header Header;
		DWORD cbBlock;
		UINT t;
		char* pBuffer;
		do
		{
			File >> Header;
			cbBlock = Header.bySize[2] | Header.bySize[1] << 8 | Header.bySize[0] << 16;
			switch (Header.by & 0x7F)
			{
			case 4:// 标签信息，注意：这一部分是小端序
			{
				File >> t;// 编码器信息大小
				File += t;// 跳过编码器信息

				UINT uCount;
				File >> uCount;// 标签数量

				for (UINT i = 0; i < uCount; ++i)
				{
					File >> t;// 标签大小

					pBuffer = new char[t + 1];
					File.Read(pBuffer, t);// 读标签
					*(pBuffer + t) = '\0';

					t = MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, NULL, 0);
					PWSTR pszLabel = new WCHAR[t];
					MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, pszLabel, t);// 转换编码，UTF-8到UTF-16LE
					delete[] pBuffer;

					int iPos = eck::FindStr(pszLabel, L"=");// 找等号
					if (iPos != eck::INVALID_STR_POS)
					{
						int cch = t - iPos;
						if (eck::FindStr(pszLabel, L"TITLE"))
						{
							mi.rsTitle.ReSize(cch);
							wcscpy(mi.rsTitle.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"ALBUM"))
						{
							mi.rsAlbum.ReSize(cch);
							wcscpy(mi.rsAlbum.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"ARTIST"))
						{
							mi.rsArtist.ReSize(cch);
							wcscpy(mi.rsArtist.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"DESCRIPTION"))
						{
							mi.rsComment.ReSize(cch);
							wcscpy(mi.rsComment.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"LYRICS"))
						{
							mi.rsLrc.ReSize(cch);
							wcscpy(mi.rsLrc.Data(), pszLabel + iPos);
						}
					}

					delete[] pszLabel;
				}
			}
			break;
			case 6:// 图片（大端序）
			{
				File += 4;// 跳过图片类型

				File >> t;// MIME类型字符串长度
				t = eck::ReverseInteger(t);// 大端序字节到整数，下同
				File += t;// 跳过MIME类型字符串

				File >> t;// 描述字符串长度
				t = eck::ReverseInteger(t);
				File += (t + 16);// 跳过描述字符串、宽度、高度、色深、索引图颜色数

				File >> t;// 图片数据长度
				t = eck::ReverseInteger(t);// 图片数据长度

				pBuffer = new char[t];
				File.Read(pBuffer, t);
				mi.pCoverData = SHCreateMemStream((const BYTE*)pBuffer, t);// 创建流对象
				delete[] pBuffer;
			}
			break;
			default:
				File += cbBlock;// 跳过块
			}

		} while (!(Header.by & 0x80));// 检查最高位，判断是不是最后一个块
	}
	return TRUE;
}


struct LRCTIMELABEL
{
	PCWSTR pszLabel;
	int pos1;
	int pos2;

	LRCTIMELABEL(PCWSTR pszLabel, int pos1, int pos2) : pszLabel(pszLabel), pos1(pos1), pos2(pos2) {}
};

void ParseLrc_ProcTimeLabel(std::vector<LRCINFO>& Result, std::vector<LRCLABEL>& Label,
	const std::vector<LRCTIMELABEL>& TimeLabel, PWSTR pszLrc, int cchLrc)
{
#pragma warning (push)
#pragma warning (disable: 6387)// 可能是NULL
#pragma warning (disable: 6053)// 可能未添加终止NULL
	PWSTR pTemp;
	int M, S, MS;
	float fTime;
	EckCounter(TimeLabel.size(), i)
	{
		auto& Label = TimeLabel[i];

		// 取分钟
		StrToIntExW(Label.pszLabel, STIF_DEFAULT, &M);
		fTime = (float)M * 60.f;
		// 取秒
		StrToIntExW(Label.pszLabel + Label.pos1 + 1, STIF_DEFAULT, &S);
		fTime += (float)S;
		// 取毫秒
		if (Label.pos2 > 0)
		{
			StrToIntExW(Label.pszLabel + Label.pos2 + 1, STIF_DEFAULT, &MS);
			fTime += ((float)MS / 1000.f);
		}

		if (fTime < 0)
			continue;

		if (cchLrc)
		{
			pTemp = (PWSTR)malloc(eck::Cch2Cb(cchLrc));
			Result.emplace_back(pTemp, nullptr, cchLrc, cchLrc, fTime, 0.f);
			wcsncpy(pTemp, pszLrc, cchLrc);
			*(pTemp + cchLrc) = L'\0';
		}
		else
			Result.emplace_back(nullptr, nullptr, 0, 0, fTime, 0.f);
	}
#pragma warning (pop)
}

BOOL ParseLrc_IsTimeLabelLegal(PCWSTR pszLabel, int cchLabel, int* pposFirstDiv, int* pposSecondDiv, BOOL* pbMS)
{
	*pposFirstDiv = -1;
	*pposSecondDiv = -1;
	int pos1, pos2;
	pos1 = eck::FindStr(pszLabel, L":");
	if (pos1 <= 0 || pos1 >= cchLabel - 1)
		return FALSE;// 没冒号、冒号在开头、冒号在结尾、冒号超过结尾都不合法

	pos2 = eck::FindStr(pszLabel, L":", pos1 + 1);
	if (pos2 == pos1 + 1)
		return FALSE;// 两个冒号挨着也不合法

	*pbMS = TRUE;
	if (pos2 < 0 || pos2 >= cchLabel)
	{
		pos2 = eck::FindStr(pszLabel, L".", pos1 + 1);
		if (pos2 < 0 || pos2 >= cchLabel)
		{
			*pbMS = FALSE;// [分:秒]
			pos2 = cchLabel;
		}
		// else [分:秒.毫秒]
	}
	// else [分:秒:毫秒]

	*pposFirstDiv = pos1;
	if (*pbMS)
		*pposSecondDiv = pos2;

	// 测试第一个字段
	if (pos1 == 0)
		return FALSE;// 没有第一个字段

	for (int i = 0; i < pos1; ++i)
		if (!iswdigit(pszLabel[i]))
			return FALSE;// 第一个字段不是数字
	// 测试第二个字段
	if (pos2 <= pos1 + 1)
		return FALSE;// 没有第二个字段

	for (int i = pos1 + 1; i < pos2; ++i)
		if (!iswdigit(pszLabel[i]))
			return FALSE;// 第二个字段不是数字
	// 测试第三个字段
	if (*pbMS)
	{
		for (int i = pos2 + 1; i < cchLabel; ++i)
			if (!iswdigit(pszLabel[i]))
				return FALSE;// 第三个字段不是数字
	}
	return TRUE;
}

BOOL ParseLrc(PCVOID p, SIZE_T cbMem, std::vector<LRCINFO>& Result, std::vector<LRCLABEL>& Label, 
	LrcEncoding uTextEncoding, float fTotalTime)
{
	Result.clear();
	Label.clear();
#pragma region 读入数据
	if (!p)
		return FALSE;
	BYTE* pFileData;
	if (cbMem)
	{
		if (cbMem < 5)
			return FALSE;
		pFileData = (BYTE*)VirtualAlloc(NULL, cbMem + 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		memcpy(pFileData, p, cbMem);

		*(pFileData + cbMem) = '\0';
		*(pFileData + cbMem + 1) = '\0';
	}
	else
	{
		PWSTR pszTemp = new WCHAR[wcslen((PCWSTR)p) + 10];
		wcscpy(pszTemp, (PCWSTR)p);
#pragma warning(suppress:6386)
		PathRenameExtensionW(pszTemp, L".lrc");
		eck::CFile File;
		File.Open((PCWSTR)pszTemp);
		delete[] pszTemp;
		if (File.GetHandle() == INVALID_HANDLE_VALUE)
			return FALSE;
		cbMem = File.GetSize32();
		if (cbMem < 5)
			return FALSE;

		pFileData = (BYTE*)VirtualAlloc(NULL, cbMem + 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		File.Read(pFileData, (DWORD)cbMem);
		File.Close();
		*(pFileData + cbMem) = '\0';
		*(pFileData + cbMem + 1) = '\0';
	}
#pragma endregion
#pragma region 判断并转换编码
	PWSTR pszOrg = (PWSTR)pFileData;
	int cchFile = (int)(cbMem / sizeof(WCHAR));

	constexpr BYTE
		c_chBomU16LE[]{ 0xFF, 0xFE },
		c_chBomU16BE[]{ 0xFE, 0xFF },
		c_chBomU8[]{ 0xEF, 0xBB, 0xBF };
	if (memcmp(pFileData, c_chBomU16LE, 2) == 0)
	{
		--cchFile;
		pszOrg = (PWSTR)(pFileData + 2);
	}
	else if (memcmp(pFileData, c_chBomU16BE, 2) == 0)
	{
		--cchFile;
		pszOrg = (PWSTR)VirtualAlloc(NULL, eck::Cch2Cb(cchFile), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
			(PCWSTR)pFileData + 1, cchFile, pszOrg, cchFile, NULL, NULL, 0);// 反转字节序
		VirtualFree(pFileData, 0, MEM_RELEASE);
		pFileData = (BYTE*)pszOrg;
	}
	else if (memcmp(pFileData, c_chBomU8, 3) == 0)
	{
		int cchBuf = MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData + 3, -1, NULL, 0);
		pszOrg = (PWSTR)VirtualAlloc(NULL, eck::Cch2Cb(cchBuf), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData + 3, -1, pszOrg, cchBuf);// 转换编码
		VirtualFree(pFileData, 0, MEM_RELEASE);
		pFileData = (BYTE*)pszOrg;
		cchFile = cchBuf - 1;
	}
	else// 无BOM
	{
		switch (uTextEncoding)
		{
		case LrcEncoding::Auto:
		{
			int i = IS_TEXT_UNICODE_REVERSE_MASK | IS_TEXT_UNICODE_NULL_BYTES;
			if (IsTextUnicode(pFileData, (int)cbMem, &i))//  先测UTF-16BE，不然会出问题
				goto GetLrc_UTF16BE;
			else
			{
				i = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_NULL_BYTES;
				if (IsTextUnicode(pFileData, (int)cbMem, &i))
					goto GetLrc_UTF16LE;
				else if (IsTextUTF8((char*)pFileData, cbMem))
					goto GetLrc_UTF8;
				else
					goto GetLrc_GB2312;
			}
		}
		break;
		case LrcEncoding::GB2312:
		{
		GetLrc_GB2312:
			int cchBuf = MultiByteToWideChar(936, 0, (CHAR*)pFileData, -1, NULL, 0);
			pszOrg = (PWSTR)VirtualAlloc(NULL, eck::Cch2Cb(cchBuf), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			MultiByteToWideChar(936, 0, (CHAR*)pFileData, -1, pszOrg, cchBuf);// 转换编码
			VirtualFree(pFileData, 0, MEM_RELEASE);
			pFileData = (BYTE*)pszOrg;
			cchFile = cchBuf - 1;
		}
		break;
		case LrcEncoding::UTF8:
		{
		GetLrc_UTF8:
			int cchBuf = MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData, -1, NULL, 0);
			pszOrg = (PWSTR)VirtualAlloc(NULL, eck::Cch2Cb(cchBuf), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData, -1, pszOrg, cchBuf);// 转换编码
			VirtualFree(pFileData, 0, MEM_RELEASE);
			pFileData = (BYTE*)pszOrg;
			cchFile = cchBuf - 1;
		}
		break;
		case LrcEncoding::UTF16LE:
		GetLrc_UTF16LE:;
			break;
		case LrcEncoding::UTF16BE:
		{
		GetLrc_UTF16BE:
			pszOrg = (PWSTR)VirtualAlloc(NULL, (cchFile + 1) * sizeof(WCHAR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
				(PCWSTR)pFileData, cchFile, pszOrg, cchFile, NULL, NULL, 0);// 反转字节序
			VirtualFree(pFileData, 0, MEM_RELEASE);
			pFileData = (BYTE*)pszOrg;
		}
		break;
		}
	}
#pragma endregion
#pragma region 按行分割
	std::vector<std::pair<PWSTR, int>> Lines{};
	Lines.reserve(50);

	constexpr WCHAR
		c_szDiv1[]{ L"\r\n" },// CRLF
		c_szDiv2[]{ L"\n" },// LF
		c_szDiv3[]{ L"\r" };// CR

	BOOL b1, b2, b3;

	int i1 = eck::FindStr(pszOrg, c_szDiv1),
		i2 = eck::FindStr(pszOrg, c_szDiv2),
		i3 = eck::FindStr(pszOrg, c_szDiv3);

	b1 = i1 >= 0;
	b2 = i2 >= 0;
	b3 = i3 >= 0;

	PWSTR pszLine;// 每行内容
	int cchLine;// 每行内容长度

	int pos1;
	int pos2 = 0;
	int cchDiv;

	if (!b1 && !b2 && !b3)// 无换行符
		Lines.emplace_back(pszOrg, cchFile);
	else
	{
		// 适配那些混用三种换行符的傻逼文件
		if (b1)// CRLF
		{
			// 思路：iStrPos1 = min(i1, i2, i3)
			pos1 = i1;
			cchDiv = 2;
			if (b2 && i1 >= i2)// LF
			{
				pos1 = i2;
				cchDiv = 1;
			}
			if (b3 && i3 < pos1)// CR
			{
				pos1 = i3;
				cchDiv = 1;
			}
		}
		else
		{
			// 思路：iStrPos1 = min(i2, i3)
			cchDiv = 1;
			if (b2 && b3)// 没有CRLF，但CR和LF同时存在
			{
				if (i2 < i3)
					pos1 = i2;
				else
					pos1 = i3;
			}
			else if (b2)// LF
				pos1 = i2;
			else// CR
				pos1 = i3;
		}

		while (pos1)
		{
			cchLine = pos1 - pos2;
			if (cchLine > 0)
			{
				pszLine = pszOrg + pos2;
				*(pszLine + cchLine) = L'\0';
				Lines.emplace_back(pszLine, cchLine);
			}
			pos2 = pos1 + cchDiv;// 跳过换行符
			/////////////取下一换行符位置
			if (b1)
				i1 = eck::FindStr(pszOrg, c_szDiv1, pos2);
			if (b2)
				i2 = eck::FindStr(pszOrg, c_szDiv2, pos2);
			if (b3)
				i3 = eck::FindStr(pszOrg, c_szDiv3, pos2);

			pos1 = 0;
			if (i1 >= 0)// CRLF
			{
				pos1 = i1;
				cchDiv = 2;
				if (i2 >= 0 && i1 >= i2)// LF
				{
					pos1 = i2;
					cchDiv = 1;
				}
				if (i3 >= 0 && i3 < pos1)// CR
				{
					pos1 = i3;
					cchDiv = 1;
				}
			}
			else
			{
				if (i2 >= 0 && i3 >= 0)// CR  LF
				{
					cchDiv = 1;
					if (i2 < i3)
						pos1 = i2;
					else
						pos1 = i3;
				}
				else if (i2 >= 0)// LF
				{
					cchDiv = 1;
					pos1 = i2;
				}
				else if (i3 >= 0)// CR
				{
					cchDiv = 1;
					pos1 = i3;
				}
			}
		}
		cchLine = cchFile - pos2;// 处理末尾一行文本
		if (cchLine > 0)
		{
			pszLine = pszOrg + pos2;
			*(pszLine + cchLine) = L'\0';
			Lines.emplace_back(pszLine, cchLine);
		}
	}
#pragma endregion
#pragma region 处理每行歌词
	int pos3;
	int cchTimeLabel;

	std::vector<LRCTIMELABEL> vTimeLabel{};

	PWSTR pszTimeLabel;
	int posTimeDiv1, posTimeDiv2;
	BOOL bMS;

	EckCounter(Lines.size(), i)
	{
		pszLine = Lines[i].first;
		vTimeLabel.clear();// 相同的时间标签

		pos1 = eck::FindStr(pszLine, L"[");// 先找左中括号
		if (pos1 < 0)// 找不到左中括号
			continue;// 到循环尾（处理下一行）

		pos2 = eck::FindStr(pszLine, L"]", pos1 + 1);// 找下一个右中括号
		if (pos2 < 0)
			continue;// 中括号错误，这时还没有时间标签被读入，因此不是歌词中间出现的中括号，应该跳过这一行
	RetryThisLine:
		pszTimeLabel = pszLine + pos1 + 1;
		if (!ParseLrc_IsTimeLabelLegal(pszTimeLabel, pos2 - pos1 - 1, &posTimeDiv1, &posTimeDiv2, &bMS))
		{
			if (posTimeDiv1 > 0)// 不合法，但有冒号，视为其他标签
			{
				*(pszTimeLabel + posTimeDiv1) = L'\0';
				*(pszLine + pos2) = L'\0';
				Label.emplace_back(pszTimeLabel, pszTimeLabel + posTimeDiv1 + 1);
				// 往后找，是否还有其他标签
				pos1 = eck::FindStr(pszLine, L"[", pos2 + 1);
				if (pos1 >= 0)
				{
					pos2 = eck::FindStr(pszLine, L"]", pos1 + 1);
					if (pos2 >= 0)
						goto RetryThisLine;
				}
			}
			continue;// 不合法，跳过这一行
		}

		for (;;)// 行中循环取标签
		{
		GetLabelLoopBegin:
			pszTimeLabel = pszLine + pos1 + 1;
			cchTimeLabel = pos2 - pos1 - 1;
			*(pszTimeLabel + posTimeDiv1) = L'\0';
			if (posTimeDiv2 > 0)
				*(pszTimeLabel + posTimeDiv2) = L'\0';
			*(pszTimeLabel + cchTimeLabel) = L'\0';
			vTimeLabel.emplace_back(pszLine + pos1 + 1, posTimeDiv1, posTimeDiv2);
			// 当前时间标签已解析完成

			pos3 = eck::FindStr(pszLine, L"[", pos1 + cchTimeLabel + 2);// 找下一个左中括号
		TryNewBracket:
			if (pos3 < 0)// 找不到，则本行解析完成
			{
				ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel, pszLine + pos2 + 1, Lines[i].second - pos2 - 1);
				break;
			}
			else if (pos3 == pos2 + 1)// 找到了，而且紧跟在上一个右中括号的后面
			{
				int posNextRightBracket = eck::FindStr(pszLine, L"]", pos3);// 找下一个右中括号
				if (posNextRightBracket < 0)// 找不到，则本行解析完成
				{
					ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel, pszLine + pos2 + 1, Lines[i].second - pos2 - 1);
					break;
				}
				else
				{
					if (ParseLrc_IsTimeLabelLegal(pszLine + pos3 + 1, posNextRightBracket - pos3 - 1, &posTimeDiv1, &posTimeDiv2, &bMS))
					{
						pos1 = pos3;
						pos2 = posNextRightBracket;
						continue;// 合法，继续循环
					}
					else// 如果不合法，跳过当前左中括号重试
					{
						pos3 = eck::FindStr(pszLine, L"[", pos3 + 1);// 找下一个左中括号
						goto TryNewBracket;
					}
				}
			}
			else// 找到了，但离上一个右中括号有一段间隔
			{
				int posNextRightBracket = eck::FindStr(pszLine, L"]", pos3);// 找下一个右中括号
				if (posNextRightBracket < 0)// 找不到，则本行解析完成，剩余部分视为歌词
				{
					ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel, pszLine + pos2 + 1, Lines[i].second - pos2 - 1);
					break;
				}
				else
				{
					if (ParseLrc_IsTimeLabelLegal(pszLine + pos3 + 1, posNextRightBracket - pos3 - 1, &posTimeDiv1, &posTimeDiv2, &bMS))
					{// 如果是合法的，则间隔部分视为歌词
						ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel, pszLine + pos2 + 1, pos3 - pos2 - 1);
						vTimeLabel.clear();
						pos1 = pos3;
						pos2 = posNextRightBracket;
						continue;// 继续循环
					}
					else// 如果不合法，跳过当前左中括号重试
					{
						for (;;)
						{
							pos3 = eck::FindStr(pszLine, L"[", pos3 + 1);// 找下一个左中括号
							if (pos3 < 0)
								goto NoValidTimeLabel;
							else
							{
								int posNextRightBracket = eck::FindStr(pszLine, L"]", pos3);// 找下一个右中括号
								if (posNextRightBracket < 0)
									goto NoValidTimeLabel;
								else if (ParseLrc_IsTimeLabelLegal(pszLine + pos3 + 1, posNextRightBracket - pos3 - 1,
									&posTimeDiv1, &posTimeDiv2, &bMS))
								{
									ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel, pszLine + pos2 + 1, pos3 - pos2 - 1);
									vTimeLabel.clear();
									pos1 = pos3;
									pos2 = posNextRightBracket;
									goto GetLabelLoopBegin;
								}
							}
						}

					NoValidTimeLabel:
						ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel, pszLine + pos2 + 1, Lines[i].second - pos2 - 1);
						break;
					}
				}
			}
		}
	}
#pragma endregion
#pragma region 合并时间相同的歌词
	std::stable_sort(Result.begin(), Result.end(), [](const LRCINFO& a, const LRCINFO& b)->bool
		{
			return a.fTime < b.fTime;
		});

	std::vector<float> vLastTime{};
	std::vector<size_t> vNeedDelIndex{};
	vNeedDelIndex.reserve(Lines.size() / 2);

	EckCounter(Result.size(), i)
	{
		auto& x = Result[i];
		if (vLastTime.size() != 0 && i != 0)
		{
			if (vLastTime[0] == x.fTime)
			{
				auto& TopItem = Result[i - vLastTime.size()];

				int cch1 = TopItem.cchTotal,
					cch2 = x.cchTotal;

				if (cch1 && !cch2)// 只有第一个
				{
				}// 什么都不做
				else if (!cch1 && cch2)// 只有第二个
				{
					TopItem.pszLrc = x.pszLrc;
					TopItem.pszTranslation = NULL;
					x.pszLrc = NULL;

					TopItem.cchLrc = x.cchLrc;
					TopItem.cchTotal = x.cchTotal;
				}
				else if (!cch1 && !cch2)// 两个都没有
				{
				}
				else// 两个都有
				{
#pragma warning (push)
#pragma warning (disable: 6308)// realloc为NULL
					TopItem.pszLrc = (PWSTR)realloc(TopItem.pszLrc, eck::Cch2Cb(cch1 + cch2 + 1));
#pragma warning (pop)
					* (TopItem.pszLrc + cch1) = L'\n';
					wcscpy(TopItem.pszLrc + cch1 + 1, x.pszLrc);

					TopItem.pszTranslation = TopItem.pszLrc + cch1 + 1;

					TopItem.cchLrc = cch1;
					TopItem.cchTotal = cch1 + cch2 + 1;
				}
				vNeedDelIndex.push_back(i);
			}
			else
				vLastTime.clear();
		}
		vLastTime.push_back(x.fTime);
	}

	for (auto it = vNeedDelIndex.rbegin(); it < vNeedDelIndex.rend(); ++it)
		Result.erase(Result.begin() + *it);

	if (!Result.empty())
	{
		EckCounter(Result.size() - 1, i)
		{
			auto& e = Result[i];
			e.fDuration = Result[i + 1].fTime - e.fTime;

			if (!e.pszLrc)
			{
				e.pszLrc = (PWSTR)malloc(eck::Cch2Cb(0));
#pragma warning (suppress: 6011)// 解引用NULL
				*e.pszLrc = L'\0';
			}
		}
		auto& f = Result.back();
		f.fDuration = fTotalTime - Result.back().fTime;
		if (!f.pszLrc)
		{
			f.pszLrc = (PWSTR)malloc(eck::Cch2Cb(0));
#pragma warning (suppress: 6011)// 解引用NULL
			*f.pszLrc = L'\0';
		}
	}
#pragma endregion
	Result.shrink_to_fit();
	Label.shrink_to_fit();
	VirtualFree(pFileData, 0, MEM_RELEASE);
	return TRUE;
}

UINT MsgBox(PCWSTR pszMainInstruction, PCWSTR pszContent, PCWSTR pszWndTitle, UINT cButtons, HICON hIcon, HWND hParent,
	UINT iDefButton, BOOL bCenterPos, PCWSTR pszCheckBoxTitle, PCWSTR pszButton1Title, PCWSTR pszButton2Title,
	PCWSTR pszButton3Title, BOOL* pbCheck)
{
	if (cButtons < 1 && cButtons >3)
	{
		EckDbgBreak();
		return 0;
	}
	TASKDIALOGCONFIG tdc = { 0 };
	tdc.cbSize = sizeof(TASKDIALOGCONFIG);
	TASKDIALOG_BUTTON tdb[3];
	tdc.cButtons = cButtons;
	tdc.pButtons = tdb;
	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | (bCenterPos ? TDF_POSITION_RELATIVE_TO_WINDOW : 0);
	tdc.pszMainInstruction = pszMainInstruction;
	tdc.pszContent = pszContent;
	tdc.pszWindowTitle = pszWndTitle;
	tdc.pszVerificationText = pszCheckBoxTitle;
	tdc.hMainIcon = hIcon;
	tdc.hwndParent = hParent;
	tdc.nDefaultButton = iDefButton;
	switch (cButtons)
	{
	case 1:
		tdb[0].nButtonID = MBBID_1;
		tdb[0].pszButtonText = !pszButton1Title ? L"确定(&O)" : pszButton1Title;
		break;
	case 2:
		tdb[0].nButtonID = MBBID_1;
		tdb[0].pszButtonText = !pszButton1Title ? L"是(&Y)" : pszButton1Title;
		tdb[1].nButtonID = MBBID_2;
		tdb[1].pszButtonText = !pszButton2Title ? L"否(&N)" : pszButton2Title;
		break;
	case 3:
		tdb[0].nButtonID = MBBID_1;
		tdb[0].pszButtonText = !pszButton1Title ? L"是(&Y)" : pszButton1Title;
		tdb[1].nButtonID = MBBID_2;
		tdb[1].pszButtonText = !pszButton2Title ? L"否(&N)" : pszButton2Title;
		tdb[2].nButtonID = MBBID_3;
		tdb[2].pszButtonText = !pszButton3Title ? L"取消(&C)" : pszButton3Title;
		break;
	}
	int iButton, iRadio;
	BOOL bCheckBox;
	TaskDialogIndirect(&tdc, &iButton, &iRadio, &bCheckBox);
	if (pbCheck)
		*pbCheck = bCheckBox;
	return iButton;
}
UTILS_NAMESPACE_END