#include "ClientCameraCommands.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "graphics/CameraControl.hpp"
#include "entity/Entity.hpp"
#include <cstdlib>
#include <cstring>

namespace F3
{
	namespace
	{
		JE::Client* PublishedClient()
		{
			return static_cast<JE::Client*>(g_pNetworkInstance->client);
		}

		void CommandCameraAzimuthInc() //588360.
		{
			JE::CameraControl* camera = PublishedClient()->child0x84;
			float azimuth = camera->orbit.azimuth * 57.295776f;
			azimuth *= 0.022222223f;
			azimuth += 1.0f;
			azimuth *= 45.0f;
			camera->SetTargetAzimuth(azimuth);
		}

		void CommandCameraAzimuthDec() //5883A0.
		{
			JE::CameraControl* camera = PublishedClient()->child0x84;
			float azimuth = camera->orbit.azimuth * 57.295776f;
			azimuth *= 0.022222223f;
			azimuth *= 45.0f;
			azimuth -= 45.0f;
			camera->SetTargetAzimuth(azimuth);
		}

		void CommandCameraTopDown() //5883E0.
		{
			PublishedClient()->child0x84->ToggleTopDown(1);
		}

		void CommandCameraTop() //588400.
		{
			PublishedClient()->child0x84->Top(1);
		}

		void CommandCameraDown() //588420.
		{
			PublishedClient()->child0x84->Down(1);
		}

		void CommandCameraScrollLeftOn() //588440.
		{
			PublishedClient()->field0x112 |= 1;
		}

		void CommandCameraScrollRightOn() //588450.
		{
			PublishedClient()->field0x112 |= 2;
		}

		void CommandCameraScrollUpOn() //588460.
		{
			PublishedClient()->field0x112 |= 4;
		}

		void CommandCameraScrollDownOn() //588470.
		{
			PublishedClient()->field0x112 |= 8;
		}

		void CommandCameraScrollLeftOff() //588480.
		{
			PublishedClient()->field0x112 &= ~1;
		}

		void CommandCameraScrollRightOff() //588490.
		{
			PublishedClient()->field0x112 &= ~2;
		}

		void CommandCameraScrollUpOff() //5884A0.
		{
			PublishedClient()->field0x112 &= ~4;
		}

		void CommandCameraScrollDownOff() //5884B0.
		{
			PublishedClient()->field0x112 &= ~8;
		}

		void CommandCameraZoomInOn() //5884C0.
		{
			PublishedClient()->child0x84->movementFlags |= 0x10;
		}

		void CommandCameraZoomInOff() //5884E0.
		{
			PublishedClient()->child0x84->movementFlags &= ~0x10;
		}

		void CommandCameraZoomOutOn() //588500.
		{
			PublishedClient()->child0x84->movementFlags |= 0x20;
		}

		void CommandCameraZoomOutOff() //588520.
		{
			PublishedClient()->child0x84->movementFlags &= ~0x20;
		}

		void CommandCameraRotateLeft() //588540.
		{
			PublishedClient()->child0x84->movementFlags ^= 0x40;
		}

		void CommandCameraRotateRight() //588560.
		{
			PublishedClient()->child0x84->movementFlags ^= 0x80;
		}

		void CommandCameraTiltUp() //588580.
		{
			PublishedClient()->child0x84->movementFlags ^= 0x100;
		}

		void CommandCameraTiltDown() //5885A0.
		{
			PublishedClient()->child0x84->movementFlags ^= 0x200;
		}

		void CommandCameraToggleConstraints() //5885E0.
		{
			JE::CameraControl* camera = PublishedClient()->child0x84;
			camera->SetConstraints(camera->constraints == 0);
		}

		void CommandCameraMouseRotateOn() //5885C0.
		{
			PublishedClient()->SetCameraMouseRotate(1);
		}

		void CommandCameraMouseRotateOff() //5885D0.
		{
			PublishedClient()->SetCameraMouseRotate(0);
		}

		void CommandCameraAzimuth(CommandArguments* arguments) //58B450.
		{
			JE::CameraControl* camera = PublishedClient()->child0x84;
			if (arguments->size() > 1)
			{
				const float value = static_cast<float>(std::atof((*arguments)[1]));
				camera->SetTargetAzimuth(value);
			}
		}

		void CommandCameraElevation(CommandArguments* arguments) //58B500.
		{
			JE::CameraControl* camera = PublishedClient()->child0x84;
			if (arguments->size() > 1)
			{
				const float value = static_cast<float>(std::atof((*arguments)[1]));
				camera->SetTargetElevation(value);
			}
		}

		void CommandCameraDistance(CommandArguments* arguments) //58B5B0.
		{
			JE::CameraControl* camera = PublishedClient()->child0x84;
			if (arguments->size() > 1)
			{
				const float value = static_cast<float>(std::atof((*arguments)[1]));
				camera->SetTargetDistance(value);
			}
		}

