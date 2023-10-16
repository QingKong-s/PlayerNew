#include "Utils.h"

UTILS_NAMESPACE_BEGIN
/// <summary>
/// ͬ����ȫ������32λС������
/// </summary>
/// <param name="p">�����ֽ���</param>
/// <returns>ת�����</returns>
PNInline DWORD SynchSafeIntToDWORD(BYTE* p)
{
	return ((p[0] & 0x7F) << 21) | ((p[1] & 0x7F) << 14) | ((p[2] & 0x7F) << 7) | (p[3] & 0x7F);
}

BOOL IsTextUTF8(char* str, ULONGLONG length)
{
	int i;
	DWORD nBytes = 0;//UFT8����1-6���ֽڱ���,ASCII��һ���ֽ�
	UCHAR chr;
	BOOL bAllAscii = TRUE; //���ȫ������ASCII, ˵������UTF-8
	for (i = 0; i < length; i++)
	{
		chr = *(str + i);
		if ((chr & 0x80) != 0) // �ж��Ƿ�ASCII����,�������,˵���п�����UTF-8,ASCII��7λ����,����һ���ֽڴ�,���λ���Ϊ0,o0xxxxxxx
			bAllAscii = FALSE;
		if (nBytes == 0) //�������ASCII��,Ӧ���Ƕ��ֽڷ�,�����ֽ���
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
		else //���ֽڷ��ķ����ֽ�,ӦΪ 10xxxxxx
		{
			if ((chr & 0xC0) != 0x80)
			{
				return FALSE;
			}
			nBytes--;
		}
	}
	if (nBytes > 0) //Υ������
	{
		return FALSE;
	}
	if (bAllAscii) //���ȫ������ASCII, ˵������UTF-8
	{
		return FALSE;
	}
	return TRUE;
}

