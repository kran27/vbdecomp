#include "EngineClient.hpp"
#include "EngineClientBinding.hpp"
#include "gui/EditTokenInterface.hpp"
#include "gui/FadeManager.hpp"
#include "audio/Audio.hpp"
#include "gui/Interfaces.hpp"
#include "gui/ConsoleInterface.hpp"
#include "gui/DebugWindow.hpp"
#include "gui/DebugToolsInterface.hpp"
#include "gui/FontTestInterface.hpp"
#include "game/GameSkillMenu.hpp"
#include "gui/Win32Input.hpp"
#include "graphics/ClientInput.hpp"
#include "graphics/ClientCursor.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/Video.hpp"
#include "network/CommandProcessor.hpp"
#include "graphics/CameraControl.hpp"
#include "graphics/VfxVeg.hpp"
#include "gui/DialogueInterface.hpp"
#include "graphics/FloatingTextInterface.hpp"

#include <new>
#include <cstdlib>
#include <stdexcept>

namespace JE
{
	void Client::OnInterfaceMessage(DWORD, const char*) {} //408090, actual empty hook.
	void Client::ClearInputSelection() //588220.
	{
		field0xE0 = 0;
		field0xDC = 0;
	}

	void Client::EditToken(DWORD stringReference, const char* tokenName) //587AB0.
	{
		// Original forwarding thunk does not check its required child.
		child0xB8->EditToken(stringReference, tokenName);
	}

