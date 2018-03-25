
//
// GameTime.cpp
//

#include "StdAfx.h"
#include "LegacyPackObj.h"
#include "PhysicsObj.h"
#include "GameTime.h"

GameTime *GameTime::current_game_time = NULL;
double GameTime::global_next_event = 0.0;

TimeOfDay::TimeOfDay()
{
    begin = 0;
}

BOOL TimeOfDay::UnPack(BYTE **ppData, ULONG& iSize)
{
    UNPACK(float, begin);
    UNPACK(int, is_night);
    UNPACK_OBJ(time_of_day_name);

    DWORD StrPackSize = 2 + ((((DWORD)time_of_day_name.m_str.length()) >= 0xFFFF)?4:0) + ((DWORD)time_of_day_name.m_str.length());
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

    DWORD StrPackSize = 2 + ((((DWORD)week_day_name.m_str.length()) >= 0xFFFF)?4:0) + ((DWORD)week_day_name.m_str.length());
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
    UNPACK(DWORD, begin);
    UNPACK_OBJ(season_name);

    DWORD StrPackSize = 2 + ((((DWORD)season_name.m_str.length()) >= 0xFFFF)?4:0) + ((DWORD)season_name.m_str.length());
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
	for (DWORD i = 0; i < seasons.num_used; i++)
	{
		delete seasons.array_data[i];
	}
	seasons.num_used = 0;

	for (DWORD i = 0; i < times_of_day.num_used; i++)
	{
		delete times_of_day.array_data[i];
	}
	times_of_day.num_used = 0;

	for (DWORD i = 0; i < days_of_the_week.num_used; i++)
	{
		delete days_of_the_week.array_data[i];
	}
	days_of_the_week.num_used = 0;
}

BOOL GameTime::UnPack(BYTE **ppData, ULONG& iSize)
{
    Destroy();

    UNPACK(double, zero_time_of_year);
    UNPACK(DWORD, zero_year);
    UNPACK(float, day_length);
    UNPACK(DWORD, days_per_year);

    year_length = day_length * days_per_year;
    UNPACK_OBJ(year_spec);

    DWORD StrPackSize = 2 + ((((DWORD)year_spec.m_str.length()) >= 0xFFFF)?4:0) + ((DWORD)year_spec.m_str.length());
    if (StrPackSize & 3)
        StrPackSize += 4 - (StrPackSize & 3);
    iSize -= StrPackSize;

    PACK_ALIGN();

    DWORD NumTODs;
    UNPACK(DWORD, NumTODs);

    for (DWORD i = 0; i < NumTODs; i++)
    {
        TimeOfDay *TOD = new TimeOfDay;

        if (!UNPACK_POBJ(TOD))
            return FALSE;

        times_of_day.add(&TOD);
    }

    DWORD NumDays;
    UNPACK(DWORD, NumDays);

    for (DWORD i = 0; i < NumDays; i++)
    {
        WeekDay *WD = new WeekDay;

        if (!UNPACK_POBJ(WD))
            return FALSE;

        days_of_the_week.add(&WD);
    }

    DWORD NumSeasons;
    UNPACK(DWORD, NumSeasons);

    for (DWORD i = 0; i < NumSeasons; i++)
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
    double var_8 = Timer::cur_time + clock_offset + time_zero_start_delta;

    if (time_of_day_begin < 0.0)
        CalcDayBegin(var_8);

    present_time_of_day = (var_8 - time_of_day_begin) / day_length;

    if (var_8 >= time_of_next_event)
    {
        if (present_time_of_day >= 0.0f)
        {
            CalcDayBegin(var_8);
            present_time_of_day = (var_8 - time_of_day_begin) / day_length;
        }

        CalcTimeOfDay(var_8);
        global_next_event = (time_of_next_event - clock_offset) + time_zero_start_delta;
    }
}

void GameTime::CalcDayBegin(double arg_0)
{
    UNFINISHED_LEGACY("GameTime::CalcDayBegin");
}

void GameTime::CalcTimeOfDay(double arg_0)
{
    UNFINISHED_LEGACY("GameTime::CalcTimeOfDay");
}






