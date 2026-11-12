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
	namespace OptionsServices
	{
		OptionsInterface* GetOptions();
		JE::ClientHotKeys& HotKeys();
		JE::ClientHotKeys* DefaultsHotKeys();
		const char* KeyName(DWORD scan); //Native Client+78; explicit key/input factor retained.
		void CloseOptions();
		void PlayTabSound(); //443130.
		void PlayToggleSound(); //443220.
	}
}
