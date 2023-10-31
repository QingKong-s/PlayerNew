#include "CWndBK.h"

LRESULT CUIElem::DefElemEventProc(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
	switch (uEvent)
	{
	case UIEE_SETRECT:
	{
		auto prc = (RECT*)wParam;
		//---------相对位置
		m_rcRelative = *prc;
		//---------绝对位置
		m_rc = *prc;
		if (m_pParent)
			OffsetRect(&m_rc, m_pParent->m_rc.left, m_pParent->m_rc.top);
		// 取D2D矩形
		m_rcF = eck::MakeD2DRcF(m_rc);
		// 计算尺寸
		m_cx = m_rc.right - m_rc.left;
		m_cy = m_rc.bottom - m_rc.top;
		m_cxHalf = m_cx / 2;
		m_cyHalf = m_cy / 2;
		// 计算窗口范围内的矩形
		m_rcInWnd = m_rc;
		if (m_rcInWnd.left < 0)
			m_rcInWnd.left = 0;
		if (m_rcInWnd.top < 0)
			m_rcInWnd.top = 0;
		if (m_rcInWnd.right > m_pBK->m_cxClient)
			m_rcInWnd.right = m_pBK->m_cxClient;
		if (m_rcInWnd.bottom > m_pBK->m_cyClient)
			m_rcInWnd.bottom = m_pBK->m_cyClient;
		//---------更新子窗口的绝对位置
		RECT rc;
		for (auto pElem : m_vChildren)
		{
			pElem->GetElemRect(&rc);
			pElem->SetElemRect(&rc);
		}
	}
	return 0;
	}
	return 0;
}

CUIElem::~CUIElem()
{
}

void CUIElem::SetParent(CUIElem* pParent)
{
	if (m_pParent == pParent)
		return;
	if (m_pParent)
	{
		auto it = std::find(m_pParent->m_vChildren.begin(), m_pParent->m_vChildren.end(), this);
		if (it != m_pParent->m_vChildren.end())
			m_pParent->m_vChildren.erase(it);
	}
	else
	{
		auto it = std::find(m_pBK->m_vElems.begin(), m_pBK->m_vElems.end(), this);
		if (it != m_pBK->m_vElems.end())
			m_pBK->m_vElems.erase(it);
	}
	m_pParent = pParent;
	if (m_pParent)
		m_pParent->m_vChildren.emplace_back(this);
	else
		m_pBK->m_vElems.emplace_back(this);
	RECT rc;
	GetElemRect(&rc);
	SetElemRect(&rc);
}