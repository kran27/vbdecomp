#pragma once

#include <windows.h>
#include <list>
#include "Material.hpp"
#include "Primitive.hpp"
#include "Skinning.hpp"

// Recovered owned layouts used by GUI::Fragment; broader graphics/rendering
// methods remain unresolved. Unknown scalar words stay uninitialized.
class Gfx_MaterialSet
{
public:
	Gfx_MaterialSet(); // 0x4ABC60.
	Gfx_MaterialSet(const Gfx_MaterialSet& source); // 0x4ABD40; copies materials, then creates group storage.
	~Gfx_MaterialSet(); // 0x41FBD0.
	Gfx_MaterialSet& operator=(const Gfx_MaterialSet& source); // 0x4ABCF0; does not rebuild groups.
	void Clear(); // 0x41F1C0; release materials, retain group storage.
	void ResetGroups(); // 0x4ABC10.
	void ResetValues(); //49C9A0; restore existing material values, no group rebuild.
	void SetCount(DWORD count); // 0x41F570; publish count, delete/clear old array, allocate.
	void RebuildGroups(); // 0x4ABDD0; material mode28 must be a valid group index.
	void SetColors(BYTE useRGB, BYTE useAlpha, DWORD color, const DWORD* emissive); // 0x4AB600.
	DWORD count;
	DWORD* groupCounts;
	std::list<DWORD>* groups; // Original dynamically allocated array of six lists.
	Gfx_Material* materials;

};

class Gfx_MeshData
{
public:
	Gfx_MeshData(); // Inlined member initialization in 0x4B24A0.
	~Gfx_MeshData(); // 0x41FDF0.
	void Free(); // 0x41F200.
	void BuildShadowGeometry(); // 0x4AB740; original WORD-index domain.
	Gfx_Primitive* SetPrimitiveCount(DWORD count); // 0x41F630; then replaces materials.
	void SetVertexDesc(DWORD fvf, DWORD stride, DWORD count); // 0x4AB680; actual global lifetime remains unbound.
	void SetIndexDesc(DWORD stride, DWORD count); // 0x4AB6C0; byte stride and index count, confirmed by 0x4911B0.

	DWORD value00;
	DWORD value04;
	DWORD value08;
	void* vertices; // +0C; Fragment points it at its embedded vertices.
	DWORD value10, vertexFVF, vertexStride, vertexCount, value20, value24;
	Gfx_MaterialSet materialSet; // +28.
	DWORD primitiveCount; // +38.
	Gfx_Primitive* primitives; // +3C; owned array, element size52.
	DWORD value40;
	void* owned44;
	void* owned48;
	DWORD value4C;
	void* owned50;
	DWORD unknown54[6]; // +54..6B; unwritten by the observed initialization.

private:
	Gfx_MeshData(const Gfx_MeshData&);
	Gfx_MeshData& operator=(const Gfx_MeshData&);
};

class Gfx_MeshInstance
{
public:
	Gfx_MeshInstance(); // Inlined initialization in 0x4B24A0.
	Gfx_MeshInstance(const Gfx_MeshInstance& source); // 0x4929D0; owned material/flag copies, borrowed mesh/skinning.
	~Gfx_MeshInstance(); // 0x420270.
	void SetMesh(Gfx_MeshData* mesh); // 0x4AB6E0; publishes before replacing flags.
	void AllocateSkinning(); // 0x4ABB70.
	void EnableMaterial(const std::string& name, BYTE enabled); //49DC20; first name match.
	Gfx_MeshData* mesh;
	DWORD value04; // +04; effective borrowed material-set override, stored as its raw word.
	Gfx_MaterialSet* materialSet; // +08; independently owned material-set copy, when present.
	DWORD value0C;
	BYTE* primitiveFlags;
	BYTE ownsSkinning;
	Gfx_Skinning* skinning;
	BYTE flag1C, flag1D, flag1E, flag1F;
	float value20;
	DWORD value24;

private:
	Gfx_MeshInstance& operator=(const Gfx_MeshInstance&);
};

// Visible definitions preserve the inlined member lifetime observed in Fragment.
// The real material-set constructor runs before these scalar writes.
inline Gfx_MeshData::Gfx_MeshData()
{
	value00 = 0;
	value04 = 0;
	value08 = 0;
	vertices = 0;
	value10 = 0;
	vertexFVF = 0;
	vertexStride = 0;
	vertexCount = 0;
	value20 = 0;
	value24 = 0;
	primitiveCount = 0;
	primitives = 0;
	value40 = 0;
	owned44 = 0;
	owned48 = 0;
	value4C = 0;
	owned50 = 0;
	// unknown54 remains unwritten.
}

inline Gfx_MeshData::~Gfx_MeshData() { Free(); }

inline Gfx_MeshInstance::Gfx_MeshInstance()
	: mesh(0), value04(0), materialSet(0), value0C(0), primitiveFlags(0), ownsSkinning(0),
	skinning(0), flag1C(0), flag1D(0), flag1E(0), flag1F(0), value20(0.05f), value24(0xFFFF0000)
{
}
