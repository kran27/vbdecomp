#include "EngineClient.hpp"
#include "network/CommandProcessor.hpp"
#include "graphics/CameraControl.hpp"
#include "startup/MessageLoop.hpp"
#include "startup/NetworkComponent.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "network/Packet.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/GraphicsScene.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/World.hpp"
#include "graphics/ClientInput.hpp"
#include "graphics/ClientPicking.hpp"
#include "graphics/ClientCursor.hpp"
#include "gui/Manager.hpp"
#include "graphics/Video.hpp"
#include "gui/FadeManager.hpp"
#include "audio/Audio.hpp"
#include <stdexcept>
#include <cstring>

namespace JE
{
	namespace
	{
		class ClientPickingOwners : public ClientPickingServices
		{
		public:
			explicit ClientPickingOwners(Client& owner) : client(owner) {}
			virtual DWORD Clock() { return EngineClientServices::AccumulatedTime(); }
			virtual OCTREE_Camera* CurrentCamera() { return client.child0x74->GetPickingCamera(); }
			virtual OCTREE* CurrentOctree() { return g_World->GetCurrentMapOctree(); }
		private:
			Client& client;
		};
	}
	BYTE Client::TryGetGroundPosition(D3DXVECTOR3& output) //587AC0.
	{
		if (!reserved0xE8[0]) return 0;
		std::memcpy(&output.x, &field0xEC, sizeof(field0xEC));
		std::memcpy(&output.y, &field0xF0, sizeof(field0xF0));
		std::memcpy(&output.z, &field0xF4, sizeof(field0xF4));
		return 1;
	}

	void Client::PickInputMouse(BYTE force)
	{
		ClientPickingOwners owners(*this);
		PickClientMouse(*this, force, owners);
	}

	void Client::UpdateInputFrame()
	{
		Input::InputEvent event;
		while (child0x78->PopEvent(event)) //587EF6/587FAF, reload after every routed event.
		{
			if (event.category == 2)
			{
				field0xC4 = event.x;
				field0xC8 = event.y;
				field0xCC = event.wheel;
				field0xD0 = WORD(event.left) | (WORD(event.middle) << 8);
				field0xC0 = 1;
				field0xD2 = event.right;
			}
			if (child0x74->ProcessInput(event)) continue; //587F50, actual frontend virtual+10.
			if (event.category == 2) field0xC0 = 0;
			if (!child0xA4->ReleaseInput(event) && !HandleInputEvent(event) && field0x110)
				child0x80->Handle(event); //599720 uses its actual captured command processor.
		}
		if (!::GUI::g_Manager.HasModal()) UpdateInputCursor(); //4B2C10, then virtual+D0.
	}

	void Client::UpdateWorldFrame()
	{
		if (state != 3) return; //58801F: the enclosing state gate is read once.
		World* world = g_World;
		const DWORD clock = GetEntityEnvironment().FrameClock();
		world->UpdateClientFrame(clock);
		world = g_World;
		world->CollectVisibleEffects();
		if (field0x108 || field0x109) return; //588059..58806B, live flags after collection.
		world = g_World;

		GraphicsSceneState graphics(*child0x74);
		world->DrawVisible(graphics);
	}

	void Client::UpdatePolling() //587EE0.
	{
		Net::Client::UpdatePolling();
		Input::InputEvent event;
		while (child0x78->PopEvent(event))
		{
			if (event.category==2)
			{
				field0xC4=event.x;
				field0xC8=event.y;
				field0xCC=event.wheel;
				std::memcpy(&field0xD0,&event.left,sizeof(field0xD0)+sizeof(field0xD2));
				field0xC0=1;
			}
			if (child0x74->ProcessInput(event)) continue;
			if (event.category==2) field0xC0=0;
			if (!child0xA4->ReleaseInput(event) && !HandleInputEvent(event) && field0x110)
				child0x80->Handle(event);
		}
		if (!static_cast<BYTE>(::GUI::g_Manager.HasModal())) UpdateInputCursor();
		child0x7C->Dispatch();
		child0x84->Update();
		child0xA4->UpdateFrame();
		child0xA8->Update();
		child0xB4->Update();
		BYTE mapChanged;
		child0x74->PrepareScene(&mapChanged);
		PickInputMouse(0);
		if (state==3)
		{
			IWorldClient* world=g_World;
			world->UpdateClientFrame(F3::GetElapsedTimeMS());
			static_cast<IWorldClient*>(g_World)->CollectVisibleEffects();
			if (!field0x108 && !field0x109)
				static_cast<IWorldClient*>(g_World)->DrawVisible();
		}
		child0x74->Present();
		child0x70->Update();
	}
	void Client::PrepareDisconnect(Net::Packet&) //5880A0.
	{
		if (g_World) static_cast<IWorldClient*>(g_World)->Reset(); //Actual IWorld+50; packet and Client are unused.
	}
	JE::Net::UInt8 Client::OnServerStart(Net::Packet& packet) //58A620.
	{
		packet.ReadDword(clientID);
		LogDebugString("Client: connected to server.  My id is %u\n",clientID);
		field0x108=0;
		field0x10A=0;
		if (child0x74->startupMovies)
		{
			child0xB4->Play("movies\\IPlogo.mve", 192, 224, 640, 320, 1, 1, NULL);
			child0xB4->SetFade(1000);
			child0xB4->SetInterfaceValue(1);
			child0xB4->Queue("movies\\BISlogo.mve", 192, 224, 640, 320, 1, 1, NULL);
		}
		//58A710 tests local-server mode only after the optional startup movies.
		//Local modes1/3 share the server World and leave its serialized snapshot unread.
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
		{
			static_cast<IWorldClient*>(g_World)->UnpackState(packet); //58A728, same packet/cursor.
			Net::Packet join;
			join.field0x30 = 67;
			Send(join, 1); //58A750, no payload writes before the reliable join.
		}
		return 1;
	}
	JE::Net::UInt8 Client::OnServerDisconnect(Net::Packet&) //588170.
	{
		if (g_World) static_cast<IWorldClient*>(g_World)->Reset(); //Shared original reset boundary, including GameWorld override.
		return 1;
	}
	JE::Net::UInt8 Client::OnServerTimeout(JE::Net::UInt32)
	{
		Disconnect(); //588190 calls virtual+34, then returns1.
		return 1;
	}
}
