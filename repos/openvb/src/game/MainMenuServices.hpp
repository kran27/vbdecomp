#pragma once

#include <windows.h>
namespace JE { class Client; }

namespace F3
{
	class MainMenuInterface;
	class CharacterGenerationInterface;
	class LoadingBarInterface;
	class OptionsInterface;
}

namespace F3 { namespace MainMenuServices
{
	OptionsInterface* GetOptionsInterface();
	void ReturnToMainMenu(); //433BE0, existing World/Client/Server lifetimes.
	void QueueCommand(const char* command); //580D10, actual live Client+7C Processor.
} }
