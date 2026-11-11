#pragma once

#include <windows.h>
#include <list>
#include <map>
#include <vector>
#include <string>
#include <d3d8.h>

class Gfx_Texture;
class Gfx_TextureManager;
extern Gfx_TextureManager* g_TextureManager; // Original loader-zero pointer70B200.

// Source adapter for the observed publication before member construction and
// clearing after member destruction, including constructor-failure cleanup.
// The original base/source spelling is unknown; this empty nonvirtual lifetime
// adds no storage or independent proprietary operation.
class Gfx_TextureManagerPublication
{
protected:
	Gfx_TextureManagerPublication();
	~Gfx_TextureManagerPublication();
};

// Original nonvirtual112-byte owner; source names are hypotheses. Real member
// offsets and early publication are retained in texture-manager-contract-v1.
// The real STL members own their storage; entries owns the texture pointers.
class Gfx_TextureManager : private Gfx_TextureManagerPublication
{
	friend class Gfx_TextureManagerPublication;
public:
	Gfx_TextureManager(); // 0x5211A0; publishes before member construction.
	~Gfx_TextureManager(); // 0x520FC0; clears textures before member destruction.
	Gfx_Texture* Lookup(DWORD handle); // 0x51F160; borrowed, no reference change.
	BYTE CopyTextureRect(DWORD destinationHandle, UINT destinationMip,
		const RECT* destinationRect, DWORD sourceHandle, UINT sourceMip,
		const RECT* sourceRect); //51F1A0; both lookups precede null tests.
	void Clear(); // 0x520A60; preserves nextHandle and recycled handles.
	DWORD ReleaseVideoResources(); //51F340; clear frames recorded in the default-pool map.
	DWORD RestoreVideoResources(); //51F3E0; recreate from existing system-copy metadata.
	void ClearPool(std::map<DWORD, std::list<int> >& pool); // 0x520890.
	void ClearObjects(); // 0x51F200; deletes before clearing the handle map.
	BYTE AcquireByName(DWORD* output, const std::string& name); // 0x51F570.
	DWORD AllocateHandle(); // 0x51F680; LIFO recycling, FFFFFFFF exhaustion.
	void RecycleHandle(DWORD* handle); // 0x520770; valid handles zeroed after scan.
	BYTE Unregister(DWORD handle); // 0x520960; removes mapping, not the pointee.
	HRESULT Register(DWORD* output, Gfx_Texture* texture); // 0x5212E0; takes ownership.
	HRESULT AddTexture(DWORD* output, Gfx_Texture* source, D3DPOOL pool); // 0x5218E0; clones, borrows source.
	HRESULT CreateAndAddTexture(DWORD* output, UINT width, UINT height, UINT levels,
		D3DFORMAT format, D3DPOOL pool); // 0x521DB0/5C08E0.
	DWORD Release(DWORD handle); // 0x520AB0; signed test after DWORD decrement.
	HRESULT LoadTextureFromBuffer(DWORD* output, std::string name, const BYTE* data,
		UINT size, D3DFORMAT format, D3DPOOL pool, UINT levels, DWORD filter,
		DWORD mipFilter, DWORD usage); // 0x521CA0; name acquisition before decode.
	HRESULT LoadTextureFromFile(DWORD* output, std::string path, D3DFORMAT format,
		D3DPOOL pool, UINT levels, DWORD filter, DWORD mipFilter, DWORD usage); // 0x521B10.
	BYTE IsCubeTexture(const std::string& path); // 0x51EF10; thiscall, ECX from521B52.

	std::map<DWORD, std::list<int> > defaultFrames; // +00.
	std::map<DWORD, std::list<int> > managedFrames; // +0C.
	std::map<DWORD, std::list<int> > systemFrames; // +18.
	std::map<DWORD, Gfx_Texture*> entries; // +24.
	DWORD nextHandle; // +30.
	std::vector<DWORD> freeHandles; // +34..43; actual VC7 vector.
	DWORD defaultFrameCount, managedFrameCount, systemFrameCount; // +44..4C.
	DWORD textureCount; // +50.
	DWORD defaultFirstMipSize, defaultTotalSize; // +54/+58.
	DWORD managedFirstMipSize, managedTotalSize; // +5C/+60.
	DWORD systemFirstMipSize, systemTotalSize, totalSize; // +64/+68/+6C.
};

// Define after the complete owner so the ordinary nonvirtual downcast can form
// its pointer without reading any unconstructed member. Limited test lifetimes
// use this same actual publication policy; no successful game children follow.
inline Gfx_TextureManagerPublication::Gfx_TextureManagerPublication()
{
	g_TextureManager = static_cast<Gfx_TextureManager*>(this);
}

inline Gfx_TextureManagerPublication::~Gfx_TextureManagerPublication()
{
	g_TextureManager = NULL;
}

