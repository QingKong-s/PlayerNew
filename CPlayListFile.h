#pragma once
#include "Utils.h"
#include "eck\CFile.h"

#include <functional>

//////////////////////////////////�ɰ汾�Ĳ����б��ļ�//////////////////////////////////
#pragma pack(push)
#pragma pack(4)
struct LISTFILEHEADER_0	// �����б��ļ�ͷ
{
	CHAR cHeader[4];	// �ļ���ʼ��ǣ�ASCII�ַ���"QKPL"
	int iCount;			// ��Ŀ��
	DWORD dwVer;		// �浵�ļ��汾��QKLFVER_���������£�
	DWORD dwReserved;	// ����������Ϊ0
};

#define QKLFVER_1				0// ����汾���б��ļ���û�м�¼ʱ��Ĺ��ܣ����������Ѿ����Լ����б�������ˣ���Ϊ�������ţ��������Լ�������汾���ƣ��������ֶι�Ȼ�����ǵ�ѡ��2333��
#define QKLFVER_2				1

struct LISTFILEITEM_0	// �����б��ļ���Ŀͷ
{
	UINT uFlags;		// ��Ŀ��־
	DWORD dwReserved1;	// ����������Ϊ0
	DWORD dwReserved2;	// ����������Ϊ0
};

#define QKLIF_INVALID			0x00000001// ��Ŀ��Ч
#define QKLIF_IGNORED			0x00000002// ����
#define QKLIF_BOOKMARK			0x00000004// ����ǩ
#define QKLIF_DRAGMARK_CURRFILE	0x00000008// �����Ϸ�ʱ��Ч�����в��ű�־����ԭ������
#define QKLIF_TIME				0x00000010// ���ڴ浵���ȡ�ļ�ʱ��Ч������ʱ���ַ���
#define QKLIF_DRAGMARK_PLLATER	0x00000020// �����Ϸ�ʱ��Ч���Ժ󲥷ű�־����ԭ������
#pragma pack(pop)
/*
* LISTFILEHEADER_0
* {
*	LISTFILEITEM_0
*	����\0
*	�ļ���\0
*	opt : // ��������QKLIF_BOOKMARKʱ
*		��ǩ��ɫ : COLORREF
*		��ǩ����\0
*		��ǩ��ע\0
* 	[���汾2] :
*		ʱ��\0
* }
*/

//////////////////////////////////�°汾�Ĳ����б��ļ�//////////////////////////////////
#pragma pack(push, 4)
struct LISTFILEHEADER_1	// �����б��ļ�ͷ
{
	CHAR chHeader[4];	// �ļ���ʼ��ǣ�ASCII�ַ���"PNPL"
	int iVer;			// �浵�ļ��汾��PNLFVER_����
	UINT ocbBookMark;	// ��ǩ��Ϣƫ��
	int cItems;			// ��Ŀ��
};

struct LISTFILEITEM_1	// �����б��ļ���Ŀͷ
{
	int cchName;		// ���Ƴ��ȣ���������β��\0
	int cchTime;		// ʱ�䳤�ȣ���������β��\0
	int cchFile;		// �ļ������ȣ���������β��\0
	BITBOOL bIgnore : 1;
	BITBOOL bDelayPlaying : 1;
	// WCHAR szName[];
	// WCHAR szTime[];
	// WCHAR szFile[];
};

struct BOOKMARKHEADER	// ��ǩ��Ϣͷ
{
	int iVer;			// ��ǩ��Ϣ�汾��PNBMVER_����
	int cBookmarks;		// ��ǩ��
};

struct BOOKMARKITEM		// ��ǩ��Ϣ��Ŀͷ
{
	int idxItem;		// ��ǩ������Ŀ��������ȡֵ��ΧΪ0��cItems������ʾ��ָ������֮ǰ
	COLORREF cr;		// ��ǩ��ɫ
	int cchName;		// ��ǩ���Ƴ��ȣ���������β��\0
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