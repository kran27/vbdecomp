#include "GameClient.hpp"
#include "GameAudio.hpp"
#include "ClientCommands.hpp"
#include "ConfigurationObject.hpp"
#include "game/GameHotKeys.hpp"
#include "game/ActionbarInterface.hpp"
#include "game/InventoryInterface.hpp"
#include "NetworkComponent.hpp"
#include "graphics/ClientCursor.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/OctreeCamera.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "gui/DebugWindow.hpp"
#include "gui/Manager.hpp"
#include <new>
#include "game/MainMenuInterface.hpp"
#include "game/MiniMapInterface.hpp"
#include "game/PipBoyInterface.hpp"
#include "entity/EntityEnvironment.hpp"
#include "graphics/CameraControl.hpp"
#include "graphics/MapMaskManager.hpp"
#include "gui/WorldContainerInterface.hpp"
#include "gui/WorldContainerServices.hpp"
#include "game/PipBoyInterface.hpp"
#include "game/OptionsInterface.hpp"
#include "gui/DialogueInterface.hpp"
#include "game/GameDialogueInterface.hpp"
#include "game/GameSkillMenu.hpp"
#include "game/GameFloatingTextInterface.hpp"
#include <stdexcept>
#include <cstring>
#include <cstdio>

namespace F3
{
	BYTE GameClient::HandleInputEvent(const JE::Input::InputEvent& event) //461930.
	{
		if (event.category == 2)
		{
			const BYTE handled = HandleMouseEvent(event);
			if (handled) return handled;
		}
		return JE::Client::HandleInputEvent(event);
	}

	BYTE GameClient::HandleMouseEvent(const JE::Input::InputEvent& event) //4615F0.
	{
		GameClient* published = static_cast<GameClient*>(g_pNetworkInstance->client);
		if (event.subtype)
		{
			if (event.subtype != 2 || event.button) return 0;
			PickInputMouse(1);
			if (field0xDC)
			{
				JE::Entity* hovered = JE::GetEntityPtr(field0xDC);
				if (hovered && hovered->visibilityOwnerFlag == clientID)
				{
					field0xE0 = field0xDC; //46169C, publish before the captured HUD callback.
					published->Interfaces()->actionbar->SelectEntity(field0xE0);
				}
				return 1;
			}
			if (event.modifiers & 1)
			{
				if (!field0xE0)
				{
					std::list<JE::Entity*>::iterator i=JE::g_createdEntities.begin();
					const std::list<JE::Entity*>::iterator end=JE::g_createdEntities.end();
					if (i!=end)
					{
						NetworkComponent* network=g_pNetworkInstance;
						do
						{
							JE::Entity* candidate=*i;
							if (candidate->visibilityOwnerFlag && candidate->visibilityOwnerFlag==
								static_cast<JE::Client*>(network->client)->clientID)
							{
								field0xE0=candidate->entityID;
								break;
							}
							++i;
						} while (i!=end);
					}
				}
				if (field0xE0)
				{
					D3DXVECTOR3 position;
					std::memcpy(&position, &field0xEC, sizeof(position)); //461740 ignores E8.
					RequestForceEntityStep(field0xE0, position);
				}
				return 0; //46175B retains the initial result, including after sending59.
			}
			if (field0xE0)
			{
				if (reserved0xE8[0]) //461764: original cached-hit byte.
				{
					D3DXVECTOR3 position;
					std::memcpy(&position, &field0xEC, sizeof(position));
					RequestMoveTo(field0xE0, position);
				}
			}
			return 1;
		}
		JE::DebugWindow* debugWindow = published->child0x8C; //4617AB, capture before GUI/camera calls.
		if (debugWindow)
		{
			float scaleX, scaleY;
			::GUI::g_Manager.GetScale(scaleX, scaleY); //4617C8; original outputs are unused.
			if (event.modifiers & 2)
			{
				{
					OCTREE_Camera* camera = child0x74->GetPickingCamera();
					if (camera)
					{
						OctreeRay ray;
						camera->MakeScreenRay(event.x, event.y, &ray);
						JE::IWorldClient* world=JE::g_World; //461823 reloads after MakeScreenRay.
						OCTREE* octree=world->GetCurrentMapOctree();
						if (octree)
						{
							OctreeGroundHit hit;
							if (octree->QueryGroundRay(ray, &hit, 1, 0))
							{
								char text[64];
								std::sprintf(text, "X: %f Y: %f Z: %f", hit.point[0], hit.point[1], hit.point[2]);
								debugWindow->Write(std::string(text), 64, 64, 64);
							}
						}
					}
				}
			}
		}
		return 0;
	}

