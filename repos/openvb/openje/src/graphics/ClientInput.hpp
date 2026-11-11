#pragma once

#include "gui/Win32Input.hpp"
#include "configuration/ConfigurationTypes.hpp"
#include <d3dx8.h>
#include <map>
#include <string>

namespace JE
{
	namespace GUI { class GraphicsInputServices; }
	class CameraControl;
	class Entity;
	class VFX_VEGInstance;
	class VisualEffect;
	class ConsoleCommandProcessor;

	// Source fields of Client used by the input prefix. This is a normal owned
	// state factor, not an overlay or an invented complete Client.
	struct ClientInputState
	{
		explicit ClientInputState(DWORD clock); // Corresponding58E900 stores, including56AF50 timestamp.
		BYTE mouseOverGUI; //+C0, most recent mouse event's Graphics result.
		LONG mouseX, mouseY, wheel; //+C4..CC.
		BYTE left, middle, right; //+D0..D2.
		BYTE mouseRotate; //+D4.
		DWORD lastPickingTime, hoveredEntityID, selectedEntityID; //+D8..E0.
		DWORD valueE4; //+E4 player EntityID, confirmed by551A60; host input snapshot, not a second owner.
		BYTE groundHit; //+E8, UNWRITTEN by58E900; picker writes only one byte.
		D3DXVECTOR3 groundPosition; //+EC..F4, raw zero DWORDs in constructor.
		VFX_VEGInstance* selectionEffect; //+F8; acquired by58F010, borrowed by Entity bindings.
		VisualEffect* hoverEffect; //+FC; owned by the currently hovered Entity binding.
		DWORD clientID; //+BC, zero in58E900.
		BYTE enabled; //+110.
		WORD keyboardScroll; //+112.
		LONG dragDelay; //+114.
	};

	class ClientInputServices
	{
	public:
		virtual ~ClientInputServices() {}
		virtual BYTE SecondaryInput(const Input::InputEvent& event) = 0; //Client+A4 virtual+1C.
		virtual BYTE HasGUICapture() = 0; //Manager4B2C10, checked after draining input.
		virtual BYTE Windowed() = 0; //SettingsBase virtual+38;588FE0's nonzero branch uses system screen dimensions.
		virtual void ScreenDimensions(LONG& width, LONG& height) = 0; //Graphics581590.
		virtual float FieldOfViewRadians() = 0; //SettingsBase+ A8 ->Viewport+110.
		virtual void ExecuteCommand(const char* text) = 0; //Console580D10.
		virtual BOOL CursorPosition(POINT* output); //Real GetCursorPos.
		virtual void PickMouse(ClientInputState& state, BYTE force); //58AB80; bind ClientPicking's actual outer owners.
		virtual void SelectOwnedEntity(Entity* entity); //Typed host boundary:46169C publishes Client E0, then Actionbar41C9D0 receives numeric ID.
		virtual void SendWorldAction(DWORD message, DWORD entityID, const D3DXVECTOR3& position); //589FD0/58A0E0.
		virtual BYTE HasDebugWindow() = 0; //Client+8C, actual optional child.
		virtual void DebugPickPosition(LONG x, LONG y); //4617DA, addressed populated debug branch.
	};

	// Actual HotKeys constructor599810 owns a real map and borrows Input/Console.
	class ClientHotKeys
	{
	public:
		explicit ClientHotKeys(Input::Win32Input& input);
		ClientHotKeys(Input::Win32Input& input, ConsoleCommandProcessor* console); //599810, actual owner captures Input/Console.
		virtual ~ClientHotKeys(); //599CD0 saves actual owner settings, then destroys its map.
		virtual void Initialize(); //5998C0, main-settings overlay.
		virtual void Save(); //599A90, actual fixed main-settings owner.
		void Save(Configuration::Settings& settings) const; //Explicit settings-factor adapter; no separate native entry.
		BYTE Bind(DWORD scan, DWORD subtype, DWORD modifiers, const char* command); //599860, low-byte success1.
		void Unbind(DWORD scan, DWORD subtype, DWORD modifiers); //5997B0.
		BYTE FindBinding(DWORD& scan, DWORD& subtype, DWORD& modifiers,
			const char* command) const; //599690; failure leaves all outputs unwritten.
		BYTE Handle(const Input::InputEvent& event) const; //599720, actual captured command processor.
		BYTE Handle(const Input::InputEvent& event, ClientInputServices& services) const; //Explicit map-factor adaptation.
		void Load(const Configuration::Settings& settings); //Explicit settings-factor adapter; overlays existing bindings.
		DWORD FindKey(const char* name) const; //Input_Win325B48B0, exact case.
		Input::Win32Input& input; //Original+4, borrowed.
	private:
		ConsoleCommandProcessor* console; //Original+8, captured after allocation by the Client factory.
	public:
		std::map<LONG,std::string> bindings; //Original+C, real supplied VC7 map.
	};

	BYTE HandleGameMouseInput(ClientInputState& state, const Input::InputEvent& event,
		ClientInputServices& services); //4615F0; exact selection policy, explicit required owners.
	BYTE HandleGameCursorInput(ClientInputState& state, const Input::InputEvent& event,
		ClientInputServices& services); //437D80; owner-independent branches, populated releases addressed.
	BYTE HandleClientCameraInput(ClientInputState& state, CameraControl& camera,
		const Input::InputEvent& event, ClientInputServices& services); //58A4A0/589130.
	void UpdateClientCursor(ClientInputState& state, CameraControl& camera,
		ClientInputServices& services); //588FE0/586230.
	void ProcessClientInput(ClientInputState& state, Input::Win32Input& input,
		CameraControl& camera, ClientHotKeys& hotKeys, GUI::GraphicsInputServices& graphics,
		ClientInputServices& services); //587EE0 input prefix, through cursor update; no full Client frame claim.
	BYTE ExecuteClientCameraCommand(const char* command, ClientInputState& state,
		CameraControl& camera); //Recovered no-argument console leaves;0 means not a camera command.
}
