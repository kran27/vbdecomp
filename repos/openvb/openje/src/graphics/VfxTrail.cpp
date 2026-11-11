#include "VfxTrail.hpp"
#include "GfxColor4.hpp"
#include <string.h>
#include "VfxLightningMesh.hpp"
#include "MeshData.hpp"
#include "PacketData.hpp"
#include "Viewport.hpp"
#include "TextureManager.hpp"
#include "SystemInterface.hpp"
#include "TextureResource.hpp"
#include "resources/ResourceMemoryStream.hpp"
#include <float.h>
#include <new>
#include <stdlib.h>
#include <stdexcept>

namespace JE
{
	//4F2DD0 inlines this real0xD4 owner. Unlike the lightning owner, Trail
	//stores both the used vertex count and a separate grow-by20 capacity.
	struct VFX_TrailMesh
	{
		VFX_TrailMesh() {}
		Gfx_MeshData mesh; //+00, embedded actual MaterialSet+28.
		Gfx_MeshInstance* instances; //+6C.
		Gfx_PacketData* packet; //+70.
		D3DXMATRIX transform; //+74.
		DWORD instanceCount; //+B4.
		VFX_LightningVertex* vertices; //+B8.
		LONG vertexCount, vertexCapacity; //+BC/+C0.
		WORD* indices; //+C4, unused by4F1D80's unindexed strip.
		DWORD indexCount, indexCapacity; //+C8/+CC.
		BYTE ready; //+D0.
	private:
		VFX_TrailMesh(const VFX_TrailMesh&);
		VFX_TrailMesh& operator=(const VFX_TrailMesh&);
	};

