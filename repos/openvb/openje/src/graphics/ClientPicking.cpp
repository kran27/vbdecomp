#include <winsock2.h>
#include "ClientPicking.hpp"
#include "OctreeCamera.hpp"
#include "AnimationInstance.hpp"
#include "AnimationObject.hpp"
#include "ModelInstance.hpp"
#include "model/ModelInstance.hpp"
#include "model/ModelInfo.hpp"
#include "entity/Entity.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "network/EngineClient.hpp"
#include <float.h>
#include <cstring>

namespace JE
{
	namespace
	{
		class BorrowedPickingFields
		{
		public:
			explicit BorrowedPickingFields(ClientInputState& input) : input(input) {}
			BYTE Enabled() const { return input.enabled; }
			bool Throttled(BYTE force, ClientPickingServices& services) const
			{ return !force && DWORD(services.Clock() - input.lastPickingTime) < 100; }
			void SetLastPickingTime(DWORD value) { input.lastPickingTime = value; }
			BYTE MouseOverGUI() const { return input.mouseOverGUI; }
			LONG MouseX() const { return input.mouseX; }
			LONG MouseY() const { return input.mouseY; }
			DWORD HoveredEntityID() const { return input.hoveredEntityID; }
			void SetHoveredEntityID(DWORD value) { input.hoveredEntityID = value; }
			VisualEffect* HoverEffect() const { return input.hoverEffect; }
			void SetHoverEffect(VisualEffect* effect) { input.hoverEffect = effect; }
			VFX_VEGInstance* SelectionEffect() const { return input.selectionEffect; }
			void ClearGroundHit() { input.groundHit = 0; }
			void SetGroundPoint(const float* point)
			{
				//The explicitly borrowed state retains its existing source policy.
				input.groundHit = 1;
				input.groundPosition = D3DXVECTOR3(point);
			}
		private:
			ClientInputState& input;
		};

		class LivePickingFields
		{
		public:
			explicit LivePickingFields(Client& client) : client(client) {}
			BYTE Enabled() const { return client.field0x110; }
			bool Throttled(BYTE force, ClientPickingServices& services) const
			{
				if (force) return false;
				const DWORD clock = services.Clock(); //58ABBB precedes the live D8 read.
				return DWORD(clock - client.field0xD8) < 100;
			}
			void SetLastPickingTime(DWORD value) { client.field0xD8 = value; }
			BYTE MouseOverGUI() const { return client.field0xC0; }
			LONG MouseX() const { return static_cast<LONG>(client.field0xC4); }
			LONG MouseY() const { return static_cast<LONG>(client.field0xC8); }
			DWORD HoveredEntityID() const { return client.field0xDC; }
			void SetHoveredEntityID(DWORD value) { client.field0xDC = value; }
			VisualEffect* HoverEffect() const
			{
				VisualEffect* effect = NULL;
				std::memcpy(&effect, &client.field0xFC, sizeof(client.field0xFC));
				return effect;
			}
			void SetHoverEffect(VisualEffect* effect)
			{ std::memcpy(&client.field0xFC, &effect, sizeof(client.field0xFC)); }
			VFX_VEGInstance* SelectionEffect() const { return client.child0xF8; }
			void ClearGroundHit() { client.reserved0xE8[0] = 0; }
			void SetGroundPoint(const float* point)
			{
				//58B160..58B17A: raw position words surround the one-byte hit store.
				std::memcpy(&client.field0xEC, point, sizeof(client.field0xEC));
				client.reserved0xE8[0] = 1;
				std::memcpy(&client.field0xF0, point + 1, sizeof(client.field0xF0));
				std::memcpy(&client.field0xF4, point + 2, sizeof(client.field0xF4));
			}
		private:
			//No construction-time field reads, especially the unwritten byte atE8.
			Client& client;
		};

