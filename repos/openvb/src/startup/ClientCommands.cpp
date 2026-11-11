#include "ClientCommands.hpp"
#include "ClientCommandExecution.hpp"
#include "ClientConsoleCommands.hpp"
#include "ClientGraphicsCommands.hpp"
#include "ClientWorldCommands.hpp"
#include "ClientEntityCommands.hpp"
#include "ClientMediaCommands.hpp"
#include "ClientMemoryCommand.hpp"
#include "ClientPipBoyCommand.hpp"
#include "ClientUseSkillCommand.hpp"
#include "ClientScriptCommands.hpp"
#include "ClientLightingCommands.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineClientInitialize.hpp"
#include "NetworkComponent.hpp"
#include "MessageLoop.hpp"
#include "ConfigurationObject.hpp"

#include <stdexcept>

namespace F3
{
	namespace
	{
		template<JE::Net::UInt32 OriginalVA> struct ClientCommandThunk
		{
			static void __cdecl Invoke(CommandArguments* arguments)
			{
				ExecuteClientCommand(OriginalVA,arguments);
			}
		};

		void __cdecl EmptyClientCommand(CommandArguments*) { } // Actual original 0x404B70.
	}

#define F3_REGISTRATION_CALLBACK(address, callback) &callback
	CommandCallback GetClientCommandCallback(JE::Net::UInt32 originalVA)
	{
		switch (originalVA)
		{
		case 0x404B70u: return &EmptyClientCommand;
		case 0x45EA50u: return &ClientCommandThunk<0x45EA50u>::Invoke;
		case 0x45EAC0u: return F3_REGISTRATION_CALLBACK(0x45EAC0u, CommandToggleInventoryWindow);
		case 0x45EB70u: return &ClientCommandThunk<0x45EB70u>::Invoke;
		case 0x45EC00u: return &ClientCommandThunk<0x45EC00u>::Invoke;
		case 0x45EC70u: return F3_REGISTRATION_CALLBACK(0x45EC70u, CommandTogglePipBoy);
		case 0x45EDB0u: return &ClientCommandThunk<0x45EDB0u>::Invoke;
		case 0x45EDC0u: return &ClientCommandThunk<0x45EDC0u>::Invoke;
		case 0x45EDD0u: return &ClientCommandThunk<0x45EDD0u>::Invoke;
		case 0x45EDE0u: return &ClientCommandThunk<0x45EDE0u>::Invoke;
		case 0x45EDF0u: return &ClientCommandThunk<0x45EDF0u>::Invoke;
		case 0x45EE00u: return &ClientCommandThunk<0x45EE00u>::Invoke;
		case 0x45EE30u: return F3_REGISTRATION_CALLBACK(0x45EE30u, CommandUseSkill);
		case 0x45EE60u: return &ClientCommandThunk<0x45EE60u>::Invoke;
		case 0x45EED0u: return &ClientCommandThunk<0x45EED0u>::Invoke;
		case 0x4606C0u: return &ClientCommandThunk<0x4606C0u>::Invoke;
		case 0x4606D0u: return &ClientCommandThunk<0x4606D0u>::Invoke;
		case 0x4606E0u: return &ClientCommandThunk<0x4606E0u>::Invoke;
		case 0x460720u: return F3_REGISTRATION_CALLBACK(0x460720u, CommandForceEntityStep);
		case 0x460920u: return F3_REGISTRATION_CALLBACK(0x460920u, CommandTestAnim);
		case 0x588290u: return F3_REGISTRATION_CALLBACK(0x588290u, CommandDisconnect);
		case 0x5882A0u: return &ClientCommandThunk<0x5882A0u>::Invoke;
		case 0x5882B0u: return F3_REGISTRATION_CALLBACK(0x5882B0u, CommandToggleFullscreen);
		case 0x5882C0u: return F3_REGISTRATION_CALLBACK(0x5882C0u, CommandToggleLighting);
		case 0x5882D0u: return F3_REGISTRATION_CALLBACK(0x5882D0u, CommandToggleShadow);
		case 0x5882E0u: return &ClientCommandThunk<0x5882E0u>::Invoke;
		case 0x5882F0u: return &ClientCommandThunk<0x5882F0u>::Invoke;
		case 0x588300u: return &ClientCommandThunk<0x588300u>::Invoke;
		case 0x588310u: return &ClientCommandThunk<0x588310u>::Invoke;
		case 0x588320u: return F3_REGISTRATION_CALLBACK(0x588320u, CommandDebugLight);
		case 0x588330u: return F3_REGISTRATION_CALLBACK(0x588330u, CommandDebugOctreeLight);
		case 0x588340u: return F3_REGISTRATION_CALLBACK(0x588340u, CommandToggleHeightFog);
		case 0x588350u: return F3_REGISTRATION_CALLBACK(0x588350u, CommandToggleOctreeNodeDebug);
		case 0x588360u: return &ClientCommandThunk<0x588360u>::Invoke;
		case 0x5883A0u: return &ClientCommandThunk<0x5883A0u>::Invoke;
		case 0x5883E0u: return &ClientCommandThunk<0x5883E0u>::Invoke;
		case 0x588400u: return &ClientCommandThunk<0x588400u>::Invoke;
		case 0x588420u: return &ClientCommandThunk<0x588420u>::Invoke;
		case 0x588440u: return &ClientCommandThunk<0x588440u>::Invoke;
		case 0x588450u: return &ClientCommandThunk<0x588450u>::Invoke;
		case 0x588460u: return &ClientCommandThunk<0x588460u>::Invoke;
		case 0x588470u: return &ClientCommandThunk<0x588470u>::Invoke;
		case 0x588480u: return &ClientCommandThunk<0x588480u>::Invoke;
		case 0x588490u: return &ClientCommandThunk<0x588490u>::Invoke;
		case 0x5884A0u: return &ClientCommandThunk<0x5884A0u>::Invoke;
		case 0x5884B0u: return &ClientCommandThunk<0x5884B0u>::Invoke;
		case 0x5884C0u: return &ClientCommandThunk<0x5884C0u>::Invoke;
		case 0x5884E0u: return &ClientCommandThunk<0x5884E0u>::Invoke;
		case 0x588500u: return &ClientCommandThunk<0x588500u>::Invoke;
		case 0x588520u: return &ClientCommandThunk<0x588520u>::Invoke;
		case 0x588540u: return &ClientCommandThunk<0x588540u>::Invoke;
		case 0x588560u: return &ClientCommandThunk<0x588560u>::Invoke;
		case 0x588580u: return &ClientCommandThunk<0x588580u>::Invoke;
		case 0x5885A0u: return &ClientCommandThunk<0x5885A0u>::Invoke;
		case 0x5885C0u: return &ClientCommandThunk<0x5885C0u>::Invoke;
		case 0x5885D0u: return &ClientCommandThunk<0x5885D0u>::Invoke;
		case 0x5885E0u: return &ClientCommandThunk<0x5885E0u>::Invoke;
		case 0x588600u: return &ClientCommandThunk<0x588600u>::Invoke;
		case 0x588610u: return &ClientCommandThunk<0x588610u>::Invoke;
		case 0x588620u: return F3_REGISTRATION_CALLBACK(0x588620u, CommandScreenCap);
		case 0x588630u: return &ClientCommandThunk<0x588630u>::Invoke;
		case 0x588690u: return F3_REGISTRATION_CALLBACK(0x588690u, CommandDebugWindow);
		case 0x5886F0u: return F3_REGISTRATION_CALLBACK(0x5886F0u, CommandStackWindow);
		case 0x588750u: return F3_REGISTRATION_CALLBACK(0x588750u, CommandDebugTools);
		case 0x5887B0u: return &ClientCommandThunk<0x5887B0u>::Invoke;
		case 0x588820u: return &ClientCommandThunk<0x588820u>::Invoke;
		case 0x588840u: return &ClientCommandThunk<0x588840u>::Invoke;
		case 0x588860u: return &ClientCommandThunk<0x588860u>::Invoke;
		case 0x588880u: return &ClientCommandThunk<0x588880u>::Invoke;
		case 0x5888E0u: return F3_REGISTRATION_CALLBACK(0x5888E0u, CommandVRAM);
		case 0x588B10u: return F3_REGISTRATION_CALLBACK(0x588B10u, CommandListTextures);
		case 0x588B30u: return F3_REGISTRATION_CALLBACK(0x588B30u, CommandToggleSampling);
		case 0x588B60u: return F3_REGISTRATION_CALLBACK(0x588B60u, CommandVSync);
		case 0x588BC0u: return F3_REGISTRATION_CALLBACK(0x588BC0u, CommandAdapterInfo);
		case 0x588D00u: return F3_REGISTRATION_CALLBACK(0x588D00u, CommandMovieTest);
		case 0x589580u: return F3_REGISTRATION_CALLBACK(0x589580u, CommandMemoryPoolTest);
		case 0x58A7B0u: return &ClientCommandThunk<0x58A7B0u>::Invoke;
		case 0x58A7D0u: return F3_REGISTRATION_CALLBACK(0x58A7D0u, CommandStartDialogue);
		case 0x58A7E0u: return F3_REGISTRATION_CALLBACK(0x58A7E0u, CommandEndDialogue);
		case 0x58A7F0u: return &ClientCommandThunk<0x58A7F0u>::Invoke;
		case 0x58B1B0u: return F3_REGISTRATION_CALLBACK(0x58B1B0u, CommandToggleStartupMovies);
		case 0x58B290u: return &ClientCommandThunk<0x58B290u>::Invoke;
		case 0x58B370u: return &ClientCommandThunk<0x58B370u>::Invoke;
		case 0x58B450u: return &ClientCommandThunk<0x58B450u>::Invoke;
		case 0x58B4A0u: return F3_REGISTRATION_CALLBACK(0x58B4A0u, CommandCameraAzimuthSpeed);
		case 0x58B500u: return &ClientCommandThunk<0x58B500u>::Invoke;
		case 0x58B550u: return F3_REGISTRATION_CALLBACK(0x58B550u, CommandCameraElevationSpeed);
		case 0x58B5B0u: return &ClientCommandThunk<0x58B5B0u>::Invoke;
		case 0x58B600u: return F3_REGISTRATION_CALLBACK(0x58B600u, CommandCameraDistanceSpeed);
		case 0x58B660u: return F3_REGISTRATION_CALLBACK(0x58B660u, CommandCameraFOV);
		case 0x58B6E0u: return &ClientCommandThunk<0x58B6E0u>::Invoke;
		case 0x58B7D0u: return F3_REGISTRATION_CALLBACK(0x58B7D0u, CommandCameraScrollSpeed);
		case 0x58B830u: return F3_REGISTRATION_CALLBACK(0x58B830u, CommandShowCursor);
		case 0x58B880u: return F3_REGISTRATION_CALLBACK(0x58B880u, CommandWarp);
		case 0x58B8B0u: return F3_REGISTRATION_CALLBACK(0x58B8B0u, CommandGuiOptimizationLevel);
		case 0x58B8E0u: return F3_REGISTRATION_CALLBACK(0x58B8E0u, CommandSetDisplayMode);
		case 0x58BA10u: return F3_REGISTRATION_CALLBACK(0x58BA10u, CommandFadeIn);
		case 0x58BAA0u: return F3_REGISTRATION_CALLBACK(0x58BAA0u, CommandFadeOut);
		case 0x58BB30u: return F3_REGISTRATION_CALLBACK(0x58BB30u, CommandEditToken);
		case 0x58BB90u: return F3_REGISTRATION_CALLBACK(0x58BB90u, CommandToggleBuilding);
		case 0x58DB80u: return F3_REGISTRATION_CALLBACK(0x58DB80u, CommandStartMap);
		case 0x58DBE0u: return &ClientCommandThunk<0x58DBE0u>::Invoke;
		case 0x58DC50u: return F3_REGISTRATION_CALLBACK(0x58DC50u, CommandConnect);
		case 0x58DDA0u: return F3_REGISTRATION_CALLBACK(0x58DDA0u, CommandCreateEntity);
		case 0x58DF10u: return F3_REGISTRATION_CALLBACK(0x58DF10u, CommandSetScript);
		case 0x58DFD0u: return F3_REGISTRATION_CALLBACK(0x58DFD0u, CommandGetGlobal);
		case 0x58E1B0u: return F3_REGISTRATION_CALLBACK(0x58E1B0u, CommandSetGlobal);
		case 0x58E340u: return F3_REGISTRATION_CALLBACK(0x58E340u, CommandToggleFOWLOS);
		case 0x58E380u: return &ClientCommandThunk<0x58E380u>::Invoke;
		case 0x58E4A0u: return F3_REGISTRATION_CALLBACK(0x58E4A0u, CommandDisableScripts);
		case 0x58E5B0u: return F3_REGISTRATION_CALLBACK(0x58E5B0u, CommandEnableScripts);
		case 0x58E6C0u: return F3_REGISTRATION_CALLBACK(0x58E6C0u, CommandOneScriptPass);
		case 0x58E7E0u: return F3_REGISTRATION_CALLBACK(0x58E7E0u, CommandToggleWater);
		default: throw std::logic_error("Unrecognized original Client command callback VA");
		}
	}

#undef F3_REGISTRATION_CALLBACK

	void RegisterClientCommand(const char* name, JE::Net::UInt32 originalVA)
	{
		ClientCommandRegistrationServices::Register(name,GetClientCommandCallback(originalVA));
	}

	namespace ClientCommandRegistrationServices
	{
		void Register(const char* name, CommandCallback callback) { RegisterConsoleCommand(name,callback); }
	}
}

namespace JE
{
	namespace EngineClientServices
	{
		void RegisterCommand(const char* name, JE::Net::UInt32 originalVA)
		{
			F3::RegisterClientCommand(name,originalVA);
		}

		JE::Net::UInt32 AccumulatedTime() { return F3::g_nElapsedTimeMS; }
	}

	namespace EngineClientInitializationServices
	{
		Configuration::Settings& MainSettings() { return *F3::GetSettingsObject(); }
		JE::Net::UInt32 NetworkMode() { return F3::g_pNetworkInstance->mode; }
	}
}
