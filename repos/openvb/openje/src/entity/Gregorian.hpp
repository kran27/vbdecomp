#pragma once

#include <windows.h>
#include <stdio.h>

namespace JE
{
	struct CalendarDate
	{
		CalendarDate() : year(0), month(1), day(1), hour(0), minute(0), second(0) {}
		DWORD year, month, day, hour, minute, second;
	};
	class Gregorian;
	struct __declspec(novtable) ICalendar
	{
		explicit ICalendar(const CalendarDate& start) { initial = start; }
		virtual Gregorian& CopyInitialFrom(const Gregorian& other) = 0; //5449B0, copies only initial date.
		virtual CalendarDate* GetDate() = 0; //5449D0, slot04.
		virtual void SetDate(const CalendarDate& date) = 0; //5A3740, slot08.
		virtual DWORD GetTimeStamp() const = 0; //41D1D0, slot0C.
		virtual void SetTimeStamp(DWORD seconds) = 0; //5449E0, slot10.
		virtual DWORD GetYear() const = 0; //5C01A0.
		virtual DWORD GetMonth() const = 0; //4080C0.
		virtual DWORD GetDay() const = 0; //544A00.
		virtual DWORD GetHour() const = 0; //544A10.
		virtual DWORD GetMinute() const = 0; //424A10.
		virtual DWORD GetSecond() const = 0; //41D1B0.
		virtual DWORD MonthsPerYear() const = 0; //544A20.
		virtual DWORD DaysInMonth(DWORD month) const = 0; // 544A30, original byte table.
		virtual DWORD HoursPerDay() const = 0; //544A40, slot34.
		virtual DWORD MinutesPerHour() const = 0; //544A50, slot38.
		virtual DWORD SecondsPerMinute() const = 0; //544A50, slot3C.
		virtual BYTE IsLeapYear(DWORD year) const = 0; // 5A3920.
		virtual const char* FormatDate() const = 0; //5A3960, slot44.
		virtual void AddYears(DWORD years) = 0; // 5A39A0.
		virtual void AddDays(DWORD days) = 0; // 5A3AB0.
		virtual void AddHours(DWORD hours) = 0; // 5A3B50.
		virtual void AddMinutes(DWORD minutes) = 0; // 5A3BE0.
		virtual void AddSeconds(DWORD seconds) = 0; // 5A3C60.
		virtual void NextMonth() = 0; //5A3CE0.
		virtual void NextDay() = 0; //5A3D60.
		virtual void NextHour() = 0; //5A3DA0.
		virtual void NextMinute() = 0; //5A3DD0.
		//VC7 emits each overload group in reverse declaration order.
		virtual DWORD YearsPassed(const CalendarDate& date) const = 0; //5A3DF0.
		virtual DWORD YearsPassed(DWORD timestamp) const = 0; //5A4070.
		virtual DWORD MonthsPassed(const CalendarDate& date) const = 0; //5A3E20.
		virtual DWORD MonthsPassed(DWORD timestamp) const = 0; //5A40D0.
		virtual DWORD DaysPassed(const CalendarDate& date) const = 0; //5A4130.
		virtual DWORD DaysPassed(DWORD timestamp) const = 0; //5A3EB0.
		virtual DWORD HoursPassed(const CalendarDate& date) const = 0; //5A4190.
		virtual DWORD HoursPassed(DWORD timestamp) const = 0; //5A3F10.
		virtual DWORD MinutesPassed(const CalendarDate& date) const = 0; //5A41F0.
		virtual DWORD MinutesPassed(DWORD timestamp) const = 0; //5A3F60.
		virtual DWORD SecondsPassed(const CalendarDate& date) const = 0; //5A4250.
		virtual DWORD SecondsPassed(DWORD timestamp) const = 0; //5A3FA0.
		virtual BYTE Write(FILE* stream) const = 0; //5A3FD0, initial/current only.
		virtual BYTE Read(FILE* stream) = 0; //5A4020, elapsedSeconds unchanged.
		CalendarDate initial;
	};
	class Gregorian : public ICalendar
	{
	public:
		explicit Gregorian(const CalendarDate& start); // 544950.
		~Gregorian() {} // Actual nonvirtual5A3730.
		//The original69166C table has41 entries; the declaration order is ABI.
		virtual Gregorian& CopyInitialFrom(const Gregorian& other); //5449B0, copies only initial date.
		virtual CalendarDate* GetDate(); //5449D0, slot04.
		virtual void SetDate(const CalendarDate& date); //5A3740, slot08.
		virtual DWORD GetTimeStamp() const; //41D1D0, slot0C.
		virtual void SetTimeStamp(DWORD seconds); //5449E0, slot10.
		virtual DWORD GetYear() const; //5C01A0.
		virtual DWORD GetMonth() const; //4080C0.
		virtual DWORD GetDay() const; //544A00.
		virtual DWORD GetHour() const; //544A10.
		virtual DWORD GetMinute() const; //424A10.
		virtual DWORD GetSecond() const; //41D1B0.
		virtual DWORD MonthsPerYear() const; //544A20.
		virtual DWORD DaysInMonth(DWORD month) const; // 544A30, original byte table.
		virtual DWORD HoursPerDay() const; //544A40, slot34.
		virtual DWORD MinutesPerHour() const; //544A50, slot38.
		virtual DWORD SecondsPerMinute() const; //544A50, slot3C.
		virtual BYTE IsLeapYear(DWORD year) const; // 5A3920.
		virtual const char* FormatDate() const; //5A3960, slot44.
		virtual void AddYears(DWORD years); // 5A39A0.
		virtual void AddDays(DWORD days); // 5A3AB0.
		virtual void AddHours(DWORD hours); // 5A3B50.
		virtual void AddMinutes(DWORD minutes); // 5A3BE0.
		virtual void AddSeconds(DWORD seconds); // 5A3C60.
		virtual void NextMonth(); //5A3CE0.
		virtual void NextDay(); //5A3D60.
		virtual void NextHour(); //5A3DA0.
		virtual void NextMinute(); //5A3DD0.
		//VC7 emits each overload group in reverse declaration order.
		virtual DWORD YearsPassed(const CalendarDate& date) const; //5A3DF0.
		virtual DWORD YearsPassed(DWORD timestamp) const; //5A4070.
		virtual DWORD MonthsPassed(const CalendarDate& date) const; //5A3E20.
		virtual DWORD MonthsPassed(DWORD timestamp) const; //5A40D0.
		virtual DWORD DaysPassed(const CalendarDate& date) const; //5A4130.
		virtual DWORD DaysPassed(DWORD timestamp) const; //5A3EB0.
		virtual DWORD HoursPassed(const CalendarDate& date) const; //5A4190.
		virtual DWORD HoursPassed(DWORD timestamp) const; //5A3F10.
		virtual DWORD MinutesPassed(const CalendarDate& date) const; //5A41F0.
		virtual DWORD MinutesPassed(DWORD timestamp) const; //5A3F60.
		virtual DWORD SecondsPassed(const CalendarDate& date) const; //5A4250.
		virtual DWORD SecondsPassed(DWORD timestamp) const; //5A3FA0.
		virtual BYTE Write(FILE* stream) const; //5A3FD0, initial/current only.
		virtual BYTE Read(FILE* stream); //5A4020, elapsedSeconds unchanged.
		CalendarDate current;
		DWORD elapsedSeconds;
	};
}
