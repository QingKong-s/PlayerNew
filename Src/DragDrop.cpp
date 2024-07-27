#include "pch.h"
#include "DragDrop.h"

static HRESULT CopyHGlobal(HGLOBAL hGlobalDst, HGLOBAL hGlobalSrc)
{
	void* pDst = GlobalLock(hGlobalDst);
	if (!pDst)
		return E_UNEXPECTED;

	void* pSrc = GlobalLock(hGlobalSrc);
	if (!pSrc)
	{
		GlobalUnlock(hGlobalDst);
		return E_UNEXPECTED;
	}

	memcpy(pDst, pSrc, std::min(GlobalSize(hGlobalSrc), GlobalSize(hGlobalDst)));
	GlobalUnlock(hGlobalSrc);
	GlobalUnlock(hGlobalDst);
	return S_OK;
}

static HGLOBAL DupHGlobal(HGLOBAL hGlobalSrc, HRESULT& hr)
{
	HGLOBAL hGlobalDst = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, GlobalSize(hGlobalSrc));
	if (!hGlobalDst)
	{
		hr = E_OUTOFMEMORY;
		return NULL;
	}

	hr = CopyHGlobal(hGlobalDst, hGlobalSrc);
	if (FAILED(hr))
	{
		GlobalFree(hGlobalDst);
		return NULL;
	}
	return hGlobalDst;
}



ULONG STDMETHODCALLTYPE CDropSource::AddRef(void)
{
	return ++m_uRefCount;
}

ULONG STDMETHODCALLTYPE CDropSource::Release(void)
{
	if (m_uRefCount == 1)
	{
		delete this;
		return 0;
	}
	return --m_uRefCount;
}



ULONG STDMETHODCALLTYPE CDropTarget::AddRef(void)
{
	return ++m_uRefCount;
}

ULONG STDMETHODCALLTYPE CDropTarget::Release(void)
{
	if (m_uRefCount == 1)
	{
		delete this;
		return 0;
	}
	return --m_uRefCount;
}



CDataObject::~CDataObject()
{
	for (auto& x : m_vStgMedium)
		ReleaseStgMedium(&x);
}

HRESULT STDMETHODCALLTYPE CDataObject::QueryInterface(REFIID iid, void** ppvObject)
{
	const static QITAB qit[]
	{
		QITABENT(CDataObject, IDataObject),
		{}
	};

	return QISearch(this, qit, iid, ppvObject);
}

ULONG STDMETHODCALLTYPE CDataObject::AddRef(void)
{
	return ++m_uRefCount;
}

ULONG STDMETHODCALLTYPE CDataObject::Release(void)
{
	if (m_uRefCount == 1)
	{
		delete this;
		return 0;
	}
	return --m_uRefCount;
}

HRESULT STDMETHODCALLTYPE CDataObject::QueryGetData(FORMATETC* pFormatEtc)
{
	return (FindFormat(pFormatEtc) == m_vFormatEtc.end()) ? DV_E_FORMATETC : S_OK;
}

HRESULT STDMETHODCALLTYPE CDataObject::GetData(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium)
{
	auto it = FindFormat(pFormatEtc);
	if (it == m_vFormatEtc.end())
		return DV_E_FORMATETC;
	auto itStg = m_vStgMedium.begin() + std::distance(m_vFormatEtc.begin(), it);

	switch (pFormatEtc->tymed)
	{
	case TYMED_HGLOBAL:
	{
		HGLOBAL hGlobalSrc = itStg->hGlobal;
		if (!hGlobalSrc)
			return E_UNEXPECTED;
		HRESULT hr;
		HGLOBAL hGlobal = DupHGlobal(hGlobalSrc, hr);
		if (hr != S_OK)
			return hr;
		*pStgMedium = *itStg;
		pStgMedium->hGlobal = hGlobal;
	}
	return S_OK;
	}
	return DV_E_FORMATETC;
}

HRESULT STDMETHODCALLTYPE CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc)
{
	return SHCreateStdEnumFmtEtc((UINT)m_vFormatEtc.size(), m_vFormatEtc.data(), ppEnumFormatEtc);
}

HRESULT STDMETHODCALLTYPE CDataObject::DAdvise(FORMATETC* pFormatEtc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT STDMETHODCALLTYPE CDataObject::DUnadvise(DWORD dwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT STDMETHODCALLTYPE CDataObject::EnumDAdvise(IEnumSTATDATA** ppEnumAdvise)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT STDMETHODCALLTYPE CDataObject::GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium)
{
	auto it = FindFormat(pFormatEtc);
	if (it == m_vFormatEtc.end())
		return DV_E_FORMATETC;
	auto itStg = m_vStgMedium.begin() + std::distance(m_vFormatEtc.begin(), it);
	HRESULT hr;

	switch (pFormatEtc->tymed)
	{
	case TYMED_HGLOBAL:
	{
		if (!itStg->hGlobal)
			return E_UNEXPECTED;
		hr = CopyHGlobal(pStgMedium->hGlobal, itStg->hGlobal);
	}
	return hr;
	}
	return DV_E_FORMATETC;
}

HRESULT STDMETHODCALLTYPE CDataObject::GetCanonicalFormatEtc(FORMATETC* pFormatEct, FORMATETC* pFormatEtcOut)
{
	*pFormatEtcOut = {};
	return DATA_S_SAMEFORMATETC;
}

HRESULT STDMETHODCALLTYPE CDataObject::SetData(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium, BOOL fRelease)
{
	if (pStgMedium->tymed != TYMED_HGLOBAL)
		return DV_E_TYMED;
	if (!pStgMedium->hGlobal)
		return E_UNEXPECTED;

	HGLOBAL hGlobal;
	if (fRelease)
		hGlobal = pStgMedium->hGlobal;
	else
	{
		HRESULT hr;
		hGlobal = DupHGlobal(pStgMedium->hGlobal, hr);
		if (hr != S_OK)
			return hr;
	}

	auto it = FindFormat(pFormatEtc);
	if (it == m_vFormatEtc.end())
	{
		m_vFormatEtc.emplace_back(*pFormatEtc);
		m_vStgMedium.emplace_back(*pStgMedium);
	}
	else
	{
		auto itStg = m_vStgMedium.begin() + std::distance(m_vFormatEtc.begin(), it);
		ReleaseStgMedium(&(*itStg));
		*itStg = *pStgMedium;
		itStg->hGlobal = hGlobal;
	}
	return S_OK;
}