/// <summary>
/// [����ID3v2��������]��ָ�����봦���ı�
/// </summary>
/// <param name="pStream">�ֽ���ָ�룻δָ��iTextEncodingʱָ�������ı�֡��ָ��iTextEncodingʱָ���ַ���</param>
/// <param name="iLength">���ȣ�δָ��iTextEncodingʱ��ʾ�����ı�֡���ȣ�����1B�ı����ǣ�������βNULL����ָ��iTextEncodingʱ��ʾ�ַ������ȣ�������βNULL��</param>
/// <param name="iTextEncoding">�Զ����ı����룻-1��ȱʡ��ָʾ��������ı�֡</param>
/// <returns>���ش�����ϵ��ı�</returns>
eck::CRefStrW GetMP3ID3v2_ProcString(BYTE* pStream, int cb, int iTextEncoding = -1)
{
	int iType = 0, cchBuf;
	if (iTextEncoding == -1)
	{
		memcpy(&iType, pStream, 1);
		++pStream;// �����ı������־
		--cb;
	}
	else
		iType = iTextEncoding;

	eck::CRefStrW rsResult{};

	switch (iType)
	{
	case 0:// ISO-8859-1����Latin-1��������-1��
		cchBuf = MultiByteToWideChar(CP_ACP, 0, (PCCH)pStream, cb, NULL, 0);
		if (cchBuf == 0)
			return {};
		rsResult.ReSizeAbs(cchBuf);
		MultiByteToWideChar(CP_ACP, 0, (PCCH)pStream, cb, rsResult.Data(), cchBuf);
		break;
	case 1:// UTF-16LE
		if (*(PWSTR)pStream == L'\xFEFF')// ��BOM��Ҫ�����������ϣֵ��һ���ҿ��ܻ��淢�ֲ������BOM������.....��
		{
			pStream += sizeof(WCHAR);
			cb -= sizeof(WCHAR);
		}
		cchBuf = cb / sizeof(WCHAR);
		rsResult.ReSizeAbs(cchBuf);
		wcsncpy(rsResult.Data(), (PWSTR)pStream, cchBuf);
		break;
	case 2:// UTF-16BE
		if (*(PWSTR)pStream == L'\xFFFE')// ��BOM
		{
			pStream += sizeof(WCHAR);
			cb -= sizeof(WCHAR);
		}
		cchBuf = cb / sizeof(WCHAR);
		rsResult.ReSizeAbs(cchBuf);
		LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
			(PCWSTR)pStream, cchBuf, rsResult.Data(), cchBuf, NULL, NULL, 0);// ��ת�ֽ���
		break;
	case 3:// UTF-8
		cchBuf = MultiByteToWideChar(CP_UTF8, 0, (PCCH)pStream, cb, NULL, 0);
		if (cchBuf == 0)
			return {};
		rsResult.ReSizeAbs(cchBuf);
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
	if (File.Open(pszFile) == INVALID_HANDLE_VALUE)
		return FALSE;
	DWORD cbFile = File.GetSize32();

	BYTE by[4];
	File >> by;// ���ļ�ͷ
	if (memcmp(by, "ID3", 3) == 0)// ID3v2
	{
		if (cbFile < sizeof(ID3v2_Header))
			return FALSE;

		eck::CMappingFile2 mf(File);
		eck::CMemReader r(mf.Create(), cbFile);

		ID3v2_Header* pHeader;
		r.SkipPointer(pHeader);
		DWORD cbTotal = SynchSafeIntToDWORD(pHeader->Size);// 28λ���ݣ�������ǩͷ����չͷ
		if (cbTotal > cbFile)
			return FALSE;

		void* pEnd = r.m_pMem + cbTotal;

		auto pExtHeader = (ID3v2_ExtHeader*)r.m_pMem;

		if (pHeader->Ver == 3)// 2.3
		{
			if (pHeader->Flags & 0x20)// ����չͷ
				r += (4 + eck::ReverseDWORD(pExtHeader->ExtHeaderSize));
		}
		else if (pHeader->Ver = 4)// 2.4
		{
			if (pHeader->Flags & 0x20)// ����չͷ
				r += SynchSafeIntToDWORD(pExtHeader->ExtHeaderSize);
			// 2.4������ͬ����ȫ��������������ߴ�����˼�¼�ߴ���ĸ��ֽ�
		}

		DWORD cbUnit;
		ID3v2_FrameHeader* pFrame;
		while (r < pEnd)
		{
			r.SkipPointer(pFrame);

			if (pHeader->Ver == 3)
				cbUnit = eck::ReverseDWORD(pFrame->Size);// 2.3��32λ���ݣ�������֡ͷ��ƫ4B��
			else if (pHeader->Ver = 4)
				cbUnit = SynchSafeIntToDWORD(pFrame->Size);//2.4��28λ���ݣ�ͬ����ȫ������

			if (memcmp(pFrame->ID, "TIT2", 4) == 0)// ����
			{
				mi.rsTitle = GetMP3ID3v2_ProcString(r, cbUnit);
				r += cbUnit;
			}
			else if (memcmp(pFrame->ID, "TPE1", 4) == 0)// ����
			{
				mi.rsArtist = GetMP3ID3v2_ProcString(r, cbUnit);
				r += cbUnit;
			}
			else if (memcmp(pFrame->ID, "TALB", 4) == 0)// ר��
			{
				mi.rsAlbum = GetMP3ID3v2_ProcString(r, cbUnit);
				r += cbUnit;
			}
			else if (memcmp(pFrame->ID, "USLT", 4) == 0)// ��ͬ�����
			{
				/*
				<֡ͷ>��֡��ʶΪUSLT��
				�ı�����						$xx
				��Ȼ���Դ���					$xx xx xx
				��������						<�ַ���> $00 (00)
				���							<�ַ���>
				*/
				DWORD cb = cbUnit;

				BYTE byEncodeingType;
				r >> byEncodeingType;// ���ı�����

				CHAR byLangCode[3];
				r >> byLangCode;// ����Ȼ���Դ���

				int t;
				if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1��UTF-8
					t = (int)strlen((PCSTR)r.m_pMem) + 1;
				else// UTF-16LE��UTF-16BE
					t = ((int)wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR);
				r += t;// ������������

				cb -= (t + 4);

				mi.rsLrc = GetMP3ID3v2_ProcString(r, cb, byEncodeingType);
				r += cb;
			}
			else if (memcmp(pFrame->ID, "COMM", 4) == 0)// ��ע
			{
				/*
				<֡ͷ>��֡��ʶΪCOMM��
				�ı�����						$xx
				��Ȼ���Դ���					$xx xx xx
				��עժҪ						<�ַ���> $00 (00)
				��ע							<�ַ���>
				*/
				DWORD cb = cbUnit;

				BYTE byEncodeingType;
				r >> byEncodeingType;// ���ı�����

				CHAR byLangCode[3];
				r >> byLangCode;// ����Ȼ���Դ���

				int t;
				if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1��UTF-8
					t = (int)strlen((PCSTR)pFrame) + 1;
				else// UTF-16LE��UTF-16BE
					t = ((int)wcslen((PCWSTR)pFrame) + 1) * sizeof(WCHAR);
				r += t;// ������עժҪ

				cb -= (t + 4);
				// ��ʱpFrameָ��ע�ַ���
				mi.rsComment = GetMP3ID3v2_ProcString(r, cb, byEncodeingType);
				r += cb;
			}
			else if (memcmp(pFrame->ID, "APIC", 4) == 0)// ͼƬ
			{
				/*
				<֡ͷ>��֡��ʶΪAPIC��
				�ı�����                        $xx
				MIME ����                       <ASCII�ַ���>$00����'image/bmp'��
				ͼƬ����                        $xx
				����                            <�ַ���>$00(00)
				<ͼƬ����>
				*/
				DWORD cb = cbUnit;

				BYTE byEncodeingType;
				r >> byEncodeingType;// ���ı�����

				int t;
				t = (int)strlen((PCSTR)r.m_pMem);
				r += (t + 2);// ����MIME�����ַ�����ͼƬ����

				cb -= (t + 3);

				if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1��UTF-8
					t = (int)strlen((PCSTR)r.m_pMem) + 1;
				else// UTF-16LE��UTF-16BE
					t = ((int)wcslen((PCWSTR)r.m_pMem) + 1) * sizeof(WCHAR);

				r += t;
				cb -= t;// ���������ַ����ͽ�βNULL

				mi.pCoverData = SHCreateMemStream(r, cb);// ����������
				r += cb;
			}
		}
	}
	else if (memcmp(by, "fLaC", 4) == 0)// Flac
	{
		FLAC_Header Header;
		DWORD dwSize;
		UINT t;
		char* pBuffer;
		do
		{
			File >> Header;
			dwSize = Header.bySize[2] | Header.bySize[1] << 8 | Header.bySize[0] << 16;
			switch (Header.by & 0x7F)
			{
			case 4:// ��ǩ��Ϣ��ע�⣺��һ������С����
			{
				File >> t;// ��������Ϣ��С
				File += t;// ������������Ϣ

				UINT uCount;
				File >> uCount;// ��ǩ����

				for (UINT i = 0; i < uCount; ++i)
				{
					File >> t;// ��ǩ��С

					pBuffer = new char[t + 1];
					File.Read(pBuffer, t);// ����ǩ
					*(pBuffer + t) = '\0';

					t = MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, NULL, 0);
					PWSTR pszLabel = (PWSTR)HeapAlloc(GetProcessHeap(), 0, t * sizeof(WCHAR));
					MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, pszLabel, t);// ת�����룬UTF-8��UTF-16LE
					delete[] pBuffer;

					int iPos = eck::FindStr(pszLabel, L"=");// �ҵȺ�
					if (iPos != eck::INVALID_STR_POS)
					{
						int cch = t - iPos;
						if (eck::FindStr(pszLabel, L"TITLE"))
						{
							mi.rsTitle.ReSizeAbs(cch);
							lstrcpyW(mi.rsTitle.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"ALBUM"))
						{
							mi.rsAlbum.ReSizeAbs(cch);
							lstrcpyW(mi.rsAlbum.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"ARTIST"))
						{
							mi.rsArtist.ReSizeAbs(cch);
							lstrcpyW(mi.rsArtist.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"DESCRIPTION"))
						{
							mi.rsComment.ReSizeAbs(cch);
							lstrcpyW(mi.rsComment.Data(), pszLabel + iPos);
						}
						else if (eck::FindStr(pszLabel, L"LYRICS"))
						{
							mi.rsLrc.ReSizeAbs(cch);
							lstrcpyW(mi.rsLrc.Data(), pszLabel + iPos);
						}
					}

					HeapFree(GetProcessHeap(), 0, pszLabel);
				}
			}
			break;
			case 6:// ͼƬ�������
			{
				File += 4;// ����ͼƬ����

				File >> t;// MIME�����ַ�������
				t = eck::ReverseDWORD(t);// ������ֽڵ���������ͬ
				File += t;// ����MIME�����ַ���

				File >> t;// �����ַ�������
				t = eck::ReverseDWORD(t);
				File += (t + 16);// ���������ַ�������ȡ��߶ȡ�ɫ�����ͼ��ɫ��

				File >> t;// ͼƬ���ݳ���
				t = eck::ReverseDWORD(t);// ͼƬ���ݳ���

				pBuffer = new char[t];
				File.Read(pBuffer, t);
				mi.pCoverData = SHCreateMemStream((const BYTE*)pBuffer, t);// ����������
				delete[] pBuffer;
			}
			break;
			default:
				File += dwSize;// ������
			}

		} while (!(Header.by & 0x80));// ������λ���ж��ǲ������һ����
	}
	return TRUE;
}


