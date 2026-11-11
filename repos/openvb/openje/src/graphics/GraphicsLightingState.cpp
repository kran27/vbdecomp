#include "GraphicsLightingState.hpp"
#include "D3DState.hpp"
#include <cstdlib>
#include <stdexcept>

namespace JE
{
	GraphicsLightingState::GraphicsLightingState()
		: standaloneConfiguredLighting(1), standaloneEnabled(0),
		standaloneActiveLightLimit(0), configuredLighting(standaloneConfiguredLighting),
		enabled(standaloneEnabled), activeLightLimit(standaloneActiveLightLimit) {}

	GraphicsLightingState::GraphicsLightingState(BYTE& configured,
		BYTE& backendEnabled, DWORD& backendLimit)
		: configuredLighting(configured), enabled(backendEnabled),
		activeLightLimit(backendLimit)
	{
		//Binding must not initialize or copy the actual owners' live state.
		//The unused standalone backing fields remain unwritten in this mode.
	}

	void GraphicsLightingState::ReadConfiguration(const Configuration::Settings& settings)
	{
		configuredLighting = 1; //581C6E resets this before reading the main configuration.
		Configuration::Settings::const_iterator section = settings.find("Graphics");
		if (section == settings.end()) return;
		Configuration::Values::const_iterator entry = section->second.find("enable lighting");
		if (entry != section->second.end()) configuredLighting = std::atol(entry->second.c_str()) > 0;
	}
	void GraphicsLightingState::InitializeDeviceLighting()
	{
		//5C0B84 reads the live cap; the below-eight path reads it again before
		//the actual setter performs its own clamp/read. No manual hardware limit.
		DWORD limit = MaximumActiveLights() >= 8 ? 8 : MaximumActiveLights();
		SetActiveLightLimit(limit);
		SetLighting(0);
	}
	void GraphicsLightingState::ApplyConfiguration() { SetLighting(configuredLighting); }
	HRESULT GraphicsLightingState::SetConfiguredLighting(BYTE value)
	{
		configuredLighting = value;
		return SetLighting(value);
	}
	HRESULT GraphicsLightingState::SetLighting(BYTE value)
	{
		enabled = value;
		return g_D3D.SetRenderState(D3DRS_LIGHTING, value);
	}
	BYTE GraphicsLightingState::Lighting() const { return enabled; }
	DWORD GraphicsLightingState::ActiveLightLimit() const { return activeLightLimit; }
	DWORD GraphicsLightingState::MaximumActiveLights() const
	{
		if (!g_D3D.currentDevice)
			throw std::logic_error("Engine5C00FB: actual selected D3D device required for light caps");
		return g_D3D.currentDevice->caps.MaxActiveLights;
	}
	DWORD GraphicsLightingState::SetActiveLightLimit(DWORD limit)
	{
		DWORD maximum = MaximumActiveLights();
		activeLightLimit = limit <= maximum ? limit : maximum;
		return activeLightLimit;
	}
}
