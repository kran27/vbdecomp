#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/ActionbarInterface.hpp"
#include "startup/EngineSettings.hpp"
#include "graphics/CameraControl.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/SystemInterface.hpp"
#include <cstdlib>
#include <stdexcept>

namespace F3
{
	void GameClient::ToggleFeedbackWindow() //Command45EE00; no arguments consumed.
	{
		GameClient* client=static_cast<GameClient*>(g_pNetworkInstance->client);
		ActionbarInterface* actionbar=client->Interfaces()->actionbar; //Actual unchecked Network.client/A0/10 chain.
		if (!actionbar->GetHidden()) actionbar->CycleCompanionMode();
	}
	void GameClient::InitializeMainmenuCamera() //460D60.
	{
		D3DXVECTOR3 target(212.60001f, 0.0f, 329.20001f);
		float azimuth = 322.79999f, elevation = 24.200001f, distance = 75.0f;
		float fov = 45.0f, nearPlane = 10.0f, farPlane = 200.0f;
		typedef JE::Configuration::Settings Settings;
		typedef JE::Configuration::Values Values;
		Settings& settings = *GetEngineSettings();
		Settings::iterator section = settings.find("Mainmenu");
		if (section != settings.end())
		{
			Values& values = section->second;
			{
				Values::const_iterator value = values.find("Target X");
				if (value != values.end()) target.x = static_cast<float>(std::atof(value->second.c_str()));
			}
			{
				Values::const_iterator value = values.find("Target Y");
				if (value != values.end()) target.y = static_cast<float>(std::atof(value->second.c_str()));
			}
			{
				Values::const_iterator value = values.find("Target Z");
				if (value != values.end()) target.z = static_cast<float>(std::atof(value->second.c_str()));
			}
			{
				Values::const_iterator value = values.find("Azimuth");
				if (value != values.end()) azimuth = static_cast<float>(std::atof(value->second.c_str()));
			}
			{
				Values::const_iterator value = values.find("Elevation");
				if (value != values.end()) elevation = static_cast<float>(std::atof(value->second.c_str()));
			}
			{
				Values::const_iterator value = values.find("Distance");
				if (value != values.end()) distance = static_cast<float>(std::atof(value->second.c_str()));
			}
			{
				Values::const_iterator value = values.find("FOV");
				if (value != values.end()) fov = static_cast<float>(std::atof(value->second.c_str()));
			}
			{
				Values::const_iterator value = values.find("zNear");
				if (value != values.end()) nearPlane = static_cast<float>(std::atof(value->second.c_str()));
			}
			{
				Values::const_iterator value = values.find("zFar");
				if (value != values.end()) farPlane = static_cast<float>(std::atof(value->second.c_str()));
			}
		}
		child0x84->SetConstraints(0);
		child0x84->SetAzimuth(azimuth);
		child0x84->SetElevation(elevation);
		child0x84->SetDistance(distance);
		child0x84->SetFieldOfView(fov);
		child0x84->SetTarget(target);
		child0x84->SetProjection(nearPlane, farPlane);
		child0x74->SetFogOfWarEnabled(0);
		g_GfxSystem->SetFlag2194(0);
	}

}