	void GameClient::UpdatePolling()
	{
		GameInterfaces* interfaces = Interfaces();
		MiniMapInterface* miniMap = interfaces->miniMap;
		PipBoyInterface* pipBoy = interfaces->pipBoy;
		MapMaskManager* masks = child0x74->maskManager;
		if (miniMap->GetHidden() &&
			(pipBoy->GetHidden() || pipBoy->CurrentPage() != 3))
		{
			masks->exploredEnabled = 0;
			JE::Client::UpdatePolling();
		}
		else
		{
			masks->exploredEnabled = 1;
			JE::Client::UpdatePolling();
		}
	}
	GameClient::GameClient(JE::Net::UInt16 port) : JE::Client(port)
	{
		RegisterClientCommand("EscapeMenu", 0x45EC00u);
		RegisterClientCommand("AttackModeCycle", 0x45EE60u);
		RegisterClientCommand("AttackModeSelect", 0x404B70u);
		RegisterClientCommand("CharacterRecordWindow", 0x404B70u);
		RegisterClientCommand("FeedbackWindow", 0x45EE00u);
		RegisterClientCommand("GameOptions", 0x45EDB0u);
		RegisterClientCommand("HotKeyOptions", 0x45EDC0u);
		RegisterClientCommand("InventoryWindow", 0x45EAC0u);
		RegisterClientCommand("LoadGameOptions", 0x45EDD0u);
		RegisterClientCommand("MultiplayerOptions", 0x45EDE0u);
		RegisterClientCommand("PipBoy", 0x45EC70u);
		RegisterClientCommand("SneakToggle", 0x404B70u);
		RegisterClientCommand("UseSkill", 0x45EE30u);
		RegisterClientCommand("VideoOptions", 0x45EDF0u);
		RegisterClientCommand("WeaponLinkToggle", 0x404B70u);
		RegisterClientCommand("WeaponToggle", 0x45EED0u);
		RegisterClientCommand("ActionbarWindow", 0x45EA50u);
		RegisterClientCommand("ForceEntityStep", 0x460720u);
		RegisterClientCommand("OptionsWindow", 0x45EB70u);
		RegisterClientCommand("TestAnim", 0x460920u);
		RegisterClientCommand("ToggleRain", 0x4606C0u);
		RegisterClientCommand("ToggleSnow", 0x4606D0u);
		RegisterClientCommand("ToggleWind", 0x4606E0u);
	}

	GameClient::~GameClient() {}
	void GameClient::OnInterfaceMessage(DWORD reference, const char* filename)
	{
		Interfaces()->auxiliary30->SetMessage(reference, filename);
	}

	void GameClient::ToggleInventoryWindow()
	{
		CommandToggleInventoryWindow(NULL);
	}

	void __cdecl CommandToggleInventoryWindow(CommandArguments*) //45EAC0.
	{
		GameClient* client = static_cast<GameClient*>(g_pNetworkInstance->client);
		GameInterfaces* interfaces = client->Interfaces();
		InventoryInterface* inventory = interfaces->inventory;
		PipBoyInterface* pipBoy = interfaces->pipBoy;
		OptionsInterface* options = interfaces->options;
		if (inventory)
		{
			if (inventory->GetHidden())
			{
				if (client->child0x9C && !client->child0x9C->AllWindowsHidden()) return;
				const DWORD actor = client->field0xE0; //Captured native Client remains live across the query.
				inventory->SetHidden(0);
				inventory->Open(actor);
				if (pipBoy) pipBoy->SetHidden(1);
				if (options)
				{
					options->CloseChildren();
					options->SetHidden(1);
				}
			}
			else inventory->SetHidden(1);
			return;
		}
	}

