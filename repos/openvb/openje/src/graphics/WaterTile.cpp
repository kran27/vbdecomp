#include "WaterTile.hpp"
#include "G3DWaterTile.hpp"
#include "Water.hpp"
#include "WaterSampling.hpp"
#include "ModelNode.hpp"
#include "SystemInterface.hpp"
#include "Texture.hpp"
#include "TextureManager.hpp"
#include "Light.hpp"
#include "resources/ResourceMemoryStream.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string.h>

namespace
{
	DWORD ReadWaterTexture(const std::string& name, BYTE resources, const char* kind)
	{
		DWORD handle = 0;
		if (name.empty()) return handle;
		if (resources)
		{
			JE::Resources::ResourceMemoryStream* stream = JE::Resources::OpenResourceMemoryStream(name.c_str(), 0);
			if (!stream) JE::LogDebugString("RecurseChildNode - Missing water %s from resource: %s\n", kind, name.c_str());
			else
			{
				g_GfxSystem->LoadTextureFromBuffer(&handle, name, stream->begin, UINT(stream->end - stream->begin),
					D3DFMT_UNKNOWN, D3DPOOL_MANAGED, UINT(-1), 3, 5, 0);
				JE::Resources::CloseResourceMemoryStream(stream);
			}
		}
		else
		{
			std::ifstream input(name.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
			if (input)
			{
				std::streamoff count = input.tellg();
				if (count > 0)
				{
					std::vector<BYTE> bytes(static_cast<size_t>(count));
					input.seekg(0); input.read(reinterpret_cast<char*>(&bytes[0]), count);
					if (!input) throw std::runtime_error("Water52859A: unable to read water texture file");
					g_GfxSystem->LoadTextureFromBuffer(&handle, name, &bytes[0], UINT(count),
						D3DFMT_UNKNOWN, D3DPOOL_MANAGED, UINT(-1), 3, 5, 0);
				}
			}
		}
		return handle;
	}
	struct LightOrder
	{
		bool operator()(const Gfx_Light* left, const Gfx_Light* right) const
		{ return LONG(left->value44) > LONG(right->value44); }
	};
}

Gfx_WaterTile::Gfx_WaterTile()
{
	//512010 constructs mesh, both strings and the actual light list first.
	vertices32 = NULL; vertices44 = NULL; vertexCount = 0;
	indices = NULL; indexCount = 0;
	//Original CC..D7 has three zero stores, but its source types remain open.
	instances = NULL; transforms = NULL; instanceCount = 0; packet = NULL;
	offsetZ = 0; offsetX = 0; rows = 0; columns = 0;
	dirty = 1;
	D3DXMatrixIdentity(&localTransform);
	primitiveCount = 0; triangleList = 0;
	for (DWORD corner = 0; corner < 4; ++corner)
		for (DWORD axis = 0; axis < 3; ++axis) cornerColors[corner][axis] = 0;
	texture = 0; ownsTexture = 0; lightmap = 0; ownsLightmap = 0;
	extentX = 0; extentZ = 0;
	textureWidth = 0; textureHeight = 0; originalTextureWidth = 0; originalTextureHeight = 0;
	surfaceU = 0; surfaceV = 0; surfaceWidth = 1; surfaceHeight = 1;
	lightU = 0; lightV = 0; lightWidth = 1; lightHeight = 1;
	directWater = NULL; flag224 = 1; flag225 = 1; mode28 = 1;
	D3DXMatrixIdentity(&extraTransform);
	blue = 128; green = 128; red = 128;
	lastTime = NULL; accumulatedTime = 0; alphaEnd = 0;
	alphaFrequency = 1; alphaPhase = 0; flag226 = 1; alpha = 255;
	value23C = 0.025f; animateAlpha = 1; normalizeAlphaTime = 1;
	alphaWaveform = 1; alphaDuration = -1;
}

Gfx_WaterTile::~Gfx_WaterTile()
{
	if (vertices32) delete[] vertices32;
	if (vertices44) delete[] vertices44;
	if (indices) delete[] indices;
	if (instances) delete[] instances;
	if (transforms) delete[] transforms;
	if (packet) { delete packet; packet = NULL; }
	if (ownsTexture && texture) g_TextureManager->Release(texture);
	if (ownsLightmap && lightmap) g_TextureManager->Release(lightmap);
	if (lastTime) delete lastTime;
	//The real mesh/material owner frees its own geometry metadata afterwards.
}

Gfx_Water* Gfx_WaterTile::GetWater()
{
	return directWater ? directWater : JE::FindWaterSampleOwner(waterName);
}

void Gfx_WaterTile::LoadSource(G3D_Node* node, const G3D_WaterTile& source, BYTE resources)
{
	waterName = node->name; //4CE920 returns the actual node's stored name.
	offsetX = source.offsetX; offsetZ = source.offsetZ;
	columns = source.columns; rows = source.rows;
	std::string surface, lighting;
	if (source.sourceVersion)
	{
		if (source.groupCount)
		{
			const G3D_MeshMaterial& first = source.materials[0];
			if (first.textureCount) surface = first.textures[0];
			if (first.textureCount > 1) lighting = first.textures[1];
		}
	}
	else if (!source.polygons.empty())
	{
		const G3D_Polygon& first = *source.polygons[0];
		if (first.textureCount) surface = first.textures[0];
		if (first.textureCount > 1) lighting = first.textures[1];
	}
	SetTexture(ReadWaterTexture(surface, resources, "texture"), 1);
	SetLightmap(ReadWaterTexture(lighting, resources, "lightmap"), 1);
}

Gfx_WaterTile* Gfx_WaterTile::Clone(Gfx_ModelNode* node) const
{
	Gfx_WaterTile* result = new Gfx_WaterTile;
	result->owner = node;
	result->waterName = waterName;
	result->columns = columns; result->rows = rows;
	result->offsetX = offsetX; result->offsetZ = offsetZ;
	result->SetTexture(texture, 0); result->SetLightmap(lightmap, 0);
	for (LONG corner = 0; corner < 4; ++corner)
	{
		DWORD red, green, blue;
		if (GetCornerColor(corner, &red, &green, &blue))
			result->SetCornerColor(corner, red, green, blue);
	}
	result->Rebuild();
	result->SetSurfaceUV(surfaceU, surfaceV, surfaceWidth, surfaceHeight);
	result->SetTextureUV(lightU, lightV, lightWidth, lightHeight);
	return result;
}

void Gfx_WaterTile::SetTexture(DWORD handle, BYTE owned)
{
	if (ownsTexture && texture) g_TextureManager->Release(texture);
	texture = handle; ownsTexture = owned;
}

void Gfx_WaterTile::SetLightmap(DWORD handle, BYTE owned)
{
	if (ownsLightmap && lightmap) g_TextureManager->Release(lightmap);
	lightmap = handle; ownsLightmap = owned;
	Gfx_Texture* source = handle ? g_TextureManager->Lookup(handle) : NULL;
	textureWidth = source && source->frameCount > 0 ? float(source->frames[0].width) : 0;
	textureHeight = source && source->frameCount > 0 ? float(source->frames[0].height) : 0;
	originalTextureWidth = textureWidth; originalTextureHeight = textureHeight;
}

void Gfx_WaterTile::SetTextureUV(float u, float v, float width, float height)
{
	lightU = u; lightV = v; lightWidth = width; lightHeight = height;
	textureWidth = width * originalTextureWidth; textureHeight = height * originalTextureHeight;
	if (instances) BuildIndices(lightmapName.c_str());
}

void Gfx_WaterTile::SetSurfaceUV(float u, float v, float width, float height)
{ surfaceU = u; surfaceV = v; surfaceWidth = width; surfaceHeight = height; }

BYTE Gfx_WaterTile::GetCornerColor(LONG corner, DWORD* red, DWORD* green, DWORD* blue) const //50FD70.
{
	if (corner < 0 || corner >= 4) return 0;
	//Separate copies preserve the original live read/store order, including aliases.
	memcpy(red, &cornerColors[corner][0], sizeof(*red));
	memcpy(green, &cornerColors[corner][1], sizeof(*green));
	memcpy(blue, &cornerColors[corner][2], sizeof(*blue));
	return 1;
}

BYTE Gfx_WaterTile::SetCornerColor(DWORD corner, DWORD, DWORD, DWORD)
{
	if (corner >= 4) return 0;
	cornerColors[corner][0] = cornerColors[corner][1] = cornerColors[corner][2] = 255;
	return 1;
}

void Gfx_WaterTile::Rebuild()
{
	if (vertices32) { delete[] vertices32; vertices32 = NULL; }
	if (vertices44) { delete[] vertices44; vertices44 = NULL; }
	if (instances) { delete[] instances; instances = NULL; }
	if (transforms) { delete[] transforms; transforms = NULL; }
	vertexCount = DWORD(columns * rows);
	if (flag226) vertices32 = new Gfx_WaterVertex32[vertexCount];
	else vertices44 = new Gfx_WaterVertex44[vertexCount];
	instanceCount = 1;
	instances = new Gfx_MeshInstance[1];
	transforms = new D3DXMATRIX[instanceCount];
	for (LONG instance = 0; instance < instanceCount; ++instance)
		D3DXMatrixIdentity(&transforms[instance]);
	Gfx_Water* water = GetWater();
	if (water) { extentX = water->tilesX * water->cellSize; extentZ = water->tilesZ * water->cellSize; }
	if (owner)
	{
		transforms[0] = static_cast<Gfx_ModelNode*>(owner)->worldTransform;
		transforms[0]._41 = float(double(transforms[0]._41) - extentX * 0.5);
		transforms[0]._43 = float(double(transforms[0]._43) - extentZ * 0.5);
	}
	else D3DXMatrixIdentity(transforms);
}

BYTE Gfx_WaterTile::AddLight(Gfx_Light* light)
{
	if (lights.size() >= g_GfxSystem->MaximumActiveLights()) return 0;
	lights.push_back(light); return 1;
}
void Gfx_WaterTile::RemoveLight(Gfx_Light* light)
{
	Gfx_LightList::iterator found = std::find(lights.begin(), lights.end(), light);
	if (found != lights.end()) lights.erase(found);
}
void Gfx_WaterTile::ClearLights() { lights.clear(); }
void Gfx_WaterTile::FinishLights() { lights.reverse(); lights.sort(LightOrder()); }
