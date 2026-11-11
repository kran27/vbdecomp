#include "network/EngineClient.hpp"
#include "game/GameOptionsInterface.hpp"
#include "game/GameInterfaces.hpp"
#include "game/OptionsInterface.hpp"
#include "startup/NetworkComponent.hpp"
#include "gui/Event.hpp"
#include "gui/Picture.hpp"
#include "gui/ScrollBar.hpp"
#include "gui/Window.hpp"
#include <cstring>

	COptionsGameInterface::COptionsGameInterface(const std::string& name)
		: JE::EngineInterface(name), F3::GameInterface(name) //444530.
	{
		//All actual control-array constructors run before reading the interface.
		//The original does not read configuration or apply backend settings.
		LoadResource("Options_Game.INT");
		rootWindow = FindWindow("picBackground");

		//Each original lookup has a distinct temporary and cleanup state.
		dials[0].BindPicture(static_cast<::GUI::Picture*>(FindWindow("picGameDifficulty")));
		dials[0].SetCount(3);
		dials[1].BindPicture(static_cast<::GUI::Picture*>(FindWindow("picCombatDifficulty")));
		dials[1].SetCount(3);
		dials[2].BindPicture(static_cast<::GUI::Picture*>(FindWindow("picViolenceLevel")));
		dials[2].SetCount(4);
		dials[3].BindPicture(static_cast<::GUI::Picture*>(FindWindow("picTargetHighlight")));
		dials[3].SetCount(3);
		dials[4].BindPicture(static_cast<::GUI::Picture*>(FindWindow("picCombatMode")));
		dials[4].SetCount(3);

		switches[0].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picCombatMessages")));
		switches[1].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picCombatTaunts")));
		switches[2].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picLanguageFilter")));
		switches[3].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picRunning")));
		switches[4].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picSubtitles")));
		switches[5].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picItemHighlight")));
		checkBoxes[0].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picAffectPlayerSpeed")));

		sliders[0].BindThumb(FindWindow("btnCombatSpeed"));
		sliders[0].BindTrack(FindWindow("btnCombatSpeedRegion"));

		sliders[1].BindThumb(FindWindow("btnTextDelay"));
		sliders[1].BindTrack(FindWindow("btnTextDelayRegion"));

		sliders[2].BindThumb(FindWindow("btnMasterVolume"));
		sliders[2].BindTrack(FindWindow("btnMasterVolumeRegion"));

		sliders[3].BindThumb(FindWindow("btnMusicVolume"));
		sliders[3].BindTrack(FindWindow("btnMusicVolumeRegion"));

		sliders[4].BindThumb(FindWindow("btnSoundEffects"));
		sliders[4].BindTrack(FindWindow("btnSoundEffectsRegion"));

		sliders[5].BindThumb(FindWindow("btnSpeechVolume"));
		sliders[5].BindTrack(FindWindow("btnSpeechVolumeRegion"));

		sliders[6].BindThumb(FindWindow("btnSoundSampling"));
		sliders[6].BindTrack(FindWindow("btnSoundSamplingRegion"));
		sliders[6].SetMaximum(3); //4451C2, before Mouse binding.

		sliders[7].BindThumb(FindWindow("btnMouse"));
		sliders[7].BindTrack(FindWindow("btnMouseRegion"));
	}

	COptionsGameInterface::~COptionsGameInterface() //444190.
	{
		//Normal reverse member destruction:8 sliders,1 checkbox,6 switches,
		//5 dials, then GameInterface. Widgets are borrowed serialized children.
	}

	int COptionsGameInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		//Physical original444260, reached by adjustor4444D0. Its decompiler
		//misidentifies this body as the adjacent deleting destructor.
		if (GetHidden())
			return ::GUI::Interface::OnEvent(window, event);

		for (int index = 0; index < 8; ++index)
		{
			::GUI::ScrollBar* scrollbar = sliders[index].scrollbar;
			if (scrollbar && scrollbar->Process(window, event))
				return JE::EngineInterface::OnEvent(window, event);
		}

		if (event->type == 6)
		{
			if (_stricmp(window->GetName(), "btnDefault") == 0)
			{
				//Original recognized button has no configuration/default action.
			}
			else if (_stricmp(window->GetName(), "btnDone") == 0)
			{
				F3::OptionsInterface* options = static_cast<F3::GameInterfaces*>(
					static_cast<JE::Client*>(F3::g_pNetworkInstance->client)->child0xA0)->options;
				options->CloseChildren();
				options->SetHidden(1);
			}
			else if (_stricmp(window->GetName(), "btnCancel") == 0)
			{
				F3::OptionsInterface* options = static_cast<F3::GameInterfaces*>(
					static_cast<JE::Client*>(F3::g_pNetworkInstance->client)->child0xA0)->options;
				options->CloseChildren();
				options->SetHidden(1);
			}
		}
		else if (event->type == 4 || event->type == 7)
		{
			for (int index = 0; index < 5; ++index)
			{
				if (window == dials[index].GetPicture())
				{
					dials[index].Next();
					break;
				}
			}
			for (int index = 0; index < 6; ++index)
			{
				if (window == switches[index].GetPicture())
				{
					switches[index].Next();
					break;
				}
			}
			for (int index = 0; index < 1; ++index)
			{
				if (window == checkBoxes[index].GetPicture())
				{
					checkBoxes[index].Next();
					break;
				}
			}
		}
		else if (event->type == 17 && window == rootWindow)
		{
			switch (event->x)
			{
			case 1:
				{
					F3::OptionsInterface* options = static_cast<F3::GameInterfaces*>(
						static_cast<JE::Client*>(F3::g_pNetworkInstance->client)->child0xA0)->options;
					options->CloseChildren();
					options->SetHidden(1);
					return 1;
				}
			}
		}
		return JE::EngineInterface::OnEvent(window, event);
	}

