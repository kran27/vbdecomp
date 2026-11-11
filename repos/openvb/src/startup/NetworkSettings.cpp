#include "NetworkComponent.hpp"
#include "NetworkSettingsServices.hpp"
#include "ConfigurationObject.hpp"

#include <cstdlib>

namespace F3
{

	void NetworkComponent::LoadConfiguration()
	{
		mode = 3;
		serverPort = 0x53FA;
		serverDebug = 0x0109C038;
		serverRate = 33;
		serverTimeout = 600000;
		clientPort = 0x57E5;
		clientDebug = 0x0109C038;
		clientRate = 0;
		clientTimeout = 600000;

		typedef JE::Configuration::Settings Settings;
		typedef JE::Configuration::Values Values;
		Settings& settings = *GetSettingsObject();
		{
			Settings::iterator section = settings.find("Network");
			if (section != settings.end())
			{
				Values& values = section->second;
				{
					Values::const_iterator value = values.find("configuration");
					if (value != values.end()) mode = static_cast<JE::Net::UInt32>(std::atol(value->second.c_str()));
				}
			}
		}
		{
			Settings::iterator section = settings.find("Server");
			if (section != settings.end())
			{
				Values& values = section->second;
				{
					Values::const_iterator value = values.find("port");
					if (value != values.end()) serverPort = static_cast<JE::Net::UInt16>(std::atol(value->second.c_str()));
				}
				{
					Values::const_iterator value = values.find("debug");
					if (value != values.end()) serverDebug = static_cast<JE::Net::UInt32>(std::atol(value->second.c_str()));
				}
				{
					Values::const_iterator value = values.find("rate");
					if (value != values.end()) serverRate = static_cast<JE::Net::UInt32>(std::atol(value->second.c_str()));
				}
				{
					Values::const_iterator value = values.find("timeout");
					if (value != values.end()) serverTimeout = static_cast<JE::Net::UInt32>(std::atol(value->second.c_str()));
				}
			}
		}
		{
			Settings::iterator section = settings.find("Client");
			if (section != settings.end())
			{
				Values& values = section->second;
				{
					Values::const_iterator value = values.find("port");
					if (value != values.end()) clientPort = static_cast<JE::Net::UInt16>(std::atol(value->second.c_str()));
				}
				{
					Values::const_iterator value = values.find("debug");
					if (value != values.end()) clientDebug = static_cast<JE::Net::UInt32>(std::atol(value->second.c_str()));
				}
				{
					Values::const_iterator value = values.find("rate");
					if (value != values.end()) clientRate = static_cast<JE::Net::UInt32>(std::atol(value->second.c_str()));
				}
				{
					Values::const_iterator value = values.find("timeout");
					if (value != values.end()) clientTimeout = static_cast<JE::Net::UInt32>(std::atol(value->second.c_str()));
				}
			}
		}
	}

	void NetworkComponent::OnCleanup()
	{
		typedef JE::Configuration::Settings Settings;
		typedef JE::Configuration::Values Values;
		char buffer[36];
		Settings* settings = GetSettingsObject();
		Values* section = &(*settings)["Network"];
		(*section)["configuration"] = _itoa(static_cast<JE::Net::Int32>(mode), buffer, 10);
		section = &(*settings)["Server"];
		(*section)["port"] = _itoa(serverPort, buffer, 10);
		(*section)["debug"] = _itoa(static_cast<JE::Net::Int32>(serverDebug), buffer, 10);
		(*section)["rate"] = _itoa(static_cast<JE::Net::Int32>(serverRate), buffer, 10);
		(*section)["timeout"] = _itoa(static_cast<JE::Net::Int32>(serverTimeout), buffer, 10);
		section = &(*settings)["Client"];
		(*section)["port"] = _itoa(clientPort, buffer, 10);
		(*section)["debug"] = _itoa(static_cast<JE::Net::Int32>(clientDebug), buffer, 10);
		(*section)["rate"] = _itoa(static_cast<JE::Net::Int32>(clientRate), buffer, 10);
		(*section)["timeout"] = _itoa(static_cast<JE::Net::Int32>(clientTimeout), buffer, 10);
	}

	namespace NetworkSettingsServices
	{
		const JE::Configuration::Values* FindSection(const JE::Configuration::Settings& settings,
			const char* name)
		{
			JE::Configuration::Settings::const_iterator found = settings.find(name);
			return found == settings.end() ? NULL : &found->second;
		}
		const std::string* FindValue(const JE::Configuration::Values& section, const char* name)
		{
			JE::Configuration::Values::const_iterator found = section.find(name);
			return found == section.end() ? NULL : &found->second;
		}
		JE::Net::Int32 ToLong(const char* text) { return static_cast<JE::Net::Int32>(std::atol(text)); }
		JE::Configuration::Values& EnsureSection(JE::Configuration::Settings& settings, const char* name)
		{
			return settings[name];
		}
		std::string& EnsureValue(JE::Configuration::Values& section, const char* name) { return section[name]; }
		char* Format(JE::Net::Int32 value, char* buffer) { return _itoa(value, buffer, 10); }
		void Assign(std::string& value, const char* text) { value.assign(text); }
	}
}
