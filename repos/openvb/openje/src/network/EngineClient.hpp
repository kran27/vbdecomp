#pragma once

#include "Client.hpp"
#include "Connection.hpp"
#include <cstddef>
#include <string>

struct Graphics;
struct D3DXVECTOR3;
class Video;

namespace JE
{
	class Audio;
	struct GraphicsSceneState;
	namespace Input { class Win32Input; struct InputEvent; }
	typedef Audio ClientChild0x70;
	typedef ::Graphics ClientChild0x74;
	typedef Input::Win32Input ClientChild0x78;
	class ConsoleCommandProcessor;
	typedef ConsoleCommandProcessor ClientChild0x7C;
	class ClientHotKeys;
	typedef ClientHotKeys ClientChild0x80;
	class CameraControl;
	typedef CameraControl ClientChild0x84;
	class ConsoleInterface;
	typedef ConsoleInterface ClientChild0x88;
	class DebugWindow;
	typedef DebugWindow ClientChild0x8C;
	class DebugToolsInterface;
	typedef DebugToolsInterface ClientChild0x94;
	class FontTestInterface;
	typedef FontTestInterface ClientChild0x98;
	class DialogueInterface;
	typedef DialogueInterface ClientChild0x9C;
	class Interfaces;
	typedef Interfaces ClientChild0xA0;
	class CursorManager;
	typedef CursorManager ClientChild0xA4;
	class FadeManager;
	typedef FadeManager ClientChild0xA8;
	class FloatingTextInterface;
	typedef FloatingTextInterface ClientChild0xAC;
	class CSMenuInterface;
	typedef CSMenuInterface ClientChild0xB0;
	typedef ::Video ClientChild0xB4;
	class EditTokenInterface;
	typedef EditTokenInterface ClientChild0xB8;
	class VFX_VEGInstance;
	typedef VFX_VEGInstance ClientChild0xF8;

