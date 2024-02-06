#include "CWndBK.h"

void CUIWaves::GetWavesData()
{
	std::thread Thread([this]()
		{
			CBass Bass;
			Bass.Open(App->GetPlayer().GetCurrFileName().Data(), BASS_STREAM_DECODE, BASS_MUSIC_DECODE | BASS_MUSIC_PRESCAN);
			if (!Bass.GetHStream())
			{
				EckDbgPrint(CBass::GetErrorMsg(CBass::GetError()));
				m_ThreadState = ThreadState::Error;
				return;
			}
			ULONGLONG ullLength = (ULONGLONG)(Bass.GetLength() * 1000.);

			size_t cBars = ullLength / 20ull;
			m_vWavesData.resize(cBars);

			EckCounter(cBars, i)
			{
				m_vWavesData[i] = Bass.GetLevel();
				if (WaitForSingleObject(m_hEvent, 0) == WAIT_OBJECT_0)
				{
					m_vWavesData.clear();
					m_ThreadState = ThreadState::Stopped;
					return;
				}
			}

			m_ThreadState = ThreadState::Stopped;
		});
	DuplicateHandle(GetCurrentProcess(), Thread.native_handle(), GetCurrentProcess(), &m_hThread, GENERIC_ALL, FALSE, 0);
	Thread.detach();
}

CUIWaves::CUIWaves()
{
	m_uType = UIET_WAVES;
	m_uFlags = UIEF_NOEVENT | UIEF_WANTTIMEREVENT | UIEF_ONLYPAINTONTIMER;
	m_hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
}

BOOL CUIWaves::InitElem()
{
	eck::SafeRelease(m_pBrLine);
	eck::SafeRelease(m_pBrCenterMark);
	auto pDC = m_pBK->m_pDC;
	pDC->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pBrLine);
	pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pBrCenterMark);
	App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		m_pBK->Dpi(18.f), L"zh-cn", &m_pTfTip);
	m_pTfTip->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	m_pTfTip->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);// 水平垂直都居中
	return TRUE;
}

LRESULT CUIWaves::OnElemEvent(UIELEMEVENT uEvent, WPARAM wParam, LPARAM lParam)
{
	auto lResult = DefElemEventProc(uEvent, wParam, lParam);
	switch (uEvent)
	{
	case UIEE_ONPLAYINGCTRL:
	{
		switch (wParam)
		{
		case PCT_PLAY:
			m_ThreadState = ThreadState::Running;
			CloseHandle(m_hThread);
			ResetEvent(m_hEvent);
			GetWavesData();
			break;
		case PCT_STOP:
			if (m_hThread)
			{
				SetEvent(m_hEvent);
				WaitForSingleObject(m_hThread, INFINITE);
				CloseHandle(m_hThread);
				m_hThread = NULL;
			}
			break;
		}
	}
	break;
	}
	return lResult;
}

CUIWaves::~CUIWaves()
{
	m_pBrLine->Release();
	m_pBrCenterMark->Release();
	m_pTfTip->Release();
	CloseHandle(m_hThread);
	CloseHandle(m_hEvent);
}

void CUIWaves::Redraw()
{
	auto pDC = m_pBK->m_pDC;
	pDC->DrawBitmap(m_pBK->m_pBmpBKStatic, &m_rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcF);// 刷背景

	PCWSTR pszText = NULL;

	if (m_hThread && WaitForSingleObject(m_hThread, 0) != WAIT_OBJECT_0)// 正在加载
		pszText = L"正在加载...";
	else if (!App->GetPlayer().GetBass().GetHStream())// 已停止
		pszText = L"未播放";
	else if (m_ThreadState == ThreadState::Error)// 出错
		pszText = L"错误！";

	if (pszText)// 应当显示提示
	{
		pDC->DrawTextW(pszText, (int)wcslen(pszText), m_pTfTip, &m_rcF, m_pBrLine);
		BkDbg_DrawElemFrame();
		return;
	}

	const int idxCurr = (int)(App->GetPlayer().GetPos() / 20ull);// 算数组索引    20ms一单位
	const int cBars = (int)m_vWavesData.size();
	if (idxCurr > cBars - 1)
		return;

	int i = idxCurr;
	int x = m_rc.left + m_cxHalf,
		y = m_rc.top + m_cyHalf;
	D2D1_POINT_2F PtF1, PtF2;

	pDC->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	pDC->PushAxisAlignedClip(&m_rcF, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	// 上面是右声道，下面是左声道
	while (TRUE)// 向右画
	{
		PtF1 = { (float)x, (float)(y - HIWORD(m_vWavesData[i]) * m_cyHalf / 32768) };
		PtF2 = { (float)x, (float)(y + LOWORD(m_vWavesData[i]) * m_cyHalf / 32768) };
		pDC->DrawLine(PtF1, PtF2, m_pBrLine, (FLOAT)m_cxLine);
		x += m_cxLine;
		i++;
		if (i > cBars - 1 || x >= m_rc.right)
			break;
	}
	i = idxCurr;
	x = m_rc.left + m_cxHalf;
	while (TRUE)// 向左画
	{
		PtF1 = { (FLOAT)x, (FLOAT)(y - HIWORD(m_vWavesData[i]) * m_cyHalf / 32768) };
		PtF2 = { (FLOAT)x, (FLOAT)(y + LOWORD(m_vWavesData[i]) * m_cyHalf / 32768) };
		pDC->DrawLine(PtF1, PtF2, m_pBrLine, (FLOAT)m_cxLine);
		x -= m_cxLine;
		i--;
		if (i < 0 || x < m_rc.left)
			break;
	}
	x = m_rc.left + m_cxHalf;

	PtF1 = { (FLOAT)x, m_rcF.top };
	PtF2 = { (FLOAT)x, m_rcF.bottom };
	pDC->DrawLine(PtF1, PtF2, m_pBrCenterMark, (FLOAT)m_pBK->m_DsF.cxWaveLine);

	pDC->PopAxisAlignedClip();
	pDC->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	BkDbg_DrawElemFrame();
}