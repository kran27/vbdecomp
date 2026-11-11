#pragma once

#include <windows.h>
#include <d3d8.h>
#include "configuration/ConfigurationTypes.hpp"

namespace JE
{
	//Non-owning aliases bind the actual frontend lighting flag and backend
	//flag/limit. The default constructor retains independent fixture storage.
	//This factor is not an allocated or overlaid Gfx_Engine/Graphics owner.
	class GraphicsLightingState
	{
	private:
		BYTE standaloneConfiguredLighting;
		BYTE standaloneEnabled;
		DWORD standaloneActiveLightLimit;

	public:
		GraphicsLightingState(); //580E20/581C30 configured1;5BFE10 backend flag/limit0.
		GraphicsLightingState(BYTE& configured, BYTE& backendEnabled, DWORD& backendLimit);
		void ReadConfiguration(const Configuration::Settings& settings); //581C30, main56AF30 settings.
		void InitializeDeviceLighting(); //Explicit5C0B80..5C0BA8 factor, after real device creation.
		void ApplyConfiguration(); //5819E2..5819F1, after device initialization.
		HRESULT SetConfiguredLighting(BYTE enabled); //581600, frontend publication before backend setter.
		HRESULT SetLighting(BYTE enabled); //5BDC90, raw BYTE publication then actual D3D cache.
		BYTE Lighting() const; //5C01F0, independent of device limits.
		DWORD ActiveLightLimit() const; //5C00E0.
		DWORD MaximumActiveLights() const; //5C00F0, current selected-device caps.
		DWORD SetActiveLightLimit(DWORD limit); //5C0530, unsigned cap clamp.
		BYTE& configuredLighting; //Frontend Graphics+54h, or standalone storage.
		BYTE& enabled; //Backend Engine+88Eh (2190 decimal), or standalone storage.
		DWORD& activeLightLimit; //Backend Engine+50h, or standalone storage.

	private:
		GraphicsLightingState(const GraphicsLightingState&);
		GraphicsLightingState& operator=(const GraphicsLightingState&);
	};
}