	class Client : public Net::Client
	{
	public:
		explicit Client(JE::Net::UInt16 port); // 0x58E900.
		~Client(); // 0x5897D0; misleading stdiobuf name.
		void Initialize(); // 0x58F010; required UI/graphics children.
		void Cleanup(); // 0x588D90.
		void CreateConnection(); // 0x587B00.
		void Connect(const char* host); //45E6B0, genuine qualified Net forwarder.
		void Connect(const Net::Address& address); //587770, default port/local-mode policy.
		void UpdatePolling(); //587EE0; actual Net prefix then required game-frame children.
		void UpdateCommandCamera(); //587FD0..587FE0, actual queue dispatch before CameraControl update.
		void UpdateInputFrame(); //587EF6..587FD0, actual child78/80 and virtual input routing.
		void DragInputCamera(LONG x, LONG y); //589130, stores live114 before camera callback.
		void UpdateWorldFrame(); //58801F..588081, state3 update/collection and live108/109 draw guards.
		void PrepareDisconnect(Net::Packet& packet); //5880A0, required World stop.
		JE::Net::UInt8 OnServerStart(Net::Packet& packet); //58A620, movies then mode-gated World snapshot/reliable67.
		JE::Net::UInt8 OnServerUpdate(Net::Packet& packet); //5880C0, entity reader and count0 dialogue.
		JE::Net::UInt8 OnServerDisconnect(Net::Packet& packet); //588170.
		void OnGamePacket(Net::Packet& packet); //58C660, original engine dispatcher.
		virtual void RefreshInterface(); //404B70, genuine empty engine hook at original+78.
		virtual void OnInterfaceMessage(DWORD value, const char* text); //408090, original+7C empty two-argument hook.
		void RemoveClientEntity(DWORD id); //588200, clear matching hover/effect.
		void PublishMapID(DWORD id); //589D90, reliable64 acknowledgement.
		void PrepareMapTransition(); //58C4F5..58C53C, camera/fog and required interface prefix.
		void PrepareMapTransition(DWORD loadingAddress, DWORD cursorAddress,
			DWORD gameplayAddress); //Shared native prefix; addresses identify absent-owner workspace calls.
		void MapTransitionInterface(DWORD address); //Explicit addressed interface child, not an original entry.
		void AddFloatingSpeechText(DWORD entityID, DWORD stringReference, DWORD duration,
			DWORD red, DWORD green, DWORD blue); //58AAD0; resolve then actual text owner virtual0.
		void AddFloatingDamageText(DWORD entityID, const std::string& text,
			DWORD red, DWORD green, DWORD blue); //587A70; actual text owner virtual4.
		void SendDialogueReply(Net::Packet& packet); //587850; message54, original Send flag0.
		void SendDialogueClose(Net::Packet& packet); //587830; message57, original Send flag1.
		void ReceiveDialogueRemoval(Net::Packet& packet); //588230; raw ID before required dialogue owner.
		void StartTestDialogue(); //589CB0, shared58A7D0: message53/test name/flag0.
		void RequestContext75(DWORD actorID, DWORD targetID); //45F790, thiscall/RET8.
		void RequestContext76(DWORD actorID, DWORD targetID); //45F870, thiscall/RET8.
		void RequestContext77(DWORD actorID, DWORD targetID); //45F950, thiscall/RET8.
		void RequestContext79(DWORD actorID, DWORD targetID); //45FB10, thiscall/RET8.
		void RequestContext80(DWORD actorID, DWORD targetID); //45FBF0, thiscall/RET8.
		void RequestGive(DWORD actorID, DWORD targetID, DWORD itemID); //45FE90, thiscall/RET12, reliable83.
		void RequestTalk(DWORD actorID, DWORD targetID); //45F5D0, thiscall/RET8, reliable73.
		void RequestHostile(DWORD actorID, DWORD targetID); //45F6B0, thiscall/RET8, prototype74.
		void RequestOperate(DWORD actorID, DWORD targetID); //45FA30, thiscall/RET8, reliable78.
		void RequestPickup(DWORD actorID, DWORD targetID); //45FDB0, thiscall/RET8, reliable82.
		void RequestAttack(DWORD attackerID, DWORD targetID, LONG calledShotPart,
			const D3DXVECTOR3* point); //45F370, thiscall/RET16, reliable91.
		void RequestWeaponModeCycle(DWORD id); //45FF80, thiscall/RET4, reliable100.
		void RequestUnarmedModeCycle(DWORD id); //460040, thiscall/RET4, reliable84.
		void RequestReload(DWORD actor,DWORD weapon,DWORD ammoID); //460100, thiscall/RET12, reliable90.
		void RequestWeaponSelection(DWORD actor,DWORD slot); //4605E0, thiscall/RET8, reliable89.
		void RequestEquip(DWORD actor, DWORD item, DWORD source, DWORD slot); //460400, RET16/95.
		void RequestUnequip(DWORD actor, DWORD item); //460500, RET8/96.
		void RequestDropItem(DWORD actor, DWORD item); //58A3C0, RET8/51.
		void RequestUse(DWORD actorID, DWORD targetID); //45F4F0, thiscall/RET8, reliable72.
		void RequestLook(DWORD actorID, DWORD targetID); //45FCD0, thiscall/RET8, reliable81.
		void RequestSkillMenu(DWORD actorID, DWORD targetID, LONG x, LONG y); //45EF70, thiscall/RET16.
		void RequestContextMenu(DWORD actorID, DWORD targetID, LONG x, LONG y); //587A80, thiscall/RET16.
		void RequestNewGame(); //589F00, shared58DBE0: mode1/message58/flag0/loading1.
		void EndDialogue(); //589E50, message56/flag0.
		void Warp(const char* map); //58A1F0, message70/NUL name/flag0.
		void RequestCreateEntity(const std::string& resource, DWORD map,
			const D3DXVECTOR3& position, BYTE flag); //589820, mode1/message9/flag1.
		void RequestSetGlobal(std::string name, LONG value); //58A9A0, by-value28+4/RET32.
		void RequestSetScript(DWORD entity, const std::string& name); //589A50, mode1/message15/flag1.
		void RequestKill(DWORD entity); //589980, reliable message11.
		void RequestForceEntityStep(DWORD entity, D3DXVECTOR3 position); //589FD0, message59/flag0.
		void RequestMoveTo(DWORD entity, D3DXVECTOR3 position); //58A0E0, message60/flag0.
		void RequestTestAnimation(DWORD entity, DWORD animation, DWORD variation,
			WORD loops, BYTE backwards); //589B60, message26/flag1.
		void FadeIn(DWORD duration, LONG red, DWORD green, DWORD blue); //588CE0, shared5B4B30.
		void FadeOut(DWORD duration, LONG red, DWORD green, DWORD blue); //588CF0, shared5B4BD0.
		void ToggleFOWLOS(); //58BCB0, flag100 then live party visibility refresh.
		void ToggleFogOfWarAutomap(); //588F20; global backend flag2194, this unread.
		void ToggleVisualEffects(); //588800, signed three-state cycle104.
		void ToggleWater(); //5888B0, signed mode cycle then actual water sampling.
		void ToggleStartupMovies(); //587930, actual Graphics74 setter.
		void ToggleDebugLights(); //5879B0, actual Client receiver.
		void ToggleDebugOctreeLights(); //5879D0.
		void ToggleBilinearFiltering(); //587910.
		void ToggleWireframe(); //587970.
		void ToggleTransparency(); //587990.
		void ToggleMipMapping(); //587A00.
		void ToggleOctreeDrawing(); //5879F0, actual owned body of command588600.
		void ToggleOctreeNodeDebug(); //587A30.
		void ToggleShadow(); //5878A0, distinct incoming/current Client captures.
		void CaptureScreen(); //587A50, actual Client receiver.
		void SetCameraMouseRotate(BYTE value); //587870, raw low BYTE publication.
		void ToggleFogOfWar(); //587A40, actual Graphics-owned tail581110.
		void ToggleCameraFog(); //587950, live child74 reload across getter.
		void ToggleLighting(); //587880, actual Client receiver.
		void ToggleHeightFog(); //587A20, actual Client receiver.
		void ToggleBuilding(LONG building); //58A890, material and map-layer/party refresh.
		void EditToken(DWORD stringReference, const char* tokenName); //587AB0, actual childB8 modal setup.
		void RequestInventoryTransfer(DWORD destinationID, DWORD itemID, DWORD sourceID); //58A980 ->58A2D0, reliable49.
		JE::Net::UInt8 OnServerTimeout(JE::Net::UInt32 elapsed); //588190, actual Disconnect then1.
		virtual Net::Connection* CreateServerConnection(); // 0x588F50; original slot128.
		virtual ClientChild0x70* CreateChild0x70(); // Original slot132.
		virtual ClientChild0x74* CreateChild0x74(); // Original slot136.
		virtual ClientChild0x78* CreateChild0x78(); // Original slot140.
		virtual ClientChild0x7C* CreateChild0x7C(); // Original slot144.
		virtual ClientChild0x80* CreateChild0x80(); // Original slot148.
		virtual ClientChild0x84* CreateChild0x84(); // Original slot152.
		virtual ClientChild0x88* CreateChild0x88(); // Original slot156.
		virtual ClientChild0x8C* CreateChild0x8C(); // Original slot160; also child0x90.
		virtual ClientChild0x94* CreateChild0x94(); // Original slot164.
		virtual ClientChild0x98* CreateChild0x98(); //58BFC0, original slot168.
		virtual ClientChild0x9C* CreateChild0x9C(); // Original slot172.
		virtual ClientChild0xA0* CreateChild0xA0(); // Original slot176.
		virtual ClientChild0xA4* CreateChild0xA4(); // Original slot180.
		virtual ClientChild0xA8* CreateChild0xA8(); // Original slot184.
		virtual ClientChild0xAC* CreateChild0xAC(); // Original slot188.
		virtual ClientChild0xB0* CreateChild0xB0(); // Original slot192.
		virtual ClientChild0xB4* CreateChild0xB4(); // Original slot196.
		ClientChild0xB8* CreateChild0xB8(); //58C2C0, ordinary nonvirtual member.
		virtual BYTE HandleInputEvent(const Input::InputEvent& event); //58A4A0, original+C8.
		virtual BYTE FilterGamePacket(Net::Packet& packet); //58C380, original+CC local-server policy.
		virtual void UpdateInputCursor(); //588FE0, original+D0.
		virtual void PickInputMouse(BYTE force); //58AB80, original+D4.
		BYTE TryGetGroundPosition(D3DXVECTOR3& output); //587AC0; false leaves output unwritten.
		virtual void ClearInputSelection(); //588220, original+D8, clear E0 then DC.
		static void CommandFontTest(); //5887B0, callback ignores supplied command arguments.
		virtual void SelectEntity(DWORD id,BYTE select,BYTE audio); //58D9A0, original+DC.

