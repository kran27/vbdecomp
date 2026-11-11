#include <winsock2.h>
#include "ClientGraphicsCommands.hpp"
#include "ClientWorldCommands.hpp"
#include "GameClient.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/StringFormat.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/D3DState.hpp"
#include "graphics/ClientCursor.hpp"
#include "gui/ConsoleInterface.hpp"
#include "gui/Manager.hpp"
#include "gui/PacketManager.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstdio>
#include <cstdlib>

namespace F3
{
	unsigned char g_samplingPaused = 1;

	void __cdecl CommandToggleGameOptions(CommandArguments*) //0x45edb0.
	{
		GameClient::ToggleOptionsPage(1);
	}

	void __cdecl CommandToggleHotKeyOptions(CommandArguments*) //0x45edc0.
	{
		GameClient::ToggleOptionsPage(2);
	}

	void __cdecl CommandToggleLoadGameOptions(CommandArguments*) //0x45edd0.
	{
		GameClient::ToggleOptionsPage(3);
	}

	void __cdecl CommandToggleMultiplayerOptions(CommandArguments*) //0x45ede0.
	{
		GameClient::ToggleOptionsPage(4);
	}

	void __cdecl CommandToggleVideoOptions(CommandArguments*) //0x45edf0.
	{
		GameClient::ToggleOptionsPage(5);
	}

	void __cdecl CommandToggleRain(CommandArguments*) //4606C0.
	{
		static_cast<GameClient*>(g_pNetworkInstance->client)->ToggleRain();
	}

	void __cdecl CommandToggleSnow(CommandArguments*) //4606D0.
	{
		static_cast<GameClient*>(g_pNetworkInstance->client)->ToggleSnow();
	}

	void __cdecl CommandToggleWind(CommandArguments*) //4606E0.
	{
		static_cast<GameClient*>(g_pNetworkInstance->client)->ToggleWind();
	}

