#pragma once

#include "gui/Win32Input.hpp"
#include "configuration/ConfigurationTypes.hpp"
#include <map>
#include <string>

namespace JE
{
	class CameraControl;
	class Entity;
	class VFX_VEGInstance;
	class VisualEffect;
	class ConsoleCommandProcessor;

	// Actual HotKeys constructor599810 owns a real map and borrows Input/Console.
	class ClientHotKeys
	{
	public:
		ClientHotKeys(Input::Win32Input& input, ConsoleCommandProcessor* console); //599810, actual owner captures Input/Console.
		virtual ~ClientHotKeys(); //599CD0 saves actual owner settings, then destroys its map.
		virtual void Initialize(); //5998C0, main-settings overlay.
		virtual void Save(); //599A90, actual fixed main-settings owner.
		BYTE Bind(DWORD scan, DWORD subtype, DWORD modifiers, const char* command); //599860, low-byte success1.
		void Unbind(DWORD scan, DWORD subtype, DWORD modifiers); //5997B0.
		BYTE FindBinding(DWORD& scan, DWORD& subtype, DWORD& modifiers,
			const char* command) const; //599690; failure leaves all outputs unwritten.
		BYTE Handle(const Input::InputEvent& event) const; //599720, actual captured command processor.
		DWORD FindKey(const char* name) const; //Input_Win325B48B0, exact case.
		Input::Win32Input& input; //Original+4, borrowed.
	private:
		ConsoleCommandProcessor* console; //Original+8, captured after allocation by the Client factory.
	public:
		std::map<LONG,std::string> bindings; //Original+C, real supplied VC7 map.
	};

}
