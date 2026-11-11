#include "SkinMeshPacket.hpp"
#include "AnimationInstance.hpp"
#include "AnimationObject.hpp"
#include "RenderStyle.hpp"
#include "SystemInterface.hpp"
#include "D3DState.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "ModelNode.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <math.h>
#include <string.h>

void Gfx_SkinMesh::CopyBoneMatrices(Gfx_AnimationInstance* instance)
{
	if (instance)
	{
		const D3DXMATRIX* matrices = reinterpret_cast<const D3DXMATRIX*>(instance->compositeMatrices);
		for (DWORD bone = 0; bone < boneCount; ++bone)
		{
			const LONG semantic = LONG(semanticBones[bone]);
			if (semantic == 212) continue;
			const DWORD index = semantic < 212 ? instance->animationObject->semanticBoneIndices[semantic] : 0;
			memcpy(boneMatrices + bone, matrices + index, 64);
		}
	}
	else
	{
		for (DWORD bone = 0; bone < boneCount; ++bone)
			D3DXMatrixIdentity(boneMatrices + bone);
	}
}

void Gfx_SkinMesh::ProcessShadowVolumeRenderSilhouette(
	std::list<Gfx_RenderSubmission16>* submissions, BYTE projectedShadows, Gfx_Camera* camera)
{
	if (!shadowMesh)
	{
		JE::LogDebugString("Gfx_SkinMesh::ProcessShadowVolumeRenderSilhouette - SkinMesh not created");
		return;
	}
	for (std::list<Gfx_RenderSubmission16>::iterator entry = submissions->begin();
		entry != submissions->end(); ++entry)
	{
		Gfx_SkinMeshPkt* packet = entry->packet;
		if (!packet->castsShadow && !entry->flag) continue;
		CopyBoneMatrices(packet->animation);
		HRESULT result = skin->UpdateSkinnedMesh(boneMatrices, NULL, shadowMesh);
		if (FAILED(result))
		{
			JE::LogDebugString("Gfx_SkinMesh::ProcessShadowVolumeRenderSilhouette - UpdateSkinnedMesh failed %x\n", result);
			continue;
		}
		D3DXMATRIX inverse;
		D3DXMatrixInverse(&inverse, NULL, &packet->transform);
		if (projectedShadows && packet->castsShadow)
		{
			Gfx_ModelNode* root = static_cast<Gfx_ModelNode*>(packet->self->mesh->owner);
			const D3DXVECTOR3 bounds = root->maximum;
			const float heightBias = g_GfxSystem->Parameter84() * bounds.y;
			const float length = g_GfxSystem->Parameter88() * bounds.y;
			BYTE* vertices;
			result = shadowMesh->LockVertexBuffer(D3DLOCK_READONLY, &vertices);
			if (FAILED(result))
			{
				JE::LogDebugString("Gfx_SkinMesh::ProcessShadowVolumeRenderSilhouette - failed to lock vertex buffer to compute shadow volume\n");
			}
			else
			{
				const D3DXVECTOR3 origin(packet->transform._41,
					packet->transform._42 + bounds.y, packet->transform._43);
				memset(shadowLightWeights, 0, sizeof(shadowLightWeights));
				D3DXVECTOR3 direction(0.0f, 1.0f, 0.0f);
				DWORD selected = 0;
				for (std::list<Gfx_Light*>::iterator light = packet->lights.begin();
					light != packet->lights.end(); ++light)
				{
					Gfx_Light* current = *light;
					if (!current->flag48) continue;
					const D3DCOLORVALUE diffuse = current->light.Diffuse;
					const double distanceZ = double(current->transform._43) - origin.z;
					D3DXVECTOR3 displacement(current->transform._41 - origin.x,
						current->transform._42 - origin.y, float(distanceZ));
					const double distance = sqrt(distanceZ * displacement.z +
						double(displacement.y) * displacement.y + double(displacement.x) * displacement.x);
					if (!(current->light.Attenuation1 == 0.0f && current->light.Attenuation2 == 0.0f)
						&& !(distance < current->light.Range))
					{
						JE::LogDebugString("Light out of range!\n");
						continue;
					}
					const double attenuation = 1.0 / (distance *
						(current->light.Attenuation2 * distance + current->light.Attenuation1) +
						current->light.Attenuation0);
					const float weight = float((double(diffuse.b) * diffuse.b +
						double(diffuse.g) * diffuse.g + double(diffuse.r) * diffuse.r) *
						(attenuation * attenuation));
					DWORD slot = 0;
					while (slot < 8 && !(weight > 0.05f && weight > shadowLightWeights[slot])) ++slot;
					if (slot == 8) continue;
					//The original copies forward, propagating the preceding slot.
					for (DWORD next = slot + 1; next < 8; ++next)
					{
						shadowLightWeights[next] = shadowLightWeights[next - 1];
						shadowLightDirections[next] = shadowLightDirections[next - 1];
					}
					shadowLightWeights[slot] = weight;
					D3DXVec3Normalize(shadowLightDirections + slot, &displacement);
					const double magnitude = sqrt(weight);
					shadowLightDirections[slot].x = float(magnitude * shadowLightDirections[slot].x);
					shadowLightDirections[slot].y = float(magnitude * shadowLightDirections[slot].y);
					shadowLightDirections[slot].z = float(magnitude * shadowLightDirections[slot].z);
					++selected;
				}
				if (selected)
				{
					//507C90 does not clamp this count to eight. Its caller's light
					//cardinality precondition remains unchanged.
					for (DWORD light = 0; light < selected; ++light) direction += shadowLightDirections[light];
					const double reciprocal = 1.0 / selected;
					direction.x = float(direction.x * reciprocal);
					direction.y = float(direction.y * reciprocal);
					direction.z = float(direction.z * reciprocal);
				}
				D3DXVECTOR3 normalized;
				D3DXVec3Normalize(&normalized, &direction);
				const double projectedY = double(normalized.y) * length + heightBias + origin.y;
				const float scaledZ = float(double(normalized.z) * length);
				D3DXVECTOR3 worldLight(float(double(normalized.x) * length + origin.x),
					float(projectedY), float(double(scaledZ) + origin.z));
				if (projectedY <= origin.y) worldLight.y = origin.y;
				D3DXVECTOR3 localLight;
				D3DXVec3TransformCoord(&localLight, &worldLight, &inverse);
				if (g_GfxSystem->DebugLightsEnabled())
				{
					static Gfx_Light light(0);
					light.transform._41 = light.light.Position.x = worldLight.x;
					light.transform._42 = light.light.Position.y = worldLight.y;
					light.transform._43 = light.light.Position.z = worldLight.z;
					light.light.Diffuse.r = light.light.Diffuse.g =
						light.light.Diffuse.b = light.light.Diffuse.a = 1.0f;
					g_GfxSystem->SubmitLight(&light, NULL);
				}
				if (g_D3D.value0A4 == 2)
				{
					for (DWORD index = 0; index < shadowFaceCount; ++index)
					{
						Gfx_SkinFace& face = faces[index];
						const D3DXVECTOR3& first = *reinterpret_cast<const D3DXVECTOR3*>(
							vertices + shadowVertexStride * face.edges[0].vertices[0]);
						const D3DXVECTOR3& second = *reinterpret_cast<const D3DXVECTOR3*>(
							vertices + shadowVertexStride * face.edges[0].vertices[1]);
						const D3DXVECTOR3& third = *reinterpret_cast<const D3DXVECTOR3*>(
							vertices + shadowVertexStride * face.edges[1].vertices[1]);
						const D3DXVECTOR3 edge0 = second - first;
						const D3DXVECTOR3 edge1 = third - first;
						D3DXVec3Cross(&face.normal, &edge0, &edge1);
						D3DXVec3Normalize(&face.normal, &face.normal);
					}
				}
				packet->skinning->ProcessProjectedShadow(faces,
					edges, edgeCount, vertices, shadowVertexStride, &localLight, faceMaterials,
					packet->materialEnabled);
				shadowMesh->UnlockVertexBuffer();
			}
		}
		else packet->skinning->value08 = 0;
		if (entry->flag)
		{
			const float channelScale = 0.0039215689f;
			float color[4] = {
				((entry->value0C >> 16) & 255) * channelScale,
				((entry->value0C >> 8) & 255) * channelScale,
				(entry->value0C & 255) * channelScale, 1.0f
			};
			DWORD parameters[4] = { entry->value08, 0, 0x3F000000, 0x3F800000 };
			// Original uploads a register after writing only its xyz components.
			float cameraPosition[4];
			cameraPosition[0] = camera->transform._41;
			cameraPosition[1] = camera->transform._42;
			cameraPosition[2] = camera->transform._43;
			D3DXMATRIX view, projection, worldView, worldViewProjection, transposeWorld;
			camera->GetViewMatrix(view);
			camera->GetProjectionMatrix(projection);
			D3DXMatrixMultiply(&worldView, &packet->transform, &view);
			D3DXMatrixMultiply(&worldViewProjection, &worldView, &projection);
			D3DXMatrixTranspose(&worldViewProjection, &worldViewProjection);
			D3DXMatrixInverse(&inverse, NULL, &packet->transform);
			D3DXMatrixTranspose(&transposeWorld, &packet->transform);
			g_D3D.device->SetVertexShaderConstant(0, &worldViewProjection, 4);
			g_D3D.device->SetVertexShaderConstant(4, &inverse, 4);
			g_D3D.device->SetVertexShaderConstant(8, &transposeWorld, 4);
			g_D3D.device->SetVertexShaderConstant(12, color, 1);
			g_D3D.device->SetVertexShaderConstant(13, parameters, 1);
			g_D3D.device->SetVertexShaderConstant(14, cameraPosition, 1);
			IDirect3DVertexBuffer8* vertices = NULL;
			IDirect3DIndexBuffer8* indices = NULL;
			shadowMesh->GetVertexBuffer(&vertices);
			shadowMesh->GetIndexBuffer(&indices);
			if (vertices)
				g_D3D.device->SetStreamSource(0, vertices, D3DXGetFVFVertexSize(shadowMesh->GetFVF()));
			if (indices) g_D3D.device->SetIndices(indices, 0);
			if (vertices && indices)
			{
				Gfx_RenderStyleController* controller = g_RenderStyles[27];
				controller->SetMaterial(NULL, 1, 0, 0, 0);
				UINT passes = 1;
				controller->styles[controller->activeIndex]->Begin(&passes);
				for (UINT pass = 0; pass < passes; ++pass)
				{
					controller->styles[controller->activeIndex]->Pass(pass);
					for (DWORD attribute = 0; attribute < shadowAttributeCount; ++attribute)
					{
						const D3DXATTRIBUTERANGE& range = shadowAttributes[attribute];
						if (!packet->materialEnabled[range.AttribId]) continue;
						g_D3D.statistics[3] += range.FaceCount;
						++g_D3D.statistics[12];
						g_D3D.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
							range.VertexStart, range.VertexCount, 3 * range.FaceStart, range.FaceCount);
					}
				}
				controller->styles[controller->activeIndex]->End();
				// As in the original, a lone returned buffer is not released here.
				vertices->Release();
				indices->Release();
			}
		}
	}
}

