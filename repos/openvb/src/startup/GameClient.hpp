#pragma once

#include "network/EngineClient.hpp"
#include "game/GameInterfaces.hpp"

namespace JE { class WorldContainerInterface; }

namespace F3
{
	class InventoryInterface;
	class ActionbarInterface;
	class FeedbackInterface;
	class MiniMapInterface;
	class EscapeMenuInterface;
	class OptionsInterface;
	class PipBoyInterface;
	typedef MainMenuInterface GameClientView;
	struct GameClient : public JE::Client
	{
	public:
		GameInterfaces* Interfaces() const { return static_cast<GameInterfaces*>(child0xA0); }
		ActionbarInterface* Actionbar() const;
		FeedbackInterface* Feedback() const;
		InventoryInterface* Inventory() const;
		MiniMapInterface* MiniMap() const;
		EscapeMenuInterface* EscapeMenu() const;
		OptionsInterface* Options() const;
		PipBoyInterface* PipBoy() const;
		JE::WorldContainerInterface* WorldContainer() const;
		void ToggleRain(); //4601F0, original no-payload reliable110.
		void ToggleSnow(); //4602A0, reliable111.
		void ToggleWind(); //460350, reliable112.
		void HideInterfaces(); //45E6C0, captures published client screens.
		void InitializeMainmenuCamera(); //460D60, actual GameClient receiver boundary.
		void ToggleOptionsWindow(); //45EB70.
		static void __cdecl ToggleOptionsPage(LONG page); //45ED10 and five original wrappers.
		void ToggleEscapeMenu(); //45EC00, actual cursor cancellation/visibility/modal policy.
		void ToggleInventoryWindow(); //45EAC0.
		void ToggleActionbarWindow(); //45EA50.
		void ToggleFeedbackWindow(); //45EE00, actual Actionbar companion-mode cycle.
		void ToggleWeapon(); //45EED0, active-panel policy, no equip action.
		void CycleAttackMode(); //45EE60, actual bound Actionbar owner.
		void RequestInventoryInspection(DWORD targetID, DWORD actorID, float screenX, float screenY);
		explicit GameClient(JE::Net::UInt16 port); // 0x460B30.
		~GameClient(); //45E6A0; no additional game-owned data.
		void Initialize(); // 0x4618F0.
		void UpdatePolling(); //45F000, actual map-mask visibility prefix then qualified engine frame.
		BYTE HandleInputEvent(const JE::Input::InputEvent& event); //461930, game mouse then qualified base.
		BYTE HandleMouseEvent(const JE::Input::InputEvent& event); //4615F0.
		void OnGamePacket(JE::Net::Packet& packet); //461A10, actual game packet branches.
		BYTE FilterGamePacket(JE::Net::Packet& packet); //45E9A0, local-server filter.
		void RefreshInterface(); //45E7F0, actual HUD children remain addressed.
		void OnInterfaceMessage(DWORD reference, const char* filename); //45E820; actual433840 API, child layout/producer unresolved.
		void OpenStore(const char* filename); //461960; supplied name ignored, screen producers/layouts unresolved.
		JE::ClientChild0x70* CreateChild0x70(); // 0x45E830.
		JE::ClientChild0x80* CreateChild0x80(); // 0x45E880.
		JE::ClientChild0x9C* CreateChild0x9C(); // 0x4613D0.
		JE::ClientChild0xA0* CreateChild0xA0(); // 0x45E8E0.
		JE::ClientChild0xA4* CreateChild0xA4(); // 0x45E940.
		JE::ClientChild0xAC* CreateChild0xAC(); // 0x461470.
		JE::ClientChild0xB0* CreateChild0xB0(); // 0x461530.
	};

	//45DFD0 allocates118h;460B30 adds commands but no data to JE::Client.
	typedef char GameClientOriginalExtent[(sizeof(GameClient) == 0x118) ? 1 : -1];

	namespace GameClientInitializationServices
	{
		void InitializeEngineClient(JE::Client& client);
		GameClientView* CaptureView(JE::ClientChild0xA0* child);
		void Show(GameClientView* view, int shown); // 0x495220; adjusts captured owner.
		void Activate(GameClientView* view); // 0x4952A0; reloads captured owner's view.
		void FinishInitialization(GameClient& client); // 0x460D60.
	}
}
