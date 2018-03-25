
#include "StdAfx.h"
#include "StatTracker.h"

CStatTracker::CStatTracker()
{
	_frameRatePeriodStart = g_pGlobals->m_CounterTime;
	_nextFrameRatePeriodEnd = _frameRatePeriodStart + g_pGlobals->m_CounterFreq;
	_frameRateCount = 0;
	_frameRateCountLastPeriod = 0;
}

void CStatTracker::StartServerFrame()
{
	if (_nextFrameRatePeriodEnd < g_pGlobals->m_CounterTime)
	{
		double periodSeconds = (g_pGlobals->m_CounterTime - _frameRatePeriodStart) / (double)g_pGlobals->m_CounterFreq;
		_frameRateCountLastPeriod = (UINT64) (_frameRateCountLastPeriod / periodSeconds);

		_frameRatePeriodStart = g_pGlobals->m_CounterTime;
		_nextFrameRatePeriodEnd = _frameRatePeriodStart + g_pGlobals->m_CounterFreq;
		_frameRateCount = 0;
	}
}

void CStatTracker::EndServerFrame()
{
	_frameRateCount++;
}

void CStatTracker::UpdateClientList(class CClient **clients, DWORD maxRange)
{
}

