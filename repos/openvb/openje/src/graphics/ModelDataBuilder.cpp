#include "ModelData.hpp"
#include "G3DMeshExport.hpp"
#include "G3DWaterTile.hpp"
#include "G3DSceneAttributes.hpp"
#include "LineSystem.hpp"
#include "ParticleSystem.hpp"
#include "WaterTile.hpp"
#include "Water.hpp"
#include "SkinRender.hpp"
#include "SystemInterface.hpp"
#include "resources/ResourceSystem.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace
{
	struct DeferredModelHandles;
	DeferredModelHandles* liveDeferredModelHandles = NULL;

	class DeferredModelReleaseOwner
	{
	public:
		virtual ~DeferredModelReleaseOwner(); //4B8450, shared body4B83D0.
	};

	//Original641680 constructs this owner before6416A0 constructs the list.
	//Reverse destruction therefore destroys the list before the cleanup owner.
	DeferredModelReleaseOwner deferredModelReleaseOwner;

	struct DeferredModelHandles
	{
		DeferredModelHandles() { liveDeferredModelHandles = this; }
		~DeferredModelHandles() { liveDeferredModelHandles = NULL; }
		std::list<DWORD> handles; //Original707F1C, including its owning sentinel.
	};

	DeferredModelHandles deferredModelHandles;

	DeferredModelReleaseOwner::~DeferredModelReleaseOwner()
	{
		//The original guards its list-head pointer, which645E00 clears before
		//645DF0 calls this destructor. Publication preserves that exit ordering
		//without accessing a C++ container after its lifetime has ended.
		if (!liveDeferredModelHandles) return;
		std::list<DWORD>& handles = liveDeferredModelHandles->handles;
		for (std::list<DWORD>::iterator entry = handles.begin(); entry != handles.end();)
		{
			DWORD handle = *entry;
			if (!g_GfxSystem->ReleaseModelData(&handle)) entry = handles.erase(entry);
			else ++entry;
		}
	}

	D3DXQUATERNION ModelRotation(const G3D_Transform& transform, BYTE leftHanded)
	{
		if (leftHanded)
			return D3DXQUATERNION(float(transform.rotation[0]), float(transform.rotation[1]),
				float(transform.rotation[2]), float(transform.rotation[3]));
		return D3DXQUATERNION(float(-transform.rotation[0]), float(-transform.rotation[1]),
			float(-transform.rotation[2]), float(transform.rotation[3]));
	}
	D3DXVECTOR3 ModelPosition(const G3D_Transform& transform, BYTE leftHanded, float scale)
	{
		double z = scale * transform.translation[2];
		if (!leftHanded) z = -z;
		return D3DXVECTOR3(float(scale * transform.translation[0]), float(scale * transform.translation[1]),
			float(z));
	}
}

DWORD Gfx_LoadModelData(const char* name, DWORD kind, DWORD textures,
	DWORD shadow, float scale, DWORD fvf) //4B8190; forward all six words unchanged.
{
	Gfx_ModelDataLoadCallback callback = g_GfxModelDataLoadCallback;
	if (callback) return callback(name, kind, textures, shadow, scale, fvf);
	return 0;
}

