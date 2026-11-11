#pragma once

#include <windows.h>
#include "configuration/ConfigurationTypes.hpp"

class Gfx_System_Interface;
struct Gfx_D3DMode;

namespace JE
{
	//Actual frontend scalar policy, not a complete Graphics owner or memory overlay.
	//The full constructor still requires the genuine Gfx_Engine lifetime.
	struct GraphicsConfiguration
	{
		GraphicsConfiguration(); //Corresponding580E20 stores; display request is unwritten.
		void Read(const Configuration::Settings& engine, const Configuration::Settings& main); //581C30.
		void Write(Configuration::Settings& main) const; //582BB0, memory only.
		BYTE initialized;
		DWORD adapter, device, displayMode;
		BYTE fullscreen;
		DWORD width, height, refresh;
		BYTE color32, vsync, statistics, clearRed, clearGreen, clearBlue;
		DWORD shadowMode;
		BYTE lighting, linearFiltering, startupMovies, wireframe, transparency, bounds;
	};

	struct GraphicsDisplayRequest
	{
		BYTE fullscreen, color32;
		DWORD width, height, refresh;
		Gfx_D3DMode* modes; //5818ED capture, borrowed across the subsequent device call.
	};
	GraphicsDisplayRequest SelectGraphicsDisplayMode(GraphicsConfiguration& state,
		Gfx_System_Interface*& backend); //581825..58194F; no device-creation claim.
	void PublishGraphicsDisplayMode(GraphicsConfiguration& state,
		const GraphicsDisplayRequest& request); //5819B2..5819E5, after successful device creation.
}
