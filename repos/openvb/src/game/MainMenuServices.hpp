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
	//Borrow actual standalone screens at the scene entry. This is not a
	//successful GameInterfaces+A0 owner or a second GUI implementation.
	struct Binding
	{
		Binding() : mainMenu(NULL), characterGeneration(NULL), loading(NULL), freshAuxiliaries(0) {}
		MainMenuInterface* mainMenu;
		CharacterGenerationInterface* characterGeneration;
		LoadingBarInterface* loading;
		BYTE freshAuxiliaries; //Explicit host domain: original4398E0 +14/24/30/3C/40 remain NULL.
	};
	Binding* SetBinding(Binding* value);
	Binding& GetBinding();
	OptionsInterface* GetOptionsInterface();
	void ShowMainMenu(); //42A200 tail after the character interface has cleared.
	void ReturnToMainMenu(); //433BE0, existing World/Client/Server lifetimes.
	void HideGameInterfaces(); //45E6C0, actual borrowed screens and explicit fresh auxiliary domain.
	void SetLoadingVisible(BYTE visible); //439720, actual optional LoadingBar child.
	BYTE HandleSceneTransition(JE::Client& client, DWORD address); //Explicit standalone join, no original entry.
	//Actual original external children. These are not an object standing in for
	//GameInterfaces/Client+A0. Required children stay addressed until recovered.
	void QueueCommand(const char* command); //580D10, actual live Client+7C Processor.
	void FadeOut(DWORD duration, DWORD value1, DWORD value2, DWORD value3); //588CF0.
	void OpenCharacterSelection(); //42F9E0, actual Client+A0 child+38, no stack args.
	void FadeIn(DWORD duration, DWORD value1, DWORD value2, DWORD value3); //588CE0.
} }