	Client::Client(JE::Net::UInt16 port)
		: Net::Client(port), child0x70(NULL), child0x74(NULL), child0x78(NULL), child0x7C(NULL),
		child0x80(NULL), child0x84(NULL), child0x88(NULL), child0x8C(NULL), child0x90(NULL),
		child0x94(NULL), child0x98(NULL), child0x9C(NULL), child0xA0(NULL), child0xA4(NULL),
		child0xA8(NULL), child0xAC(NULL), child0xB0(NULL), child0xB4(NULL), child0xB8(NULL),
		clientID(0), field0xC0(0), field0xC4(0), field0xC8(0), field0xCC(0), field0xD0(0),
		field0xD2(0), field0xD4(0), field0xD8(0), field0xDC(0), field0xE0(0), field0xE4(0),
		field0xEC(0), field0xF0(0), field0xF4(0), child0xF8(NULL), field0xFC(0), field0x100(1),
		field0x104(0), field0x108(0), field0x109(0), field0x10A(0), field0x10C(0), field0x110(0),
		field0x112(0), field0x114(0)
	{
		field0xD8 = EngineClientServices::AccumulatedTime();
		// Every original registration remains observable, including aliases and
		// callbacks whose game behavior is still an explicit throwing boundary.
		struct Registration { const char* name; JE::Net::UInt32 address; };
		static const Registration commands[] =
		{
			{ "StartMap", 0x58DB80 },
			{ "RequestNewGame", 0x58DBE0 },
			{ "NewGame", 0x58DBE0 },
			{ "Connect", 0x58DC50 },
			{ "Disconnect", 0x588290 },
			{ "Kill", 0x58A7B0 },
			{ "StartDialogue", 0x58A7D0 },
			{ "EndDialogue", 0x58A7E0 },
			{ "EndDialog", 0x58A7E0 },
			{ "CreateEntity", 0x58DDA0 },
			{ "SetScript", 0x58DF10 },
			{ "ToggleStatistics", 0x5882A0 },
			{ "ToggleFullscreen", 0x5882B0 },
			{ "ToggleLighting", 0x5882C0 },
			{ "ToggleShadow", 0x5882D0 },
			{ "ToggleBilinearFiltering", 0x5882E0 },
			{ "ToggleStartupMovies", 0x58B1B0 },
			{ "ToggleCameraFog", 0x5882F0 },
			{ "ToggleWireframe", 0x588300 },
			{ "ToggleTransparency", 0x588310 },
			{ "ToggleOctreeDrawing", 0x588600 },
			{ "ToggleMipMapping", 0x588610 },
			{ "DebugLight", 0x588320 },
			{ "DebugOctreeLight", 0x588330 },
			{ "ToggleHeightFog", 0x588340 },
			{ "ToggleOctreeNodeDebug", 0x588350 },
			{ "ToggleFogOfWar", 0x58B290 },
			{ "ToggleFOWAutomap", 0x58B370 },
			{ "ToggleFOWLOS", 0x58E340 },
			{ "ToggleVisualEffects", 0x58E380 },
			{ "ToggleWater", 0x58E7E0 },
			{ "ToggleBuilding", 0x58BB90 },
			{ "CameraReset", 0x58A7F0 },
			{ "CameraSetTarget", 0x58B6E0 },
			{ "CameraAzimuth", 0x58B450 },
			{ "CameraAzimuthInc", 0x588360 },
			{ "CameraAzimuthDec", 0x5883A0 },
			{ "CameraAzimuthSpeed", 0x58B4A0 },
			{ "CameraElevation", 0x58B500 },
			{ "CameraElevationSpeed", 0x58B550 },
			{ "CameraDistance", 0x58B5B0 },
			{ "CameraDistanceSpeed", 0x58B600 },
			{ "CameraFOV", 0x58B660 },
			{ "CameraTopDown", 0x5883E0 },
			{ "CameraTop", 0x588400 },
			{ "CameraDown", 0x588420 },
			{ "CameraScrollSpeed", 0x58B7D0 },
			{ "CameraScrollLeft_On", 0x588440 },
			{ "CameraScrollRight_On", 0x588450 },
			{ "CameraScrollUp_On", 0x588460 },
			{ "CameraScrollDown_On", 0x588470 },
			{ "CameraScrollLeft_Off", 0x588480 },
			{ "CameraScrollRight_Off", 0x588490 },
			{ "CameraScrollUp_Off", 0x5884A0 },
			{ "CameraScrollDown_Off", 0x5884B0 },
			{ "CameraZoomIn_On", 0x5884C0 },
			{ "CameraZoomIn_Off", 0x5884E0 },
			{ "CameraZoomOut_On", 0x588500 },
			{ "CameraZoomOut_Off", 0x588520 },
			{ "CameraRotateLeft", 0x588540 },
			{ "CameraRotateRight", 0x588560 },
			{ "CameraTiltUp", 0x588580 },
			{ "CameraTiltDown", 0x5885A0 },
			{ "CameraMouseRotate_On", 0x5885C0 },
			{ "CameraMouseRotate_Off", 0x5885D0 },
			{ "CameraToggleConstraints", 0x5885E0 },
			{ "ScreenCap", 0x588620 },
			{ "ShowCursor", 0x58B830 },
			{ "Warp", 0x58B880 },
			{ "GuiOptimizationLevel", 0x58B8B0 },
			{ "ConsoleWindow", 0x588630 },
			{ "DebugWindow", 0x588690 },
			{ "StackWindow", 0x5886F0 },
			{ "DebugTools", 0x588750 },
			{ "FontTest", 0x5887B0 },
			{ "GetGlobal", 0x58DFD0 },
			{ "SetGlobal", 0x58E1B0 },
			{ "ToggleScene", 0x588820 },
			{ "ToggleEntities", 0x588840 },
			{ "ToggleGUI", 0x588860 },
			{ "ToggleScripts", 0x588880 },
			{ "DisableScripts", 0x58E4A0 },
			{ "EnableScripts", 0x58E5B0 },
			{ "OneScriptPass", 0x58E6C0 },
			{ "VRAM", 0x5888E0 },
			{ "ListTextures", 0x588B10 },
			{ "ToggleSampling", 0x588B30 },
			{ "vsync", 0x588B60 },
			{ "AdapterInfo", 0x588BC0 },
			{ "SetDisplaymode", 0x58B8E0 },
			{ "MemoryPoolTest", 0x589580 },
			{ "FadeIn", 0x58BA10 },
			{ "FadeOut", 0x58BAA0 },
			{ "MovieTest", 0x588D00 },
			{ "EditToken", 0x58BB30 },
		};
		for (unsigned i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i)
			EngineClientServices::RegisterCommand(commands[i].name, commands[i].address);
	}

