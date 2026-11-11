#include "StringsComponent.hpp"
#include "CommonException.hpp"
#include "ConfigurationObject.hpp"
#include "EngineStringSystem.hpp"
#include "SaveDirectory.hpp"

#include <new>

namespace F3
{
	StringsComponent* g_pStringsInstance = NULL;
	static const char g_szDuplicateStrings[] = "Strings object already exists.";

	bool GetString(JE::Net::UInt32 strref, std::string& output) //56C280.
	{
		g_EngineStringSystem.GetString(strref, output);
		return output.size();
	}

	StringsComponent::StringsComponent()
	{
		if (g_pStringsInstance) throw Common::Exception(g_szDuplicateStrings);
		g_pStringsInstance = this; // Original publication survives later failure.
		g_EngineStringSystem.Initialize(GetCurrentWorkingDirectory(), *GetSettingsObject());
	}
	StringsComponent::~StringsComponent() { g_pStringsInstance = NULL; }
	void StringsComponent::Initialize() { OnInitialize(); }
	void StringsComponent::Cleanup() { OnCleanup(); }
	void StringsComponent::Update() { }
	void StringsComponent::OnInitialize() { }
	void StringsComponent::OnCleanup() { }
	GameComponent* __cdecl CreateStrings()
	{
		return new StringsComponent;
	}
	namespace StringsComponentServices
	{
		void* Allocate(std::size_t size) { return ::operator new(size); }
		void Free(void* block) { ::operator delete(block); }
		void InitializeSystem(const char* root, const JE::Configuration::Settings& settings)
		{
			g_EngineStringSystem.Initialize(root, settings);
		}
	}
}
