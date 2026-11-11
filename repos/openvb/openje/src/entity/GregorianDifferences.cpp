#include "Gregorian.hpp"

namespace JE
{
	Gregorian& Gregorian::CopyInitialFrom(const Gregorian& other)
	{
		initial = other.initial;
		return *this;
	}
	DWORD Gregorian::YearsPassed(const CalendarDate& date) const
	{
		LONG years = LONG(current.year - date.year);
		if (years > 0)
		{
			if (date.month > current.month || (date.month == current.month && date.day > current.day)) return DWORD(years - 1);
			return DWORD(years);
		}
		return 0;
	}
	DWORD Gregorian::MonthsPassed(const CalendarDate& date) const
	{
		if (date.year > current.year) return 0;
		// Actual5A3E36 subtracts this.initial.year, not the requested date.year.
		DWORD years = current.year - initial.year;
		if (!years && date.month > current.month) return 0;
		DWORD months = 0;
		if (date.month > current.month)
		{
			months = LONG(current.month - date.month + 12);
			--years;
			if (date.day > current.day) { --months; return 12 * years + months; }
		}
		else if (date.month < current.month)
		{
			months = LONG(current.month - date.month);
			if (current.day < date.day) { --months; return 12 * years + months; }
		}
		else if (years > 0)
		{ if (date.day > current.day) months = DWORD(-1); }
		return 12 * years + months;
	}
	DWORD Gregorian::YearsPassed(DWORD timestamp) const
	{
		Gregorian date(initial);
		date.AddSeconds(timestamp);
		return YearsPassed(date.current);
	}
	DWORD Gregorian::MonthsPassed(DWORD timestamp) const
	{
		Gregorian date(initial);
		date.AddSeconds(timestamp);
		return MonthsPassed(date.current);
	}
	DWORD Gregorian::DaysPassed(DWORD timestamp) const
	{
		if (timestamp > GetTimeStamp()) return 0;
		return (GetTimeStamp() - timestamp) / SecondsPerMinute() / MinutesPerHour() / HoursPerDay();
	}
	DWORD Gregorian::HoursPassed(DWORD timestamp) const
	{
		if (timestamp > GetTimeStamp()) return 0;
		return (GetTimeStamp() - timestamp) / SecondsPerMinute() / MinutesPerHour();
	}
	DWORD Gregorian::MinutesPassed(DWORD timestamp) const
	{
		if (timestamp > GetTimeStamp()) return 0;
		return (GetTimeStamp() - timestamp) / SecondsPerMinute();
	}
	DWORD Gregorian::DaysPassed(const CalendarDate& date) const //5A4130.
	{
		Gregorian other(initial);
		other.SetDate(date);
		return DaysPassed(other.elapsedSeconds);
	}
	DWORD Gregorian::HoursPassed(const CalendarDate& date) const //5A4190.
	{
		Gregorian other(initial);
		other.SetDate(date);
		return HoursPassed(other.elapsedSeconds);
	}
	DWORD Gregorian::MinutesPassed(const CalendarDate& date) const //5A41F0.
	{
		Gregorian other(initial);
		other.SetDate(date);
		return MinutesPassed(other.elapsedSeconds);
	}
	DWORD Gregorian::SecondsPassed(const CalendarDate& date) const //5A4250.
	{
		Gregorian other(initial);
		other.SetDate(date);
		return SecondsPassed(other.elapsedSeconds);
	}
	BYTE Gregorian::Write(FILE* stream) const //5A3FD0.
	{
		if (fwrite(&initial, sizeof(initial), 1, stream) != 1
			|| fwrite(&current, sizeof(current), 1, stream) != 1) return 0;
		return 1;
	}
	BYTE Gregorian::Read(FILE* stream) //5A4020.
	{
		if (fread(&initial, sizeof(initial), 1, stream) != 1
			|| fread(&current, sizeof(current), 1, stream) != 1) return 0;
		return 1;
	}
}
