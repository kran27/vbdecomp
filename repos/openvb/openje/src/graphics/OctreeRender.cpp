#include "OctreeRender.hpp"
#include "SystemInterface.hpp"
#include "D3DState.hpp"
#include "../resources/ResourceMemoryStream.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <dxerr8.h>
#include <math.h>
#include <new>
#include <string.h>

namespace
{
	const char* const materialNames[33] =
	{
		"NO_TEXTURE", "BASE", "BASE_2X", "BASE_EM", "BASE_2X_EM", "BASE_LM",
		"BASE_LM_2X", "LM_ONLY", "BASE_IMAGE_ALPHA", "BASE_IMAGE_ALPHA_2X",
		"BASE_IMAGE_ALPHA_EM", "BASE_IMAGE_ALPHA_2X_EM", "BASE_LM_IMAGE_ALPHA",
		"BASE_LM_IMAGE_ALPHA_2X", "NO_TEXTURE_VERTEX_ALPHA", "BASE_VERTEX_ALPHA",
		"BASE_VERTEX_ALPHA_2X", "BASE_VERTEX_ALPHA_EM", "BASE_VERTEX_ALPHA_2X_EM",
		"BASE_IMAGE_VERTEX_ALPHA", "BASE_IMAGE_VERTEX_ALPHA_2X", "BASE_IMAGE_VERTEX_ALPHA_EM",
		"BASE_IMAGE_VERTEX_ALPHA_2X_EM", "BASE_LM_VERTEX_ALPHA", "BASE_LM_VERTEX_ALPHA_2X",
		"BASE_LM_IMAGE_VERTEX_ALPHA", "BASE_LM_IMAGE_VERTEX_ALPHA_2X", "EXPAND",
		"SK_BASE_LM_2X", "SK_BASE_LM_VERTEX_ALPHA_2X", "SK_BASE_LM_IMAGE_ALPHA_2X",
		"SK_BASE_LM_IMAGE_VERTEX_ALPHA_2X", "INVISIBLE"
	};
	const char* const blendNames[6] =
	{
		"OPAQUE", "ALPHABLEND", "MODCOLOR", "SUBTRACT", "REVSUBTRACT", "ADD"
	};

	DWORD ReadWord(const BYTE* data, DWORD offset)
	{
		DWORD result;
		memcpy(&result, data + offset, 4);
		return result;
	}
	float ReadFloat(const BYTE* data, DWORD offset)
	{
		float result;
		memcpy(&result, data + offset, 4);
		return result;
	}

	struct MaterialStyle
	{
		DWORD style, blend;
	};

	// Shallow records, as copied into the original temporary list. Buffers are
	// explicitly consumed below; no invented rollback is added on child failure.
	struct Batch
	{
		DWORD node, material, lightmap, texture, count;
		WORD* indices;
		DWORD floorTriangles;
		WORD* floorIndices;
	};
	struct FloorAccumulator
	{
		FloorAccumulator() : node(0), triangles(0), count(0), indices(0) {}
		~FloorAccumulator() { if (indices) { delete[] indices; indices = NULL; } }
		DWORD node, triangles, count;
		WORD* indices;
	};
	struct BufferBatches
	{
		BufferBatches() : count(0), nodes(0), floorSegments(0) {}
		DWORD count;
		std::list<Batch> batches;
		DWORD nodes, floorSegments;
		std::map<DWORD, FloorAccumulator> floors;
	};
}

