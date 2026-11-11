#include "World.hpp"
#include <cstdlib>
#include <cstdio>

namespace JE
{
	WorldParameters::WorldParameters()
	{
	}
	WorldParameters::~WorldParameters()
	{
	}
	BYTE WorldParameters::GetString(const std::string& variable,std::string section,std::string& output) const
	{
		if (section.empty()) section="Globals";
		BYTE result=0;
		Configuration::Settings::const_iterator group=values.find(section);
		if (group!=values.end())
		{
			const Configuration::Values& entries=group->second;
			Configuration::Values::const_iterator found=entries.find(variable);
			if (found!=entries.end())
			{
				output=found->second;
				result=1;
			}
		}
		return result; //Missing values preserve caller output.
	}
	BYTE WorldParameters::HasValue(std::string variable, std::string section)
	{
		if (section.empty()) section="Globals";
		BYTE result=0;
		Configuration::Settings::iterator group=values.find(section);
		if (group!=values.end())
		{
			Configuration::Values& entries=group->second;
			Configuration::Values::iterator found=entries.find(variable);
			if (found!=entries.end()) result=1;
		}
		return result;
	}
	LONG WorldParameters::GetInteger(std::string variable, std::string section) const
	{
		if (section.empty()) section="Globals";
		LONG result=0;
		Configuration::Settings::const_iterator group=values.find(section);
		if (group!=values.end())
		{
			const Configuration::Values& entries=group->second;
			Configuration::Values::const_iterator found=entries.find(variable);
			if (found!=entries.end()) result=std::atol(found->second.c_str());
		}
		return result;
	}

	void WorldParameters::SetInteger(std::string variable, LONG value, std::string section)
	{
		if (section.empty()) section="Globals";
		Configuration::Settings::iterator group=values.find(section);
		char text[260];
		if (group!=values.end())
		{
			std::sprintf(text,"%d",static_cast<int>(value));
			Configuration::Values& entries=group->second;
			//5AA152..5AA1C0 copies an existing-section key through c_str/strlen.
			std::string key(variable.c_str());
			entries[key]=text;
		}
		else
		{
			Configuration::Values created;
			std::sprintf(text,"%d",static_cast<int>(value));
			created[variable]=text;
			values[section]=created;
		}
	}
}
