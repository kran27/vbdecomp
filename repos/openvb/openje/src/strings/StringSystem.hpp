#pragma once

#include "StringTable.hpp"
#include "configuration/ConfigurationTypes.hpp"

#include <vector>

namespace JE
{
	namespace SS
	{
		class StringSystem
		{
		public:
			StringSystem(); // 0x4C5C80
			~StringSystem(); // 0x4C5B40; ordinary destructor, not a virtual slot.
			virtual void Initialize(const char* root, const Configuration::Settings& settings); // 0x4C5E30
			virtual void GetString(JE::Net::UInt32 strref, std::string& output); // 0x4C5A30
			unsigned char GetByIndex(JE::Net::UInt32 index, char* output); // 0x4C5900
			JE::Net::UInt32 AddString(const char* text); //4C6130; selected count + appended index, no+1.
			unsigned char ReplaceString(JE::Net::UInt32 index, const char* text); //4C5AC0; extra strings only.
			unsigned char LoadTable(const std::string& path, std::ifstream& input,
				unsigned char*& entries, JE::Net::UInt32& count); //4C5D10; four independent arguments.

			JE::Net::UInt32 gender;
			unsigned char* maleEntries;
			JE::Net::UInt32 maleCount;
			std::ifstream maleInput;
			unsigned char* femaleEntries;
			JE::Net::UInt32 femaleCount;
			std::ifstream femaleInput;
			unsigned char* selectedEntries;
			JE::Net::UInt32 selectedCount;
			std::ifstream* selectedInput;
			std::vector<std::string> extraStrings;
		};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
		typedef char StringSystemSize[(sizeof(StringSystem) == 0x16C) ? 1 : -1];
		typedef char StringSystemGenderOffset[(offsetof(StringSystem, gender) == 4) ? 1 : -1];
		typedef char StringSystemMaleEntriesOffset[(offsetof(StringSystem, maleEntries) == 8) ? 1 : -1];
		typedef char StringSystemMaleCountOffset[(offsetof(StringSystem, maleCount) == 0xC) ? 1 : -1];
		typedef char StringSystemMaleInputOffset[(offsetof(StringSystem, maleInput) == 0x10) ? 1 : -1];
		typedef char StringSystemFemaleEntriesOffset[(offsetof(StringSystem, femaleEntries) == 0xAC) ? 1 : -1];
		typedef char StringSystemFemaleCountOffset[(offsetof(StringSystem, femaleCount) == 0xB0) ? 1 : -1];
		typedef char StringSystemFemaleInputOffset[(offsetof(StringSystem, femaleInput) == 0xB4) ? 1 : -1];
		typedef char StringSystemSelectedEntriesOffset[(offsetof(StringSystem, selectedEntries) == 0x150) ? 1 : -1];
		typedef char StringSystemSelectedCountOffset[(offsetof(StringSystem, selectedCount) == 0x154) ? 1 : -1];
		typedef char StringSystemSelectedInputOffset[(offsetof(StringSystem, selectedInput) == 0x158) ? 1 : -1];
		typedef char StringSystemExtraStringsOffset[(offsetof(StringSystem, extraStrings) == 0x15C) ? 1 : -1];
#endif

		namespace StringSystemServices
		{
			enum Diagnostic { MaleLoadFailure, FemaleLoadFailure, NoFemaleTable };
			const char* Setting(const Configuration::Settings& settings,
				const char* key, const char* fallback);
			void Report(Diagnostic kind, const char* path);
			void Assign(std::string& output, const char* text);
		}
	}
}
