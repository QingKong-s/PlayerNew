#pragma once
#include <Windows.h>
#include <Shlwapi.h>
#include <ShlObj_core.h>

#include <vector>
#include <algorithm>

#include "Utils.h"

class CDropTarget : public IDropTarget
{
private:
	ULONG m_uRefCount = 1;
public:
	// IUnknown
	//HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
	// IDropTarget
	//HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	//HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	//HRESULT STDMETHODCALLTYPE DragLeave(void);
	//HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
};

class CDropSource : public IDropSource
{
private:
	ULONG m_uRefCount = 1;
public:
	// IUnknown
	//HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
	// IDropSource
	//HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
	//HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect);
};

class CDataObject :public IDataObject
{
private:
	ULONG m_uRefCount = 1;
	std::vector<FORMATETC> m_vFormatEtc{};
	std::vector<STGMEDIUM> m_vStgMedium{};

	PNInline auto FindFormat(FORMATETC* pFormatEtc)
	{
		return std::find_if(m_vFormatEtc.begin(), m_vFormatEtc.end(), [pFormatEtc](const FORMATETC& x)->bool
			{
				return x.cfFormat == pFormatEtc->cfFormat &&
					x.tymed == pFormatEtc->tymed &&
					x.dwAspect == pFormatEtc->dwAspect;
			});
	}
public:
	~CDataObject();
	// IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
	// IDataObject
	HRESULT STDMETHODCALLTYPE GetData(FORMATETC* pFormatEtc, STGMEDIUM* pmedium);
	HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pmedium);
	HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC* pFormatEtc);
	HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC* pFormatEct, FORMATETC* pFormatEtcOut);
	HRESULT STDMETHODCALLTYPE SetData(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium, BOOL fRelease);
	HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc);
	HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC* pFormatEtc, DWORD advf, IAdviseSink*, DWORD*);
	HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection);
	HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);
};