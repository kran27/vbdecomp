#include "Gregorian.hpp"

namespace JE
{
	Gregorian::Gregorian(const CalendarDate& start) : ICalendar(start), elapsedSeconds(0)
	{
		SetDate(start);
	}
	DWORD Gregorian::DaysInMonth(DWORD month) const
	{
		static const BYTE days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
		return days[month];
	}
	CalendarDate* Gregorian::GetDate() { return &current; } //5449D0.
	DWORD Gregorian::GetTimeStamp() const { return elapsedSeconds; }
	DWORD Gregorian::GetYear() const { return current.year; } //5C01A0.
	DWORD Gregorian::GetMonth() const { return current.month; } //4080C0.
	DWORD Gregorian::GetDay() const { return current.day; } //544A00.
	DWORD Gregorian::GetHour() const { return current.hour; } //544A10.
	DWORD Gregorian::GetMinute() const { return current.minute; } //424A10.
	DWORD Gregorian::GetSecond() const { return current.second; } //41D1B0.
	DWORD Gregorian::MonthsPerYear() const { return 12; } //544A20.
	DWORD Gregorian::HoursPerDay() const { return 24; }
	DWORD Gregorian::MinutesPerHour() const { return 60; }
	DWORD Gregorian::SecondsPerMinute() const { return 60; }
	const char* Gregorian::FormatDate() const //5A3960.
	{
		static const char* const months[] = {
			"January", "February", "March", "April", "May", "June", "July", "August",
			"September", "October", "November", "December"
		};
		//Ordinary storage for the original shared formatting result. Eighty bytes
		//cover the format even when all six DWORD date fields have ten digits.
		static char text[80];
		sprintf(text, "%s %u, %u  %uh%02um%02us", months[current.month - 1],
			current.day, current.year, current.hour, current.minute, current.second);
		return text;
	}
	void Gregorian::SetTimeStamp(DWORD seconds)
	{
		current=initial;
		AddSeconds(seconds); //5449E0 does not reset existing elapsedSeconds.
	}
	DWORD Gregorian::SecondsPassed(DWORD timestamp) const
	{
		if (timestamp > GetTimeStamp()) return 0;
		return GetTimeStamp() - timestamp;
	}
	BYTE Gregorian::IsLeapYear(DWORD year) const
	{
		return !(year & 3) && (year % 100 || !(year % 400));
	}
	void Gregorian::SetDate(const CalendarDate& date)
	{
		current = initial;
		elapsedSeconds = 0;
		if (date.year > current.year)
		{
			AddYears(date.year - current.year - 1);
			AddDays(DaysInMonth(current.month) - current.day);
			while (current.month < 12) AddDays(DaysInMonth(current.month + 1));
			AddHours(23 - current.hour);
			AddMinutes(59 - current.minute);
			AddSeconds(60 - current.second);
		}
		if (date.month > current.month)
		{
			AddDays(DaysInMonth(current.month) - current.day);
			while (current.month < date.month - 1) AddDays(DaysInMonth(current.month + 1));
			AddHours(23 - current.hour);
			AddMinutes(59 - current.minute);
			AddSeconds(60 - current.second);
		}
		if (date.day > current.day)
		{
			AddDays(date.day - current.day - 1);
			AddHours(23 - current.hour);
			AddMinutes(59 - current.minute);
			AddSeconds(60 - current.second);
		}
		if (date.hour > current.hour)
		{
			AddHours(date.hour - current.hour - 1);
			AddMinutes(59 - current.minute);
			AddSeconds(60 - current.second);
		}
		if (date.minute > current.minute)
		{
			AddMinutes(date.minute - current.minute - 1);
			AddSeconds(60 - current.second);
		}
		if (date.second > current.second) AddSeconds(date.second - current.second);
	}
	void Gregorian::AddYears(DWORD years)
	{
		if (!years) return;
		DWORD month = current.month, seconds = 0;
		int months = 12;
		do
		{
			if (month > 12) month = 1;
			seconds += 86400 * DaysInMonth(month++);
			--months;
		}
		while (months);
		DWORD baseSeconds = years * seconds;
		int leapDays = 0;
		DWORD year = 0;
		do
		{
			if (IsLeapYear(year + current.year)) ++leapDays;
			++year;
		}
		while (year <= years);
		if (leapDays && IsLeapYear(current.year) && current.month > 2) --leapDays;
		elapsedSeconds += baseSeconds + 86400 * leapDays;
		if (IsLeapYear(current.year) && !IsLeapYear(years + current.year)
			&& current.month == 2 && current.day == DaysInMonth(2) + 1)
		{
			current.year += years;
			if (++current.month > 12) current.month = 1;
			current.day = 1;
		}
		else current.year += years;
	}
	void Gregorian::AddDays(DWORD days)
	{
		while (days > 0)
		{
			DWORD remaining = DaysInMonth(current.month) - current.day;
			if (IsLeapYear(current.year) && current.month == 2) ++remaining;
			if (days <= remaining)
			{
				elapsedSeconds += 86400 * days;
				current.day += days;
				return;
			}
			DWORD month = current.month + 1;
			if (month > 12)
			{
				DWORD year = current.year + 1;
				current.month = 1;
				current.year = year;
			}
			else current.month = month;
			++remaining;
			elapsedSeconds += 86400 * remaining;
			days -= remaining;
			current.day = 1;
		}
	}
	void Gregorian::AddHours(DWORD hours)
	{
		if (hours >= 24)
		{
			DWORD remainder = hours % 24;
			AddDays(hours / 24);
			hours = remainder;
		}
		while (hours > 0)
		{
			DWORD remaining = 24 - current.hour;
			if (hours < remaining)
			{
				DWORD hour = hours + current.hour;
				elapsedSeconds += 3600 * hours;
				current.hour = hour;
				return;
			}
			AddDays(1);
			elapsedSeconds += 3600 * remaining;
			hours -= remaining;
			current.hour = 0;
		}
	}
	void Gregorian::AddMinutes(DWORD minutes)
	{
		if (minutes >= 60)
		{
			DWORD remainder = minutes % 60;
			AddHours(minutes / 60);
			minutes = remainder;
		}
		while (minutes)
		{
			DWORD remaining = 60 - current.minute;
			if (minutes < remaining)
			{
				DWORD minute = minutes + current.minute;
				elapsedSeconds += 60 * minutes;
				current.minute = minute;
				return;
			}
			AddHours(1);
			current.minute = 0;
			elapsedSeconds += 60 * remaining;
			minutes -= remaining;
		}
	}
	void Gregorian::AddSeconds(DWORD seconds)
	{
		if (seconds >= 60)
		{
			DWORD remainder = seconds % 60;
			AddMinutes(seconds / 60);
			seconds = remainder;
		}
		while (seconds)
		{
			DWORD remaining = 60 - current.second;
			if (seconds < remaining)
			{
				DWORD elapsed = seconds + elapsedSeconds;
				current.second += seconds;
				elapsedSeconds = elapsed;
				return;
			}
			AddMinutes(1);
			current.second = 0;
			elapsedSeconds += remaining;
			seconds -= remaining;
		}
	}
	void Gregorian::NextMonth() //5A3CE0.
	{
		if (++current.month > 12)
		{
			++current.year;
			current.month = 1;
		}
		DWORD remaining = DaysInMonth(current.month) - current.day;
		if (IsLeapYear(current.year) && current.month == 2) ++remaining;
		elapsedSeconds += 60 * (60 * (24 * remaining - current.hour)
			- current.minute) - current.second;
		current.day = 1;
		current.hour = 0;
		current.minute = 0;
		current.second = 0;
	}
	void Gregorian::NextDay() //5A3D60.
	{
		AddDays(1);
		elapsedSeconds += -current.second - 60 * (current.minute + 60 * current.hour);
		current.hour = 0;
		current.minute = 0;
		current.second = 0;
	}
	void Gregorian::NextHour() //5A3DA0.
	{
		AddHours(1);
		elapsedSeconds += -current.second - 60 * current.minute;
		current.minute = 0;
		current.second = 0;
	}
	void Gregorian::NextMinute() //5A3DD0.
	{
		AddMinutes(1);
		elapsedSeconds -= current.second;
		current.second = 0;
	}
}
