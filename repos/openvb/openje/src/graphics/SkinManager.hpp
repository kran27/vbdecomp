#pragma once

#include <windows.h>
#include <deque>
#include <map>

class Gfx_SkinMesh;
class Gfx_SkinMeshManager;
extern Gfx_SkinMeshManager* g_SkinMeshManager; //Pointer0x707CE8; host storage is not manager construction.

// Ordinary source adapter for publication before members and clearing after
// reverse member destruction. Original inheritance/source spelling is unknown.
class Gfx_SkinMeshManagerPublication
{
protected:
	Gfx_SkinMeshManagerPublication();
	~Gfx_SkinMeshManagerPublication();
};

// Actual nonvirtual36-byte owner with real VC7 containers. Mesh pointers are
// borrowed, including NULL; only the deque/map storage belongs to this owner.
class Gfx_SkinMeshManager : private Gfx_SkinMeshManagerPublication
{
	friend class Gfx_SkinMeshManagerPublication;
public:
	Gfx_SkinMeshManager(); // 0x50B6B0; publishes before nextHandle/containers.
	~Gfx_SkinMeshManager(); // 0x50B5E0; never deletes/dereferences mapped meshes.
	DWORD Register(Gfx_SkinMesh* mesh); // 0x50B660; duplicate key overwrites its pointer.
	Gfx_SkinMesh* Lookup(DWORD handle); // 0x50B520; borrowed result, NULL if absent.
	void Remove(DWORD handle); // 0x50B740; by value, erase before recycling local.
	void Visit(); // 0x50B4F0; calls shared empty device boundary in key order.
	DWORD AllocateHandle(); // 0x50B560; deque BACK first, FFFFFFFF fresh exhaustion.
	void RecycleHandle(DWORD& handle); // 0x5CB5A0; push BACK before clearing reference.

	DWORD nextHandle; // +00.
	std::deque<DWORD> freeHandles; // +04..17; actual VC7 deque.
	std::map<DWORD, Gfx_SkinMesh*> entries; // +18..23; borrowed pointees.

private:
	Gfx_SkinMeshManager(const Gfx_SkinMeshManager&);
	Gfx_SkinMeshManager& operator=(const Gfx_SkinMeshManager&);
};

inline Gfx_SkinMeshManagerPublication::Gfx_SkinMeshManagerPublication()
{
	g_SkinMeshManager = static_cast<Gfx_SkinMeshManager*>(this);
}

inline Gfx_SkinMeshManagerPublication::~Gfx_SkinMeshManagerPublication()
{
	g_SkinMeshManager = NULL;
}

// ABI adapter for the observed shared0x404B70 one-byte RET: ECX carries an
// opaque pointer, including NULL, with no access or stack arguments. Its source
// name/fastcall spelling is unproven. No SkinMesh layout/lifetime or extra game
// operation is supplied. Keep its ordinary definition in a separate class TU.
void __fastcall Gfx_SkinMeshDeviceChange(Gfx_SkinMesh* mesh);
