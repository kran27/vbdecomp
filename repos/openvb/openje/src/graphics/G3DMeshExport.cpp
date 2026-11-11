#include "G3DMeshExport.hpp"
#include "SystemInterface.hpp"
#include "SkinRender.hpp"
#include "D3DState.hpp"
#include "../resources/ResourceMemoryStream.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <string.h>
#include <new>
#include <stdlib.h>

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char G3DLegacyVertexSnapshotExtent[(sizeof(G3D_Vertex) == 144) ? 1 : -1];
typedef char G3DLegacyPolygonSnapshotExtent[(sizeof(G3D_Polygon) == 144) ? 1 : -1];
#endif

namespace
{
	const char* const techniques[33] =
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
	const char* const blends[6] = {"OPAQUE", "ALPHABLEND", "MODCOLOR", "SUBTRACT", "REVSUBTRACT", "ADD"};

	DWORD Lookup(const std::string& value, const char* const* names, DWORD count, const char* kind)
	{
		if (value.empty())
		{
			JE::LogDebugString(!strcmp(kind, "material")
				? "Gfx_G3D_Mesh - No Material specified!\n"
				: "Gfx_G3D_Mesh - No blend_state specified!\n");
			return 0;
		}
		for (DWORD index = 0; index < count; ++index) if (value == names[index]) return index;
		JE::LogDebugString("Gfx_G3D_Mesh - Unknown %s: %s\n", kind, value.c_str());
		return 0;
	}

	void SnapshotColor(D3DCOLORVALUE& output, const D3DCOLORVALUE& source)
	{
		const double alpha = source.a, blue = source.b, green = source.g;
		memcpy(&output.r, &source.r, 4);
		output.g = static_cast<float>(green);
		output.b = static_cast<float>(blue);
		output.a = static_cast<float>(alpha);
	}

	void SnapshotMaterial(D3DMATERIAL8& output, const Gfx_G3D_Material& source)
	{
		SnapshotColor(output.Diffuse, source.colors[1]);
		SnapshotColor(output.Ambient, source.colors[0]);
		SnapshotColor(output.Specular, source.colors[3]);
		SnapshotColor(output.Emissive, source.colors[2]);
		memcpy(&output.Power, &source.power, 4);
	}

	//Source factoring of the two exporter bodies; no separate original routine.
	void LoadTexture(const std::string& source, std::string& name, DWORD& handle,
		DWORD& textures, DWORD count, BYTE loadTextures, BYTE resources,
		const char* missingResource, const char* missingFile)
	{
		if (!loadTextures)
		{
			textures = count;
			if (resources) name = source;
			else name.assign(source.c_str());
			return;
		}
		char path[256];
		strcpy(path, source.c_str());
		char* extension = strrchr(path, '.');
		strcpy(extension, ".dds");
		if (resources)
		{
			JE::Resources::ResourceMemoryStream* stream = JE::Resources::OpenResourceMemoryStream(path, 0);
			if (!stream)
			{
				strcpy(extension, ".tga");
				stream = JE::Resources::OpenResourceMemoryStream(path, 0);
			}
			if (stream)
			{
				const BYTE* data = stream->begin;
				const UINT bytes = static_cast<UINT>(stream->end - data);
				g_GfxSystem->LoadTextureFromBuffer(&handle, path, data,
					bytes, D3DFMT_UNKNOWN,
					D3DPOOL_MANAGED, UINT(-1), 3, 5, 0);
				JE::Resources::CloseResourceMemoryStream(stream);
			}
			else JE::LogDebugString(missingResource, path);
		}
		else
		{
			if (FAILED(g_GfxSystem->LoadTextureFromFile(&handle, path, D3DFMT_UNKNOWN,
				D3DPOOL_MANAGED, UINT(-1), 3, 5, 0)))
			{
				strcpy(extension, ".tga");
				if (FAILED(g_GfxSystem->LoadTextureFromFile(&handle, path, D3DFMT_UNKNOWN,
					D3DPOOL_MANAGED, UINT(-1), 3, 5, 0)))
					JE::LogDebugString(missingFile, path);
			}
		}
		if (handle) textures = count;
		name.assign(path);
	}

