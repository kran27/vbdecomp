#include "SkinServices.hpp"
#include "configuration/Configuration.hpp"
#include "configuration/ResourceConfigurationRead.hpp"
#include "resources/ResourceSystem.hpp"

namespace GUI
{
	namespace SkinServices
	{
		BYTE ReadFile(const std::string& name, JE::Configuration::Settings& settings)
		{
			return JE::Configuration::ReadFile(name, settings);
		}
		BYTE ReadResource(const std::string& name, JE::Configuration::Settings& settings)
		{
			return JE::Configuration::ReadResourceConfigurationFile(name, settings);
		}
		JE::Resources::ResourceData* Demand(const char* name)
		{
			return JE::Resources::g_pResourceSystem->Demand(name, 1);
		}
		DWORD Size(JE::Resources::ResourceData* resource) { return resource->size; }
		const BYTE* Data(JE::Resources::ResourceData* resource)
		{
			return static_cast<const BYTE*>(resource->data);
		}
		void Release(JE::Resources::ResourceData* resource)
		{
			JE::Resources::g_pResourceSystem->ReleaseResource(resource);
		}
	}
}
