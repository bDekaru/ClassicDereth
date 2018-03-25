
#include "StdAfx.h"
#include "PhatSDK.h"

CPhatSDKImpl *g_pPhatSDK = NULL;

double Timer::cur_time = 0.0;
double TIME_SKEW = 1.0;

void CPhatSDKImpl::UpdateInternalTime()
{
	Timer::cur_time = GetCurrTime() * TIME_SKEW;
}