	void ApplyTextureParameters(Gfx_Material& material, const Gfx_G3D_Group& source,
		BYTE staticMesh)
	{
		if (source.scrollUEnabled)
		{
			material.values140[0] |= 1;
			material.textureParameters[0].SetParam(0, source.scrollU);
		}
		if (source.scrollVEnabled)
		{
			material.values140[0] |= 2;
			material.textureParameters[0].SetParam(1, source.scrollV);
		}
		if (source.sinUEnabled)
		{
			material.values140[0] |= 4;
			material.textureParameters[0].SetParam(2, source.sinU[0]);
			material.textureParameters[0].SetParam(3, source.sinU[1]);
		}
		if (source.sinVEnabled)
		{
			material.values140[0] |= 8;
			material.textureParameters[0].SetParam(4, source.sinV[0]);
			material.textureParameters[0].SetParam(5, source.sinV[1]);
		}
		if (source.rotationEnabled)
		{
			material.values140[0] |= 16;
			material.textureParameters[0].SetParam(6, source.rotation[0]);
			material.textureParameters[0].SetParam(7, source.rotation[1]);
			material.textureParameters[0].SetParam(8, source.rotation[2]);
		}
		if (source.animationEnabled)
		{
			material.values140[0] |= 32;
			material.textureParameters[0].SetParam(9, source.animationRate);
			material.textureParameters[0].SetParam(10, static_cast<float>(source.animationParameters[0]));
			material.textureParameters[0].SetParam(11, static_cast<float>(source.animationParameters[1]));
			if (staticMesh)
				material.textureParameters[0].SetParam(12, static_cast<float>(source.animationParameters[2]));
		}
		material.flagBD = 1;
	}
}

Gfx_G3D_Mesh::Gfx_G3D_Mesh()
{
	maximumBoneWeights = g_GfxSystem->MaximumVertexBlendMatrices();
	vertexCount = 0;
	positions = NULL;
	normals = NULL;
	colors = NULL;
	textureCoordinates = NULL;
	lightmapCoordinates = NULL;
	hasBones = 0;
	boneWeights = NULL;
	minimum[2] = 0;
	minimum[1] = 0;
	minimum[0] = 0;
	maximum[2] = 0;
	maximum[1] = 0;
	maximum[0] = 0;
	totalTriangleCount = 0;
	groupCount = 0;
	names = NULL;
	groups = NULL;
	vertexStride = 0;
	vertices = NULL;
	types = NULL;
	indexCounts = NULL;
	primitiveCounts = NULL;
	indices = NULL;
}

Gfx_G3D_Mesh::~Gfx_G3D_Mesh()
{
	Cleanup();
}

void Gfx_G3D_Mesh::DetachBorrowed()
{
	vertices = NULL;
	boneWeights = NULL;
	names = NULL;
	indexCounts = NULL;
	primitiveCounts = NULL;
	indices = NULL;
}

