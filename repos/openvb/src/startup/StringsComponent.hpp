#pragma once

#include "EngineRegistry.hpp"
#include "configuration/ConfigurationTypes.hpp"

#include <cstddef>

namespace F3
{
	class StringsComponent : public GameComponent
	{
	public:
		StringsComponent(); // 0x59C240
		~StringsComponent(); // 0x59C210
		void Initialize(); // 0x59C230 -> hook +16
		void Cleanup(); // 0x5B1510 -> hook +20
		void Update(); // 0x404B70
		virtual void OnInitialize(); // original +16, 0x404B70
		virtual void OnCleanup(); // original +20, 0x404B70
	};
	extern StringsComponent* g_pStringsInstance; // 0x70C664
	GameComponent* __cdecl CreateStrings(); // 0x45E330

	namespace StringsComponentServices
	{
		void* Allocate(std::size_t size);
		void Free(void* block);
		void InitializeSystem(const char* root, const JE::Configuration::Settings& settings);
	}
}
