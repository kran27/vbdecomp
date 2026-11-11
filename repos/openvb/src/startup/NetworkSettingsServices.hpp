#pragma once

#include "configuration/ConfigurationTypes.hpp"
#include "network/Types.hpp"

namespace F3
{
	namespace NetworkSettingsServices
	{
		const JE::Configuration::Values* FindSection(const JE::Configuration::Settings& settings,
			const char* name);
		const std::string* FindValue(const JE::Configuration::Values& section, const char* name);
		JE::Net::Int32 ToLong(const char* text);
		JE::Configuration::Values& EnsureSection(JE::Configuration::Settings& settings, const char* name);
		std::string& EnsureValue(JE::Configuration::Values& section, const char* name);
		char* Format(JE::Net::Int32 value, char* buffer);
		void Assign(std::string& value, const char* text);
	}
}
