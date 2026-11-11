#include "EngineClientInitialize.hpp"
#include "startup/NetworkComponent.hpp"
#include "audio/AudioFactory.hpp"
#include "gui/Interfaces.hpp"
#include "gui/ConsoleInterface.hpp"
#include "gui/EditTokenInterface.hpp"
#include "gui/FadeManager.hpp"
#include "game/GameSkillMenu.hpp"
#include "gui/DebugWindow.hpp"
#include "gui/DebugToolsInterface.hpp"
#include "gui/FontTestInterface.hpp"
#include "gui/Win32Input.hpp"
#include "graphics/ClientInput.hpp"
#include "graphics/ClientCursor.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/Video.hpp"
#include "network/CommandProcessor.hpp"
#include "graphics/CameraControl.hpp"
#include "graphics/VfxVeg.hpp"
#include "gui/DialogueInterface.hpp"

#include <stdexcept>
#include <new>
#include <cstring>
#include <cstdlib>

namespace JE
{
	void Client::Initialize()
	{
		namespace Services = EngineClientInitializationServices;
		Services::InitializeNetClient(*this);
		child0x70 = CreateChild0x70();
		Services::InitializeChild(child0x70);
		Services::PrepareChild(child0x70);
		child0x74 = CreateChild0x74();
		Services::InitializeChild(child0x74);
		Services::ConfigureChild(child0x74, 1);
		child0x78 = CreateChild0x78();
		Services::InitializeChild(child0x78);
		child0x7C = CreateChild0x7C();
		Services::InitializeChild(child0x7C);
		child0x80 = CreateChild0x80();
		Services::InitializeChild(child0x80);
		child0x84 = CreateChild0x84();
		Services::InitializeChild(child0x84);
		child0x88 = CreateChild0x88();
		Services::ShowChild(child0x88, 1);
		child0x8C = CreateChild0x8C();
		Services::ShowChild(child0x8C, 1);
		Services::SetDimensions(child0x8C, 420, 110);
		Services::SizeChild(child0x8C, 1024, 768);
		child0x90 = CreateChild0x8C();
		Services::ShowChild(child0x90, 1);
		Services::SetDimensions(child0x90, 300, 420);
		Services::SizeChild(child0x90, 1024, 768);
		child0x94 = CreateChild0x94();
		Services::ShowChild(child0x94, 1);
		Services::SizeChild(child0x94, 1024, 768);
		child0xB4 = CreateChild0xB4();
		Services::InitializeChild(child0xB4);
		Services::PrepareChild(child0xB4);
		child0xA8 = CreateChild0xA8();
		Services::InitializeChild(child0xA8);
		Services::FinishChild(child0xA8);
		child0xA4 = CreateChild0xA4();
		Services::InitializeChild(child0xA4);
		Services::FinishChild(child0xA4);
		child0xA0 = CreateChild0xA0();
		Services::InitializeChild(child0xA0);
		Services::FinishChild(child0xA0);
		child0x9C = CreateChild0x9C();
		child0xAC = CreateChild0xAC();
		child0xB0 = CreateChild0xB0();
		child0xB8 = CreateChild0xB8();
		float selectionScale;
		std::memcpy(&selectionScale, &Services::g_SelectionEffectArgument, sizeof(selectionScale));
		child0xF8 = GetVfxVegCache().Acquire("FX_Selection_Critter_Medium_01.veg", selectionScale);
		field0x108 = 1;
		field0x10A = 1;
		if (F3::g_pNetworkInstance->GetMode() & 1) Services::ConnectLocalhost(*this);
	}

