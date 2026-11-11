#include "ResourcesComponent.hpp"
#include "CommonException.hpp"
#include "ConfigurationObject.hpp"
#include "EngineSettings.hpp"
#include "SaveDirectory.hpp"
#include "configuration/ResourceConfigurationRead.hpp"
#include "resources/ResourceSystem.hpp"

#include <new>

namespace F3
{
	ResourcesComponent* g_pResourcesInstance = NULL;
	static const char g_szDuplicateResources[] = "Resources object already exists.";

	ResourcesComponent::ResourcesComponent()
	{
		if (g_pResourcesInstance)
			throw Common::Exception(g_szDuplicateResources);
		// A later failure does not undo this original publication.
		g_pResourcesInstance = this;
		JE::Resources::g_pResourceSystem = new JE::Resources::ResourceSystem;
		JE::Resources::g_pResourceSystem->Initialize(GetCurrentWorkingDirectory(), *GetSettingsObject());
		JE::Configuration::ReadResourceConfigurationFile(std::string("sys.ini"), *GetEngineSettings());
	}

	ResourcesComponent::~ResourcesComponent()
	{
		// Destruction only clears the component singleton; Cleanup owns the manager.
		g_pResourcesInstance = NULL;
	}

	void ResourcesComponent::Initialize() { OnInitialize(); }
	void ResourcesComponent::OnInitialize() { }
	void ResourcesComponent::OnCleanup() { }
	void ResourcesComponent::Update() { }

	void ResourcesComponent::Cleanup()
	{
		OnCleanup();
		JE::Resources::ResourceSystem* manager = JE::Resources::g_pResourceSystem;
		if (manager)
		{
			manager->~ResourceSystem();
			::operator delete(manager);
		}
		// Original 0x59C0E0 does not clear the manager global after destruction.
	}

	GameComponent* __cdecl CreateResources()
	{
		return new ResourcesComponent;
	}


	namespace ResourceComponentServices
	{
		void* Allocate(std::size_t size) { return ::operator new(size); }
		void Free(void* block) { ::operator delete(block); }
		JE::Resources::ResourceSystem* CreateManager() { return new JE::Resources::ResourceSystem; }
		void DestroyManager(JE::Resources::ResourceSystem* manager)
		{
			// The original calls the destructor then frees the captured allocation.
			// A destructor departure does not free the game-owned manager storage.
			manager->~ResourceSystem();
			::operator delete(manager);
		}
	}
}