void Gfx_G3D_Mesh::Cleanup()
{
	if (positions) { delete[] positions; positions = NULL; }
	if (normals) { delete[] normals; normals = NULL; }
	if (colors) { delete[] colors; colors = NULL; }
	if (textureCoordinates) { delete[] textureCoordinates; textureCoordinates = NULL; }
	if (lightmapCoordinates) { delete[] lightmapCoordinates; lightmapCoordinates = NULL; }
	hasBones = 0;
	if (boneWeights)
	{
		for (DWORD channel = 0; channel < maximumBoneWeights; ++channel)
			if (boneWeights[channel])
			{
				delete[] boneWeights[channel];
				boneWeights[channel] = NULL;
			}
		if (boneWeights) { delete[] boneWeights; boneWeights = NULL; }
	}
	if (names) { delete[] names; names = NULL; }
	for (LONG group = 0; group < static_cast<LONG>(groupCount); ++group)
	{
		if (groups[group].names)
		{
			delete[] groups[group].names;
			groups[group].names = NULL;
		}
		if (groups[group].sourceIndices)
		{
			delete[] groups[group].sourceIndices;
			groups[group].sourceIndices = NULL;
		}
		if (indices && indices[group])
		{
			delete[] static_cast<BYTE*>(indices[group]);
			indices[group] = NULL;
		}
	}
	if (groups) { delete[] groups; groups = NULL; }
	if (indices) { delete[] indices; indices = NULL; }
	if (vertices) { delete[] vertices; vertices = NULL; }
	if (types) { delete[] types; types = NULL; }
	if (indexCounts) { delete[] indexCounts; indexCounts = NULL; }
	if (primitiveCounts) { delete[] primitiveCounts; primitiveCounts = NULL; }
	vertexCount = 0;
	totalTriangleCount = 0;
	groupCount = 0;
}

BYTE Gfx_G3D_Mesh::LoadMesh(G3D_Node* node, G3D_Mesh* mesh, float scale, BYTE leftHanded, DWORD fvf)
{
	Cleanup();
	G3D_Transform transform(node->transform); //526828; owns its name through return.
	if (!mesh->converted)
	{
		LoadLegacy(node, mesh, scale, leftHanded, fvf);
		return 1;
	}
	G3D_ConvertedMeshData sourceData;
	mesh->GetConvertedData(&sourceData);
	vertexStride = sourceData.vertexStride;
	vertexCount = sourceData.vertexCount;
	vertices = sourceData.vertices;
	memcpy(minimum, sourceData.minimum, sizeof(minimum));
	memcpy(maximum, sourceData.maximum, sizeof(maximum));
	groupCount = sourceData.groupCount;
	hasBones = sourceData.hasBones;
	boneWeights = sourceData.boneWeights;
	names = sourceData.materialNames;
	indexCounts = sourceData.indexCounts;
	primitiveCounts = sourceData.primitiveCounts;
	indices = sourceData.indices;
	groups = new Gfx_G3D_Group[groupCount];
	types = static_cast<D3DPRIMITIVETYPE*>(::operator new(4 * groupCount));
	for (LONG index = 0; index < static_cast<LONG>(groupCount); ++index)
	{
		types[index] = D3DPT_TRIANGLELIST;
		const G3D_MeshMaterial& source = sourceData.materials[index];
		Gfx_G3D_Group& group = groups[index];
		group.textureCount = source.textureCount;
		group.baseTexture = source.textures[0];
		group.lightmapTexture = source.textures[1];
		group.nameCount = source.nameCount;
		group.names = NULL;
		if (group.nameCount)
		{
			group.names = new std::string[group.nameCount];
			for (LONG zone = 0; zone < static_cast<LONG>(group.nameCount); ++zone)
				group.names[zone] = source.names[zone];
		}
		group.sourceIndexCount = 0;
		group.sourceIndices = NULL;
		group.name = source.name;
		group.style = Lookup(source.technique, techniques, 33, "material");
		memcpy(&group.values.colors[0], source.colors, 16);
		memcpy(&group.values.colors[1], source.colors + 4, 16);
		memcpy(&group.values.colors[2], source.colors + 8, 16);
		memcpy(&group.values.colors[3], source.colors + 12, 16);
		group.values.power = source.power;
		group.blend = Lookup(source.blend, blends, 6, "blend_state");
		memcpy(group.flags, source.flags180, 2);
		group.scrollUEnabled = source.flags180[2];
		group.scrollVEnabled = source.flag188;
		group.sinUEnabled = source.flag196;
		group.sinVEnabled = source.flag208;
		group.rotationEnabled = source.flag220;
		group.animationEnabled = source.flag236;
		// Original copies the raw words, including integer texture-frame parameters.
		memcpy(&group.scrollU, &source.value184, 4);
		memcpy(&group.scrollV, &source.value192, 4);
		memcpy(group.sinU, source.parametersA, 8);
		memcpy(group.sinV, source.parametersB, 8);
		memcpy(group.rotation, source.parametersC, 12);
		memcpy(&group.animationRate, source.parametersD, 4);
		memcpy(group.animationParameters, source.parametersD + 1, 12);
	}
	return 1;
}

