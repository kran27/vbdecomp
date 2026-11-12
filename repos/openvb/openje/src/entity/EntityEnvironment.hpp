#pragma once

#include <windows.h>
#include <string>
#include "configuration/ConfigurationTypes.hpp"

class OCTREE_Camera;
class MapMaskManager;

namespace JE
{
	class Entity;
	class Server;
	// Typed host boundary for existing Network/Server/Client/party services. This
	// is not a replacement World or a reconstructed original game class.
	class EntityEnvironment
	{
	public:
		virtual ~EntityEnvironment() {}
		virtual DWORD NetworkMode() = 0;
		virtual void SetEntityTeam(Entity* entity, DWORD team) = 0; //Server555890.
		virtual void SetGlobal(const std::string& name, LONG value) = 0; //Server5566B0; actual settings then packet65.
		virtual Server* GetServer() = 0; //Borrowed actual EngineServer owner for script mutation APIs.
		virtual std::string PlayerResourceName() = 0;
		virtual void PublishPlayerID(DWORD id) = 0;
		virtual DWORD ClientPlayerID() = 0; // Actual GameClient current player member, not a class-number test.
		virtual void ClearClientSelection(DWORD entityID) = 0; //551C50, clear Client+E0 only when equal.
		virtual DWORD FrameClock() = 0; // Original56AF50 shared WinMM message-pump snapshot.
		virtual const Configuration::Settings& EngineSettings() = 0; // Actual56AF40 settings owner.
		virtual BYTE IsPartyMember(Entity* entity) = 0;
		virtual void BeforeWorldDestruction() = 0; // Original46F1FF..46F226.
		virtual void ClearGraphicsSelectionCache() = 0; // Graphics581330 optional+14 child.
		virtual MapMaskManager* GetMapMaskManager() = 0; //Graphics581310/581320, borrowed real+14 owner (may beNULL).
		virtual DWORD LineOfSightMode() = 0; //Actual Client BYTE+100; only the low byte is tested.
		virtual OCTREE_Camera* CurrentMapCamera() = 0; //Graphics581190, actual Octree2Gfx+1C.
		virtual DWORD RenderMode() = 0; //Actual JE::Client+104, distinct from Network mode.
		virtual DWORD WaterMode() = 0; // Client+10C:0 On,1 Not drawn,2 Off;550AD0/58E7E0.
	};
	EntityEnvironment& GetEntityEnvironment();
}
