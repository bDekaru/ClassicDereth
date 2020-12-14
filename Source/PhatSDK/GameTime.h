
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
    uint32_t is_night;
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
    uint32_t begin;
};

class GameTime
{
public:
    GameTime();
    ~GameTime();

    void Destroy();
    BOOL UnPack(BYTE **ppData, ULONG& iSize);

    void UseTime();
	void CalcDayBegin(double current);
	void CalcTimeOfDay(double current);

    double zero_time_of_year;
    uint32_t zero_year;
    float day_length;
    uint32_t days_per_year;
    SmartArray<TimeOfDay *>    times_of_day;
    SmartArray<WeekDay *>    days_of_the_week;
    SmartArray<Season *>    seasons;
    PString year_spec;
    double year_length;
    float present_time_of_day;
    double time_of_day_begin;
    double time_of_next_event;
	float present_time_in_day_unit;
    uint32_t current_year;
    uint32_t current_day;
    uint32_t current_season;
    uint32_t current_week_day;
    uint32_t current_time_of_day;
    double clock_offset;
    double time_zero_start_delta;
	double last_cycle_change;

    static GameTime *current_game_time;
    static double global_next_event;

	const static float DayStart;
	const static float NightStart;

	//static bool IsNight() { return current_game_time->times_of_day.array_data[current_game_time->current_time_of_day]->is_night == 1; }
	static bool IsNight() { return current_game_time->present_time_of_day < DayStart || current_game_time->present_time_of_day >= NightStart; }
	static double LastCycleChange() { return current_game_time->last_cycle_change; }
};
