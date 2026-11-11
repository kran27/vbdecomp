#pragma once

#include "strings/StringSystem.hpp"
#include "strings/TokenTable.hpp"

namespace F3
{
	class EngineStringSystem : public JE::SS::StringSystem
	{
	public:
		EngineStringSystem(); //0x56C360, ordinary base and token-map construction.
		// Original 0x56C360/0x56C3E0: normal base and token-map lifetime.
		~EngineStringSystem(); // 0x56C3E0, ordinary explicit destructor; own vptr precedes member cleanup.
		void Initialize(const char* root, const JE::Configuration::Settings& settings); // 0x56BCA0
		void GetString(JE::Net::UInt32 strref, std::string& output); // 0x56C260
		void ResolveTokens(std::string& output); // 0x56BE30
		void SetTokenValue(const char* name, JE::Net::Int32 value); //56B940, existing token-table owner.
		void SetTokenText(const char* name, const char* text); //56BD50, original direct thiscall API.
		JE::SS::TokenTable tokens;
	};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char EngineStringSystemSize[(sizeof(EngineStringSystem) == 0x178) ? 1 : -1];
	typedef char EngineStringSystemTokensOffset[(offsetof(EngineStringSystem, tokens) == 0x16C) ? 1 : -1];
#endif

	// Original in-place static object 0x70C2D8; real MSVC owns static lifetime.
	extern EngineStringSystem g_EngineStringSystem;
	bool GetString(JE::Net::UInt32 strref, std::string& output); // 0x56C280

	namespace StringResolverServices
	{
		// Actual general World/vslot108 -> parameters90.GetString5A9F80.
		// Missing variables preserve output and return false.
		bool ReadGlobalVariable(const std::string& name, std::string& value);
		void InitializeBase(EngineStringSystem& system, const char* root,
			const JE::Configuration::Settings& settings);
		void GetBase(EngineStringSystem& system, JE::Net::UInt32 strref, std::string& output);
		unsigned char LoadTokens(JE::SS::TokenTable& table, const std::string& name);
		JE::Net::Int32 TokenValue(const JE::SS::TokenTable& table, const std::string& name);
		unsigned char GetByIndex(EngineStringSystem& system, JE::Net::UInt32 index, char* output);
	}
}
