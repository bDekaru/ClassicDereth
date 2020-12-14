
//
// GameTime.cpp
//

#include <StdAfx.h>
#include "LegacyPackObj.h"
#include "PhysicsObj.h"
#include "GameTime.h"

GameTime *GameTime::current_game_time = NULL;
double GameTime::global_next_event = 0.0;
const float GameTime::DayStart = 0.25f;
const float GameTime::NightStart = 0.75f;

TimeOfDay::TimeOfDay()
{
	begin = 0;
}

BOOL TimeOfDay::UnPack(BYTE **ppData, ULONG& iSize)
{
	UNPACK(float, begin);
	UNPACK(int, is_night);
	UNPACK_OBJ(time_of_day_name);

	uint32_t StrPackSize = 2 + ((((uint32_t)time_of_day_name.m_str.length()) >= 0xFFFF) ? 4 : 0) + ((uint32_t)time_of_day_name.m_str.length());
	if (StrPackSize & 3)
		StrPackSize += 4 - (StrPackSize & 3);
	iSize -= StrPackSize;

	PACK_ALIGN();

	return((((signed)iSize) >= 0) ? TRUE : FALSE);
}

WeekDay::WeekDay()
{
}

BOOL WeekDay::UnPack(BYTE **ppData, ULONG& iSize)
{
	UNPACK_OBJ(week_day_name);

	uint32_t StrPackSize = 2 + ((((uint32_t)week_day_name.m_str.length()) >= 0xFFFF) ? 4 : 0) + ((uint32_t)week_day_name.m_str.length());
	if (StrPackSize & 3)
		StrPackSize += 4 - (StrPackSize & 3);
	iSize -= StrPackSize;

	PACK_ALIGN();

	return((((signed)iSize) >= 0) ? TRUE : FALSE);
}

Season::Season()
{
	begin = 0;
}

BOOL Season::UnPack(BYTE **ppData, ULONG& iSize)
{
	UNPACK(uint32_t, begin);
	UNPACK_OBJ(season_name);

	uint32_t StrPackSize = 2 + ((((uint32_t)season_name.m_str.length()) >= 0xFFFF) ? 4 : 0) + ((uint32_t)season_name.m_str.length());
	if (StrPackSize & 3)
		StrPackSize += 4 - (StrPackSize & 3);
	iSize -= StrPackSize;

	PACK_ALIGN();

	return((((signed)iSize) >= 0) ? TRUE : FALSE);
}

GameTime::GameTime()
{
	zero_time_of_year = 0.0;
	zero_year = 0;
	day_length = 0;
	days_per_year = 0;
	year_spec = "";
	year_length = 0.0;
	present_time_of_day = 0;
	time_of_day_begin = INVALID_TIME;
	time_of_next_event = 0;
	current_year = 0;
	current_day = 0;
	current_season = 0;
	current_week_day = 0;
	current_time_of_day = 0;
	clock_offset = 0;
	time_zero_start_delta = 0;
	last_cycle_change = 0;
}

GameTime::~GameTime()
{
	Destroy();
}

void GameTime::Destroy()
{
	zero_time_of_year = 0.0;
	zero_year = 0;
	day_length = 0;
	days_per_year = 0;
	year_length = 0.0;
	present_time_of_day = 0;
	time_of_day_begin = INVALID_TIME;
	time_of_next_event = 0;
	current_year = 0;
	current_day = 0;
	current_season = 0;
	current_week_day = 0;
	current_time_of_day = 0;
	clock_offset = 0;

	// not exact but close enough
	for (uint32_t i = 0; i < seasons.num_used; i++)
	{
		delete seasons.array_data[i];
	}
	seasons.num_used = 0;

	for (uint32_t i = 0; i < times_of_day.num_used; i++)
	{
		delete times_of_day.array_data[i];
	}
	times_of_day.num_used = 0;

	for (uint32_t i = 0; i < days_of_the_week.num_used; i++)
	{
		delete days_of_the_week.array_data[i];
	}
	days_of_the_week.num_used = 0;
}

BOOL GameTime::UnPack(BYTE **ppData, ULONG& iSize)
{
	Destroy();

	UNPACK(double, zero_time_of_year);
	UNPACK(uint32_t, zero_year);
	UNPACK(float, day_length);
	UNPACK(uint32_t, days_per_year);

	year_length = day_length * days_per_year;
	UNPACK_OBJ(year_spec);

	uint32_t StrPackSize = 2 + ((((uint32_t)year_spec.m_str.length()) >= 0xFFFF) ? 4 : 0) + ((uint32_t)year_spec.m_str.length());
	if (StrPackSize & 3)
		StrPackSize += 4 - (StrPackSize & 3);
	iSize -= StrPackSize;

	PACK_ALIGN();

	uint32_t NumTODs;
	UNPACK(uint32_t, NumTODs);

	for (uint32_t i = 0; i < NumTODs; i++)
	{
		TimeOfDay *TOD = new TimeOfDay;

		if (!UNPACK_POBJ(TOD))
			return FALSE;

		times_of_day.add(&TOD);
	}

	uint32_t NumDays;
	UNPACK(uint32_t, NumDays);

	for (uint32_t i = 0; i < NumDays; i++)
	{
		WeekDay *WD = new WeekDay;

		if (!UNPACK_POBJ(WD))
			return FALSE;

		days_of_the_week.add(&WD);
	}

	uint32_t NumSeasons;
	UNPACK(uint32_t, NumSeasons);

	for (uint32_t i = 0; i < NumSeasons; i++)
	{
		Season *S = new Season;

		if (!UNPACK_POBJ(S))
			return FALSE;

		seasons.add(&S);
	}

	if (((signed)iSize) < 0)
		return FALSE;

	return TRUE;
}

void GameTime::UseTime()
{
	double current = Timer::cur_time + clock_offset + time_zero_start_delta;

	if (time_of_day_begin < 0.0)
		CalcDayBegin(current);

	present_time_of_day = (current - time_of_day_begin) / day_length;

	if (current >= time_of_next_event)
	{
		if (present_time_of_day >= 1.0f)
		{
			CalcDayBegin(current);
			present_time_of_day = (current - time_of_day_begin) / day_length;
		}

		CalcTimeOfDay(current);
		global_next_event = (time_of_next_event - clock_offset) + time_zero_start_delta;

		if (present_time_of_day >= DayStart)
		{
			if (present_time_of_day >= NightStart)
			{
				last_cycle_change = time_of_day_begin + (day_length * NightStart);
			}
			else
			{
				last_cycle_change = time_of_day_begin + (day_length * DayStart);
			}
		}
	}
}

void GameTime::CalcDayBegin(double current)
{
	double total = current + zero_time_of_year;
	double yo = floor(total / year_length);
	current_year = uint32_t(yo) + zero_year;

	double diy = total - yo * year_length;
	current_day = uint32_t(diy / day_length);

	time_of_day_begin = current - (diy - current_day * day_length);
}

void GameTime::CalcTimeOfDay(double current)
{
	double doff = (current - time_of_day_begin) / double(day_length);

	int di = 0;
	for (di = 0; di < times_of_day.num_used - 1; di++)
	{
		if (doff < double(times_of_day.array_data[di + 1]->begin))
			break;
	}

	current_time_of_day = di;

	if (di == times_of_day.num_used - 1)
	{
		// end of day
		time_of_next_event = time_of_day_begin + day_length;
	}
	else
	{
		double next_start = times_of_day.array_data[di + 1]->begin * day_length;
		time_of_next_event = time_of_day_begin + next_start;
	}
}






