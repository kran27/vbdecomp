#include "RenderManager.hpp"
#include "PacketData.hpp"
#include "RenderStyle.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include "Viewport.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cmath>
#include <cstring>

void Gfx_RenderManager::DrawGroup(UINT pass, Gfx_RenderGroup* group, BYTE shadowOnly)
{
	// This identity is captured before the SDK callback. Bounds are read from
	// that same mesh afterward, even if the callback changes group.mesh.
	Gfx_MeshData* initialMesh = group->mesh;
	g_D3D.device->SetVertexShader(initialMesh->vertexFVF);
	D3DXVECTOR3 bounds;
	std::memcpy(&bounds, initialMesh->unknown54 + 3, sizeof(bounds));
	const float shadowYOffset = g_GfxSystem->Parameter84() * bounds.y;
	const float shadowDistance = g_GfxSystem->Parameter88() * bounds.y;
	DWORD previousStyle = 33;
	Gfx_RenderStyleController* controller = 0;
	std::list<Gfx_MeshInstance>::iterator instance = group->instances.begin();
	while (instance != group->instances.end())
	{
		if (shadowOnly && !instance->flag1F)
		{
			++instance;
			continue;
		}
		Gfx_PacketData* packet = dynamic_cast<Gfx_PacketData*>(
			reinterpret_cast<Gfx_BasePacketData*>(instance->value0C));
		bool restoreAmbient = false;
		D3DCOLOR oldAmbient;
		D3DXMATRIX world, scaleMatrix;
		const D3DXMATRIX* worldPointer;
		if (packet)
		{
			if (packet->ambientOverride)
			{
				g_GfxSystem->GetAmbientColor(&oldAmbient);
				g_GfxSystem->SetAmbientColor(&packet->ambientColor);
				restoreAmbient = true;
			}
			std::memcpy(&scaleMatrix, &g_D3D.identity, sizeof(scaleMatrix));
			bool scaled = packet->scale.x != 1.0f || packet->scale.y != 1.0f || packet->scale.z != 1.0f;
			if (scaled)
			{
				scaleMatrix._11 = packet->scale.x;
				scaleMatrix._22 = packet->scale.y;
				scaleMatrix._33 = packet->scale.z;
			}
			worldPointer = packet->transform;
			if (packet->transformEnabled)
			{
				std::memcpy(&world, worldPointer, sizeof(world));
				if (scaled)
				{
					D3DXMatrixMultiply(&world, &scaleMatrix, &world);
					worldPointer = &world;
				}
			}
			else
			{
				std::memcpy(&world, &g_D3D.identity, sizeof(world));
				worldPointer = &world;
				if (scaled) std::memcpy(&world, &scaleMatrix, sizeof(world));
			}
			g_D3D.SetTransform(D3DTS_WORLD, &world);
			if (!packet->perPrimitiveLights)
			{
				if (!packet->lights.empty()) Gfx_ApplyLights(&packet->lights, 0, 0);
				else Gfx_ResetLights();
			}
		}
		else
		{
			std::memcpy(&world, &g_D3D.identity, sizeof(world));
			worldPointer = &world;
			g_D3D.SetTransform(D3DTS_WORLD, &g_D3D.identity);
			Gfx_ResetLights();
		}

		Gfx_MeshData* mesh = instance->mesh;
		Gfx_MaterialSet* materials = instance->value04 ?
			reinterpret_cast<Gfx_MaterialSet*>(instance->value04) : &mesh->materialSet;
		Gfx_Primitive* primitives = mesh->primitives;
		g_D3D.statistics[4] += mesh->vertexCount;
		std::list<DWORD>::iterator material = materials->groups[pass].begin();
		while (material != materials->groups[pass].end())
		{
			Gfx_Primitive* primitive = primitives + *material;
			if (primitive->value00 == 0xFFFFFFFF || instance->primitiveFlags[primitive->value00])
			{
				Gfx_Material* value = materials->materials + *material;
				DWORD style = value->style2;
				if (value->flag240)
				{
					if (shadowOnly)
					{
						DWORD color = instance->value24;
						// The fourth word of constant14 is the adjacent RGB.x word
						// (original B+BC cameraXYZ, B+C8 RGB). Keep that overlap,
						// including mutations through the earlier SDK color argument.
						struct ShaderVectors
						{
							D3DXVECTOR3 cameraPosition;
							D3DXVECTOR4 rgb;
							D3DXVECTOR4 parameters;
						} vectors;
						vectors.rgb.x = static_cast<float>((color >> 16) & 255) * (1.0f / 255.0f);
						vectors.rgb.y = static_cast<float>((color >> 8) & 255) * (1.0f / 255.0f);
						vectors.rgb.z = static_cast<float>(color & 255) * (1.0f / 255.0f);
						vectors.rgb.w = 1.0f;
						vectors.parameters = D3DXVECTOR4(instance->value20, 0.0f, 0.5f, 1.0f);
						Gfx_Camera* camera = owner->activeCamera;
						std::memcpy(&vectors.cameraPosition, &camera->transform._41, sizeof(vectors.cameraPosition));
						D3DXMATRIX view, projection, worldView, viewCopy, worldViewProjection;
						D3DXMATRIX constants, inverseWorld, transposedWorld;
						camera->GetViewMatrix(view);
						camera->GetProjectionMatrix(projection);
						D3DXMatrixMultiply(&worldView, worldPointer, &view);
						std::memcpy(&viewCopy, &worldView, sizeof(viewCopy));
						D3DXMatrixMultiply(&worldViewProjection, &viewCopy, &projection);
						std::memcpy(&constants, &worldViewProjection, sizeof(constants));
						D3DXMatrixTranspose(&constants, &constants);
						D3DXMatrixInverse(&inverseWorld, 0, worldPointer);
						D3DXMatrixTranspose(&transposedWorld, worldPointer);
						g_D3D.device->SetVertexShaderConstant(0, &constants, 4);
						g_D3D.device->SetVertexShaderConstant(4, &inverseWorld, 4);
						g_D3D.device->SetVertexShaderConstant(8, &transposedWorld, 4);
						g_D3D.device->SetVertexShaderConstant(12, &vectors.rgb, 1);
						g_D3D.device->SetVertexShaderConstant(13, &vectors.parameters, 1);
						g_D3D.device->SetVertexShaderConstant(14, &vectors.cameraPosition, 1);
						style = 27;
					}
					if (previousStyle != style)
					{
						controller = g_RenderStyles[style];
						previousStyle = style;
					}
					if (packet && packet->perPrimitiveLights)
					{
						D3DXVECTOR4 extra;
						std::memcpy(&extra, &packet->extraLight, sizeof(extra));
						if (static_cast<LONG>(style) >= 28 && static_cast<LONG>(style) <= 31)
						{
							extra.x = extra.y = extra.z = 0;
						}
						if (packet->LightCount(primitive->value00))
							Gfx_ApplyLights(packet->FindLights(primitive->value00), &extra, 0);
						else if (extra.x == 0.0f && extra.y == 0.0f && extra.z == 0.0f)
							Gfx_ResetLights();
						else Gfx_ApplyLights(0, &extra, 0);
					}
					BYTE flagA = flag40 && instance->flag1D;
					BYTE flagB = flag48 && instance->flag1E;
					if (controller)
					{
						controller->SetMaterial(value, 0, flagB, flagA,
							reinterpret_cast<DWORD>(heightFog));
						UINT passes = 1;
						if (primitive->value24 & 1)
						{
							D3DPRIMITIVETYPE type = primitive->type;
							DWORD first = primitive->value2C;
							DWORD count = primitive->value30;
							controller->styles[controller->activeIndex]->Begin(&passes);
							for (UINT i = 0; i < passes; ++i)
							{
								controller->styles[controller->activeIndex]->Pass(i);
								g_D3D.statistics[3] += count;
								++g_D3D.statistics[12];
								g_D3D.device->DrawIndexedPrimitive(type, 0, mesh->vertexCount, group->indexBase + first, count);
							}
						}
						else
						{
							D3DPRIMITIVETYPE type = primitive->type;
							DWORD first = primitive->value28;
							DWORD count = primitive->value30;
							controller->styles[controller->activeIndex]->Begin(&passes);
							for (UINT j = 0; j < passes; ++j)
							{
								controller->styles[controller->activeIndex]->Pass(j);
								g_D3D.statistics[3] += count;
								++g_D3D.statistics[11];
								g_D3D.device->DrawPrimitive(type, group->vertexBase + first, count);
							}
						}
						controller->styles[controller->activeIndex]->End();
					}
				}
			}
			++material;
		}

		if (projectedShadows && pass == 0 && instance->flag1C)
		{
			Gfx_MeshData* shadowMesh = group->mesh;
			DWORD stride = shadowMesh->vertexStride;
			D3DXMATRIX shadowWorld, inverseShadowWorld;
			std::memcpy(&shadowWorld, &g_D3D.identity, sizeof(shadowWorld));
			if (packet && packet->transformEnabled)
				std::memcpy(&shadowWorld, packet->transform, sizeof(shadowWorld));
			D3DXMatrixInverse(&inverseShadowWorld, 0, &shadowWorld);
			D3DXVECTOR3 shadowOrigin(shadowWorld._41, shadowWorld._42 + bounds.y, shadowWorld._43);
			D3DXVECTOR3 lightDirectionSum(0, 0, 0);
			DWORD contributingLightCount = 0;
			// A NULL packet is not repaired here. The original accesses +38 when
			// this independently gated branch is reached.
			Gfx_LightList::iterator light = packet->lights.begin();
			while (light != packet->lights.end())
			{
				Gfx_Light* shadowLight = *light;
				float lightDeltaX = shadowLight->transform._41 - shadowOrigin.x;
				float lightDeltaY = shadowLight->transform._42 - shadowOrigin.y;
				float lightDeltaZ = shadowLight->transform._43 - shadowOrigin.z;
				float lightDistance = static_cast<float>(std::sqrt(lightDeltaX * lightDeltaX + lightDeltaY * lightDeltaY + lightDeltaZ * lightDeltaZ));
				float constantAttenuation = shadowLight->light.Attenuation0;
				float linearAttenuation = shadowLight->light.Attenuation1;
				float quadraticAttenuation = shadowLight->light.Attenuation2;
				if ((linearAttenuation == 0.0f && quadraticAttenuation == 0.0f) || lightDistance < shadowLight->light.Range)
				{
					++contributingLightCount;
					// Retain the observed reciprocal sequence; algebraic cancellation
					// changes zeros/infinities and intermediate x87 rounding.
					float lightDirectionScale = 1.0f / (lightDistance * (1.0f / ((quadraticAttenuation * lightDistance + linearAttenuation) * lightDistance + constantAttenuation)));
					float scaledLightDeltaY = lightDeltaY * lightDirectionScale;
					float scaledLightDeltaZ = lightDeltaZ * lightDirectionScale;
					lightDirectionSum.x += lightDeltaX * lightDirectionScale;
					lightDirectionSum.y += scaledLightDeltaY;
					lightDirectionSum.z += scaledLightDeltaZ;
				}
				else JE::LogDebugString("Light out of range!\n");
				++light;
			}
			if (contributingLightCount)
			{
				float inverseCount = 1.0f / contributingLightCount;
				lightDirectionSum.x *= inverseCount;
				lightDirectionSum.y *= inverseCount;
				lightDirectionSum.z *= inverseCount;
			}
			D3DXVECTOR3 shadowPoint, localShadowPoint;
			D3DXVec3Normalize(&shadowPoint, &lightDirectionSum);
			shadowPoint.z *= shadowDistance;
			shadowPoint.x = shadowPoint.x * shadowDistance + shadowOrigin.x;
			shadowPoint.y = shadowPoint.y * shadowDistance + shadowYOffset + shadowOrigin.y;
			shadowPoint.z += shadowOrigin.z;
			if (shadowPoint.y <= shadowOrigin.y) shadowPoint.y = shadowOrigin.y;
			D3DXVec3TransformCoord(&localShadowPoint, &shadowPoint, &inverseShadowWorld);
			instance->skinning->ProcessProjectedShadow(shadowMesh->owned44, shadowMesh->owned50,
				shadowMesh->value4C, shadowMesh->vertices, stride, &localShadowPoint,
				static_cast<const DWORD*>(shadowMesh->owned48), instance->primitiveFlags);
		}
		if (restoreAmbient) g_GfxSystem->SetAmbientColor(&oldAmbient);
		++instance;
	}
}