	ClientChild0x70* Client::CreateChild0x70() { return new Audio; }
	ClientChild0x74* Client::CreateChild0x74() { return new Graphics; }
	ClientChild0x78* Client::CreateChild0x78() { return new Input::Win32Input; } //587BD0.
	ClientChild0x7C* Client::CreateChild0x7C() { return new ConsoleCommandProcessor; } //587C30.
	ClientChild0x80* Client::CreateChild0x80()
	{
		return new ClientHotKeys(*child0x78, child0x7C); //587C90, capture children after allocation.
	}
	ClientChild0x84* Client::CreateChild0x84() { return new CameraControl(this); } //587D00.
	ClientChild0x88* Client::CreateChild0x88() { return new ConsoleInterface("Console"); }
	ClientChild0x8C* Client::CreateChild0x8C()
	{
		return new DebugWindow("Debug");
	}
	ClientChild0x94* Client::CreateChild0x94()
	{
		return new DebugToolsInterface("DebugTools");
	}
	ClientChild0x9C* Client::CreateChild0x9C()
	{
		return new DialogueInterface("DialogueInterface"); //58C080.
	}
	ClientChild0x98* Client::CreateChild0x98()
	{
		return new FontTestInterface("FontTest");
	}
	ClientChild0xA0* Client::CreateChild0xA0() //587E80, actual Interfaces owner.
	{
		return new Interfaces;
	}
	ClientChild0xA4* Client::CreateChild0xA4() //587D60; GameClient overrides with its actual cursor.
	{
		return new CursorManager;
	}
	ClientChild0xA8* Client::CreateChild0xA8() //587DC0, actual FadeManager18h owner.
	{
		return new FadeManager;
	}
	ClientChild0xB0* Client::CreateChild0xB0() //58C200, engine CSMenuInterface.
	{
		return new CSMenuInterface("CSMenuInterface");
	}
	ClientChild0xB4* Client::CreateChild0xB4() //587E20.
	{
		return new ::Video;
	}

	ClientChild0xB8* Client::CreateChild0xB8() { return new EditTokenInterface("EditToken"); } //58C2C0.

	namespace EngineClientInitializationServices
	{
		JE::Net::UInt32 g_SelectionEffectArgument = 0x41200000; // Original data bytes.
	}

	namespace EngineClientInitializationServices
	{
		void InitializeNetClient(Net::Client& client) { client.Net::Client::Initialize(); }
		void InitializeChild(ClientChild0x70* audio) { audio->Initialize(); }
		void PrepareChild(ClientChild0x70* audio) { audio->Prepare(); }
		void InitializeChild(ClientChild0x74* graphics) { graphics->ReadSettings(); }
		void ConfigureChild(ClientChild0x74* graphics, int value) { graphics->Initialize(value); }
		void InitializeChild(ClientChild0x78* input) { input->Initialize(); }
		void InitializeChild(ClientChild0x7C* child) { child->Initialize(); }
		void InitializeChild(ClientChild0x80* hotKeys) { hotKeys->Initialize(); }
		void InitializeChild(ClientChild0x84* child) { child->LoadConfiguration(); }
		void ShowChild(ClientChild0x88* console, int hidden) { console->SetHidden(BYTE(hidden)); }
		void ShowChild(ClientChild0x8C* child, int hidden) { child->SetHidden(BYTE(hidden)); }
		void SetDimensions(ClientChild0x8C* child, int width, int height) { child->Resize(width, height); }
		void SizeChild(ClientChild0x8C* child, int x, int y) { child->SetPosition(x, y); }
		void ShowChild(ClientChild0x94* child, int hidden) { child->SetHidden(BYTE(hidden)); }
		void SizeChild(ClientChild0x94* child, int x, int y) { child->SetPosition(x, y); }
		void InitializeChild(ClientChild0xB4* child) { child->Initialize(); }
		void PrepareChild(ClientChild0xB4* child) { child->PrepareUI(); }
		void InitializeChild(ClientChild0xA8* child) { child->Initialize(); } //58F1BB, virtual+4.
		void FinishChild(ClientChild0xA8* child) { child->InitializeUI(); } //58F1C6, virtual+8.
		void InitializeChild(ClientChild0xA4* child) { child->OnCleanup(); } //58F1DD, original empty virtual+4.
		void FinishChild(ClientChild0xA4* child) { child->Initialize(); } //58F1E8, real virtual+8 (Game437BB0).
		void InitializeChild(ClientChild0xA0* child) { child->Initialize(); }
		void FinishChild(ClientChild0xA0* child) { child->Activate(); }
		ClientEffectManager* GetEffectManager() { return &GetVfxVegCache(); } //4E1DE0.
		ClientChild0xF8* DemandEffect(ClientEffectManager* manager, const char* name, JE::Net::UInt32 argument)
		{
			//58F24A captures6FF0D8 before the getter.4E2AD0 forwards this raw
			//float word to48EFB0; it has no incoming x87 argument.
			float scale;
			std::memcpy(&scale, &argument, sizeof(scale));
			return manager->Acquire(name, scale);
		}
		void ConnectLocalhost(Client& client) { client.Connect("localhost"); }
	}
}