		ClientChild0x70* child0x70;
		ClientChild0x74* child0x74;
		ClientChild0x78* child0x78;
		ClientChild0x7C* child0x7C;
		ClientChild0x80* child0x80;
		ClientChild0x84* child0x84;
		ClientChild0x88* child0x88;
		ClientChild0x8C* child0x8C;
		ClientChild0x8C* child0x90;
		ClientChild0x94* child0x94;
		ClientChild0x98* child0x98;
		ClientChild0x9C* child0x9C;
		ClientChild0xA0* child0xA0;
		ClientChild0xA4* child0xA4;
		ClientChild0xA8* child0xA8;
		ClientChild0xAC* child0xAC;
		ClientChild0xB0* child0xB0;
		ClientChild0xB4* child0xB4;
		ClientChild0xB8* child0xB8;
		JE::Net::UInt32 clientID; //Original+BC, assigned by greeting58A620; no child owner.
		JE::Net::UInt8 field0xC0;
		JE::Net::UInt32 field0xC4, field0xC8, field0xCC;
		JE::Net::UInt16 field0xD0;
		JE::Net::UInt8 field0xD2;
		JE::Net::UInt8 reserved0xD3; //Unwritten byte between the original three mouse buttons and+D4.
		JE::Net::UInt8 field0xD4;
		JE::Net::UInt32 field0xD8, field0xDC, field0xE0, field0xE4;
		JE::Net::UInt8 reserved0xE8[4]; //Original58E900 leaves+E8..EB unwritten before+EC.
		JE::Net::UInt32 field0xEC, field0xF0, field0xF4;
		ClientChild0xF8* child0xF8;
		JE::Net::UInt32 field0xFC;
		JE::Net::UInt8 field0x100;
		JE::Net::UInt32 field0x104;
		JE::Net::UInt8 field0x108, field0x109, field0x10A;
		JE::Net::UInt32 field0x10C;
		JE::Net::UInt8 field0x110;
		JE::Net::UInt16 field0x112;
		JE::Net::UInt32 field0x114;

