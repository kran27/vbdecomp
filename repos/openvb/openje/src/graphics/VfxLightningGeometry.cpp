#include "VfxEffects.hpp"
#include "VfxLightningMesh.hpp"
#include "Camera.hpp"
#include "ParticleSystem.hpp"
#include <math.h>
#include <new>
#include <stdlib.h>

namespace
{
	DWORD PackColor(LONG red, LONG green, LONG blue, LONG alpha)
	{
		return DWORD(blue) | (DWORD(green) << 8) | (DWORD(red) << 16) | (DWORD(alpha) << 24);
	}

	double ColorPhase(BYTE overLifetime, float lifetime, float normalizedTime,
		float absoluteTime, LONG waveform, float frequency, float phase)
	{
		double time = overLifetime && lifetime > 0 ? normalizedTime : absoluteTime;
		float value = float(time * frequency + phase);
		return Gfx_ParticleWaveform(waveform, 0, 1, value); //512F70.
	}

	void WriteVertex(JE::VFX_LightningVertex& output, const D3DXVECTOR3& position, DWORD color)
	{
		output.x = position.x;
		output.y = position.y;
		output.z = position.z;
		output.color = color;
		//4EA6F0 never writes the two texture coordinates in this24-byte value.
	}

	D3DXVECTOR3 EdgeNormal(const D3DXVECTOR3& direction, const D3DXVECTOR3& cameraForward)
	{
		D3DXVECTOR3 result;
		D3DXVec3Cross(&result, &direction, &cameraForward);
		D3DXVec3Normalize(&result, &result);
		return result;
	}
}

