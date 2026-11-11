#include "StringSystem.hpp"
#include "configuration/ConfigurationGetters.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE
{
	namespace SS
	{
		void StringSystem::Initialize(const char* root, const Configuration::Settings& settings)
		{
			std::string path(root);
			// The original tests the terminal NUL, even for an existing separator.
			if (path.c_str()[path.size()] != '\\') path += '\\';
			path += Configuration::GetString(settings, "Strings", "Male", "English.stf");
			if (!LoadTable(path, maleInput, maleEntries, maleCount))
			{
				maleEntries = NULL; maleCount = 0;
				JE::LogDebugString("StringSystem::Init - Couldn't open male string table %s\n", path.c_str());
			}
			path = path.substr(0, path.find_last_of('\\') + 1);
			const char* noFemale = "";
			const char* femaleName = Configuration::GetString(settings, "Strings", "Female", noFemale);
			if (femaleName != noFemale)
			{
				path += femaleName;
				if (!LoadTable(path, femaleInput, femaleEntries, femaleCount))
				{
					femaleEntries = NULL; femaleCount = 0;
					JE::LogDebugString("StringSystem::Init - Couldn't open female string table %s\n", path.c_str());
				}
			}
			else
			{
				femaleEntries = NULL; femaleCount = 0;
				JE::LogDebugString("StringSystem::Init - No female string table\n");
			}
			selectedInput = &maleInput;
			gender = 0;
			selectedEntries = maleEntries;
			selectedCount = maleCount;
		}

	}
}
