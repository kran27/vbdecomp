#pragma once

#include "EngineClient.hpp"
#include "configuration/ConfigurationTypes.hpp"

namespace JE
{
	class VFX_VEGCache;
	typedef VFX_VEGCache ClientEffectManager;
	namespace EngineClientInitializationServices
	{
		Configuration::Settings& MainSettings(); //Actual fixed56AF30 object, never engine settings.
		void InitializeNetClient(Net::Client& client);
		void InitializeChild(ClientChild0x70* child);
		void PrepareChild(ClientChild0x70* child); // 0x590A20.
		void InitializeChild(ClientChild0x74* child);
		void ConfigureChild(ClientChild0x74* child, int enabled);
		void InitializeChild(ClientChild0x78* child);
		void InitializeChild(ClientChild0x7C* child);
		void InitializeChild(ClientChild0x80* child);
		void InitializeChild(ClientChild0x84* child);
		void ShowChild(ClientChild0x88* child, int shown);
		void ShowChild(ClientChild0x8C* child, int shown);
		void SetDimensions(ClientChild0x8C* child, int width, int height); // 0x558190.
		void SizeChild(ClientChild0x8C* child, int width, int height);
		void ShowChild(ClientChild0x94* child, int shown);
		void SizeChild(ClientChild0x94* child, int width, int height);
		void InitializeChild(ClientChild0xB4* child);
		void PrepareChild(ClientChild0xB4* child); // 0x5B52E0.
		void InitializeChild(ClientChild0xA8* child);
		void FinishChild(ClientChild0xA8* child);
		void InitializeChild(ClientChild0xA4* child);
		void FinishChild(ClientChild0xA4* child);
		void InitializeChild(ClientChild0xA0* child);
		void FinishChild(ClientChild0xA0* child);
		// The DWORD at 0x6FF0D8 is captured before the effect-manager getter.
		extern JE::Net::UInt32 g_SelectionEffectArgument;
		ClientEffectManager* GetEffectManager(); // 0x4E1DE0.
		ClientChild0xB8* CreateChild0xB8();
		ClientChild0xF8* DemandEffect(ClientEffectManager* manager, const char* name,
			JE::Net::UInt32 argument); // 0x4E2AD0; captured raw float scale word.
		JE::Net::UInt32 NetworkMode(); // Canonical Network singleton's live mode.
		void ConnectLocalhost(Client& client); // Original virtual slot48.
	}
}