	VFX_TrailData::VFX_TrailData()
	{
	}
	void VFX_TrailData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input = parameters.Find("Color1")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color1, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("Alpha1")) != NULL) alpha1 = BYTE(input->GetInteger());
		if ((input = parameters.Find("WidthScale1")) != NULL) widthScale1 = float(input->GetFloat());
		if ((input = parameters.Find("Color2")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color2, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("Alpha2")) != NULL) alpha2 = BYTE(input->GetInteger());
		if ((input = parameters.Find("WidthScale2")) != NULL) widthScale2 = float(input->GetFloat());
		if ((input = parameters.Find("EndPointTarget")) != NULL)
		{
			const VFX_Target target = input->GetTarget();
			endPointTarget = target;
		}
		if ((input = parameters.Find("EndPointSocket")) != NULL) endPointSocket = input->GetInteger();
		if ((input = parameters.Find("EndPointSnapBone")) != NULL) endPointSnapBone = input->GetString();
		if ((input = parameters.Find("EndPointTargetBodyPart")) != NULL) endPointTargetBodyPart = input->GetBool();
		if ((input = parameters.Find("EndPointOffset")) != NULL) { const JE::VFX_Vector converted = input->GetVector3(); memcpy(&endPointOffset, &converted, sizeof(converted)); }
		if ((input = parameters.Find("SegmentLifetime")) != NULL) segmentLifetime = float(input->GetFloat());
		if ((input = parameters.Find("Alignment")) != NULL) alignment = input->GetEnum();
		if ((input = parameters.Find("Texture")) != NULL)
		{
			texture.type = input->GetResourceType(); texture.name = input->GetResourceName();
		}
		if ((input = parameters.Find("TextureUVs")) != NULL) { const JE::VFX_TextureCoordinates converted = input->GetTextureUVs(); memcpy(textureUVs, &converted, sizeof(converted)); }
		if ((input = parameters.Find("RenderPhase")) != NULL) renderPhase = input->GetEnum();
		if ((input = parameters.Find("Modulate2x")) != NULL) modulate2x = input->GetBool();
	}

	VFX_Trail::VFX_Trail()
	{
		dirty = 1;
		D3DXMatrixIdentity(&transform);
		texture = 0;
		ownsTexture = 1;
		lastTime = 0;
		meshes.resize(2, NULL);
		for (LONG i = static_cast<LONG>(meshes.size()); i > 0; --i)
		{
			VFX_TrailMesh* mesh = new VFX_TrailMesh;
			meshes[i - 1] = mesh;
			mesh->instanceCount = 1;
			mesh->instances = new Gfx_MeshInstance[mesh->instanceCount];
			for (LONG instance = 0; instance < static_cast<LONG>(mesh->instanceCount); ++instance)
			{
				mesh->instances[instance].flag1D = 1;
				mesh->instances[instance].flag1E = 1;
			}
			D3DXMatrixIdentity(&mesh->transform);
			mesh->vertices = NULL;
			mesh->vertexCount = 0;
			mesh->vertexCapacity = 0;
			mesh->indices = NULL;
			mesh->indexCount = 0;
			mesh->indexCapacity = 0;
			mesh->packet = new Gfx_PacketData;
			mesh->ready = 0;
		}
		currentBucket = 0;
	}
	VFX_Trail::~VFX_Trail()
	{
		for (std::vector<VFX_TrailMesh*>::iterator entry = meshes.begin();
			entry != meshes.end(); ++entry)
		{
			VFX_TrailMesh* mesh = *entry;
			if (mesh->vertices) ::operator delete[](mesh->vertices);
			if (mesh->indices) ::operator delete[](mesh->indices);
			if (mesh->instances)
			{
				delete[] mesh->instances;
				mesh->instances = NULL;
			}
			if (mesh->packet)
			{
				delete mesh->packet;
				mesh->packet = NULL;
			}
			delete mesh;
		}
		if (texture && ownsTexture) g_TextureManager->Release(texture);
		//Automatic vector/list/data/base destruction supplies the native lifetime order.
	}
	VFX_Trail& VFX_Trail::CopyFrom(const VFX_Trail& other) //0x4F2CB0.
	{
		if (texture && ownsTexture) g_TextureManager->Release(texture);
		VFX_EffectInstance::CopyFrom(other);
		data = other.data;
		transform = other.transform;
		texture = other.texture;
		ownsTexture = 0;
		return *this;
	}
	void VFX_Trail::SetData(const VFX_BaseData* input)
	{
		VFX_EffectInstance::SetData(input);
		data = *static_cast<const VFX_TrailData*>(input);
		if (texture && ownsTexture)
		{
			g_TextureManager->Release(texture);
		}
		texture = Gfx_RequestTextureResource(data.texture.name.c_str(), UINT(-1), 3, 5, 0);
		ownsTexture = 1;
		dirty = 1;
	}
	void VFX_Trail::Reset() { active = 0; }
	BYTE VFX_Trail::GetFlag() const { return 0; }
	BYTE VFX_Trail::SkipBounds() const { return 0; }
	void VFX_Trail::SetTransform(const D3DXMATRIX& input) { transform = input; dirty = 1; }
	BYTE VFX_Trail::Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const
	{
		VFX_UpdateContext context(source, target, group, sourceTransform, targetTransform);
		if (!VFX_EffectInstance::Prepare(source, target, group, sourceTransform, targetTransform)) return 0;
		return PrepareTarget(source, target, group, data.endPointTarget, data.endPointSocket,
			data.endPointSnapBone, sourceTransform, targetTransform);
	}
	void VFX_Trail::Update(float time) { VFX_UpdateContext context; Update(time, context); }
	void VFX_Trail::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_UpdateContext context(source, target, group, sourceTransform, targetTransform);
		VFX_EffectInstance::Update(time, source, target, group, sourceTransform, targetTransform);
		double elapsed = 0;
		if (lastTime != time)
		{
			dirty = 1;
			elapsed = double(time) - lastTime;
			lastTime = time;
		}
		D3DXVECTOR3 position(transform._41, transform._42, transform._43);
		if (elapsed > 0 && (samples.empty() || samples.front().start.x != position.x
			|| samples.front().start.y != position.y || samples.front().start.z != position.z))
		{
			D3DXMATRIX endTransform;
			const LONG socket = data.endPointSocket;
			const float offset[3] = {data.endPointOffset.xy.values[0], data.endPointOffset.xy.values[1], data.endPointOffset.z};
			std::string snapBone(data.endPointSnapBone);
			const BYTE rotate = data.rotateWithTarget;
			//Explicit matrices use the BASE target; model lookup uses EndPointTarget.
			if (data.target.selector == 1 && context.sourceTransform)
				GetVfxTransform(context.sourceTransform, endTransform, offset, rotate, scale);
			else if (data.target.selector == 2 && context.targetTransform)
				GetVfxTransform(context.targetTransform, endTransform, offset, rotate, scale);
			else
			{
				Gfx_ModelInstance* model = ResolveVfxModel(data.endPointTarget.selector, context);
				GetVfxModelTransform(model, socket, snapBone, endTransform, offset, rotate, scale);
			}
			VFX_TrailSample sample;
			sample.start = position;
			sample.end = D3DXVECTOR3(endTransform._41, endTransform._42, endTransform._43);
			sample.birthTime = time;
			sample.age = 0;
			sample.fresh = 1;
			samples.push_front(sample);
			dirty = 1;
		}
		std::list<VFX_TrailSample>::iterator i = samples.begin();
		while (i != samples.end())
		{
			i->age = float(double(time) - i->birthTime);
			if (i->fresh) { i->fresh = 0; ++i; }
			else if (i->age < 0 || i->age > data.segmentLifetime) i = samples.erase(i);
			else ++i;
		}
	}
	void VFX_Trail::GetSampleEndpoints(const VFX_TrailSample& sample, D3DXVECTOR3& start,
		D3DXVECTOR3& end) const
	{
		float age = sample.age / data.segmentLifetime;
		switch (data.alignment)
		{
		case 0:
			start = sample.start;
			end = (sample.end - start) * ((data.widthScale2 - data.widthScale1) * age + data.widthScale1) + start;
			break;
		case 1:
			{
				end = sample.end;
				start = sample.start;
				D3DXVECTOR3 delta = end - start;
				D3DXVECTOR3 scaled = delta * ((data.widthScale2 - data.widthScale1) * age + data.widthScale1);
				D3DXVECTOR3 half = (delta - scaled) * 0.5f;
				start = end - scaled - half;
				end = end - half;
			}
			break;
		case 2:
			end = sample.end;
			start = end - (end - sample.start) * ((data.widthScale2 - data.widthScale1) * age + data.widthScale1);
			break;
		}
	}
	void VFX_Trail::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const
	{
		low.x = FLT_MAX; low.y = FLT_MAX; low.z = FLT_MAX;
		high.x = -FLT_MAX; high.y = -FLT_MAX; high.z = -FLT_MAX;
		for (std::list<VFX_TrailSample>::const_iterator i = samples.begin(); i != samples.end(); ++i)
		{
			D3DXVECTOR3 start, end;
			GetSampleEndpoints(*i, start, end);
			low.x = (start.x < end.x ? start.x : end.x) < low.x ? (start.x < end.x ? start.x : end.x) : low.x;
			low.y = (start.y < end.y ? start.y : end.y) < low.y ? (start.y < end.y ? start.y : end.y) : low.y;
			low.z = (start.z < end.z ? start.z : end.z) < low.z ? (start.z < end.z ? start.z : end.z) : low.z;
			high.x = (start.x > end.x ? start.x : end.x) > high.x ? (start.x > end.x ? start.x : end.x) : high.x;
			high.y = (start.y > end.y ? start.y : end.y) > high.y ? (start.y > end.y ? start.y : end.y) : high.y;
			high.z = (start.z > end.z ? start.z : end.z) > high.z ? (start.z > end.z ? start.z : end.z) : high.z;
		}
	}
	void VFX_Trail::Draw() { Draw(NULL); }
	void VFX_Trail::Draw(Gfx_Viewport* viewport)
	{
		if (viewport != lastViewport)
		{
			LONG bucket = LONG(currentBucket + 1);
			dirty = 1;
			currentBucket = bucket;
			if (bucket >= LONG(meshes.size())) currentBucket = 0;
		}
		if (dirty)
		{
			BuildGeometry(viewport ? viewport->activeCamera : g_GfxSystem->Camera());
			dirty = 0;
		}
		lastViewport = viewport;
		VFX_TrailMesh* mesh = meshes[currentBucket];
		if (mesh && mesh->ready)
		{
			g_GfxSystem->QueueMeshInstances(mesh->instances, viewport, mesh->instanceCount);
		}
	}
	void VFX_Trail::BuildGeometry(Gfx_Camera* camera)
	{
		(void)camera; //Original4F1D80 receives this argument but does not read it.
		VFX_TrailMesh& result = *meshes[currentBucket];
		result.ready = 0;
		if (samples.size() <= 1) return;
		LONG segmentCount = LONG(samples.size() - 1);
		LONG vertices = (segmentCount > 1 ? 2 * segmentCount - 2 : 0) + 4;
		if (vertices > result.vertexCapacity)
		{
			if (result.vertices) { ::operator delete[](result.vertices); result.vertices = NULL; }
			LONG capacity = result.vertexCapacity + 20;
			result.vertexCapacity = vertices > capacity ? vertices : capacity;
			if (result.vertexCapacity > 0)
				result.vertices = static_cast<VFX_LightningVertex*>(::operator new[](24 * result.vertexCapacity));
		}
		result.vertexCount = vertices;
		LONG sampleIndex = 0, output = 0;
		D3DXVECTOR3 previousStart, previousEnd;
		float previousAge;
		for (std::list<VFX_TrailSample>::const_iterator i = samples.begin(); i != samples.end(); ++i, ++sampleIndex)
		{
			D3DXVECTOR3 start, end;
			GetSampleEndpoints(*i, start, end);
			float fraction = float(double(sampleIndex) / float(segmentCount));
			float age = float(double(i->age) / data.segmentLifetime);
			if (sampleIndex && i->age <= data.segmentLifetime)
			{
				const Gfx_Color4f first = {{float(data.color1[0]), float(data.color1[1]), float(data.color1[2]),
					data.alpha1 * 0.0039215689f}};
				const Gfx_Color4f last = {{float(data.color2[0]), float(data.color2[1]), float(data.color2[2]),
					data.alpha2 * 0.0039215689f}};
				Gfx_Color4f previousColor, color;
				Gfx_Color4f previousDifference, previousScaled, difference, scaled;
				Gfx_AddColor4f(&previousColor, first, *Gfx_ScaleColor4f(&previousScaled,
					*Gfx_SubtractColor4f(&previousDifference, last, first), previousAge));
				Gfx_AddColor4f(&color, first, *Gfx_ScaleColor4f(&scaled,
					*Gfx_SubtractColor4f(&difference, last, first), age));
				if (sampleIndex == 1)
				{
					result.vertices[output].x = previousStart.x;
					result.vertices[output].y = previousStart.y;
					result.vertices[output].z = previousStart.z;
					result.vertices[output].color = (((DWORD(LONG(previousColor.values[3] * 255.0f)) << 8
						| DWORD(LONG(previousColor.values[0]))) << 8 | DWORD(LONG(previousColor.values[1]))) << 8
						| DWORD(LONG(previousColor.values[2])));
					if (texture)
					{
						result.vertices[output].u = data.textureUVs[0];
						result.vertices[output].v = data.textureUVs[3];
					}
					++output;
					result.vertices[output].x = previousEnd.x;
					result.vertices[output].y = previousEnd.y;
					result.vertices[output].z = previousEnd.z;
					result.vertices[output].color = (((DWORD(LONG(previousColor.values[3] * 255.0f)) << 8
						| DWORD(LONG(previousColor.values[0]))) << 8 | DWORD(LONG(previousColor.values[1]))) << 8
						| DWORD(LONG(previousColor.values[2])));
					if (texture)
					{
						result.vertices[output].u = data.textureUVs[0];
						result.vertices[output].v = data.textureUVs[1];
					}
					++output;
				}
				result.vertices[output].x = start.x;
				result.vertices[output].y = start.y;
				result.vertices[output].z = start.z;
				result.vertices[output].color = (((DWORD(LONG(color.values[3] * 255.0f)) << 8
					| DWORD(LONG(color.values[0]))) << 8 | DWORD(LONG(color.values[1]))) << 8
					| DWORD(LONG(color.values[2])));
				if (texture)
				{
					result.vertices[output].u = float((double(data.textureUVs[2]) - data.textureUVs[0]) * fraction + data.textureUVs[0]);
					result.vertices[output].v = data.textureUVs[3];
				}
				++output;
				result.vertices[output].x = end.x;
				result.vertices[output].y = end.y;
				result.vertices[output].z = end.z;
				result.vertices[output].color = (((DWORD(LONG(color.values[3] * 255.0f)) << 8
					| DWORD(LONG(color.values[0]))) << 8 | DWORD(LONG(color.values[1]))) << 8
					| DWORD(LONG(color.values[2])));
				if (texture)
				{
					result.vertices[output].u = float((double(data.textureUVs[2]) - data.textureUVs[0]) * fraction + data.textureUVs[0]);
					result.vertices[output].v = data.textureUVs[1];
				}
				++output;
			}
			previousStart = start; previousEnd = end; previousAge = age;
		}
		Gfx_MeshData& mesh = result.mesh;
		mesh.vertices = result.vertices;
		mesh.value00 = 0;
		mesh.value04 = DWORD(-1);
		mesh.SetVertexDesc(0x142, D3DXGetFVFVertexSize(0x142), result.vertexCount);
		mesh.materialSet.count = 0;
		mesh.materialSet.ResetGroups();
		if (mesh.materialSet.materials) { delete[] mesh.materialSet.materials; mesh.materialSet.materials = NULL; }
		Gfx_Primitive* primitive = mesh.SetPrimitiveCount(1);
		primitive->value00 = 0;
		primitive->type = D3DPT_TRIANGLESTRIP;
		primitive->value24 = 0; //The strip is unindexed; no index allocation or descriptor.
		Gfx_Material material;
		material.mode28 = data.renderPhase;
		material.flagBC = 0;
		material.flagB9 = 0;
		material.style1 = material.style2 = 19 + (data.modulate2x != 0);
		material.textureHandles[0] = texture;
		mesh.materialSet.materials[0] = material;
		primitive->value28 = 0;
		primitive->value30 = 2 * segmentCount;
		result.packet->transform = &result.transform;
		result.packet->transformEnabled = 1;
		mesh.materialSet.RebuildGroups();
		mesh.value04 = DWORD(-1);
		result.instances[0].SetMesh(&mesh);
		result.instances[0].value0C = reinterpret_cast<DWORD>(result.packet);
		result.ready = 1;
	}
}