void Gfx_G3D_Model::BuildNode(G3D_Node& source, Gfx_ModelNode& target, DWORD fvf, float scale,
	Gfx_ModelData526620& data, BYTE textures, DWORD shadow) //527FD0.
{
	const G3D_Transform transform(source.transform);
	const BYTE handedness = leftHanded;
	const D3DXQUATERNION rotation = ModelRotation(transform, handedness);
	const D3DXVECTOR3 position = ModelPosition(transform, handedness, scale);
	target.SetRotation(rotation);
	target.position = position;
	target.localTransform._41 = target.position.x;
	target.localTransform._42 = target.position.y;
	target.localTransform._43 = target.position.z;
	target.dirty = 1;
	const LONG attributeCount = LONG(source.attributes.size());
	for (LONG i = 0; i < attributeCount; ++i)
	{
		G3D_WaterTile* waterTile = dynamic_cast<G3D_WaterTile*>(source.attributes[i]);
		if (waterTile)
		{
			Gfx_WaterTile* attribute = new Gfx_WaterTile;
			attribute->LoadSource(&source, *waterTile, resources);
			target.attributes.push_back(attribute);
			attribute->owner = &target;
			continue; //Actual528184 tests this subtype before generic G3D_Mesh.
		}
		G3D_Mesh* mesh = dynamic_cast<G3D_Mesh*>(source.attributes[i]);
		if (mesh)
		{
			Gfx_G3D_Mesh* converter = new Gfx_G3D_Mesh;
			if (converter->LoadMesh(&source, mesh, scale, leftHanded, fvf))
			{
				target.minimum = D3DXVECTOR3(converter->minimum);
				target.maximum = D3DXVECTOR3(converter->maximum);
				Gfx_NodeAttribute* attribute = converter->hasBones
					? static_cast<Gfx_NodeAttribute*>(converter->ExportGfxSkinMesh(
						fvf, boneCount, &data, textures, shadow, resources))
					: static_cast<Gfx_NodeAttribute*>(converter->ExportGfxModelMesh(fvf, textures, resources));
				target.attributes.push_back(attribute);
				attribute->owner = &target;
				{
					const std::string name = mesh->owner->GetName();
					data.name = name;
				}
				if (mesh->converted) converter->DetachBorrowed();
			}
			delete converter;
		}
		else
		{
			G3D_OcclusionZones* zones = dynamic_cast<G3D_OcclusionZones*>(source.attributes[i]);
			if (!zones)
			{
				if (G3D_ParticleEmitter* emitter = dynamic_cast<G3D_ParticleEmitter*>(source.attributes[i]))
				{
					Gfx_ParticleSystem* attribute = new Gfx_ParticleSystem;
					attribute->scale = float(emitter->scale * double(scale));
					target.attributes.push_back(attribute);
					attribute->owner = &target; //528BD0, before resource loading.
					const std::string name(emitter->effectName);
					DWORD texture = 0;
					if (resources)
					{
						JE::Resources::ResourceMemoryStream* stream =
							JE::Resources::OpenResourceMemoryStream(name.c_str(), 0);
						//528C4B requires this stream; the original has no NULL check.
						attribute->LoadConfiguration(stream->begin, DWORD(stream->end - stream->begin));
						JE::Resources::CloseResourceMemoryStream(stream);
						stream = JE::Resources::OpenResourceMemoryStream(attribute->configuration.textureName, 0);
						if (stream)
						{
							g_GfxSystem->LoadTextureFromBuffer(&texture, attribute->configuration.textureName,
								stream->begin, UINT(stream->end - stream->begin), D3DFMT_UNKNOWN,
								D3DPOOL_MANAGED, UINT(-1), 3, 5, 0);
							JE::Resources::CloseResourceMemoryStream(stream);
						}
						else JE::LogDebugString("RecurseChildNode - Missing particle from resource: %s\n",
							attribute->configuration.textureName);
					}
					else
					{
						FILE* stream = fopen(name.c_str(), "rb");
						if (stream)
						{
							fseek(stream, 0, SEEK_END);
							const LONG size = ftell(stream);
							if (size > 0)
							{
								void* bytes = ::operator new(size);
								if (bytes)
								{
									fseek(stream, 0, SEEK_SET);
									fread(bytes, size, 1, stream);
									attribute->LoadConfiguration(static_cast<const BYTE*>(bytes), size);
									free(bytes); //528D68: before the texture API, using the original free API.
									g_GfxSystem->LoadTextureFromFile(&texture, attribute->configuration.textureName,
										D3DFMT_UNKNOWN, D3DPOOL_MANAGED, UINT(-1), 3, 5, 0);
								}
							}
							fclose(stream);
						}
					}
					attribute->SetTexture(texture, 1, 1); //528DBA, even for a zero handle.
					continue;
				}
				if (G3D_WaterAttribute* water = dynamic_cast<G3D_WaterAttribute*>(source.attributes[i]))
				{
					Gfx_Water* attribute = new Gfx_Water;
					attribute->SetRegistryName(water->GetName());
					const G3D_WaterDimensions dimensions(water->dimensions);
					const __int64 rows = static_cast<__int64>(dimensions.rows);
					const __int64 columns = static_cast<__int64>(dimensions.columns);
					attribute->sampleHeight = LONG(rows);
					attribute->sampleWidth = LONG(columns);
					attribute->SetTextureName(water->textureName);
					attribute->cellSize = water->cellSize * scale;
					attribute->SetSecondaryTextureName(water->secondaryTextureName);
					attribute->tilesX = water->tilesX;
					attribute->tilesZ = water->tilesZ;
					for (std::list<Gfx_WaterTexturePatch>::const_iterator patch = water->texturePatches.begin();
						patch != water->texturePatches.end(); ++patch)
						attribute->AddTexturePatch(patch->x, patch->z, patch->textureName, patch->tileName);
					target.attributes.push_back(attribute);
					attribute->owner = &target; //No PrepareSamples/Update/registry call in this branch.
					continue;
				}
				G3D_LineSystem* lines = dynamic_cast<G3D_LineSystem*>(source.attributes[i]);
				if (!lines) continue; //Original unknown/non-render attribute policy.
				Gfx_LineSystem* attribute = new Gfx_LineSystem;
				const LONG count = LONG(lines->segments.size());
				for (LONG index = 0; index < count; ++index)
				{
					const G3D_LineSegment segment(lines->segments[index]);
					D3DXVECTOR3 from, to;
					for (DWORD axis = 0; axis < 3; ++axis)
					{
						from[axis] = float(double(scale) * segment.vertices[0].position[axis]);
						to[axis] = float(double(scale) * segment.vertices[1].position[axis]);
					}
					LONG channels[8];
					for (LONG endpoint = 1; endpoint >= 0; --endpoint)
						for (LONG channel = 3; channel >= 0; --channel)
							channels[endpoint * 4 + channel] = LONG(static_cast<__int64>(
								double(segment.vertices[endpoint].color[channel]) * 255.0));
					attribute->AddSegment(from.x, from.y, from.z, to.x, to.y, to.z, channels[0], channels[1], channels[2],
						channels[3], channels[4], channels[5], channels[6], channels[7]);
				}
				target.attributes.push_back(attribute);
				attribute->owner = &target; //5291D9, after the completed segment loop.
				continue;
			}
			if (extraNames)
			{
				delete[] extraNames;
				extraNames = NULL;
				JE::LogDebugString("ERROR: Mesh \"%s\" contains multiple occlusion zone definitions!\n", data.name.c_str());
			}
			extraNameCount = DWORD(zones->names.size());
			extraNames = new std::string[extraNameCount];
			for (DWORD n = 0; n < extraNameCount; ++n)
			{
				const std::string name = zones->GetName(n);
				extraNames[n] = name;
			}
		}
	}
	const LONG childCount = LONG(source.children.size());
	for (LONG i = 0; i < childCount; ++i)
	{
		Gfx_ModelNode* child = new Gfx_ModelNode;
		child->owner = target.owner;
		target.children.push_back(child);
		child->parent = &target; child->root = target.root; child->owner = target.owner;
		BuildNode(*source.children[i], *child, fvf, scale, data, textures, shadow);
	}
}

