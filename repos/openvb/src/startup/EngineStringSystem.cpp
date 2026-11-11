#include "EngineStringSystem.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "entity/World.hpp"

#include <cstring>
#include <stdexcept>

namespace F3
{
	EngineStringSystem g_EngineStringSystem;

	EngineStringSystem::EngineStringSystem()
	{
		// The base and real token map perform their ordinary construction.
	}

	EngineStringSystem::~EngineStringSystem()
	{
		// The real compiler emits token-map and qualified base cleanup.
	}

	void EngineStringSystem::SetTokenValue(const char* name, JE::Net::Int32 value) //56B940.
	{
		tokens.Set(name, value);
	}

	void EngineStringSystem::SetTokenText(const char* name, const char* text) //56BD50.
	{
		JE::Net::Int32 index = tokens.Get(name);
		if (!index)
		{
			index = AddString(text);
			tokens.Set(name, index);
		}
		else ReplaceString(index, text); //The original ignores replacement failure.
	}

	void EngineStringSystem::Initialize(const char* root, const JE::Configuration::Settings& settings)
	{
		StringResolverServices::InitializeBase(*this, root, settings);
		const std::string filename("tokens.tok");
		StringResolverServices::LoadTokens(tokens, filename); // Original result ignored.
	}

	void EngineStringSystem::GetString(JE::Net::UInt32 strref, std::string& output)
	{
		JE::SS::StringSystem::GetString(strref, output);
		ResolveTokens(output); // Also runs for strref zero, whose base call preserves output.
	}

	namespace StringResolverServices
	{
		bool ReadGlobalVariable(const std::string& name, std::string& value)
		{
			return JE::GetWorldInstance()->GetGlobalParameters()->GetString(name, std::string(), value) != 0;
		}
		void InitializeBase(EngineStringSystem& system, const char* root, const JE::Configuration::Settings& settings)
		{
			system.JE::SS::StringSystem::Initialize(root, settings);
		}
		void GetBase(EngineStringSystem& system, JE::Net::UInt32 strref, std::string& output)
		{
			system.JE::SS::StringSystem::GetString(strref, output);
		}
		unsigned char LoadTokens(JE::SS::TokenTable& table, const std::string& name) { return table.Load(name); }
		JE::Net::Int32 TokenValue(const JE::SS::TokenTable& table, const std::string& name) { return table.Get(name); }
		unsigned char GetByIndex(EngineStringSystem& system, JE::Net::UInt32 index, char* output) { return system.GetByIndex(index, output); }
	}
}
