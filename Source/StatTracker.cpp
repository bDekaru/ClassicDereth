
#include <StdAfx.h>
#include "StatTracker.h"

CStatTracker::CStatTracker()
{
	_frameRateCountLast = 0;
	Reset(g_pGlobals->m_last);
}

void CStatTracker::Reset(time_point &now)
{
	_frameStart = now;
	_frameNext = _frameStart + seconds(1);
	_frameRateCount = 0;
}

void CStatTracker::StartServerFrame()
{
	time_point now = g_pGlobals->m_last;
	if (_frameNext < now)
	{
		std::chrono::duration<double> elapsed = now - _frameStart;
		_frameRateCountLast = static_cast<uint32_t>(_frameRateCount / elapsed.count());
		Reset(now);
	}
}

void CStatTracker::EndServerFrame()
{
	_frameRateCount++;
}

void CStatTracker::UpdateClientList(class CClient **clients, uint32_t maxRange)
{
}

