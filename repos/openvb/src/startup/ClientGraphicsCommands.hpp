#pragma once

#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	void __cdecl CommandToggleFullscreen(CommandArguments* arguments); //5882B0.
	void __cdecl CommandToggleStartupMovies(CommandArguments* arguments); //58B1B0.
	void __cdecl CommandToggleBuilding(CommandArguments* arguments); //58BB90.
	void __cdecl CommandToggleShadow(CommandArguments* arguments); //5882D0.
	void __cdecl CommandDebugLight(CommandArguments* arguments); //588320; shared5879B0.
	void __cdecl CommandDebugOctreeLight(CommandArguments* arguments); //588330; shared5879D0.
	void __cdecl CommandToggleHeightFog(CommandArguments* arguments); //588340; shared587A20/5810B0.
	void __cdecl CommandToggleOctreeNodeDebug(CommandArguments* arguments); //588350.
	void __cdecl CommandScreenCap(CommandArguments* arguments); //588620.
	void __cdecl CommandVRAM(CommandArguments* arguments); //5888E0.
	void __cdecl CommandListTextures(CommandArguments* arguments); //588B10.
	void __cdecl CommandToggleSampling(CommandArguments* arguments); //588B30.
	void __cdecl CommandVSync(CommandArguments* arguments); //588B60.
	void __cdecl CommandAdapterInfo(CommandArguments* arguments); //588BC0.
	void __cdecl CommandShowCursor(CommandArguments* arguments); //58B830.
	void __cdecl CommandGuiOptimizationLevel(CommandArguments* arguments); //58B8B0.
	void __cdecl CommandSetDisplayMode(CommandArguments* arguments); //58B8E0.
	unsigned char ExecuteClientGraphicsCommand(JE::Net::UInt32 originalVA, CommandArguments* arguments);
	extern unsigned char g_samplingPaused; //6FF2A8, initial1; no VTune API was called in the original.
}
