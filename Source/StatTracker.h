
#pragma once

class CStatTracker
{
public:
	CStatTracker();

	void StartServerFrame();
	void EndServerFrame();

	void UpdateClientList(class CClient **clients, DWORD maxRange);

private:
	UINT64 _frameRatePeriodStart;
	UINT64 _nextFrameRatePeriodEnd;
	UINT64 _frameRateCount;
	UINT64 _frameRateCountLastPeriod;
};