void Gfx_SkinMesh::Render(UINT pass, std::list<Gfx_RenderSubmission16>* submissions,
	BYTE, Gfx_HeightFog* fog, BYTE flag48)
{
	if (maximumBlendMatrices)
	{
		for (std::list<Gfx_RenderSubmission16>::iterator entry = submissions->begin();
			entry != submissions->end(); ++entry)
		{
			Gfx_SkinMeshPkt* packet = entry->packet;
			D3DCOLOR oldAmbient;
			if (packet->ambientOverride)
			{
				g_GfxSystem->GetAmbientColor(&oldAmbient);
				g_GfxSystem->SetAmbientColor(&packet->ambientColor);
			}
			Gfx_AnimationInstance* animation = packet->animation;
			const D3DXMATRIX* matrices = animation ?
				reinterpret_cast<const D3DXMATRIX*>(animation->compositeMatrices) : NULL;
			const bool overridden = packet->overrideMaterials != NULL;
			std::list<DWORD>* selected = overridden ? packet->groups : groups;
			if (!selected[pass].empty())
			{
				if (animation && animation->flagC4) animation->RenderOBB(packet->transform);
				if (!packet->lights.empty()) Gfx_ApplyLights(&packet->lights, NULL, 0);
				else Gfx_ResetLights();
				for (std::list<DWORD>::iterator group = selected[pass].begin();
					group != selected[pass].end(); ++group)
				{
					const DWORD index = *group;
					const DWORD attribute = combinations[index].AttribId;
					if (!packet->materialEnabled[attribute]) continue;
					const DWORD blend = blendCounts[index];
					if (blend >= maximumBlendMatrices) continue;
					g_D3D.SetRenderState(D3DRS_VERTEXBLEND, blend);
					DWORD slot = 0;
					DWORD remainingMatrices = blend + 1;
					do
					{
						D3DXMATRIX transform;
						const D3DXMATRIX* value = &packet->transform;
						if (animation)
						{
							const LONG semantic = static_cast<LONG>(semanticBones[combinations[index].BoneId[slot]]);
							const DWORD bone = semantic < 212 ? animation->animationObject->semanticBoneIndices[semantic] : 0;
							D3DXMatrixMultiply(&transform, matrices + bone, &packet->transform);
							value = &transform;
						}
						g_D3D.SetTransform(D3DTS_WORLDMATRIX(slot), value);
						++slot;
					}
					while (--remainingMatrices);
					Gfx_Material* material = overridden ? packet->GetMaterial(attribute) : &materials[attribute];
					Gfx_RenderStyleController* controller = g_RenderStyles[material->style2];
					const BYTE materialFlag = packet->flag98;
					const BYTE alphaFlag = flag48 && packet->flag99;
					if (!controller) continue;
					controller->SetMaterial(material, 1, alphaFlag, materialFlag, reinterpret_cast<DWORD>(fog));
					UINT passes;
					controller->styles[controller->activeIndex]->Begin(&passes);
					for (UINT renderPass = 0; renderPass < passes; ++renderPass)
					{
						controller->styles[controller->activeIndex]->Pass(renderPass);
						blended->DrawSubset(index);
					}
					g_D3D.statistics[3] += passes * combinations[index].FaceCount;
					g_D3D.statistics[4] += passes * combinations[index].VertexCount;
					g_D3D.statistics[12] += passes;
					controller->styles[controller->activeIndex]->End();
				}
			}
			if (packet->ambientOverride) g_GfxSystem->SetAmbientColor(&oldAmbient);
		}
	}
	if (softwareBlends)
	{
		g_D3D.SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, TRUE);
		for (std::list<Gfx_RenderSubmission16>::iterator entry = submissions->begin();
			entry != submissions->end(); ++entry)
		{
			Gfx_SkinMeshPkt* packet = entry->packet;
			Gfx_AnimationInstance* animation = packet->animation;
			const D3DXMATRIX* matrices = animation ?
				reinterpret_cast<const D3DXMATRIX*>(animation->compositeMatrices) : NULL;
			const bool overridden = packet->overrideMaterials != NULL;
			if (!packet->lights.empty()) Gfx_ApplyLights(&packet->lights, &packet->extraLight, 0);
			else Gfx_ResetLights();
			std::list<DWORD>* selected = overridden ? packet->groups : groups;
			for (std::list<DWORD>::iterator group = selected[pass].begin();
				group != selected[pass].end(); ++group)
			{
				const DWORD index = *group;
				const DWORD attribute = combinations[index].AttribId;
				if (!packet->materialEnabled[attribute]) continue;
				const DWORD blend = blendCounts[index];
				if (blend < maximumBlendMatrices) continue;
				g_D3D.SetRenderState(D3DRS_VERTEXBLEND, blend);
				DWORD slot = 0;
				DWORD remainingMatrices = blend + 1;
				do
				{
					D3DXMATRIX transform;
					const D3DXMATRIX* value = &packet->transform;
					if (animation)
					{
						const LONG semantic = static_cast<LONG>(semanticBones[combinations[index].BoneId[slot]]);
						const DWORD bone = semantic < 212 ? animation->animationObject->semanticBoneIndices[semantic] : 0;
						D3DXMatrixMultiply(&transform, matrices + bone, &packet->transform);
						value = &transform;
					}
					g_D3D.SetTransform(D3DTS_WORLDMATRIX(slot), value);
					++slot;
				}
				while (--remainingMatrices);
				Gfx_Material* material = overridden ? packet->GetMaterial(attribute) : &materials[attribute];
				Gfx_RenderStyleController* controller = g_RenderStyles[material->style2];
				const BYTE materialFlag = packet->flag98;
				const BYTE alphaFlag = flag48 && packet->flag99;
				if (!controller) continue;
				controller->SetMaterial(material, 1, alphaFlag, materialFlag, reinterpret_cast<DWORD>(fog));
				UINT passes;
				controller->styles[controller->activeIndex]->Begin(&passes);
				for (UINT renderPass = 0; renderPass < passes; ++renderPass)
				{
					controller->styles[controller->activeIndex]->Pass(renderPass);
					blended->DrawSubset(index);
				}
				g_D3D.statistics[3] += passes * combinations[index].FaceCount;
				g_D3D.statistics[4] += passes * combinations[index].VertexCount;
				g_D3D.statistics[12] += passes;
				controller->styles[controller->activeIndex]->End();
			}
		}
		g_D3D.SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, FALSE);
	}
}