	Client::~Client() { Client::Cleanup(); }

	void Client::Cleanup()
	{
		if (child0xF8)
		{
			EngineClientServices::DeleteChild(child0xF8);
			child0xF8 = NULL;
		}
		if (child0xB8) EngineClientServices::DeleteChild(child0xB8);
		if (child0xB0) EngineClientServices::DeleteChild(child0xB0);
		if (child0xAC) EngineClientServices::DeleteChild(child0xAC);
		if (child0x9C) EngineClientServices::DeleteChild(child0x9C);
		if (child0xA0) EngineClientServices::DeleteChild(child0xA0);
		if (child0xA4) EngineClientServices::DeleteChild(child0xA4);
		if (child0xA8) EngineClientServices::DeleteChild(child0xA8);
		if (child0xB4) EngineClientServices::DeleteChild(child0xB4);
		ClientChild0x84* captured = child0x84;
		if (captured)
		{
			EngineClientServices::DestroyChild(captured);
			EngineClientServices::FreeChild(captured);
		}
		if (child0x88) EngineClientServices::DeleteChild(child0x88);
		if (child0x8C) EngineClientServices::DeleteChild(child0x8C);
		if (child0x94) EngineClientServices::DeleteChild(child0x94);
		if (child0x98) EngineClientServices::DeleteChild(child0x98);
		if (child0x80) EngineClientServices::DeleteChild(child0x80);
		if (child0x7C) EngineClientServices::DeleteChild(child0x7C);
		if (child0x78) EngineClientServices::DeleteChild(child0x78);
		if (child0x74) EngineClientServices::DeleteChild(child0x74);
		if (child0x70) EngineClientServices::DeleteChild(child0x70);
		EngineClientServices::CleanupNetClient(*this);
	}

	void Client::CreateConnection() { connection = CreateServerConnection(); }

	Net::Connection* Client::CreateServerConnection()
	{
		return new Net::EngineClientServerConnection;
	}

	namespace EngineClientServices
	{
		void CleanupNetClient(Net::Client& client) { client.Net::Client::Cleanup(); }
		void DeleteChild(ClientChild0x70* audio) { delete audio; }
		void DeleteChild(ClientChild0x74* graphics) { delete graphics; }
		void DeleteChild(ClientChild0x78* input) { delete input; }
		void DeleteChild(ClientChild0x7C* child) { delete child; }
		void DeleteChild(ClientChild0x80* hotKeys) { delete hotKeys; }
		void DeleteChild(ClientChild0x88* console) { delete console; }
		void DeleteChild(ClientChild0x8C* child)
		{
			delete child;
		}
		void DeleteChild(ClientChild0x94* child)
		{
			delete child;
		}
		void DeleteChild(ClientChild0x98* child)
		{
			delete child;
		}
		void DeleteChild(ClientChild0x9C* child)
		{
			delete child;
		}
		void DeleteChild(ClientChild0xA0* child)
		{
			delete child;
		}
		void DeleteChild(ClientChild0xA4* child) { delete child; } //588E29, actual virtual deleting destructor.
		void DeleteChild(ClientChild0xA8* child) { delete child; } //588E39; FadeManager deliberately omits GUI deletes.
		void DeleteChild(ClientChild0xAC* child)
		{
			delete child;
		}
		void DeleteChild(ClientChild0xB0* child)
		{
			delete child;
		}
		void DeleteChild(ClientChild0xB4* child)
		{
			delete child;
		}
		void DeleteChild(ClientChild0xB8* child) { delete child; }
		void DeleteChild(ClientChild0xF8* child) { delete child; } //588DA1, direct delete rather than cache Release.
		void DestroyChild(ClientChild0x84* child) { child->CameraControl::~CameraControl(); }
		void FreeChild(ClientChild0x84* child) { std::free(child); }
	}

	namespace EngineClientConnectionServices
	{
		void* Allocate(std::size_t size) { return ::operator new(size); }
		void Construct(Net::EngineClientServerConnection* connection)
		{
			::new (connection) Net::EngineClientServerConnection;
		}
		void Free(void* block) throw() { ::operator delete(block); }
	}
}
