#pragma once

#include "configuration/ConfigurationTypes.hpp"
#include <windows.h>

namespace JE { namespace Resources { struct ResourceData; } }

namespace GUI
{
	// Explicit source boundary to existing reader/resource implementations whose
	// headers still need legacy migration. No old object storage is overlaid.
	namespace SkinServices
	{
		BYTE ReadFile(const std::string& name, JE::Configuration::Settings& settings);
		BYTE ReadResource(const std::string& name, JE::Configuration::Settings& settings);
		JE::Resources::ResourceData* Demand(const char* name);
		DWORD Size(JE::Resources::ResourceData* resource);
		const BYTE* Data(JE::Resources::ResourceData* resource);
		void Release(JE::Resources::ResourceData* resource);
	}
}
