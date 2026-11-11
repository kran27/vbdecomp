#pragma once

#include "EngineRegistry.hpp"

#include <cstddef>

namespace JE { namespace Resources { class ResourceSystem; } }

namespace F3
{
	class ResourcesComponent : public GameComponent
	{
	public:
		ResourcesComponent(); // 0x59C110
		~ResourcesComponent(); // 0x59C0A0
		void Initialize(); // 0x59C230
		void Cleanup(); // 0x59C0E0
		void Update(); // 0x404B70
		virtual void OnInitialize(); // original slot +16, 0x404B70
		virtual void OnCleanup(); // original slot +20, 0x404B70
	};

	extern ResourcesComponent* g_pResourcesInstance; // Original 0x70C660.
	GameComponent* __cdecl CreateResources(); // 0x45E110

	namespace ResourceComponentServices
	{
		void* Allocate(std::size_t size);
		void Free(void* block);
		JE::Resources::ResourceSystem* CreateManager();
		void DestroyManager(JE::Resources::ResourceSystem* manager);
	}
}