	void CommandToggleScene() //588820.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		client->field0x108 = !client->field0x108;
	}

	void CommandToggleEntities() //588840.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		client->field0x109 = !client->field0x109;
	}

	void CommandToggleGUI() //588860.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		client->field0x10A = !client->field0x10A;
	}

	void __cdecl CommandToggleStartupMovies(CommandArguments*) //58B1B0.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleStartupMovies();
		std::string text;
		BYTE enabled = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x74->startupMovies;
		JE::FormatString(text, "Startup Movies: %s", enabled ? "on" : "off");
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88->Print(text.c_str());
	}

	void __cdecl CommandToggleBuilding(CommandArguments* arguments) //58BB90.
	{
		if (arguments->size() >= 2)
		{
			LONG building = std::atol((*arguments)[1]);
			static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleBuilding(building);
			std::string text;
			if (static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x74->GetBuildingVisible(building))
				JE::FormatString(text, "Building %d: roof shown");
			else
				JE::FormatString(text, "Building %d: roof hidden");
			// Original supplies no %d operand. Preserve that observed defect.
			static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88->Print(text.c_str());
		}
	}

	void __cdecl CommandToggleFogOfWar(CommandArguments*) //58B290.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleFogOfWar();
		std::string text;
		BYTE enabled = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x74->GetFogOfWarEnabled();
		JE::FormatString(text, "Fog of war: %s", enabled ? "on" : "off");
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88->Print(text.c_str());
	}

	void __cdecl CommandToggleStatistics(CommandArguments*) //5882A0.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x74->ToggleStatistics();
	}

	void __cdecl CommandToggleCameraFog(CommandArguments*) //5882F0.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleCameraFog();
	}

	void __cdecl CommandToggleFullscreen(CommandArguments*) //5882B0.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x74->ToggleFullscreen();
	}

	void __cdecl CommandToggleShadow(CommandArguments*) //5882D0.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleShadow();
	}

	void __cdecl CommandDebugLight(CommandArguments*) //588320; original shared5879B0 tail.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleDebugLights();
	}

	void __cdecl CommandDebugOctreeLight(CommandArguments*) //588330; original shared5879D0 tail.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleDebugOctreeLights();
	}

	void __cdecl CommandToggleHeightFog(CommandArguments*) //588340; original shared587A20/5810B0 tails.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleHeightFog();
	}

	void __cdecl CommandToggleBilinearFiltering(CommandArguments*) //5882E0.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleBilinearFiltering();
	}

	void __cdecl CommandToggleWireframe(CommandArguments*) //588300.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleWireframe();
	}

	void __cdecl CommandToggleTransparency(CommandArguments*) //588310.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleTransparency();
	}

	void __cdecl CommandToggleMipMapping(CommandArguments*) //588610.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleMipMapping();
	}

	void __cdecl CommandToggleOctreeDrawing(CommandArguments*) //588600.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleOctreeDrawing();
	}

	void __cdecl CommandToggleOctreeNodeDebug(CommandArguments*) //588350.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleOctreeNodeDebug();
	}

	void __cdecl CommandScreenCap(CommandArguments*) //588620.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->CaptureScreen();
	}

	void __cdecl CommandVRAM(CommandArguments*) //5888E0.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		Graphics* graphics = client->child0x74;
		if (graphics)
		{
			DWORD statistics[17];
			graphics->GetVRAMStatistics(statistics);
			JE::ConsoleInterface* console = client->child0x88;
			if (console)
			{
				console->Print("VRAM usage (est):");
				console->Print("================");
				console->Print("Current Geometry Data: %.2f (MB)", (statistics[3] + statistics[4]) * 0.00000095367431640625f);
				console->Print("    Vertex: %.2f Peak: %.2f (MB)", statistics[3] * 0.00000095367431640625f, statistics[1] * 0.00000095367431640625f);
				console->Print("    Index: %.2f Peak %.2f (MB)", statistics[4] * 0.00000095367431640625f, statistics[2] * 0.00000095367431640625f);
				console->Print("Current Texture Data: Base: %.2f Total: %.2f (MB)", statistics[15] * 0.00000095367431640625f, statistics[16] * 0.00000095367431640625f);
				console->Print("    Default: Base: %.2f Total: %.2f (MB)", statistics[9] * 0.00000095367431640625f, statistics[10] * 0.00000095367431640625f);
				console->Print("    Managed: Base: %.2f Total: %.2f (MB)", statistics[11] * 0.00000095367431640625f, statistics[12] * 0.00000095367431640625f);
				console->Print("    System: Base: %.2f Total: %.2f (MB)", statistics[13] * 0.00000095367431640625f, statistics[14] * 0.00000095367431640625f);
			}
		}
	}

	void __cdecl CommandListTextures(CommandArguments*) //588B10.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		if (client->child0x74) client->child0x74->PrintTexturePools();
	}

	void __cdecl CommandToggleSampling(CommandArguments*) //588B30.
	{
		if (g_samplingPaused) JE::LogDebugString("Resuming VTune sampling...\n");
		else JE::LogDebugString("Pausing VTune sampling...\n");
		g_samplingPaused = !g_samplingPaused;
	}

	void __cdecl CommandVSync(CommandArguments*) //588B60.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		Graphics* graphics = client->child0x74;
		if (graphics)
		{
			BYTE enabled = !graphics->GetVerticalSync();
			client->child0x74->SetVerticalSync(enabled);
			JE::ConsoleInterface* console = client->child0x88;
			if (console) console->Print("VSYNC %s", enabled ? "ON" : "OFF");
		}
	}

	void __cdecl CommandAdapterInfo(CommandArguments*) //588BC0.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		Graphics* graphics = client->child0x74;
		if (graphics)
		{
			JE::ConsoleInterface* console = client->child0x88;
			if (console)
			{
				char buffer[256];
				std::sprintf(buffer, "Adapter: %s", graphics->GetAdapterDescription());
				console->Print(buffer);
				JE::LogDebugString(buffer);
				JE::LogDebugString("\n");
				DWORD count = client->child0x74->GetModeCount();
				Gfx_D3DMode* modes = client->child0x74->GetModes();
				for (DWORD i = 0; i < count; ++i)
				{
					Gfx_D3DMode& mode = modes[i];
					const char* stencil = mode.stencil ? "Stencil" : "";
					const char* depth = mode.backBufferFormat == D3DFMT_X8R8G8B8 || mode.backBufferFormat == D3DFMT_A8R8G8B8 ? "32bpp" : "16bpp";
					std::sprintf(buffer, "%d.) %d x %d x %s @ %dHz %s", i, mode.display.Width, mode.display.Height, depth, mode.display.RefreshRate, stencil);
					console->Print(buffer);
					JE::LogDebugString(buffer);
					JE::LogDebugString("\n");
				}
			}
		}
	}

	void __cdecl CommandShowCursor(CommandArguments* arguments) //58B830.
	{
		BYTE visible = 1;
		if (arguments->size() == 2 && std::atol((*arguments)[1]) == 0) visible = 0;
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA4->SetVisible(visible);
	}

	void __cdecl CommandGuiOptimizationLevel(CommandArguments* arguments) //58B8B0.
	{
		if (arguments->size() == 2) ::GUI::g_Manager.packets->optimizationLevel = std::atol((*arguments)[1]);
	}

	void __cdecl CommandSetDisplayMode(CommandArguments* arguments) //58B8E0.
	{
		if (static_cast<int>(arguments->size()) >= 2)
		{
			JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
			if (client->child0x74)
			{
				DWORD index = std::atol((*arguments)[1]);
				if (client->child0x74->SelectDisplayMode(index))
				{
					Gfx_D3DMode* mode = client->child0x74->GetCurrentMode();
					::GUI::g_Manager.SetResolution(mode->display.Width, mode->display.Height);
					::GUI::g_Manager.RefreshPositions();
					const char* stencil = mode->stencil ? "Stencil" : "";
					const char* depth = mode->backBufferFormat == D3DFMT_X8R8G8B8 || mode->backBufferFormat == D3DFMT_A8R8G8B8 ? "32bpp" : "16bpp";
					char buffer[256];
					std::sprintf(buffer, "Mode = %d x %d x %s @ %dHz %s", mode->display.Width, mode->display.Height, depth, mode->display.RefreshRate, stencil);
					JE::LogDebugString(buffer);
					JE::LogDebugString("\n");
					JE::ConsoleInterface* console = client->child0x88;
					if (console) console->Print(buffer);
				}
			}
		}
	}

	void __cdecl CommandToggleFOWAutomap(CommandArguments*) //58B370.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleFogOfWarAutomap();
		std::string text;
		JE::FormatString(text, "Fog of war automap: %s", g_GfxSystem->Flag2194() ? "on" : "off");
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88->Print(text.c_str());
	}

	void __cdecl CommandToggleVisualEffects(CommandArguments*) //58E380.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleVisualEffects();
		std::string text;
		switch (static_cast<JE::Client*>(g_pNetworkInstance->client)->field0x104)
		{
		case 0: text.assign("Visual Effects: On"); break;
		case 1: text.assign("Visual Effects: Not drawn"); break;
		case 2: text.assign("Visual Effects: Off"); break;
		}
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88->Print(text.c_str());
	}

	void __cdecl CommandToggleWater(CommandArguments*) //58E7E0.
	{
		static_cast<JE::Client*>(g_pNetworkInstance->client)->ToggleWater();
		std::string text;
		switch (static_cast<JE::Client*>(g_pNetworkInstance->client)->field0x10C)
		{
		case 0: text.assign("Water: On"); break;
		case 1: text.assign("Water: Not drawn"); break;
		case 2: text.assign("Water: Off"); break;
		}
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x88->Print(text.c_str());
	}

	unsigned char ExecuteClientGraphicsCommand(JE::Net::UInt32 originalVA, CommandArguments* arguments)
	{
		switch (originalVA)
		{
		case 0x45edb0: CommandToggleGameOptions(arguments); return 1;
		case 0x45edc0: CommandToggleHotKeyOptions(arguments); return 1;
		case 0x45edd0: CommandToggleLoadGameOptions(arguments); return 1;
		case 0x45ede0: CommandToggleMultiplayerOptions(arguments); return 1;
		case 0x45edf0: CommandToggleVideoOptions(arguments); return 1;
		case 0x4606C0: CommandToggleRain(arguments); return 1;
		case 0x4606D0: CommandToggleSnow(arguments); return 1;
		case 0x4606E0: CommandToggleWind(arguments); return 1;
		case 0x58B290: CommandToggleFogOfWar(arguments); return 1;
		case 0x58B370: CommandToggleFOWAutomap(arguments); return 1;
		case 0x58E380: CommandToggleVisualEffects(arguments); return 1;
		case 0x58B1B0: CommandToggleStartupMovies(arguments); return 1;
		case 0x58BB90: CommandToggleBuilding(arguments); return 1;
		case 0x5882A0: CommandToggleStatistics(arguments); return 1;
		case 0x5882F0: CommandToggleCameraFog(arguments); return 1;
		case 0x5882B0: CommandToggleFullscreen(arguments); return 1;
		case 0x5882D0: CommandToggleShadow(arguments); return 1;
		case 0x588320: CommandDebugLight(arguments); return 1;
		case 0x588330: CommandDebugOctreeLight(arguments); return 1;
		case 0x588340: CommandToggleHeightFog(arguments); return 1;
		case 0x5882E0: CommandToggleBilinearFiltering(arguments); return 1;
		case 0x588300: CommandToggleWireframe(arguments); return 1;
		case 0x588310: CommandToggleTransparency(arguments); return 1;
		case 0x588610: CommandToggleMipMapping(arguments); return 1;
		case 0x588600: CommandToggleOctreeDrawing(arguments); return 1;
		case 0x588350: CommandToggleOctreeNodeDebug(arguments); return 1;
		case 0x588620: CommandScreenCap(arguments); return 1;
		case 0x588820: CommandToggleScene(); return 1;
		case 0x588840: CommandToggleEntities(); return 1;
		case 0x588860: CommandToggleGUI(); return 1;
		case 0x5888E0: CommandVRAM(arguments); return 1;
		case 0x588B10: CommandListTextures(arguments); return 1;
		case 0x588B30: CommandToggleSampling(arguments); return 1;
		case 0x588B60: CommandVSync(arguments); return 1;
		case 0x588BC0: CommandAdapterInfo(arguments); return 1;
		case 0x58B830: CommandShowCursor(arguments); return 1;
		case 0x58B8B0: CommandGuiOptimizationLevel(arguments); return 1;
		case 0x58B8E0: CommandSetDisplayMode(arguments); return 1;
		default: return 0;
		}
	}
}