/// <summary>
/// [��ȡ������ݸ�������]������ʱ���ǩ�����ı���ǩת���ɸ������������䰴����װ�ص���������У�������ɺ�����ԭ����
/// </summary>
/// <param name="Result">�������</param>
/// <param name="TimeLabel">ʱ���ǩ����</param>
/// <param name="pszLrc">��ʱ���ǩ���������г�Ա����Ӧ�ĸ��</param>
void ParseLrc_ProcLabel(std::vector<LRCINFO>& Result, std::vector<LRCLABEL>& Label,
	std::vector<PWSTR>& TimeLabel, PWSTR pszLrc, int cchLrc)
{
#pragma warning (push)
#pragma warning (disable: 6387)// ������NULL
#pragma warning (disable: 6053)// ����δ�����ֹNULL
	PWSTR pszTimeLabel;

	PWSTR pTemp;
	int cchTemp;
	int M, S, MS;
	BOOL IsMS = TRUE;
	float fTime;
	int iStrPos1, iStrPos2;
	EckCounter(TimeLabel.size(), i)
	{
		pszTimeLabel = TimeLabel[i];

		iStrPos1 = eck::FindStr(pszTimeLabel, L":");
		if (iStrPos1 < 0)
			continue;// ûð�ţ���ѭ��β
		iStrPos2 = eck::FindStr(pszTimeLabel, L":", iStrPos1 + 1);

		if (iStrPos2 <= 0)// �Ƿ�[��:��:����]
		{
			iStrPos2 = eck::FindStr(pszTimeLabel, L".", iStrPos1 + 1);
			if (iStrPos2 <= 0)// �Ƿ�[��:��.����]
			{
				IsMS = FALSE;// [��:��]
				iStrPos2 = (int)wcslen(pszTimeLabel) + 1;
			}
		}
		///////////////////ȡ����
		cchTemp = iStrPos1;
		pTemp = (PWSTR)_malloca(eck::Cch2Cb(cchTemp));
		wcsncpy(pTemp, pszTimeLabel, cchTemp);
		*(pTemp + cchTemp) = L'\0';
		if (!StrToIntExW(pTemp, STIF_DEFAULT, &M))
		{
			// ת��ʧ�ܣ���Ϊ��ʱ���ǩ
			Label.emplace_back(pTemp, eck::LTrimStr(pszTimeLabel + iStrPos1 + 1));
			_freea(pTemp);
			continue;
		}
		fTime = (float)M * 60.0f;
		_freea(pTemp);
		///////////////////ȡ��
		cchTemp = iStrPos2 - iStrPos1 - 1;
		pTemp = (PWSTR)_malloca(eck::Cch2Cb(cchTemp));
		wcsncpy(pTemp, pszTimeLabel + iStrPos1 + 1, cchTemp + 1);
		*(pTemp + cchTemp) = L'\0';
		if (!StrToIntExW(pTemp, STIF_DEFAULT, &S))
			continue;
		fTime += S;
		_freea(pTemp);
		///////////////////ȡ����
		if (IsMS)
		{
			cchTemp = (int)wcslen(pszTimeLabel) - iStrPos2;
			if (cchTemp == 2)// ֻ����λxxʱ��ʾxx0�����Ƚ����ˣ���λ��ʮ���룩
			{
				pTemp = (PWSTR)_malloca(eck::Cch2Cb(cchTemp + 1));
				wcsncpy(pTemp, pszTimeLabel + iStrPos2 + 1, cchTemp);
				*(pTemp + cchTemp) = L'0';
				*(pTemp + cchTemp + 1) = L'\0';
			}
			else
			{
				pTemp = (PWSTR)_malloca(eck::Cch2Cb(cchTemp));
				wcsncpy(pTemp, pszTimeLabel + iStrPos2 + 1, cchTemp);
				*(pTemp + cchTemp) = L'\0';
			}

			if (!StrToIntExW(pTemp, STIF_DEFAULT, &MS))
				continue;
			fTime += ((float)MS / 1000.f);
			_freea(pTemp);
		}
		///////////////////
		if (fTime < 0)
			continue;

		if (cchLrc)
		{
			pTemp = (PWSTR)malloc(eck::Cch2Cb(cchLrc));
			Result.emplace_back(pTemp, nullptr, fTime, cchLrc, cchLrc);
			wcsncpy(pTemp, pszLrc, cchLrc);
			*(pTemp + cchLrc) = L'\0';
		}
		else
			Result.emplace_back(nullptr, nullptr, fTime, 0, 0);
	}
#pragma warning (pop)
}

