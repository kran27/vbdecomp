#pragma once

#include "MeshData.hpp"

// Normal source objects for the actual render-node attribute and its static mesh.
// Only the ordinary lifetime and data-producing methods used here are recovered;
// clone/submission virtual APIs and complete original table matching remain open.
class Gfx_NodeAttribute
{
public:
	Gfx_NodeAttribute() : owner(NULL) {}
	virtual ~Gfx_NodeAttribute() = 0;
	virtual void Update(float) {} // Actual second slot, shared5A9320 RET4.
	void* owner; // +04; borrowed node, initializedNULL.
	std::string name; // +08.
};

// A pure destructor still defines the ordinary base and string teardown.
inline Gfx_NodeAttribute::~Gfx_NodeAttribute()
{
}

class Gfx_ModelMesh : public Gfx_NodeAttribute
{
public:
	explicit Gfx_ModelMesh(DWORD value); // 0x49DDD0.
	virtual ~Gfx_ModelMesh(); // 0x50E3D0.
	void Initialize(BYTE stored, BYTE shadow); // 0x50E330.
	BYTE SetVertexData(DWORD fvf, DWORD stride, DWORD count, const void* vertices); // 0x50E760.
	BYTE SetPrimitives(DWORD count, const D3DPRIMITIVETYPE* types,
		const std::string* names, const DWORD* indexCounts,
		const DWORD* primitiveCounts, const void* const* indices); // 0x50E4F0.
	void SetMaterial(DWORD index, DWORD style, const std::string& name, DWORD unused, DWORD blend,
		const D3DMATERIAL8& values, DWORD textures, const std::string* names,
		const DWORD* handles, BYTE flagB9, BYTE flagBA); // 0x50E690; unconsumed return omitted.
	DWORD value24;
	BYTE flag28, flag29;
	void* ownedVertices;
	void* ownedIndices;
	Gfx_MeshData mesh;

private:
	Gfx_ModelMesh(const Gfx_ModelMesh&);
	Gfx_ModelMesh& operator=(const Gfx_ModelMesh&);
};