		template<class Fields>
		void PickClientMouseFields(Fields& state, BYTE force, ClientPickingServices& services)
		{
			if (!state.Enabled()) return;
			if (state.Throttled(force, services)) return;
			state.SetLastPickingTime(services.Clock());
			if (state.MouseOverGUI() == 1) return;
			OCTREE_Camera* camera = services.CurrentCamera();
			if (!camera) return;
			OctreeRay ray;
			camera->MakeScreenRay(state.MouseX(), state.MouseY(), &ray);
			Entity* closest = NULL;
			float closestDistance = FLT_MAX;
			DWORD more = 0;
			do
			{
				OctreeObjectHit candidates[20];
				camera->QueryObjects(ray, 20, candidates, &more);
				for (OctreeObjectHit* hit = candidates; hit->hit; ++hit)
				{
					Entity* entity = dynamic_cast<Entity*>(static_cast<DynamicObject*>(hit->object));
					if (!entity || !entity->renderEnabled || !entity->selectable || !entity->model) continue;
					ModelInstance* model = entity->model;
					if (model->animation)
					{
						Gfx_AnimationInstance* animation = model->animation;
						if (animation->animationObject->collisionMesh && animation->animationObject->meshBinding)
						{
							const D3DXMATRIX transform = model->instances[0]->transform;
							const D3DXVECTOR3 origin(ray.origin), direction(ray.direction.value);
							float distance = 0;
							if (animation->RayIntersection(transform, origin, direction, distance)
								&& distance < closestDistance)
							{ closestDistance = distance; closest = entity; }
						}
						else
						{
							LogDebugString("Client::HandleMousePicking - No Collision Data for %s!\n",
								animation->GetSkeleton()->name);
							if (hit->distance < closestDistance)
							{ closestDistance = hit->distance; closest = entity; }
						}
					}
					else
					{
						Gfx_ModelInstance* base = model->instances[0];
						if (base)
						{
							D3DXVECTOR3 minimum, maximum, origin, direction;
							base->GetBounds(minimum, maximum);
							const D3DXMATRIX transform = base->transform;
							D3DXMATRIX inverse;
							D3DXMatrixInverse(&inverse, NULL, &transform);
							const D3DXVECTOR3 worldOrigin(ray.origin), worldDirection(ray.direction.value);
							D3DXVec3TransformCoord(&origin, &worldOrigin, &inverse);
							D3DXVec3TransformNormal(&direction, &worldDirection, &inverse);
							OctreeRay localRay(&origin.x, &direction.x);
							float distance = FLT_MAX, point[3];
							if ((localRay.*localRay.testBounds)(&minimum.x, &maximum.x, &distance, point)
								&& distance < closestDistance)
							{ closestDistance = distance; closest = entity; }
						}
						else
						{
							//5648D0 ->5647E0 returns slot0/variation0's actual resource name.
							LogDebugString("Client::HandleMousePicking - No BaseModel for collision checking: %s!\n",
								model->info->models[0][0].name.c_str());
						}
					}
				}
			} while (more);

			if (state.HoveredEntityID())
			{
				Entity* old = GetEntityPtr(state.HoveredEntityID());
				if (!old) state.SetHoveredEntityID(0);
				else if (old == closest)
				{
					if (old->IsUnderFogOfWar() && state.HoverEffect())
					{
						old->RemoveVisualEffect(state.HoverEffect(), 1);
						state.SetHoverEffect(NULL);
						state.SetHoveredEntityID(0);
					}
				}
				else
				{
					old = GetEntityPtr(state.HoveredEntityID()); //Original reloads the lookup.
					if (old && state.HoverEffect())
					{
						old->RemoveVisualEffect(state.HoverEffect(), 1);
						state.SetHoverEffect(NULL);
					}
				}
			}
			if (!closest || closest->IsUnderFogOfWar())
			{
				OCTREE* tree = services.CurrentOctree();
				if (tree)
				{
					OctreeGroundHit ground;
					if (!tree->QueryGroundRay(ray, &ground, 1, 0)) state.ClearGroundHit();
					else
					{
						state.SetGroundPoint(ground.point);
					}
				}
				state.SetHoveredEntityID(0);
			}
			else
			{
				state.ClearGroundHit();
				state.SetHoverEffect(closest->AttachVisualEffect(state.SelectionEffect(), 0, 0, 0, 1));
				state.SetHoveredEntityID(closest->entityID);
			}
		}
	}

	void PickClientMouse(ClientInputState& state, BYTE force, ClientPickingServices& services)
	{
		BorrowedPickingFields fields(state);
		PickClientMouseFields(fields, force, services);
	}

	void PickClientMouse(Client& client, BYTE force, ClientPickingServices& services)
	{
		LivePickingFields fields(client);
		PickClientMouseFields(fields, force, services);
	}
}