	private:
		void ClearHoverEffect(); //5881A0, live entity/effect cleanup.
	};

#if defined(_MSC_VER) && _MSC_VER == 1300
	//Compile-only proofs for this reconstructed derived span. Net::Client's
	//internal field/vtable layout remains separate; its present size alone is
	//not an original-base ABI proof. Original factory543820 allocates118h.
	typedef char ClientOriginalSize[(sizeof(Client) == 0x118) ? 1 : -1];
	typedef char ClientCurrentNetPrefix[(sizeof(Net::Client) == 0x70) ? 1 : -1];
	typedef char ClientOriginal70[(offsetof(Client, child0x70) == 0x70) ? 1 : -1];
	typedef char ClientOriginal74[(offsetof(Client, child0x74) == 0x74) ? 1 : -1];
	typedef char ClientOriginal78[(offsetof(Client, child0x78) == 0x78) ? 1 : -1];
	typedef char ClientOriginal7C[(offsetof(Client, child0x7C) == 0x7C) ? 1 : -1];
	typedef char ClientOriginal80[(offsetof(Client, child0x80) == 0x80) ? 1 : -1];
	typedef char ClientOriginal84[(offsetof(Client, child0x84) == 0x84) ? 1 : -1];
	typedef char ClientOriginal88[(offsetof(Client, child0x88) == 0x88) ? 1 : -1];
	typedef char ClientOriginal8C[(offsetof(Client, child0x8C) == 0x8C) ? 1 : -1];
	typedef char ClientOriginal90[(offsetof(Client, child0x90) == 0x90) ? 1 : -1];
	typedef char ClientOriginal94[(offsetof(Client, child0x94) == 0x94) ? 1 : -1];
	typedef char ClientOriginal98[(offsetof(Client, child0x98) == 0x98) ? 1 : -1];
	typedef char ClientOriginal9C[(offsetof(Client, child0x9C) == 0x9C) ? 1 : -1];
	typedef char ClientOriginalA0[(offsetof(Client, child0xA0) == 0xA0) ? 1 : -1];
	typedef char ClientOriginalA4[(offsetof(Client, child0xA4) == 0xA4) ? 1 : -1];
	typedef char ClientOriginalA8[(offsetof(Client, child0xA8) == 0xA8) ? 1 : -1];
	typedef char ClientOriginalAC[(offsetof(Client, child0xAC) == 0xAC) ? 1 : -1];
	typedef char ClientOriginalB0[(offsetof(Client, child0xB0) == 0xB0) ? 1 : -1];
	typedef char ClientOriginalB4[(offsetof(Client, child0xB4) == 0xB4) ? 1 : -1];
	typedef char ClientOriginalB8[(offsetof(Client, child0xB8) == 0xB8) ? 1 : -1];
	typedef char ClientOriginalBC[(offsetof(Client, clientID) == 0xBC) ? 1 : -1];
	typedef char ClientOriginalC0[(offsetof(Client, field0xC0) == 0xC0) ? 1 : -1];
	typedef char ClientOriginalC4[(offsetof(Client, field0xC4) == 0xC4) ? 1 : -1];
	typedef char ClientOriginalC8[(offsetof(Client, field0xC8) == 0xC8) ? 1 : -1];
	typedef char ClientOriginalCC[(offsetof(Client, field0xCC) == 0xCC) ? 1 : -1];
	typedef char ClientOriginalD0[(offsetof(Client, field0xD0) == 0xD0) ? 1 : -1];
	typedef char ClientOriginalD2[(offsetof(Client, field0xD2) == 0xD2) ? 1 : -1];
	typedef char ClientOriginalD3[(offsetof(Client, reserved0xD3) == 0xD3) ? 1 : -1];
	typedef char ClientOriginalD4[(offsetof(Client, field0xD4) == 0xD4) ? 1 : -1];
	typedef char ClientOriginalD8[(offsetof(Client, field0xD8) == 0xD8) ? 1 : -1];
	typedef char ClientOriginalDC[(offsetof(Client, field0xDC) == 0xDC) ? 1 : -1];
	typedef char ClientOriginalE0[(offsetof(Client, field0xE0) == 0xE0) ? 1 : -1];
	typedef char ClientOriginalE4[(offsetof(Client, field0xE4) == 0xE4) ? 1 : -1];
	typedef char ClientOriginalE8[(offsetof(Client, reserved0xE8) == 0xE8) ? 1 : -1];
	typedef char ClientOriginalEC[(offsetof(Client, field0xEC) == 0xEC) ? 1 : -1];
	typedef char ClientOriginalF0[(offsetof(Client, field0xF0) == 0xF0) ? 1 : -1];
	typedef char ClientOriginalF4[(offsetof(Client, field0xF4) == 0xF4) ? 1 : -1];
	typedef char ClientOriginalF8[(offsetof(Client, child0xF8) == 0xF8) ? 1 : -1];
	typedef char ClientOriginalFC[(offsetof(Client, field0xFC) == 0xFC) ? 1 : -1];
	typedef char ClientOriginal100[(offsetof(Client, field0x100) == 0x100) ? 1 : -1];
	typedef char ClientOriginal104[(offsetof(Client, field0x104) == 0x104) ? 1 : -1];
	typedef char ClientOriginal108[(offsetof(Client, field0x108) == 0x108) ? 1 : -1];
	typedef char ClientOriginal109[(offsetof(Client, field0x109) == 0x109) ? 1 : -1];
	typedef char ClientOriginal10A[(offsetof(Client, field0x10A) == 0x10A) ? 1 : -1];
	typedef char ClientOriginal10C[(offsetof(Client, field0x10C) == 0x10C) ? 1 : -1];
	typedef char ClientOriginal110[(offsetof(Client, field0x110) == 0x110) ? 1 : -1];
	typedef char ClientOriginal112[(offsetof(Client, field0x112) == 0x112) ? 1 : -1];
	typedef char ClientOriginal114[(offsetof(Client, field0x114) == 0x114) ? 1 : -1];
#endif