void Gfx_G3D_Mesh::LoadLegacy(G3D_Node* node, G3D_Mesh* mesh, float scale,
	BYTE leftHanded, DWORD fvf)
{
	{
		const std::string name = mesh->GetName();
		JE::LogDebugString("Gfx_G3D_Mesh::LoadMesh - This G3D/B3D (%s) has not been re-converted!\n",
			name.c_str());
	}
	const DWORD count = static_cast<DWORD>(mesh->legacyVertices.size());
	g_GfxSystem->ModeIndex(); // Original queries this before allocating channels.
	positions = static_cast<D3DXVECTOR3*>(::operator new(12 * count));
	normals = static_cast<D3DXVECTOR3*>(::operator new(12 * count));
	colors = static_cast<D3DXVECTOR4*>(::operator new(16 * count));
	textureCoordinates = static_cast<D3DXVECTOR2*>(::operator new(8 * count));
	lightmapCoordinates = static_cast<D3DXVECTOR2*>(::operator new(8 * count));
	boneWeights = static_cast<Gfx_G3D_BoneWeight**>(::operator new(4 * maximumBoneWeights));
	for (DWORD channel = 0; channel < maximumBoneWeights; ++channel)
		boneWeights[channel] = static_cast<Gfx_G3D_BoneWeight*>(::operator new(8 * count));
	if (!positions) JE::FatalError("Gfx_G3D_Mesh: Failed to allocate memory for Coords!\n");
	if (!normals) JE::FatalError("Gfx_G3D_Mesh: Failed to allocate memory for Normals!\n");
	if (!colors) JE::FatalError("Gfx_G3D_Mesh: Failed to allocate memory for ARGB!\n");
	if (!textureCoordinates) JE::FatalError("Gfx_G3D_Mesh: Failed to allocate memory for Texture Coords!\n");
	if (!lightmapCoordinates) JE::FatalError("Gfx_G3D_Mesh: Failed to allocate memory for Lightmap Coords!\n");
	if (!boneWeights) JE::FatalError("Gfx_G3D_Mesh: Failed to allocate memory for Bone Weights!\n");
	vertexCount = count;
	for (LONG index = 0; index < static_cast<LONG>(count); ++index)
	{
		const G3D_Vertex source = mesh->legacyVertices[index]; //52706B; complete144B snapshot.
		D3DXVECTOR3& position = positions[index];
		position.x = static_cast<float>(scale * source.position[0]);
		position.y = static_cast<float>(scale * source.position[leftHanded ? 1 : 2]);
		position.z = static_cast<float>(scale * source.position[leftHanded ? 2 : 1]);
		for (DWORD axis = 0; axis < 3; ++axis)
		{
			float value = position[axis];
			if (index == 0 || value < minimum[axis]) minimum[axis] = value;
			if (index == 0 || value > maximum[axis]) maximum[axis] = value;
		}
		// Original swaps position Y/Z for RH input, but only negates normal Z.
		normals[index].x = static_cast<float>(source.normal[0]);
		normals[index].y = static_cast<float>(source.normal[1]);
		normals[index].z = static_cast<float>(leftHanded ? source.normal[2] : -source.normal[2]);
		colors[index].w = source.color[3];
		colors[index].z = source.color[0];
		colors[index].y = source.color[1];
		colors[index].x = source.color[2];
		if (source.textureCount)
			textureCoordinates[index] = D3DXVECTOR2(static_cast<float>(source.textureCoordinates[0][0]),
				static_cast<float>(source.textureCoordinates[0][1]));
		if (source.textureCount > 1)
			lightmapCoordinates[index] = D3DXVECTOR2(static_cast<float>(source.textureCoordinates[1][0]),
				static_cast<float>(source.textureCoordinates[1][1]));
		if (source.boneCount) hasBones = 1;
		for (LONG channel = 0; channel < static_cast<LONG>(maximumBoneWeights); ++channel)
		{
			Gfx_G3D_BoneWeight& weight = boneWeights[channel][index];
			if (channel < source.boneCount)
			{
				weight.bone = source.boneWeights[channel].bone;
				weight.weight = static_cast<float>(source.boneWeights[channel].weight);
			}
			else { weight.bone = -1; weight.weight = 0.0f; }
		}
	}
	const DWORD polygonCount = static_cast<DWORD>(mesh->polygons.size());
	groups = new Gfx_G3D_Group[polygonCount];
	if (!groups) JE::FatalError("Gfx_G3D_Mesh: Failed to allocate memory for Groups!\n");
	groupCount = polygonCount;
	types = static_cast<D3DPRIMITIVETYPE*>(::operator new(4 * groupCount));
	names = new std::string[groupCount];
	for (LONG index = 0; index < static_cast<LONG>(polygonCount); ++index)
	{
		const G3D_Polygon polygon = *mesh->polygons[index];
		Gfx_G3D_Group& group = groups[index];
		const std::string materialName = polygon.GetMaterial();
		names[index] = materialName;
		group.name = materialName;
		const G3D_Material& material = *node->model->FindMaterial(materialName);
		const std::string technique = material.technique;
		group.style = Lookup(technique, techniques, 33, "material");
		group.values.colors[0].r = group.values.colors[0].g = group.values.colors[0].b =
			group.values.colors[0].a = 1.0f;
		memcpy(&group.values.colors[1], &material.diffuse, 16);
		memcpy(&group.values.colors[2], &material.emissive, 16);
		memcpy(&group.values.colors[3], &material.specular, 16);
		group.values.power = static_cast<float>(material.power);
		const std::string blend = material.GetBlend();
		group.blend = Lookup(blend, blends, 6, "blend_state");
		group.flags[0] = material.twoSided == 0;
		group.flags[1] = material.noShadow == 0;
		const double scrollU = material.scrollU;
		group.scrollUEnabled = material.scrollUEnabled;
		if (group.scrollUEnabled) group.scrollU = static_cast<float>(scrollU);
		group.scrollVEnabled = material.scrollVEnabled;
		const double scrollV = material.scrollV;
		if (group.scrollVEnabled) group.scrollV = static_cast<float>(scrollV);
		const double sinU0 = material.sinUFirst, sinU1 = material.sinUSecond;
		group.sinUEnabled = material.sinUEnabled;
		if (group.sinUEnabled)
		{
			group.sinU[0] = static_cast<float>(sinU0);
			group.sinU[1] = static_cast<float>(sinU1);
		}
		const double sinV0 = material.sinVFirst, sinV1 = material.sinVSecond;
		group.sinVEnabled = material.sinVEnabled;
		if (group.sinVEnabled)
		{
			group.sinV[0] = static_cast<float>(sinV0);
			group.sinV[1] = static_cast<float>(sinV1);
		}
		const double rotation0 = material.rotationFirst, rotation1 = material.rotationSecond;
		const double rotation2 = material.rotationThird;
		group.rotationEnabled = material.rotationEnabled;
		if (group.rotationEnabled)
		{
			group.rotation[0] = static_cast<float>(rotation0);
			group.rotation[1] = static_cast<float>(rotation1);
			group.rotation[2] = static_cast<float>(rotation2);
		}
		const double animationRate = material.animationRate;
		LONG animationParameters[3];
		memcpy(animationParameters, &material.animationFirst, 12);
		group.animationEnabled = material.animationEnabled;
		if (group.animationEnabled)
		{
			group.animationRate = static_cast<float>(animationRate);
			memcpy(group.animationParameters, animationParameters, 12);
		}
		const DWORD textureCount = polygon.textureCount;
		group.textureCount = textureCount;
		if (textureCount)
		{
			std::string textureName = polygon.GetTextureName(0);
			{
				const std::string filename = node->model->FindTexture(textureName)->GetFilename();
				group.baseTexture = filename;
			}
			node->model->FindTexture(textureName); //5279EF; original unused repeated lookup.
			if (static_cast<LONG>(textureCount) > 1)
			{
				{
					const std::string secondName = polygon.GetTextureName(1);
					textureName = secondName;
				}
				{
					const std::string filename = node->model->FindTexture(textureName)->GetFilename();
					group.lightmapTexture = filename;
				}
				node->model->FindTexture(textureName); //527AFA; result deliberately ignored.
			}
		}
		group.names = NULL;
		group.nameCount = static_cast<DWORD>(polygon.names.size());
		if (group.nameCount)
		{
			group.names = new std::string[group.nameCount];
			for (LONG name = 0; name < static_cast<LONG>(group.nameCount); ++name)
				group.names[name] = polygon.GetName(name);
		}
		if (polygon.primitive != 0)
		{
			JE::LogDebugString("Unsupported Primitive Type\n");
			continue; //527C98; primitive/index fields stay unwritten, final packing still runs.
		}
		group.primitiveType = D3DPT_TRIANGLELIST;
		types[index] = D3DPT_TRIANGLELIST;
		const DWORD sourceIndexCount = static_cast<DWORD>(polygon.indices.size());
		group.sourceIndices = static_cast<DWORD*>(::operator new(4 * sourceIndexCount));
		if (!group.sourceIndices)
			JE::FatalError("Gfx_G3D_Mesh: Failed to allocate memory for indices!\n");
		group.sourceIndexCount = sourceIndexCount;
		const DWORD indexLimit = g_D3D.vertexLimit;
		if (sourceIndexCount >= indexLimit)
			JE::LogDebugString("Gfx_G3D_Mesh::LoadMesh - G3D Mesh's IndexCount is %d (max = %d)!\n",
				sourceIndexCount, indexLimit);
		for (LONG item = 0; item < static_cast<LONG>(sourceIndexCount); item += 3)
		{
			group.sourceIndices[item] = polygon.indices[item];
			group.sourceIndices[item + 1] = polygon.indices[item + (leftHanded ? 1 : 2)];
			group.sourceIndices[item + 2] = polygon.indices[item + (leftHanded ? 2 : 1)];
		}
		totalTriangleCount += static_cast<LONG>(sourceIndexCount) / 3;
	}
	PackVertices(fvf);
	BuildIndices();
}

