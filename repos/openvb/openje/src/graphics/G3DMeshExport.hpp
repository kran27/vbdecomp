#pragma once

#include "G3DModel.hpp"
#include "ModelMesh.hpp"
#include <cstddef>

class Gfx_SkinMesh;
class Gfx_ModelData526620;

//Stored color order differs from D3DMATERIAL8; exporters build a separate snapshot.
struct Gfx_G3D_Material
{
	D3DCOLORVALUE colors[4];
	float power;
};

struct Gfx_G3D_Group
{
	Gfx_G3D_Group() {} // Original524580 constructs strings, leaves scalar fields unwritten.
	std::string name;
	DWORD style;
	Gfx_G3D_Material values;
	DWORD blend;
	BYTE flags[2];
	BYTE scrollUEnabled;
	float scrollU;
	BYTE scrollVEnabled;
	float scrollV;
	BYTE sinUEnabled;
	float sinU[2];
	BYTE sinVEnabled;
	float sinV[2];
	BYTE rotationEnabled;
	float rotation[3];
	BYTE animationEnabled;
	float animationRate;
	LONG animationParameters[3];
	DWORD textureCount;
	std::string baseTexture;
	std::string lightmapTexture;
	DWORD nameCount;
	std::string* names;
	D3DPRIMITIVETYPE primitiveType;
	DWORD sourceIndexCount;
	DWORD* sourceIndices;
};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char GfxG3DMaterialSize[(sizeof(Gfx_G3D_Material) == 68) ? 1 : -1];
typedef char GfxG3DMaterialColors[(offsetof(Gfx_G3D_Material, colors) == 0) ? 1 : -1];
typedef char GfxG3DMaterialPower[(offsetof(Gfx_G3D_Material, power) == 64) ? 1 : -1];
typedef char GfxG3DGroupSize[(sizeof(Gfx_G3D_Group) == 260) ? 1 : -1];
#define GFX_G3D_GROUP_OFFSET(field, offset) \
	typedef char GfxG3DGroup_##field[(offsetof(Gfx_G3D_Group, field) == offset) ? 1 : -1]
GFX_G3D_GROUP_OFFSET(style, 0x1C);
GFX_G3D_GROUP_OFFSET(values, 0x20);
GFX_G3D_GROUP_OFFSET(blend, 0x64);
GFX_G3D_GROUP_OFFSET(flags, 0x68);
GFX_G3D_GROUP_OFFSET(scrollUEnabled, 0x6A);
GFX_G3D_GROUP_OFFSET(scrollU, 0x6C);
GFX_G3D_GROUP_OFFSET(scrollVEnabled, 0x70);
GFX_G3D_GROUP_OFFSET(scrollV, 0x74);
GFX_G3D_GROUP_OFFSET(sinUEnabled, 0x78);
GFX_G3D_GROUP_OFFSET(sinU, 0x7C);
GFX_G3D_GROUP_OFFSET(sinVEnabled, 0x84);
GFX_G3D_GROUP_OFFSET(sinV, 0x88);
GFX_G3D_GROUP_OFFSET(rotationEnabled, 0x90);
GFX_G3D_GROUP_OFFSET(rotation, 0x94);
GFX_G3D_GROUP_OFFSET(animationEnabled, 0xA0);
GFX_G3D_GROUP_OFFSET(animationRate, 0xA4);
GFX_G3D_GROUP_OFFSET(animationParameters, 0xA8);
GFX_G3D_GROUP_OFFSET(textureCount, 0xB4);
GFX_G3D_GROUP_OFFSET(baseTexture, 0xB8);
GFX_G3D_GROUP_OFFSET(lightmapTexture, 0xD4);
GFX_G3D_GROUP_OFFSET(nameCount, 0xF0);
GFX_G3D_GROUP_OFFSET(names, 0xF4);
GFX_G3D_GROUP_OFFSET(primitiveType, 0xF8);
GFX_G3D_GROUP_OFFSET(sourceIndexCount, 0xFC);
GFX_G3D_GROUP_OFFSET(sourceIndices, 0x100);
#undef GFX_G3D_GROUP_OFFSET
#endif