	namespace EngineClientServices
	{
		Client* CurrentClient(); //Actual live Network+client provider; no replacement owner.
		// OpenVB supplies the actual canonical clock and console adapters.
		JE::Net::UInt32 AccumulatedTime();
		void RegisterCommand(const char* name, JE::Net::UInt32 originalAddress);
		void CleanupNetClient(Net::Client& client);
		void DeleteChild(ClientChild0x70* child);
		void DeleteChild(ClientChild0x74* child);
		void DeleteChild(ClientChild0x78* child);
		void DeleteChild(ClientChild0x7C* child);
		void DeleteChild(ClientChild0x80* child);
		void DestroyChild(ClientChild0x84* child);
		void FreeChild(ClientChild0x84* child);
		void DeleteChild(ClientChild0x88* child);
		void DeleteChild(ClientChild0x8C* child);
		void DeleteChild(ClientChild0x94* child);
		void DeleteChild(ClientChild0x98* child);
		void DeleteChild(ClientChild0x9C* child);
		void DeleteChild(ClientChild0xA0* child);
		void DeleteChild(ClientChild0xA4* child);
		void DeleteChild(ClientChild0xA8* child);
		void DeleteChild(ClientChild0xAC* child);
		void DeleteChild(ClientChild0xB0* child);
		void DeleteChild(ClientChild0xB4* child);
		void DeleteChild(ClientChild0xB8* child);
		void DeleteChild(ClientChild0xF8* child);
	}
	namespace EngineClientConnectionServices
	{
		void* Allocate(std::size_t size);
		void Construct(Net::EngineClientServerConnection* connection);
		void Free(void* block) throw();
	}
}
