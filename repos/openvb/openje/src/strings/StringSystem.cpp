#include "StringSystem.hpp"
#include "configuration/ConfigurationGetters.hpp"
#include "diagnostics/DebugOutput.hpp"

#include <cstring>

namespace JE
{
	namespace SS
	{
		StringSystem::StringSystem()
		{
			gender = 0;
			maleEntries = NULL; maleCount = 0;
			femaleEntries = NULL; femaleCount = 0;
			selectedEntries = NULL; selectedCount = 0; selectedInput = NULL;
		}

		StringSystem::~StringSystem() 
		{
			if (maleEntries) ::operator delete[](maleEntries);
			maleInput.close();
			if (femaleEntries) ::operator delete[](femaleEntries);
			femaleInput.close();
			// Real vector then female/male stream members destruct after this body.
		}

		JE::Net::UInt32 StringSystem::AddString(const char* text) //4C6130.
		{
			{
				std::string value(text);
				extraStrings.push_back(value);
			} //Original destroys its temporary before reloading the selected table count.
			return selectedCount + static_cast<JE::Net::UInt32>(extraStrings.size()) - 1;
		}

		unsigned char StringSystem::ReplaceString(JE::Net::UInt32 index, const char* text) //4C5AC0.
		{
			JE::Net::UInt32 count = selectedCount;
			if (index < count || index >= count + extraStrings.size()) return 0;
			extraStrings[index - count].assign(text);
			return 1;
		}

		namespace StringSystemServices
		{
			const char* Setting(const Configuration::Settings& settings, const char* key, const char* fallback)
			{
				return Configuration::GetString(settings, "Strings", key, fallback);
			}
			void Report(Diagnostic kind, const char* path)
			{
				if (kind == MaleLoadFailure)
					JE::LogDebugString("StringSystem::Init - Couldn't open male string table %s\n", path);
				else if (kind == FemaleLoadFailure)
					JE::LogDebugString("StringSystem::Init - Couldn't open female string table %s\n", path);
				else JE::LogDebugString("StringSystem::Init - No female string table\n");
			}
			void Assign(std::string& output, const char* text) { output = text; }
		}
	}
}
