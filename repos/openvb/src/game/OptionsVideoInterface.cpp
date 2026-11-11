#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/OptionsVideoInterface.hpp"
#include "game/OptionsInterface.hpp"
#include "gui/Event.hpp"
#include "gui/Picture.hpp"
#include "gui/ScrollBar.hpp"
#include "gui/Window.hpp"
#include <cstring>

namespace F3
{
	OptionsVideoInterface::OptionsVideoInterface(const std::string& name)
		: JE::EngineInterface(name), GameInterface(name) //4496A0.
	{
		LoadResource("Options_Video.INT");
		rootWindow = FindWindow("picBackground");
		dials[0].BindPicture(static_cast<::GUI::Picture*>(FindWindow("picShadowType")));
		dials[0].SetCount(3);
		dials[1].BindPicture(static_cast<::GUI::Picture*>(FindWindow("picDynamicLights")));
		dials[1].SetCount(4);

		//Each original resource lookup owns its own temporary and unwind state.
		switches[0].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picBilinearFiltering")));
		switches[1].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picDetailTextures")));
		switches[2].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picEffectsDetail")));
		switches[3].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picFullscreen")));
		switches[4].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picAllowRefocus")));
		switches[5].BindPicture(
			static_cast<::GUI::Picture*>(FindWindow("picTextureDetail")));

		sliders[0].BindThumb(FindWindow("btnPrimaryMonitor"));
		sliders[0].BindTrack(FindWindow("btnPrimaryMonitorRegion"));
		sliders[0].SetMaximum(3);

		sliders[1].BindThumb(FindWindow("btnPrimaryRes"));
		sliders[1].BindTrack(FindWindow("btnPrimaryResRegion"));
		sliders[1].SetMaximum(4);

		sliders[2].BindThumb(FindWindow("btnPrimaryColorDepth"));
		sliders[2].BindTrack(FindWindow("btnPrimaryColorDepthRegion"));
		sliders[2].SetMaximum(3);

		sliders[3].BindThumb(FindWindow("btnPrimaryBrightness"));
		sliders[3].BindTrack(FindWindow("btnPrimaryBrightnessRegion"));

		sliders[4].BindThumb(FindWindow("btnSecondaryMonitor"));
		sliders[4].BindTrack(FindWindow("btnSecondaryMonitorRegion"));
		sliders[4].SetMaximum(4);

		sliders[5].BindThumb(FindWindow("btnSecondaryRes"));
		sliders[5].BindTrack(FindWindow("btnSecondaryResRegion"));
		sliders[5].SetMaximum(4);

		sliders[6].BindThumb(FindWindow("btnSecondaryColorDepth"));
		sliders[6].BindTrack(FindWindow("btnSecondaryColorDepthRegion"));
		sliders[6].SetMaximum(3);

		sliders[7].BindThumb(FindWindow("btnSecondaryBrightness"));
		sliders[7].BindTrack(FindWindow("btnSecondaryBrightnessRegion"));
	}

	OptionsVideoInterface::~OptionsVideoInterface() //449340.
	{
		//Original reverse member destruction:8 sliders,6 switches,2 dials,base.
	}

	void OptionsVideoInterface::PrepareForOpen() //Native4430E9 calls before Video show.
	{
	}

	int OptionsVideoInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		//Logical449400 through adjustor449640; raw context resolves malformed IDA chunks.
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
				//Original recognized Default branch has no backend/configuration action.
			}
			else if (_stricmp(window->GetName(), "btnDone") == 0)
			{
				OptionsInterface* options = static_cast<GameInterfaces*>(static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0)->options;
				options->CloseChildren();
				options->SetHidden(1);
			}
			else if (_stricmp(window->GetName(), "btnCancel") == 0)
			{
				OptionsInterface* options = static_cast<GameInterfaces*>(static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0)->options;
				options->CloseChildren();
				options->SetHidden(1);
			}
		}
		else if (event->type == 4 || event->type == 7)
		{
			for (int index = 0; index < 2; ++index)
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
		}
		else if (event->type == 17 && window == rootWindow)
		{
			switch (event->x)
			{
			case 1:
				{
					OptionsInterface* options = static_cast<GameInterfaces*>(static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0)->options;
					options->CloseChildren();
					options->SetHidden(1);
					return 1;
				}
			}
		}
		return JE::EngineInterface::OnEvent(window, event);
	}
}
