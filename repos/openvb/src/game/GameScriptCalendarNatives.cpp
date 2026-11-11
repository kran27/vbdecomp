#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "entity/Gregorian.hpp"
#include "entity/World.hpp"
#include <stdlib.h>

namespace F3
{
	cell AMX_NATIVE_CALL NativeYear(AMX* vm, cell*)
	{
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "Year() was unable to retrieve 'myself' id.");
			return 0;
		}
		JE::Gregorian* calendar = JE::GetWorldServerInstance()->GetCalendar();
		DWORD id = reinterpret_cast<DWORD>(owner);
		cell result = 0;
		if (id & 0xE0000000)
			result = cell(calendar->GetYear());
		else
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (entity)
			{
				DWORD offset = entity->value29C;
				if (offset)
				{
					JE::Gregorian* adjusted = JE::GetWorldServerInstance()->NewCalendar();
					adjusted->CopyInitialFrom(*calendar);
					adjusted->AddSeconds(offset);
					result = cell(adjusted->GetYear());
					free(adjusted);
				}
				else result = cell(calendar->GetYear());
			}
			else LogScriptError(vm, "Year() was unable to retrieve 'myself' entity.");
		}
		return result;
	}
	cell AMX_NATIVE_CALL NativeMonth(AMX* vm, cell*)
	{
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "Month() was unable to retrieve 'myself' id.");
			return 0;
		}
		JE::Gregorian* calendar = JE::GetWorldServerInstance()->GetCalendar();
		DWORD id = reinterpret_cast<DWORD>(owner);
		cell result = 0;
		if (id & 0xE0000000)
			result = cell(calendar->GetMonth());
		else
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (entity)
			{
				DWORD offset = entity->value29C;
				if (offset)
				{
					JE::Gregorian* adjusted = JE::GetWorldServerInstance()->NewCalendar();
					adjusted->CopyInitialFrom(*calendar);
					adjusted->AddSeconds(offset);
					result = cell(adjusted->GetMonth());
					free(adjusted);
				}
				else result = cell(calendar->GetMonth());
			}
			else LogScriptError(vm, "Month() was unable to retrieve 'myself' entity.");
		}
		return result;
	}
	cell AMX_NATIVE_CALL NativeDay(AMX* vm, cell*)
	{
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "Day() was unable to retrieve 'myself' id.");
			return 0;
		}
		JE::Gregorian* calendar = JE::GetWorldServerInstance()->GetCalendar();
		DWORD id = reinterpret_cast<DWORD>(owner);
		cell result = 0;
		if (id & 0xE0000000)
			result = cell(calendar->GetDay());
		else
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (entity)
			{
				DWORD offset = entity->value29C;
				if (offset)
				{
					JE::Gregorian* adjusted = JE::GetWorldServerInstance()->NewCalendar();
					adjusted->CopyInitialFrom(*calendar);
					adjusted->AddSeconds(offset);
					result = cell(adjusted->GetDay());
					free(adjusted);
				}
				else result = cell(calendar->GetDay());
			}
			else LogScriptError(vm, "Day() was unable to retrieve 'myself' entity.");
		}
		return result;
	}
	cell AMX_NATIVE_CALL NativeHour(AMX* vm, cell*)
	{
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "Hour() was unable to retrieve 'myself' id.");
			return 0;
		}
		JE::Gregorian* calendar = JE::GetWorldServerInstance()->GetCalendar();
		DWORD id = reinterpret_cast<DWORD>(owner);
		cell result = 0;
		if (id & 0xE0000000)
			result = cell(calendar->GetHour());
		else
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (entity)
			{
				DWORD offset = entity->value29C;
				if (offset)
				{
					JE::Gregorian* adjusted = JE::GetWorldServerInstance()->NewCalendar();
					adjusted->CopyInitialFrom(*calendar);
					adjusted->AddSeconds(offset);
					result = cell(adjusted->GetHour());
					free(adjusted);
				}
				else result = cell(calendar->GetHour());
			}
			else LogScriptError(vm, "Hour() was unable to retrieve 'myself' entity.");
		}
		return result;
	}
	cell AMX_NATIVE_CALL NativeMinute(AMX* vm, cell*)
	{
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "Minute() was unable to retrieve 'myself' id.");
			return 0;
		}
		JE::Gregorian* calendar = JE::GetWorldServerInstance()->GetCalendar();
		DWORD id = reinterpret_cast<DWORD>(owner);
		cell result = 0;
		if (id & 0xE0000000)
			result = cell(calendar->GetMinute());
		else
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (entity)
			{
				DWORD offset = entity->value29C;
				if (offset)
				{
					JE::Gregorian* adjusted = JE::GetWorldServerInstance()->NewCalendar();
					adjusted->CopyInitialFrom(*calendar);
					adjusted->AddSeconds(offset);
					result = cell(adjusted->GetMinute());
					free(adjusted);
				}
				else result = cell(calendar->GetMinute());
			}
			else LogScriptError(vm, "Minute() was unable to retrieve 'myself' entity.");
		}
		return result;
	}
	cell AMX_NATIVE_CALL NativeSecond(AMX* vm, cell*)
	{
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "Second() was unable to retrieve 'myself' id.");
			return 0;
		}
		JE::Gregorian* calendar = JE::GetWorldServerInstance()->GetCalendar();
		DWORD id = reinterpret_cast<DWORD>(owner);
		cell result = 0;
		if (id & 0xE0000000)
			result = cell(calendar->GetSecond());
		else
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (entity)
			{
				JE::GetWorldServerInstance()->GetCalendar();
				DWORD offset = entity->value29C;
				if (offset)
				{
					JE::Gregorian* adjusted = JE::GetWorldServerInstance()->NewCalendar();
					adjusted->CopyInitialFrom(*calendar);
					adjusted->AddSeconds(offset);
					result = cell(adjusted->GetSecond());
					free(adjusted);
				}
				else result = cell(calendar->GetSecond());
			}
			else LogScriptError(vm, "Second() was unable to retrieve 'myself' entity.");
		}
		return result;
	}
	cell AMX_NATIVE_CALL NativeYearsPassed(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "YearsPassed")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "YearsPassed() was unable to retrieve 'myself' id.");
			return 0;
		}
		DWORD id = reinterpret_cast<DWORD>(owner), offset = 0;
		if (!(id & 0xE0000000))
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (!entity)
				LogScriptError(vm, "YearsPassed() was unable to retrieve 'myself' entity.");
			offset = entity->value29C;
		}
		JE::IWorldServer* world = JE::GetWorldServerInstance();
		JE::Gregorian* calendar = world->GetCalendar();
		if (!offset) return cell(calendar->YearsPassed(DWORD(parameters[1])));
		JE::Gregorian* adjusted = world->NewCalendar();
		adjusted->SetTimeStamp(calendar->GetTimeStamp());
		adjusted->AddSeconds(offset);
		cell result = cell(adjusted->YearsPassed(DWORD(parameters[1])));
		free(adjusted);
		return result;
	}
	cell AMX_NATIVE_CALL NativeMonthsPassed(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "MonthsPassed")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "MonthsPassed() was unable to retrieve 'myself' id.");
			return 0;
		}
		DWORD id = reinterpret_cast<DWORD>(owner), offset = 0;
		if (!(id & 0xE0000000))
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (!entity)
				LogScriptError(vm, "MonthsPassed() was unable to retrieve 'myself' entity.");
			offset = entity->value29C;
		}
		JE::IWorldServer* world = JE::GetWorldServerInstance();
		JE::Gregorian* calendar = world->GetCalendar();
		if (!offset) return cell(calendar->MonthsPassed(DWORD(parameters[1])));
		JE::Gregorian* adjusted = world->NewCalendar();
		adjusted->SetTimeStamp(calendar->GetTimeStamp());
		adjusted->AddSeconds(offset);
		cell result = cell(adjusted->MonthsPassed(DWORD(parameters[1])));
		free(adjusted);
		return result;
	}
	cell AMX_NATIVE_CALL NativeDaysPassed(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "DaysPassed")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "DaysPassed() was unable to retrieve 'myself' id.");
			return 0;
		}
		DWORD id = reinterpret_cast<DWORD>(owner), offset = 0;
		if (!(id & 0xE0000000))
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (!entity)
				LogScriptError(vm, "DaysPassed() was unable to retrieve 'myself' entity.");
			offset = entity->value29C;
		}
		JE::IWorldServer* world = JE::GetWorldServerInstance();
		JE::Gregorian* calendar = world->GetCalendar();
		if (!offset) return cell(calendar->DaysPassed(DWORD(parameters[1])));
		JE::Gregorian* adjusted = world->NewCalendar();
		adjusted->SetTimeStamp(calendar->GetTimeStamp());
		adjusted->AddSeconds(offset);
		cell result = cell(adjusted->DaysPassed(DWORD(parameters[1])));
		free(adjusted);
		return result;
	}
	cell AMX_NATIVE_CALL NativeHoursPassed(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "HoursPassed")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "HoursPassed() was unable to retrieve 'myself' id.");
			return 0;
		}
		DWORD id = reinterpret_cast<DWORD>(owner), offset = 0;
		if (!(id & 0xE0000000))
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (!entity)
				LogScriptError(vm, "HoursPassed() was unable to retrieve 'myself' entity.");
			offset = entity->value29C;
		}
		JE::IWorldServer* world = JE::GetWorldServerInstance();
		JE::Gregorian* calendar = world->GetCalendar();
		if (!offset) return cell(calendar->HoursPassed(DWORD(parameters[1])));
		JE::Gregorian* adjusted = world->NewCalendar();
		adjusted->SetTimeStamp(calendar->GetTimeStamp());
		adjusted->AddSeconds(offset);
		cell result = cell(adjusted->HoursPassed(DWORD(parameters[1])));
		free(adjusted);
		return result;
	}
	cell AMX_NATIVE_CALL NativeMinutesPassed(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "MinutesPassed")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "MinutesPassed() was unable to retrieve 'myself' id.");
			return 0;
		}
		DWORD id = reinterpret_cast<DWORD>(owner), offset = 0;
		if (!(id & 0xE0000000))
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (!entity)
				LogScriptError(vm, "MinutesPassed() was unable to retrieve 'myself' entity.");
			offset = entity->value29C;
		}
		JE::IWorldServer* world = JE::GetWorldServerInstance();
		JE::Gregorian* calendar = world->GetCalendar();
		if (!offset) return cell(calendar->MinutesPassed(DWORD(parameters[1])));
		JE::Gregorian* adjusted = world->NewCalendar();
		adjusted->SetTimeStamp(calendar->GetTimeStamp());
		adjusted->AddSeconds(offset);
		cell result = cell(adjusted->MinutesPassed(DWORD(parameters[1])));
		free(adjusted);
		return result;
	}
	cell AMX_NATIVE_CALL NativeSecondsPassed(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "SecondsPassed")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "SecondsPassed() was unable to retrieve 'myself' id.");
			return 0;
		}
		DWORD id = reinterpret_cast<DWORD>(owner), offset = 0;
		if (!(id & 0xE0000000))
		{
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (!entity)
				LogScriptError(vm, "SecondsPassed() was unable to retrieve 'myself' entity.");
			offset = entity->value29C;
		}
		JE::IWorldServer* world = JE::GetWorldServerInstance();
		JE::Gregorian* calendar = world->GetCalendar();
		if (!offset) return cell(calendar->SecondsPassed(DWORD(parameters[1])));
		JE::Gregorian* adjusted = world->NewCalendar();
		adjusted->SetTimeStamp(calendar->GetTimeStamp());
		adjusted->AddSeconds(offset);
		cell result = cell(adjusted->SecondsPassed(DWORD(parameters[1])));
		free(adjusted);
		return result;
	}
}