		void CommandCameraSetTarget(CommandArguments* arguments) //58B6E0.
		{
			if (arguments->size() > 1)
			{
				JE::Entity* entity = JE::GetEntityPtr(DWORD(std::atol((*arguments)[1])));
				if (entity)
				{
					D3DXVECTOR3 target;
					std::memcpy(&target, &entity->position, sizeof(target));
					PublishedClient()->child0x84->SetTarget(target);
				}
				return;
			}
			std::list<JE::Entity*>::iterator end = JE::g_createdEntities.end();
			JE::Entity* entity = NULL;
			std::list<JE::Entity*>::iterator i = JE::g_createdEntities.begin();
			if (i != end)
			{
				NetworkComponent* network = g_pNetworkInstance;
				do
				{
					JE::Entity* candidate = *i;
					if (candidate->visibilityOwnerFlag)
					{
						JE::Client* client = static_cast<JE::Client*>(network->client);
						if (candidate->visibilityOwnerFlag == client->clientID)
						{
							entity = candidate;
							if (candidate->entityID == client->field0xE0) break;
						}
					}
					++i;
				} while (i != end);
				if (entity)
				{
					D3DXVECTOR3 target;
					std::memcpy(&target, &entity->position, sizeof(target));
					PublishedClient()->child0x84->SetTarget(target);
				}
			}
		}

		void CommandCameraReset() //58A7F0.
		{
			JE::CameraControl* camera = PublishedClient()->child0x84;
			camera->Reset();
			JE::Client* client = PublishedClient(); //Reload after the actual Reset child.
			DWORD clientID = client->clientID;
			DWORD selectedID = client->field0xE0;
			JE::Entity* entity = NULL;
			std::list<JE::Entity*>::iterator end = JE::g_createdEntities.end();
			for (std::list<JE::Entity*>::iterator i = JE::g_createdEntities.begin(); i != end; ++i)
			{
				JE::Entity* candidate = *i;
				if (candidate->visibilityOwnerFlag && candidate->visibilityOwnerFlag == clientID)
				{
					entity = candidate;
					if (candidate->entityID == selectedID) break;
				}
			}
			if (entity)
			{
				D3DXVECTOR3 target = entity->position;
				camera->SetTarget(target);
			}
		}
	}

	unsigned char ExecuteClientCameraCommand(JE::Net::UInt32 originalVA, CommandArguments* arguments)
	{
		//Actual no-argument leaves, invoked only by the command processor.
		if (originalVA == 0x5885C0)
		{
			CommandCameraMouseRotateOn();
			return 1;
		}
		if (originalVA == 0x5885D0)
		{
			CommandCameraMouseRotateOff();
			return 1;
		}
		switch (originalVA)
		{
		case 0x588440: CommandCameraScrollLeftOn(); return 1;
		case 0x588450: CommandCameraScrollRightOn(); return 1;
		case 0x588460: CommandCameraScrollUpOn(); return 1;
		case 0x588470: CommandCameraScrollDownOn(); return 1;
		case 0x588480: CommandCameraScrollLeftOff(); return 1;
		case 0x588490: CommandCameraScrollRightOff(); return 1;
		case 0x5884a0: CommandCameraScrollUpOff(); return 1;
		case 0x5884b0: CommandCameraScrollDownOff(); return 1;
		case 0x588360: CommandCameraAzimuthInc(); return 1;
		case 0x5883A0: CommandCameraAzimuthDec(); return 1;
		case 0x5883E0: CommandCameraTopDown(); return 1;
		case 0x588400: CommandCameraTop(); return 1;
		case 0x588420: CommandCameraDown(); return 1;
		case 0x5884C0: CommandCameraZoomInOn(); return 1;
		case 0x5884E0: CommandCameraZoomInOff(); return 1;
		case 0x588500: CommandCameraZoomOutOn(); return 1;
		case 0x588520: CommandCameraZoomOutOff(); return 1;
		case 0x588540: CommandCameraRotateLeft(); return 1;
		case 0x588560: CommandCameraRotateRight(); return 1;
		case 0x588580: CommandCameraTiltUp(); return 1;
		case 0x5885A0: CommandCameraTiltDown(); return 1;
		case 0x5885E0: CommandCameraToggleConstraints(); return 1;
		case 0x58B6E0:
			CommandCameraSetTarget(arguments);
			return 1;
		case 0x58A7F0:
			CommandCameraReset();
			return 1;
		case 0x58B450: CommandCameraAzimuth(arguments); return 1;
		case 0x58B500: CommandCameraElevation(arguments); return 1;
		case 0x58B5B0: CommandCameraDistance(arguments); return 1;
		default:
			return 0;
		}
	}
}