namespace JE
{
	void VFX_Lightning::BuildGeometry(Gfx_Camera* camera) //4EA6F0.
	{
		std::list<VFX_LightningMesh*>& bucket = meshes[value268];
		for (std::list<VFX_LightningMesh*>::iterator i = bucket.begin(); i != bucket.end(); ++i)
		{
			VFX_LightningMesh* owned = *i;
			if (owned->vertices) delete[] owned->vertices;
			if (owned->indices) delete[] owned->indices;
			if (owned->instances)
			{
				delete[] owned->instances;
				owned->instances = NULL;
			}
			if (owned->packet)
			{
				delete owned->packet;
				owned->packet = NULL;
			}
			delete owned;
		}
		bucket.clear();
		if (segments.empty()) return;

		float normalizedTime;
		if (currentLifetime > 0)
		{
			normalizedTime = float((double(lastTime) - cycleStart) / currentLifetime);
			if (!(normalizedTime >= 0)) normalizedTime = 0;
			else if (normalizedTime > 1) normalizedTime = 1;
		}
		else normalizedTime = 0;

		LONG outerRed, outerGreen, outerBlue, outerAlpha;
		if (data.adjustOuterColor)
		{
			double phase = ColorPhase(data.adjustOuterColorOverLifetime, currentLifetime,
				normalizedTime, lastTime, data.outerColorWaveform, data.outerColorFreq, data.outerColorPhase);
			outerRed = LONG((LONG(data.finalOuterColor[0]) - data.outerColor[0]) * phase + data.outerColor[0]);
			outerGreen = LONG((LONG(data.finalOuterColor[1]) - data.outerColor[1]) * phase + data.outerColor[1]);
			outerBlue = LONG((LONG(data.finalOuterColor[2]) - data.outerColor[2]) * phase + data.outerColor[2]);
		}
		else
		{
			outerRed = data.outerColor[0]; outerGreen = data.outerColor[1]; outerBlue = data.outerColor[2];
		}
		if (data.adjustOuterAlpha)
		{
			double phase = ColorPhase(data.adjustOuterAlphaOverLifetime, currentLifetime,
				normalizedTime, lastTime, data.outerAlphaWaveform, data.outerAlphaFreq, data.outerAlphaPhase);
			outerAlpha = LONG((LONG(data.finalOuterAlpha) - data.outerAlpha) * phase + data.outerAlpha);
		}
		else outerAlpha = data.outerAlpha;
		DWORD outerColor = PackColor(outerRed, outerGreen, outerBlue, outerAlpha);

		DWORD innerColor;
		if (data.innerGlow)
		{
			LONG red, green, blue, alpha;
			if (data.adjustInnerColor)
			{
				double phase = ColorPhase(data.adjustInnerColorOverLifetime, currentLifetime,
					normalizedTime, lastTime, data.innerColorWaveform, data.innerColorFreq, data.innerColorPhase);
				red = LONG((LONG(data.finalInnerColor[0]) - data.innerColor[0]) * phase + data.innerColor[0]);
				green = LONG((LONG(data.finalInnerColor[1]) - data.innerColor[1]) * phase + data.innerColor[1]);
				blue = LONG((LONG(data.finalInnerColor[2]) - data.innerColor[2]) * phase + data.innerColor[2]);
			}
			else
			{
				red = data.innerColor[0]; green = data.innerColor[1]; blue = data.innerColor[2];
			}
			if (data.adjustInnerAlpha)
			{
				double phase = ColorPhase(data.adjustInnerAlphaOverLifetime, currentLifetime,
					normalizedTime, lastTime, data.innerAlphaWaveform, data.innerAlphaFreq, data.innerAlphaPhase);
				alpha = LONG((LONG(data.finalInnerAlpha) - data.innerAlpha) * phase + data.innerAlpha);
			}
			else alpha = data.innerAlpha;
			innerColor = PackColor(red, green, blue, alpha);
		}

		for (std::list<VFX_LightningSegment>::const_iterator segment = segments.begin();
			segment != segments.end(); ++segment)
		{
			LONG edges = LONG(segment->points.size()) - 1;
			if (edges <= 0) continue;
			VFX_LightningMesh* result = new VFX_LightningMesh;
			result->instanceCount = 1;
			result->instances = new Gfx_MeshInstance[result->instanceCount];
			for (DWORD index = 0; index < result->instanceCount; ++index)
			{
				result->instances[index].flag1D = 1;
				result->instances[index].flag1E = 1;
			}
			DWORD columns = data.innerGlow ? 3 : 2;
			result->vertexCount = columns * DWORD(edges + 1);
			result->vertices = new VFX_LightningVertex[result->vertexCount];
			D3DXMATRIX cameraTransform = camera->transform;
			D3DXVECTOR3 cameraForward(cameraTransform._31, cameraTransform._32, cameraTransform._33);

			DWORD vertex = 0;
			std::list<VFX_LightningPoint>::const_iterator previous = segment->points.end();
			std::list<VFX_LightningPoint>::const_iterator current = segment->points.begin();
			std::list<VFX_LightningPoint>::const_iterator next = current;
			++next;
			while (current != segment->points.end())
			{
				D3DXVECTOR3 normal;
				float width = current->width * scale;
				if (previous == segment->points.end())
				{
					normal = EdgeNormal(next->position - current->position, cameraForward);
					width *= data.tipWidthScale;
				}
				else if (next == segment->points.end())
				{
					normal = EdgeNormal(current->position - previous->position, cameraForward);
					width *= data.tipWidthScale;
				}
				else
				{
					D3DXVECTOR3 before = EdgeNormal(current->position - previous->position, cameraForward);
					D3DXVECTOR3 after = EdgeNormal(next->position - current->position, cameraForward);
					//The original weights these already normalized edges by each
					//other's length, then normalizes their sum for the interior join.
					float beforeLength = D3DXVec3Length(&before);
					float afterLength = D3DXVec3Length(&after);
					normal = before * afterLength + after * beforeLength;
					D3DXVec3Normalize(&normal, &normal);
				}
				D3DXVECTOR3 offset = normal * width * 0.5f;
				WriteVertex(result->vertices[vertex++], current->position - offset, outerColor);
				if (data.innerGlow) WriteVertex(result->vertices[vertex++], current->position, innerColor);
				WriteVertex(result->vertices[vertex++], current->position + offset, outerColor);
				previous = current;
				current = next;
				if (next != segment->points.end()) ++next;
			}

			if (data.innerGlow)
			{
				result->indexCount = 4 * DWORD(edges) + 3;
				result->indices = new WORD[result->indexCount];
				DWORD index = 0;
				for (LONG point = 0; point <= edges; ++point)
				{
					result->indices[index++] = WORD(3 * point);
					result->indices[index++] = WORD(3 * point + 1);
				}
				for (LONG point = edges; point >= 0; --point)
				{
					result->indices[index++] = WORD(3 * point + 2);
					if (point > 0) result->indices[index++] = WORD(3 * point - 2);
				}
			}

			Gfx_MeshData& mesh = result->mesh;
			mesh.value00 = 0;
			mesh.value04 = DWORD(-1);
			mesh.vertices = result->vertices;
			mesh.SetVertexDesc(0x142, D3DXGetFVFVertexSize(0x142), result->vertexCount);
			if (data.innerGlow)
			{
				mesh.value10 = reinterpret_cast<DWORD>(result->indices);
				mesh.SetIndexDesc(2, result->indexCount);
			}
			result->packet = new Gfx_PacketData;
			mesh.materialSet.count = 0;
			mesh.materialSet.ResetGroups();
			if (mesh.materialSet.materials) { delete[] mesh.materialSet.materials; mesh.materialSet.materials = NULL; }
			Gfx_Primitive* primitive = mesh.SetPrimitiveCount((data.innerGlow != 0) + 1);
			Gfx_Material material;
			material.mode28 = data.renderPhase;
			material.flagBC = 0;
			material.flagB9 = 0;
			material.style1 = material.style2 = 15 + (data.modulate2x != 0);
			mesh.materialSet.materials[0] = material;
			primitive[0].type = D3DPT_TRIANGLESTRIP;
			primitive[0].value00 = 0;
			primitive[0].value24 = data.innerGlow ? 1 : 0;
			primitive[0].value28 = 0;
			primitive[0].value30 = 2 * edges;
			if (data.innerGlow)
			{
				primitive[0].value2C = 0;
				primitive[1].value00 = 1;
				primitive[1].value28 = 0;
				primitive[1].value24 = 1;
				primitive[1].value2C = 2 * edges + 1;
				primitive[1].type = D3DPT_TRIANGLESTRIP;
				primitive[1].value30 = 2 * edges;
				mesh.materialSet.materials[1] = material;
			}
			result->packet->transform = &result->transform;
			result->packet->transformEnabled = 1;
			mesh.materialSet.RebuildGroups();
			mesh.value04 = DWORD(-1);
			result->instances[0].SetMesh(&mesh);
			result->instances[0].value0C = reinterpret_cast<DWORD>(result->packet);
			bucket.push_back(result);
		}
	}
}
