
#pragma once

class CStatTracker
{
public:
	CStatTracker();

	void StartServerFrame();
	void EndServerFrame();

	void UpdateClientList(class CClient **clients, uint32_t maxRange);

private:
	time_point _frameStart;
	time_point _frameNext;
	uint32_t _frameRateCount;
	uint32_t _frameRateCountLast;

	void Reset(time_point &now);
};
