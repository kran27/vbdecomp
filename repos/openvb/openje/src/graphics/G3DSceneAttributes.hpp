#pragma once

#include "G3DModel.hpp"
#include "Water.hpp"

struct G3D_LineSegment
{
	G3D_LineSegment(); //4CA480, two ordinary Vertex constructors.
	void ReadText(); //4D76B0; unchecked number of authored Vertex records.
	void WriteText(FILE* stream); //4CA4B0.
	void Read(FILE* stream); //4CA500.
	void Read(const BYTE* data, DWORD& offset); //4CA530.
	void Write(FILE* stream); //4CA560.
	G3D_Vertex vertices[2];
};

class G3D_LineSystem : public G3D_BaseAttribute
{
public:
	G3D_LineSystem(); //4D78F0.
	virtual ~G3D_LineSystem(); //4D7960.
	virtual void ReadText(); //4D8270.
	virtual void WriteText(FILE* stream); //4CBC50.
	virtual void Read(FILE* stream); //4D84E0.
	virtual void Read(const BYTE* data, DWORD& offset); //4D8630.
	virtual void Write(FILE* stream); //4CBCC0.
	std::vector<G3D_LineSegment> segments; //+24, original288B element.
};

class G3D_ParticleEmitter : public G3D_BaseAttribute
{
public:
	G3D_ParticleEmitter(); //4CBB90.
	virtual ~G3D_ParticleEmitter(); //4CBBF0.
	virtual void ReadText(); //4D2260; token kind is deliberately not checked.
	virtual void WriteText(FILE* stream); //4CAC90.
	virtual void Read(FILE* stream); //4D2330.
	virtual void Read(const BYTE* data, DWORD& offset); //4D23E0.
	virtual void Write(FILE* stream); //4CEF30.
	std::string effectName; //+24.
	double scale; //+40, captured from the original shared global.
};

class G3D_LightmapSurfaceAttribute : public G3D_BaseAttribute
{
public:
	G3D_LightmapSurfaceAttribute(); //4CB350.
	virtual ~G3D_LightmapSurfaceAttribute(); //4CB3A0.
	virtual void ReadText(); //4D0C80.
	virtual void WriteText(FILE* stream); //4CAC10.
	virtual void Read(FILE* stream); //4D0F00.
	virtual void Read(const BYTE* data, DWORD& offset); //4D0F30.
	virtual void Write(FILE* stream); //4CEBB0.
	std::string material, lightmap; //+24/+40; no base-name record is serialized.
};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
#define F3_G3D_SCENE_OFFSET(type, member, value) typedef char F3_##type##_##member[(offsetof(type, member) == value) ? 1 : -1]
typedef char F3_G3D_WaterDimensions_Size[(sizeof(G3D_WaterDimensions) == 16) ? 1 : -1];
typedef char F3_G3D_WaterAttribute_Size[(sizeof(G3D_WaterAttribute) == 0x88) ? 1 : -1];
typedef char F3_G3D_LineSegment_Size[(sizeof(G3D_LineSegment) == 0x120) ? 1 : -1];
typedef char F3_G3D_LineSystem_Size[(sizeof(G3D_LineSystem) == 0x34) ? 1 : -1];
typedef char F3_G3D_ParticleEmitter_Size[(sizeof(G3D_ParticleEmitter) == 0x48) ? 1 : -1];
typedef char F3_G3D_LightmapSurfaceAttribute_Size[(sizeof(G3D_LightmapSurfaceAttribute) == 0x5C) ? 1 : -1];
F3_G3D_SCENE_OFFSET(G3D_WaterAttribute, dimensions, 0x28);
F3_G3D_SCENE_OFFSET(G3D_WaterAttribute, textureName, 0x38);
F3_G3D_SCENE_OFFSET(G3D_WaterAttribute, cellSize, 0x54);
F3_G3D_SCENE_OFFSET(G3D_WaterAttribute, secondaryTextureName, 0x58);
F3_G3D_SCENE_OFFSET(G3D_WaterAttribute, tilesX, 0x74);
F3_G3D_SCENE_OFFSET(G3D_WaterAttribute, tilesZ, 0x78);
F3_G3D_SCENE_OFFSET(G3D_WaterAttribute, texturePatches, 0x7C);
F3_G3D_SCENE_OFFSET(G3D_LineSystem, segments, 0x24);
F3_G3D_SCENE_OFFSET(G3D_ParticleEmitter, effectName, 0x24);
F3_G3D_SCENE_OFFSET(G3D_ParticleEmitter, scale, 0x40);
F3_G3D_SCENE_OFFSET(G3D_LightmapSurfaceAttribute, material, 0x24);
F3_G3D_SCENE_OFFSET(G3D_LightmapSurfaceAttribute, lightmap, 0x40);
#undef F3_G3D_SCENE_OFFSET
#endif