//5240E0's raw100B owner. Converted aliases require the caller's523E70 detach.
class Gfx_G3D_Mesh
{
public:
	Gfx_G3D_Mesh(); // 0x5240E0.
	~Gfx_G3D_Mesh(); //524AA0 ->524700.
	void Cleanup(); //524700; leaves bounds and vertexStride unchanged.
	void DetachBorrowed(); //523E70; six pointer stores, no ownership flag.
	BYTE LoadMesh(G3D_Node* node, G3D_Mesh* mesh, float scale, BYTE leftHanded, DWORD fvf); // 0x5267E0.
	Gfx_ModelMesh* ExportGfxModelMesh(DWORD fvf, BYTE loadTextures, BYTE resources); // 0x524AB0.
	Gfx_SkinMesh* ExportGfxSkinMesh(DWORD fvf, DWORD boneCount, Gfx_ModelData526620* model,
		BYTE loadTextures, DWORD unused, BYTE resources); //5255C0, six stack words.
	DWORD maximumBoneWeights;
	DWORD vertexCount;
	D3DXVECTOR3* positions;
	D3DXVECTOR3* normals;
	D3DXVECTOR4* colors;
	D3DXVECTOR2* textureCoordinates;
	D3DXVECTOR2* lightmapCoordinates;
	BYTE hasBones;
	Gfx_G3D_BoneWeight** boneWeights;
	float minimum[3], maximum[3];
	DWORD totalTriangleCount;
	DWORD groupCount;
	std::string* names;
	Gfx_G3D_Group* groups;
	DWORD vertexStride;
	BYTE* vertices;
	D3DPRIMITIVETYPE* types;
	DWORD* indexCounts;
	DWORD* primitiveCounts;
	void** indices;

private:
	void LoadLegacy(G3D_Node* node, G3D_Mesh* mesh, float scale, BYTE leftHanded, DWORD fvf);
	void PackVertices(DWORD fvf); // 0x523E90; real D3DXGetFVFVertexSize.
	void BuildIndices(); // 0x524140.
	Gfx_G3D_Mesh(const Gfx_G3D_Mesh&);
	Gfx_G3D_Mesh& operator=(const Gfx_G3D_Mesh&);
};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char GfxG3DMeshSize[(sizeof(Gfx_G3D_Mesh) == 100) ? 1 : -1];
#define GFX_G3D_MESH_OFFSET(field, offset) \
	typedef char GfxG3DMesh_##field[(offsetof(Gfx_G3D_Mesh, field) == offset) ? 1 : -1]
GFX_G3D_MESH_OFFSET(maximumBoneWeights, 0x00);
GFX_G3D_MESH_OFFSET(vertexCount, 0x04);
GFX_G3D_MESH_OFFSET(positions, 0x08);
GFX_G3D_MESH_OFFSET(normals, 0x0C);
GFX_G3D_MESH_OFFSET(colors, 0x10);
GFX_G3D_MESH_OFFSET(textureCoordinates, 0x14);
GFX_G3D_MESH_OFFSET(lightmapCoordinates, 0x18);
GFX_G3D_MESH_OFFSET(hasBones, 0x1C);
GFX_G3D_MESH_OFFSET(boneWeights, 0x20);
GFX_G3D_MESH_OFFSET(minimum, 0x24);
GFX_G3D_MESH_OFFSET(maximum, 0x30);
GFX_G3D_MESH_OFFSET(totalTriangleCount, 0x3C);
GFX_G3D_MESH_OFFSET(groupCount, 0x40);
GFX_G3D_MESH_OFFSET(names, 0x44);
GFX_G3D_MESH_OFFSET(groups, 0x48);
GFX_G3D_MESH_OFFSET(vertexStride, 0x4C);
GFX_G3D_MESH_OFFSET(vertices, 0x50);
GFX_G3D_MESH_OFFSET(types, 0x54);
GFX_G3D_MESH_OFFSET(indexCounts, 0x58);
GFX_G3D_MESH_OFFSET(primitiveCounts, 0x5C);
GFX_G3D_MESH_OFFSET(indices, 0x60);
#undef GFX_G3D_MESH_OFFSET
#endif