void Gfx_DeferModelDataRelease(DWORD handle) //4B86E0, append at the sentinel's tail.
{
	deferredModelHandles.handles.push_back(handle);
}

Gfx_G3D_Model::Gfx_G3D_Model()
	: leftHanded(0), resources(1), source(NULL), ownsSource(0), boneCount(0),
	extraNameCount(0), extraNames(NULL)
{
}

Gfx_G3D_Model::~Gfx_G3D_Model()
{
	if (ownsSource && source) { delete source; source = NULL; }
	if (extraNames) { delete[] extraNames; extraNames = NULL; }
}

void Gfx_G3D_Model::LoadResource(const std::string& name)
{
	if (ownsSource && source) { delete source; source = NULL; }
	source = new G3D_Model;
	ownsSource = 1;
	JE::Resources::ResourceMemoryStream* stream =
		JE::Resources::g_pResourceSystem->OpenMemoryStream(name.c_str(), 0);
	if (stream)
	{
		source->Read(static_cast<const BYTE*>(stream->resource->data));
		JE::Resources::g_pResourceSystem->CloseMemoryStream(stream);
	}
}

BYTE Gfx_G3D_Model::SetSource(G3D_Model* value, BYTE mode)
{
	if (ownsSource && source) { delete source; source = NULL; }
	resources = mode;
	source = value;
	ownsSource = 0;
	return mode;
}

