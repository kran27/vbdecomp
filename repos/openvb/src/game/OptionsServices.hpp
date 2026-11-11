#pragma once

#include <windows.h>

namespace GUI { class Interface; }
namespace JE { class ClientHotKeys; }
class COptionsGameInterface;

namespace F3
{
	class OptionsInterface;
	typedef ::COptionsGameInterface GameOptionsInterface;
	class HotKeysOptionsInterface;
	class OptionsLoadInterface;
	class OptionsMultiplayerInterface;
	class OptionsVideoInterface;
	//Host compatibility entry; native4452B0 is a HotKeysOptionsInterface member.
	void __stdcall ReplaceCommandBinding(LONG scan, DWORD subtype, DWORD modifiers,
		const char* command);
	namespace OptionsServices
	{
		//Explicit borrowed siblings of the standalone interfaces. This does not
		//construct a successful replacement for the still-unbound Client+A0.
		struct Pages
		{
			Pages() : game(NULL), hotKeys(NULL), load(NULL), multiplayer(NULL), video(NULL) {}
			GameOptionsInterface* game;
			HotKeysOptionsInterface* hotKeys;
			OptionsLoadInterface* load;
			OptionsMultiplayerInterface* multiplayer;
			OptionsVideoInterface* video;
		};
		struct Binding
		{
			Binding() : keys(NULL), defaultKeys(NULL), pipBoy(NULL) {}
			Pages pages;
			JE::ClientHotKeys* keys;
			JE::ClientHotKeys* defaultKeys; //Nullable actual GameHotKeys RTTI result.
			::GUI::Interface* pipBoy; //Original optional +44 owner, not fabricated.
		};
		Binding* SetBinding(Binding* value);
		const Binding& GetBinding();
		OptionsInterface* GetOptions();
		//Absent bindings use current Client+80; explicit bindings retain their key policy.
		JE::ClientHotKeys& HotKeys();
		//Explicit adapter: preserve arbitrary borrowed Binding.keys policy;
		//without a binding invoke the real4452B0 implementation. No new native entry.
		void ReplaceCommandBinding(LONG scan, DWORD subtype, DWORD modifiers,
			const char* command);
		JE::ClientHotKeys* DefaultsHotKeys();
		const char* KeyName(DWORD scan); //Native Client+78; explicit key/input factor retained.
		void CloseOptions();
		void PlayTabSound(); //443130.
		void PlayToggleSound(); //443220.
	}
}