Gfx_SkinMesh* Gfx_G3D_Mesh::ExportGfxSkinMesh(DWORD fvf,
	DWORD boneCount, Gfx_ModelData526620* model, BYTE loadTextures, DWORD unused,
	BYTE resources)
{
	Gfx_SkinMesh* output = new Gfx_SkinMesh;
	if (!output) return NULL;
	output->SetMaterialCount(groupCount);
	output->ResetMaterialExtraNames();
	std::string textureNames[2];
	for (LONG index = 0; index < static_cast<LONG>(groupCount); ++index)
	{
		Gfx_G3D_Group& group = groups[index];
		D3DMATERIAL8 values;
		SnapshotMaterial(values, group.values);
		const DWORD nameCount = groups[index].nameCount;
		if (nameCount)
		{
			output->SetMaterialExtraNameCount(index, nameCount);
			for (DWORD name = 0; name < nameCount; ++name)
				output->materialNamesExtra[index].names[name] = groups[index].names[name];
		}
		DWORD handles[2] = {0, 0};
		textureNames[0] = "";
		textureNames[1] = "";
		DWORD textures = 0;
		Gfx_G3D_Group* textureGroup = &groups[index];
		if (textureGroup->textureCount)
		{
			LoadTexture(textureGroup->baseTexture, textureNames[0], handles[0], textures, 1,
				loadTextures, resources, "ExportGfxSkinMesh - Missing texture from resource: %s\n",
				"ExportGfxSkinMesh - Missing texture file: %s\n");
			if (fvf & 0x200)
			{
				textureGroup = &groups[index];
				if (!textureGroup->lightmapTexture.empty())
					LoadTexture((loadTextures && !resources ? textureGroup->baseTexture : textureGroup->lightmapTexture),
						textureNames[1], handles[1], textures, 2, loadTextures, resources,
						"ExportGfxSkinMesh - Missing lightmap from resource: %s\n",
						"ExportGfxSkinMesh - Missing lightmap file: %s\n");
			}
		}
		output->SetMaterial(index, group.style, group.name, group.blend, values,
			textures, textureNames, handles, group.flags[0], group.flags[1]);
		Gfx_Material* material = output->materials + index;
		if (material) ApplyTextureParameters(*material, group, 0);
	}
	if (vertexStride)
	{
		switch (vertexStride)
		{
		case 36: fvf = 338; break;
		case 44: fvf = 594; break;
		default:
			JE::FatalError("Gfx_G3D_Mesh::ExportGfxModelMesh - Bad VertexSize for preconverted mesh: %d\n",
				vertexStride);
		}
	}
	output->CreateSkinMeshFVF(fvf, vertexCount, vertices, groupCount, names,
		indexCounts, primitiveCounts, indices, boneCount, model, boneWeights, unused);
	output->RebuildGroups(output->materials, output->groups);
	return output;
}