Gfx_ModelData526620* Gfx_G3D_Model::Build(DWORD fvf, DWORD kind, BYTE textures,
	DWORD shadow, float scale) //529490.
{
	if (!source) return NULL;
	leftHanded = source->coordinateSystem.compare(0, source->coordinateSystem.size(),
		"LEFT_HANDED", strlen("LEFT_HANDED")) == 0;
	Gfx_ModelData526620* data = new Gfx_ModelData526620(kind);
	std::list<G3D_Bone>& bones = source->bones;
	boneCount = DWORD(bones.size()); //529544..52954E, before traversal.
	for (std::list<G3D_Bone>::iterator i = bones.begin(); i != bones.end(); ++i)
	{
		Gfx_ModelBone bone;
		{
			const std::string name = i->GetName();
			bone.name = name;
		}
		const G3D_Transform transform(i->transform);
		const BYTE handedness = leftHanded;
		const D3DXQUATERNION rotation = ModelRotation(transform, handedness);
		const D3DXVECTOR3 position = ModelPosition(transform, handedness, scale);
		bone.SetQuaternionRotation(rotation);
		bone.SetPosition(position.x, position.y, position.z);
		data->bones.push_back(bone);
	}
	std::map<std::string, G3D_WaterAttribute>& waterDefinitions = source->waterByName;
	for (std::map<std::string, G3D_WaterAttribute>::iterator entry = waterDefinitions.begin();
		entry != waterDefinitions.end(); ++entry)
	{
		const G3D_WaterAttribute& definition = entry->second;
		Gfx_Water* water = new Gfx_Water;
		water->registryName = definition.GetName();
		const G3D_WaterDimensions dimensions(definition.dimensions);
		const __int64 rows = static_cast<__int64>(dimensions.rows);
		const __int64 columns = static_cast<__int64>(dimensions.columns);
		water->sampleWidth = static_cast<LONG>(columns);
		water->sampleHeight = static_cast<LONG>(rows);
		{
			const std::string name(definition.textureName);
			water->textureName = name;
		}
		water->cellSize = definition.cellSize * scale;
		{
			const std::string name(definition.secondaryTextureName);
			water->secondaryTextureName = name;
		}
		water->tilesX = definition.tilesX;
		water->tilesZ = definition.tilesZ;
		water->PrepareSamples();
		for (std::list<Gfx_WaterTexturePatch>::const_iterator patch = definition.texturePatches.begin();
			patch != definition.texturePatches.end(); ++patch)
			water->AddTexturePatch(patch->x, patch->z, patch->textureName, patch->tileName);
		water->Update(0.0f);
		data->AddWater(water);
	}
	if (source->root) BuildNode(*source->root, data->root, fvf, scale, *data, textures, shadow);
	if (extraNameCount && extraNames)
	{
		data->SetExtraNameCount(extraNameCount);
		for (DWORD index = 0; index < extraNameCount; ++index)
			data->extraNames[index] = extraNames[index];
	}
	data->Initialize(shadow);
	return data;
}

Gfx_ModelData526620* Gfx_BuildModelData(G3D_Model* source, DWORD kind, BYTE textures,
	BYTE shadow, float scale, DWORD fvf, BYTE resources)
{
	Gfx_G3D_Model converter;
	converter.SetSource(source, resources);
	return converter.Build(fvf, kind, textures, shadow, scale);
}

DWORD Gfx_LoadModelData(G3D_Model* source, DWORD kind, DWORD textures, DWORD shadow,
	float scale, DWORD fvf, Gfx_ModelData526620** output, BYTE resources) //529D80.
{
	DWORD handle = 0;
	if (source)
	{
		Gfx_G3D_Model converter;
		converter.SetSource(source, resources);
		Gfx_ModelData526620* data = converter.Build(fvf, kind, textures, shadow, scale);
		if (data)
		{
			if (output) *output = data;
			handle = g_GfxSystem->RegisterModelData(data);
		}
	}
	return handle;
}

DWORD Gfx_LoadModelData(const std::string& name, DWORD kind, BYTE textures,
	DWORD shadow, float scale, DWORD fvf) //529E30.
{
	Gfx_G3D_Model converter;
	converter.LoadResource(name);
	Gfx_ModelData526620* data = converter.Build(fvf, kind, textures, shadow, scale);
	DWORD handle = 0;
	if (data) handle = g_GfxSystem->RegisterModelData(data);
	return handle;
}

DWORD Gfx_LoadModelData(const char* name, DWORD kind, BYTE textures, BYTE shadow,
	float scale, DWORD fvf, Gfx_ModelData526620** output)
{
	//Explicit compatibility/inspection API; the original wrapper has six words.
	Gfx_G3D_Model converter;
	converter.LoadResource(std::string(name));
	Gfx_ModelData526620* data = converter.Build(fvf, kind, textures, shadow, scale);
	if (!data) return 0;
	if (output) *output = data;
	return g_GfxSystem->RegisterModelData(data);
}