	void GameClient::OpenStore(const char*) //461960, supplied store name is ignored.
	{
		GameInterfaces* interfaces = static_cast<GameClient*>(g_pNetworkInstance->client)->Interfaces();
		InventoryInterface* inventory = interfaces->inventory;
		StoreInventoryScreen* store = interfaces->auxiliary40;
		StoreCatalogueScreen* catalogue = interfaces->auxiliary3C;
		catalogue->SetHidden(0);
		catalogue->Configure("TestStore.sto");
		inventory->SetHidden(1);
		CommandArguments arguments;
		CommandToggleInventoryWindow(&arguments);
		store->Refresh();
		store->SetHidden(0);
		store->PlaceForStore(1);
	}

	void GameClient::RequestInventoryInspection(DWORD targetID, DWORD actorID, float screenX, float screenY)
	{
		JE::WorldContainerInterface* popup = WorldContainer();
		JE::DebugWindow* debugWindow = JE::WorldContainerServices::CaptureInspectionDebug();
		popup->SetHidden(0);
		popup->Populate(targetID, actorID);
		const LONG y = LONG(static_cast<__int64>(screenY));
		const LONG x = LONG(static_cast<__int64>(screenX));
		popup->SetAbsolutePosition(x, y);
		JE::WorldContainerServices::WriteInspectionDebug(debugWindow);
	}

	void GameClient::Initialize()
	{
		namespace Services = GameClientInitializationServices;
		Services::InitializeEngineClient(*this);
		GameClientView* view = Services::CaptureView(child0xA0);
		view->PrepareForOpen(); //Actual461904, before virtual-base adjustment for Show.
		// 0x404B70 is an actual empty hook on the captured owner.
		Services::Show(view, 0);
		Services::Activate(view);
		Services::FinishInitialization(*this);
	}

	JE::ClientChild0x70* GameClient::CreateChild0x70() { return new GameAudio; }
	JE::ClientChild0x80* GameClient::CreateChild0x80()
	{
		//45E8B4: capture the published children only after allocation.
		return new GameHotKeys(*child0x78, child0x7C);
	}
	JE::ClientChild0x9C* GameClient::CreateChild0x9C()
	{
		return new GameDialogueInterface("GameDialogueInterface"); //4613D0.
	}
	JE::ClientChild0xA0* GameClient::CreateChild0xA0()
	{
		return new GameInterfaces;
	}
	JE::ClientChild0xA4* GameClient::CreateChild0xA4() //45E940.
	{
		return new GameCursorManager;
	}
	JE::ClientChild0xAC* GameClient::CreateChild0xAC()
	{
		return new GameFloatingTextInterface("GameFloatingTextInterface"); //461470.
	}
	JE::ClientChild0xB0* GameClient::CreateChild0xB0()
	{
		return new GameSkillMenu("GameCSMenuInterface");
	}

	namespace GameClientInitializationServices
	{
		void InitializeEngineClient(JE::Client& client) { client.JE::Client::Initialize(); }
		GameClientView* CaptureView(JE::ClientChild0xA0* child) { return static_cast<GameInterfaces*>(child)->mainMenu; }
		void Show(GameClientView* view, int hidden) { view->SetHidden(BYTE(hidden)); }
		void Activate(GameClientView* view) { view->BringToFront(); }
		void FinishInitialization(GameClient& client) { client.InitializeMainmenuCamera(); }
	}
}

namespace F3
{
	ActionbarInterface* GameClient::Actionbar() const { return Interfaces()->actionbar; }
	FeedbackInterface* GameClient::Feedback() const { return Interfaces()->feedback; }
	InventoryInterface* GameClient::Inventory() const { return Interfaces()->inventory; }
	MiniMapInterface* GameClient::MiniMap() const { return Interfaces()->miniMap; }
	EscapeMenuInterface* GameClient::EscapeMenu() const { return Interfaces()->escapeMenu; }
	OptionsInterface* GameClient::Options() const { return Interfaces()->options; }
	PipBoyInterface* GameClient::PipBoy() const { return Interfaces()->pipBoy; }
	JE::WorldContainerInterface* GameClient::WorldContainer() const { return Interfaces()->container; }
}