Gfx_ModelMesh* Gfx_G3D_Mesh::ExportGfxModelMesh(DWORD fvf, BYTE loadTextures, BYTE resources)
{
	Gfx_ModelMesh* output = new Gfx_ModelMesh(1);
	if (!output) return NULL;
	if (vertexStride)
	{
		switch (vertexStride)
		{
		case 28: fvf = 82; break;
		case 36: fvf = 338; break;
		case 44: fvf = 594; break;
		default:
			JE::FatalError("Gfx_G3D_Mesh::ExportGfxModelMesh - Bad VertexSize for preconverted mesh: %d\n",
				vertexStride);
		}
	}
	const DWORD count = vertexCount;
	const BYTE* data = vertices;
	const DWORD stride = D3DXGetFVFVertexSize(fvf);
	output->SetVertexData(fvf, stride, count, data);
	output->SetPrimitives(groupCount, types, names, indexCounts, primitiveCounts, indices);
	memcpy(output->mesh.unknown54, minimum, 12);
	memcpy(output->mesh.unknown54 + 3, maximum, 12);
	std::string textureNames[2];
	for (LONG index = 0; index < static_cast<LONG>(groupCount); ++index)
	{
		Gfx_G3D_Group& group = groups[index];
		D3DMATERIAL8 values;
		SnapshotMaterial(values, group.values);
		DWORD handles[2] = {0, 0};
		textureNames[0] = "";
		textureNames[1] = "";
		DWORD textures = 0;
		Gfx_G3D_Group* textureGroup = &groups[index];
		if (textureGroup->textureCount)
		{
			LoadTexture(textureGroup->baseTexture, textureNames[0], handles[0], textures, 1,
				loadTextures, resources, "ExportGfxModelMesh - Missing texture from resource: %s\n",
				"ExportGfxModelMesh - Missing texture file: %s\n");
			if (fvf & 0x200)
			{
				textureGroup = &groups[index];
				if (!textureGroup->lightmapTexture.empty())
					LoadTexture(textureGroup->lightmapTexture, textureNames[1], handles[1], textures, 2,
						loadTextures, resources, "ExportGfxModelMesh - Missing lightmap from resource: %s\n",
						"ExportGfxModelMesh - Missing lightmap file: %s\n");
			}
		}
		output->SetMaterial(index, group.style, group.name, 0, group.blend, values,
			textures, textureNames, handles, group.flags[0], 1);
		Gfx_Material* material = output->mesh.materialSet.materials + index;
		if (material) ApplyTextureParameters(*material, group, 1);
	}
	return output;
}
