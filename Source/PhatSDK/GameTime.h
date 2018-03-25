
#pragma once
#include "PString.h"
#include "SmartArray.h"

class TimeOfDay
{
public:
    TimeOfDay();
	
	BOOL UnPack(BYTE **ppData, ULONG& iSize);

    PString time_of_day_name;
    float begin;
    DWORD is_night;
};

class WeekDay
{
public:
    WeekDay();

    BOOL UnPack(BYTE **ppData, ULONG& iSize);

    PString week_day_name;
};

class Season
{
public:
    Season();

    BOOL UnPack(BYTE **ppData, ULONG& iSize);

    PString season_name;
    DWORD begin;
};

class GameTime
{
public:
    GameTime();
    ~GameTime();

    void Destroy();
    BOOL UnPack(BYTE **ppData, ULONG& iSize);

    void UseTime();
    void CalcDayBegin(double arg_0);
    void CalcTimeOfDay(double arg_0);

    double zero_time_of_year;
    DWORD zero_year;
    float day_length;
    DWORD days_per_year;
    SmartArray<TimeOfDay *>    times_of_day;
    SmartArray<WeekDay *>    days_of_the_week;
    SmartArray<Season *>    seasons;
    PString year_spec;
    double year_length;
    float present_time_of_day;
    double time_of_day_begin;
    double time_of_next_event;
	float present_time_in_day_unit;
    DWORD current_year;
    DWORD current_day;
    DWORD current_season;
    DWORD current_week_day;
    DWORD current_time_of_day;
    double clock_offset;
    double time_zero_start_delta;

    static GameTime *current_game_time;
    static double global_next_event;
};