void Octree2Gfx::ConstructRenderData()
{
	// The pre-existing slot17 name is provisional: this consumer uses its return
	// as the index element byte width (original global70839C), not a mode ordinal.
	DWORD indexStride = g_GfxSystem->ModeIndex();
	OctreeLight* sourceLights = NULL;
	octree->GetLights(&lightCount, &sourceLights);
	DWORD sourceCount, sourceAttributes;
	OctreeVertexStreams* sourceBuffers = NULL;
	octree->GetVertexBuffers(&sourceCount, &sourceAttributes, &sourceBuffers);
	char** names = NULL;
	octree->GetTextureNames(&textureCount, &names);
	OctreeMaterial* sourceMaterials = NULL;
	octree->GetMaterials(&materialCount, &sourceMaterials);
	opacityMaterials.clear();
	colorMaterials.clear();
	enabledMaterials.clear();
	MaterialStyle* styles = new MaterialStyle[materialCount];
	for (DWORD index = 0; index < materialCount; ++index)
	{
		const char* name = reinterpret_cast<const char*>(sourceMaterials[index].bytes + 4);
		DWORD style = 0;
		while (style < 33 && strcmp(materialNames[style], name)) ++style;
		if (style == 33)
		{
			JE::LogDebugString("Octree2Gfx::ConstructRenderData() - Unknown material = %s\n", name);
			style = 0;
		}
		styles[index].style = style;
		const char* blend = reinterpret_cast<const char*>(sourceMaterials[index].bytes + 88);
		DWORD mode = 0;
		while (mode < 6 && strcmp(blendNames[mode], blend)) ++mode;
		if (mode == 6)
		{
			JE::LogDebugString("Octree2Gfx::ConstructRenderData() - %s has unknown blendstate = %s\n",
				name, blend);
			mode = 0;
		}
		styles[index].blend = mode;
	}
	std::map<DWORD, BufferBatches> buffers;
	boundsVertexCount = 0;
	nodeCount = 0;
	OCTREE_Scene* scene = NULL;
	sceneMode = 1;
	do
	{
		if (octree->CreateScene(&scene, sceneMode) != 2) break;
	}
	while (sceneMode--);
	if (!scene) JE::FatalError("Octree2Gfx::ConstructRenderData - Failed to create scene\n");
	DWORD previousBuffer = 0xFFFFFFFF;
	DWORD previousNode = 0xFFFFFFFF;
	DWORD nodeOrdinal = 0;
	DWORD nodeIndices = 0;
	OctreeBatchInfo info;
	do
	{
		while (scene->NextBatchInfo(&info))
		{
			bool changed = previousBuffer == 0xFFFFFFFF || previousBuffer != info.vertexBuffer;
			if (changed)
			{
				previousBuffer = info.vertexBuffer;
				nodeOrdinal = 0;
			}
			if (changed || previousNode != nodeOrdinal)
			{
				previousNode = nodeOrdinal;
				nodeIndices = 0;
			}
			Batch batch;
			batch.node = nodeOrdinal;
			batch.material = info.material;
			batch.lightmap = info.lightmap;
			batch.texture = info.texture;
			batch.count = info.indexCount;
			batch.indices = new WORD[batch.count];
			batch.floorTriangles = 0;
			batch.floorIndices = NULL;
			scene->CopyIndices(batch.indices);
			DWORD* flags = scene->CurrentTriangleFlags();
			if (flags)
			{
				flags += nodeIndices / 3;
				for (DWORD triangle = 0; triangle < batch.count / 3; ++triangle)
				{
					if (flags[triangle] & 1) ++batch.floorTriangles;
				}
				if (batch.floorTriangles)
				{
					batch.floorIndices = new WORD[3 * batch.floorTriangles];
					WORD* destination = batch.floorIndices;
					for (DWORD triangle = 0; triangle < batch.count / 3; ++triangle)
					{
						if (flags[triangle] & 1)
						{
							memcpy(destination, batch.indices + 3 * triangle, 6);
							destination += 3;
						}
					}
				}
			}
			nodeIndices += batch.count;
			BufferBatches& buffer = buffers[info.vertexBuffer];
			buffer.batches.push_back(batch);
			buffer.count += batch.count;
			buffer.floors[batch.node].triangles += batch.floorTriangles;
		}
		++nodeOrdinal;
		++nodeCount;
		++buffers[info.vertexBuffer].nodes;
	}
	while (scene->AdvanceNode());
	scene->Release();
	meshCount = static_cast<DWORD>(buffers.size());
	for (std::map<DWORD, BufferBatches>::iterator buffer = buffers.begin();
		buffer != buffers.end(); ++buffer)
	{
		for (std::list<Batch>::iterator batch = buffer->second.batches.begin();
			batch != buffer->second.batches.end(); ++batch)
		{
			if (batch->floorTriangles)
			{
				FloorAccumulator& floor = buffer->second.floors[batch->node];
				floor.node = batch->node;
				if (!floor.indices) floor.indices = new WORD[3 * floor.triangles];
				memcpy(floor.indices + floor.count, batch->floorIndices, 6 * batch->floorTriangles);
				floor.count += 3 * batch->floorTriangles;
				delete[] batch->floorIndices;
				batch->floorIndices = NULL;
			}
		}
		for (std::map<DWORD, FloorAccumulator>::iterator floor = buffer->second.floors.begin();
			floor != buffer->second.floors.end(); ++floor)
		{
			if (floor->second.triangles) ++buffer->second.floorSegments;
		}
	}
	vertices = new BYTE*[meshCount]; memset(vertices, 0, 4 * meshCount);
	indices = new BYTE*[meshCount]; memset(indices, 0, 4 * meshCount);
	floorGroups = new OctreeFloorGroup[meshCount];
	meshes = new Gfx_MeshData[meshCount];
	instances = new Gfx_MeshInstance[meshCount];
	packets = new Gfx_PacketData*[meshCount]; memset(packets, 0, 4 * meshCount);
	textureHandles = new DWORD[textureCount];
	nodeMinimum = new D3DXVECTOR3*[meshCount]; memset(nodeMinimum, 0, 4 * meshCount);
	nodeMaximum = new D3DXVECTOR3*[meshCount]; memset(nodeMaximum, 0, 4 * meshCount);
	boundsVertexCount = 8 * nodeCount;
	boundsVertices = new BYTE[16 * boundsVertexCount];
	boundsIndices = new WORD[24 * nodeCount];
	if (lightCount)
	{
		lights = new Gfx_Light[lightCount];
		for (DWORD index = 0; index < lightCount; ++index)
		{
			const BYTE* source = reinterpret_cast<const BYTE*>(sourceLights + index);
			Gfx_Light& light = lights[index];
			switch (sourceLights[index].words[0])
			{
			case 1:
				light.light.Type = D3DLIGHT_SPOT;
				light.light.Falloff = 1;
				light.light.Theta = ReadFloat(source, 56) * 2;
				light.light.Phi = ReadFloat(source, 60) * 2;
				light.flag48 = 0;
				break;
			case 2:
				light.light.Type = D3DLIGHT_POINT;
				light.flag48 = 1;
				break;
			case 3:
				light.light.Type = D3DLIGHT_SPOT;
				light.light.Theta = ReadFloat(source, 56) * 2;
				light.light.Phi = ReadFloat(source, 60) * 2;
				light.light.Falloff = 1;
				light.flag48 = 1;
				break;
			default:
				light.light.Type = D3DLIGHT_POINT;
				light.flag48 = 0;
			}
			float range = ReadFloat(source, 48);
			switch (sourceLights[index].words[1])
			{
			case 1:
				light.light.Attenuation0 = 1;
				light.light.Attenuation1 = light.light.Attenuation2 = 0;
				light.light.Range = 5000;
				break;
			case 2:
				light.light.Attenuation0 = 1;
				light.light.Attenuation1 = static_cast<float>(1.0 / (range * 0.050000001f) - 1.0 / range);
				light.light.Attenuation2 = 0;
				light.light.Range = range;
				break;
			case 3:
				light.light.Attenuation0 = 0;
				light.light.Attenuation1 = 1.0f / range;
				light.light.Attenuation2 = 0;
				light.light.Range = range * 20;
				break;
			default:
				light.light.Attenuation0 = light.light.Attenuation1 = 0;
				light.light.Attenuation2 = 1.0f / (range * range);
				light.light.Range = static_cast<float>(sqrt(range * range * 20.0));
			}
			float intensity = ReadFloat(source, 44);
			light.light.Diffuse.r = ReadFloat(source, 8) * intensity;
			light.light.Diffuse.g = ReadFloat(source, 12) * intensity;
			light.light.Diffuse.b = ReadFloat(source, 16) * intensity;
			light.light.Diffuse.a = 0.5f;
			light.SetPosition(ReadFloat(source, 20), ReadFloat(source, 24), ReadFloat(source, 28));
			light.light.Direction.x = ReadFloat(source, 32);
			light.light.Direction.y = ReadFloat(source, 36);
			light.light.Direction.z = ReadFloat(source, 40);
		}
	}
	textureHandles[0] = 0;
	for (DWORD index = 1; index < textureCount; ++index)
	{
		char path[260];
		strcpy(path, names[index]);
		char* extension = strrchr(path, '.');
		strcpy(extension, ".dds");
		if (resourceTextures)
		{
			JE::Resources::ResourceMemoryStream* stream = JE::Resources::OpenResourceMemoryStream(path, 0);
			if (!stream)
			{
				strcpy(extension, ".tga");
				stream = JE::Resources::OpenResourceMemoryStream(path, 0);
				if (!stream)
				{
					JE::LogDebugString("Octree2Gfx::ConstructRenderData - Failed to load %s from ResourceSystem (no .dds or .tga)\n", names[index]);
					continue;
				}
			}
			HRESULT result = g_GfxSystem->LoadTextureFromBuffer(textureHandles + index, path,
				stream->begin, static_cast<UINT>(stream->end - stream->begin),
				D3DFMT_UNKNOWN, D3DPOOL_MANAGED, static_cast<UINT>(-1), 1, 5, 0);
			if (FAILED(result))
			{
				if (!strcmp(extension, ".dds"))
					JE::LogDebugString("Octree2Gfx::ConstructRenderData - Failed to load %s from ResourceSystem (.dds) (%x %s)\n",
						names[index], result, DXGetErrorDescription8A(result));
				else
					JE::LogDebugString("Octree2Gfx::ConstructRenderData - Failed to load %s from ResourceSystem (.tga) (%x %s)\n",
						names[index], result, DXGetErrorDescription8A(result));
			}
			JE::Resources::CloseResourceMemoryStream(stream);
		}
		else
		{
			HRESULT result = g_GfxSystem->LoadTextureFromFile(textureHandles + index, path,
				D3DFMT_UNKNOWN, D3DPOOL_MANAGED, static_cast<UINT>(-1), 1, 5, 0);
			if (FAILED(result))
			{
				strcpy(extension, ".tga");
				result = g_GfxSystem->LoadTextureFromFile(textureHandles + index, path,
					D3DFMT_UNKNOWN, D3DPOOL_MANAGED, static_cast<UINT>(-1), 1, 5, 0);
				if (FAILED(result))
					JE::LogDebugString("Octree2Gfx::ConstructRenderData - Failed to load %s (no .dds or .tga) (%x %s)\n",
						names[index], result, DXGetErrorDescription8A(result));
			}
		}
	}
	for (DWORD bufferIndex = 0; bufferIndex < meshCount; ++bufferIndex)
	{
		BufferBatches& buffer = buffers[bufferIndex];
		OctreeVertexStreams& source = sourceBuffers[bufferIndex];
		DWORD stride = D3DXGetFVFVertexSize(0x252);
		vertices[bufferIndex] = new BYTE[stride * source.count];
		OctreeFloorGroup& floor = floorGroups[bufferIndex];
		floor.count = buffer.floorSegments;
		floor.segments = new OctreeFloorSegment[floor.count];
		DWORD segment = 0;
		for (std::map<DWORD, FloorAccumulator>::iterator current = buffer.floors.begin();
			current != buffer.floors.end(); ++current)
		{
			if (current->second.triangles)
			{
				OctreeFloorSegment& destination = floor.segments[segment++];
				destination.nodeOrdinal = current->second.node;
				destination.triangleCount = current->second.triangles;
				destination.indexCount = 3 * destination.triangleCount;
				destination.indices = new WORD[destination.indexCount];
				memcpy(destination.indices, current->second.indices, 2 * destination.indexCount);
			}
		}
		octree->PackVertexStreams(0x1F00, &source, reinterpret_cast<DWORD*>(vertices[bufferIndex]));
		indices[bufferIndex] = new BYTE[indexStride * buffer.count];
		Gfx_MeshData& mesh = meshes[bufferIndex];
		mesh.value00 = 0;
		mesh.vertices = vertices[bufferIndex];
		mesh.SetVertexDesc(0x252, stride, source.count);
		Gfx_Primitive* primitives = mesh.SetPrimitiveCount(static_cast<DWORD>(buffer.batches.size()));
		nodeMinimum[bufferIndex] = new D3DXVECTOR3[nodeCount];
		nodeMaximum[bufferIndex] = new D3DXVECTOR3[nodeCount];
		DWORD primitiveIndex = 0, indexBase = 0, byteCursor = 0;
		DWORD currentNode = 0;
		bool first = true;
		D3DXVECTOR3 low, high;
		for (std::list<Batch>::iterator batch = buffer.batches.begin();
			batch != buffer.batches.end(); ++batch, ++primitiveIndex)
		{
			if (currentNode != batch->node || first)
			{
				currentNode = batch->node;
				memcpy(&low, vertices[bufferIndex] + 44 * batch->indices[0], 12);
				high = low;
				first = false;
			}
			if (batch->count > g_D3D.vertexLimit)
				JE::LogDebugString("Octree2Gfx::ConstructRenderData - Node's IndexCount %d exceeds max. (%d)!\n",
					batch->count, g_D3D.vertexLimit);
			memcpy(indices[bufferIndex] + byteCursor, batch->indices, indexStride * batch->count);
			// The original skips the first index of EVERY batch, even after the
			// first batch of a node. Preserve that bounds quirk.
			for (DWORD index = 1; index < batch->count; ++index)
			{
				D3DXVECTOR3 position;
				memcpy(&position, vertices[bufferIndex] + 44 * batch->indices[index], 12);
				if (position.x < low.x) low.x = position.x;
				if (position.y < low.y) low.y = position.y;
				if (position.z < low.z) low.z = position.z;
				if (position.x > high.x) high.x = position.x;
				if (position.y > high.y) high.y = position.y;
				if (position.z > high.z) high.z = position.z;
			}
			delete[] batch->indices;
			batch->indices = NULL;
			Gfx_Primitive& primitive = primitives[primitiveIndex];
			primitive.value00 = batch->node;
			primitive.type = D3DPT_TRIANGLELIST;
			primitive.value24 = 1;
			primitive.value28 = 0;
			primitive.value2C = indexBase;
			primitive.value30 = batch->count / 3;
			nodeMinimum[bufferIndex][currentNode] = low;
			nodeMaximum[bufferIndex][currentNode] = high;
			if (low.x < minimum.x) minimum.x = low.x;
			if (low.y < minimum.y) minimum.y = low.y;
			if (low.z < minimum.z) minimum.z = low.z;
			if (high.x > maximum.x) maximum.x = high.x;
			if (high.y > maximum.y) maximum.y = high.y;
			if (high.z > maximum.z) maximum.z = high.z;
			Gfx_Material& material = mesh.materialSet.materials[primitiveIndex];
			MaterialStyle& style = styles[batch->material];
			material.style1 = material.style2 = style.style;
			const BYTE* sourceMaterial = sourceMaterials[batch->material].bytes;
			D3DMATERIAL8 values;
			values.Diffuse.r = sourceMaterial[86] * 0.0039215689f;
			values.Diffuse.g = sourceMaterial[85] * 0.0039215689f;
			values.Diffuse.b = sourceMaterial[84] * 0.0039215689f;
			values.Diffuse.a = 1;
			values.Ambient.r = values.Ambient.g = values.Ambient.b = values.Ambient.a = 1;
			values.Specular.r = values.Specular.g = values.Specular.b = 0;
			values.Specular.a = 1;
			values.Emissive.r = values.Emissive.g = values.Emissive.b = 0;
			values.Emissive.a = 1;
			values.Power = 0;
			material.SetValues(values);
			material.textureHandles[0] = textureHandles[batch->texture];
			material.textureHandles[1] = textureHandles[batch->lightmap];
			material.mode24 = material.mode28 = style.blend;
			DWORD effects = ReadWord(sourceMaterial, 136);
			for (DWORD effect = 0; effect < 6; ++effect)
			{
				if (effects & (1 << effect))
				{
					const DWORD starts[6] = {0, 1, 2, 4, 6, 9};
					const DWORD lengths[6] = {1, 1, 2, 2, 3, 4};
					for (DWORD parameter = starts[effect]; parameter < starts[effect] + lengths[effect]; ++parameter)
						material.textureParameters[0].SetParam(parameter, ReadFloat(sourceMaterial, 140 + 4 * parameter));
				}
			}
			material.values140[0] |= effects;
			material.flagBD = 1;
			material.flagB8 = 1;
			DWORD flags = ReadWord(sourceMaterial, 192);
			material.flagB9 = (flags & 2) == 0;
			DWORD group = (flags >> 5) & 15;
			if (flags & 0x10)
			{
				opacityMaterials[group].push_back(&material);
				materialStates[group] = 1;
				material.flag240 = 1;
				material.ResetTextureFactor();
				material.mode28 = material.mode24;
			}
			if (flags & 8)
			{
				colorMaterials[group].push_back(&material);
				materialStates[group] = 1;
				material.flag240 = 1;
				material.ResetTextureFactor();
				material.mode28 = material.mode24;
			}
			if (flags & 4)
			{
				enabledMaterials[group].push_back(&material);
				materialStates[group] = 1;
				material.flag240 = 0;
			}
			indexBase += batch->count;
			byteCursor += indexStride * batch->count;
		}
		mesh.value10 = reinterpret_cast<DWORD>(indices[bufferIndex]);
		mesh.SetIndexDesc(indexStride, buffer.count);
		mesh.materialSet.RebuildGroups();
		g_GfxSystem->CreateGeometryData(&mesh);
		packets[bufferIndex] = Gfx_CreatePacketData(0);
		packets[bufferIndex]->transformEnabled = 0;
		packets[bufferIndex]->transform = NULL;
		packets[bufferIndex]->perPrimitiveLights = 1;
		instances[bufferIndex].SetMesh(&mesh);
		instances[bufferIndex].value0C = reinterpret_cast<DWORD>(packets[bufferIndex]);
	}
	if (showBounds)
	{
		boundsMesh = new Gfx_MeshData;
		boundsMesh->value04 = static_cast<DWORD>(-1);
		boundsMesh->vertices = boundsVertices;
		boundsMesh->value10 = reinterpret_cast<DWORD>(boundsIndices);
		boundsPrimitive = boundsMesh->SetPrimitiveCount(1);
		boundsPrimitive->type = D3DPT_LINELIST;
		boundsPrimitive->value24 = 1;
		boundsPrimitive->value28 = boundsPrimitive->value2C = 0;
		Gfx_Material& material = boundsMesh->materialSet.materials[0];
		material.style1 = material.style2 = 14;
		material.mode24 = material.mode28 = 1;
		boundsMesh->materialSet.RebuildGroups();
		boundsInstance = new Gfx_MeshInstance;
		boundsInstance->SetMesh(boundsMesh);
	}
	delete[] styles;
}