BOOL ParseLrc(PCVOID p, SIZE_T cbMem, std::vector<LRCINFO>& Result, std::vector<LRCLABEL>& Label, int iDefTextEncoding)
{
	Result.clear();
#pragma region ��������
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
		eck::CFile File;
		File.Open((PCWSTR)p);
		cbMem = File.GetSize32();
		if (cbMem < 5)
			return FALSE;

		pFileData = (BYTE*)VirtualAlloc(NULL, cbMem + 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		File.Read(pFileData, cbMem);
		File.Close();
		*(pFileData + cbMem) = '\0';
		*(pFileData + cbMem + 1) = '\0';
	}
#pragma endregion
#pragma region �жϲ�ת������
	PWSTR pszOrg = (PWSTR)pFileData;
	int cchFile = cbMem / sizeof(WCHAR);

	UINT uCode;

	constexpr BYTE c_chBomU16LE[] = { 0xFF, 0xFE };
	constexpr BYTE c_chBomU16BE[] = { 0xFE, 0xFF };
	constexpr BYTE c_chBomU8[] = { 0xEF, 0xBB, 0xBF };
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
			(PCWSTR)pFileData + 1, cchFile, pszOrg, cchFile, NULL, NULL, 0);// ��ת�ֽ���
		VirtualFree(pFileData, 0, MEM_RELEASE);
		pFileData = (BYTE*)pszOrg;
	}
	else if (memcmp(pFileData, c_chBomU8, 3) == 0)
	{
		int cchBuf = MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData + 3, -1, NULL, 0);
		pszOrg = (PWSTR)VirtualAlloc(NULL, eck::Cch2Cb(cchBuf), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData + 3, -1, pszOrg, cchBuf);// ת������
		VirtualFree(pFileData, 0, MEM_RELEASE);
		pFileData = (BYTE*)pszOrg;
		cchFile = cchBuf - 1;
	}
	else// ��BOM
	{
		switch (iDefTextEncoding)
		{
		case 0:// �Զ�
		{
			int i = IS_TEXT_UNICODE_REVERSE_MASK | IS_TEXT_UNICODE_NULL_BYTES;
			if (IsTextUnicode(pFileData, cbMem, &i))//  �Ȳ�UTF-16BE����Ȼ�������
				goto GetLrc_UTF16BE;
			else
			{
				i = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_NULL_BYTES;
				if (IsTextUnicode(pFileData, cbMem, &i))
					goto GetLrc_UTF16LE;
				else if (IsTextUTF8((char*)pFileData, cbMem))
					goto GetLrc_UTF8;
				else
					goto GetLrc_GB2312;
			}
		}
		break;
		case 1:// GB2312
		{
		GetLrc_GB2312:
			int cchBuf = MultiByteToWideChar(936, 0, (CHAR*)pFileData, -1, NULL, 0);
			pszOrg = (PWSTR)VirtualAlloc(NULL, eck::Cch2Cb(cchBuf), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			MultiByteToWideChar(936, 0, (CHAR*)pFileData, -1, pszOrg, cchBuf);// ת������
			VirtualFree(pFileData, 0, MEM_RELEASE);
			pFileData = (BYTE*)pszOrg;
			cchFile = cchBuf - 1;
		}
		break;
		case 2:// UTF-8
		{
		GetLrc_UTF8:
			int cchBuf = MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData, -1, NULL, 0);
			pszOrg = (PWSTR)VirtualAlloc(NULL, eck::Cch2Cb(cchBuf), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData, -1, pszOrg, cchBuf);// ת������
			VirtualFree(pFileData, 0, MEM_RELEASE);
			pFileData = (BYTE*)pszOrg;
			cchFile = cchBuf - 1;
		}
		break;
		case 3:// UTF-16LE
		GetLrc_UTF16LE:;
			break;
		case 4:// UTF-16BE
		{
		GetLrc_UTF16BE:
			pszOrg = (PWSTR)VirtualAlloc(NULL, (cchFile + 1) * sizeof(WCHAR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
				(PCWSTR)pFileData, cchFile, pszOrg, cchFile, NULL, NULL, 0);// ��ת�ֽ���
			VirtualFree(pFileData, 0, MEM_RELEASE);
			pFileData = (BYTE*)pszOrg;
		}
		break;
		}
	}
#pragma endregion
#pragma region ���зָ�
	std::vector<std::pair<PWSTR, int>> Lines{};
	Lines.reserve(50);

	WCHAR szDiv1[3] = L"\r\n";// CRLF
	WCHAR szDiv2[2] = L"\n";// LF
	WCHAR szDiv3[2] = L"\r";// CR

	BOOL b1, b2, b3;

	int i1 = eck::FindStr(pszOrg, szDiv1),
		i2 = eck::FindStr(pszOrg, szDiv2),
		i3 = eck::FindStr(pszOrg, szDiv3);

	b1 = i1 >= 0;
	b2 = i2 >= 0;
	b3 = i3 >= 0;

	PWSTR pszLine;// ÿ������
	int cchLine;// ÿ�����ݳ���

	int pos1;
	int pos2 = 0;
	int cchDiv;

	if (!b1 && !b2 && !b3)// �޻��з�
		Lines.emplace_back(pszOrg, cchFile);
	else
	{
		// ������Щ�������ֻ��з���ɵ���ļ�
		if (b1)// CRLF
		{
			// ˼·��iStrPos1 = min(i1, i2, i3)
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
			// ˼·��iStrPos1 = min(i2, i3)
			cchDiv = 1;
			if (b2 && b3)// û��CRLF����CR��LFͬʱ����
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
			pos2 = pos1 + cchDiv;// �������з�
			/////////////ȡ��һ���з�λ��
			if (b1)
				i1 = eck::FindStr(pszOrg, szDiv1, pos2);
			if (b2)
				i2 = eck::FindStr(pszOrg, szDiv2, pos2);
			if (b3)
				i3 = eck::FindStr(pszOrg, szDiv3, pos2);

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
		cchLine = cchFile - pos2;// ����ĩβһ���ı�
		if (cchLine > 0)
		{
			pszLine = pszOrg + pos2;
			*(pszLine + cchLine) = L'\0';
			Lines.emplace_back(pszLine, cchLine);
		}
	}
#pragma endregion
#pragma region ����ÿ�и��
	int pos3;
	std::vector<PWSTR> SameTimeLabel;
	int cchSentence;

	EckCounter(Lines.size(), i)
	{
		pszLine = Lines[i].first;
		pos1 = eck::FindStr(pszLine, L"[");// ������������
		pos2 = 0;
		if (pos1 < 0)// �Ҳ�����������
			continue;// ��ѭ��β��������һ�У�   
		SameTimeLabel.clear();// ��ͬ��ʱ���ǩ
		for (;;)// ����ѭ��ȡ��ǩ��һ���п����ж����ǩ��
		{
			pos2 = eck::FindStr(pszLine, L"]", pos2);// ����һ����������
			if (pos2 < 0 || pos2 <= pos1)
				pos1 = pos2 = 0;// �����Ŵ���������һ��

			cchSentence = pos2 - pos1 - 1;
			pos3 = eck::FindStr(pszLine, L"[", pos1 + cchSentence + 1);// ����һ����������
			// pos1=��������֮ǰ�ĵ�һ����������λ��
			// pos2=��������λ��
			// pos3=��������֮��ĵ�һ����������λ��
			if (pos3 - pos2 - 1 == 0)// �����ţ����������֣�[xx:xx][yy:yy]zzzzzzzzzzzzz
			{
				*(pszLine + pos1 + cchSentence + 1) = L'\0';
				SameTimeLabel.emplace_back(pszLine + pos1 + 1);
				pos1 = pos3;
				pos2 = pos1 + 1;
			}
			else if (pos3 < 0 || pos2 < 0)// û����һ����ǩ�ˣ���һ�е�ͷ��
			{
				*(pszLine + pos1 + cchSentence + 1) = L'\0';
				SameTimeLabel.emplace_back(pszLine + pos1 + 1);

				ParseLrc_ProcLabel(Result, Label, SameTimeLabel, pszLine + pos2 + 1, Lines[i].second - pos2 - 1);
				break;
			}
			else// ������һ���е�һ�䣬������[xx:xx]aaaaaaaaa[yy:yy]bbbbbbbbbbbb�����ڴ�����a��b��
			{
				*(pszLine + pos1 + cchSentence + 1) = L'\0';
				SameTimeLabel.emplace_back(pszLine + pos1 + 1);

				ParseLrc_ProcLabel(Result, Label, SameTimeLabel, pszLine + pos2 + 1, pos3 - pos2 - 1);

				SameTimeLabel.clear();
				pos1 = pos3;
				pos2 = pos1 + 1;
			}
			pos1 = pos2;
		}
	}
#pragma endregion
#pragma region �ϲ�ʱ����ͬ�ĸ��
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

				//if (cch1 && !cch2)// ֻ�е�һ��
				//{
				//}// ʲô������
				/*else */if (!cch1 && cch2)// ֻ�еڶ���
				{
					TopItem.pszLrc = x.pszLrc;
					TopItem.pszTranslation = NULL;
					x.pszLrc = NULL;

					TopItem.cchLrc = x.cchLrc;
					TopItem.cchTotal = x.cchTotal;
				}
				//else if (!cch1 && !cch2)// ������û��
				//{
				//}
				else// ��������
				{
#pragma warning (push)
#pragma warning (disable: 6308)// reallocΪNULL
					TopItem.pszLrc = (PWSTR)realloc(TopItem.pszLrc, eck::Cch2Cb(cch1 + cch2 + 1));
#pragma warning (pop)
					*(TopItem.pszLrc + cch1) = L'\n';
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

	for (auto& x : Result)
	{
		if (!x.pszLrc)
		{
			x.pszLrc = (PWSTR)malloc(eck::Cch2Cb(0));
#pragma warning (push)
#pragma warning (disable: 6011)// ������NULL
			*x.pszLrc = L'\0';
#pragma warning (pop)
		}
	}
#pragma endregion
	Result.shrink_to_fit();
	Label.shrink_to_fit();
	VirtualFree(pFileData, 0, MEM_RELEASE);
	return TRUE;
}
UTILS_NAMESPACE_END