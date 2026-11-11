#pragma once

#include <windows.h>
#include <map>
#include <deque>
#include <list>
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

class Gfx_BufferRecord;
class Gfx_MeshData;
class Gfx_BufferManager;
extern Gfx_BufferManager* g_BufferManager; // 0x707CE0; one loader-zero pointer.

// Source lifetime adapter: original publication precedes all container members,
// and singleton clearing follows their reverse destruction, including unwind.
class Gfx_BufferManagerPublication
{
protected:
	Gfx_BufferManagerPublication();
	~Gfx_BufferManagerPublication();
};

// Descriptive name for original 80-byte owner constructed at 0x507810.
// Actual known owners/containers. Live +2C operations remain explicit unresolved
// boundaries; no concrete class or missing virtual signature is fabricated.
class Gfx_BufferManager : private Gfx_BufferManagerPublication
{
	friend class Gfx_BufferManagerPublication;
public:
	Gfx_BufferManager(); // 0x507810; actual publication, members and stream owners.
	~Gfx_BufferManager(); // 0x507580; validated domain requires unknown2C==NULL.
	void ReleaseAll(BYTE destroy); // 0x506B70; live unknown2C branch unresolved.
	void Restore(); // 0x506890; InitializeStreams followed by borrowed visits.
	void InitializeStreams(BYTE omitDoNotClip); // 0x5067C0; BYTE name describes observed usage-bit policy.
	HRESULT BindVertexData(DWORD handle); // 0x506AE0 -> 0x506760.
	Gfx_BufferRecord* FindStoredBuffer(DWORD handle); //506B30; borrowed record, FFFFFFFF or missing returns NULL.
	DWORD AllocateHandle(); // 0x506C10; reusable deque handles before monotonic IDs.
	bool CreateStoredBuffers(Gfx_MeshData* mesh); // 0x507660; publishes even on failed upload.
	void ReleaseStoredBuffers(BYTE destroy); // 0x506A10; leaves high-water values unchanged.
	void ReleaseStoredHandle(DWORD handle); // 0x5077A0; no recycled-handle publication.

	DWORD usage; // +00; 0x20(D3DUSAGE_DONOTCLIP), or0 for nonzero input.
	DWORD nextHandle; // +04; starts1, FFFFFFFE is the exhausted sentinel.
	std::deque<DWORD> freeHandles; // +08..1B; reused from the back, actual VC7 deque.
	std::map<DWORD, Gfx_BufferRecord*> records; // +1C; actual pointer-valued handle map.
	Gfx_StreamingVertexBuffer* vertexStream; // +28; actual owned wrapper.
	void* unknown2C; // +2C; original constructor writesNULL, concrete type unresolved.
	Gfx_StreamingIndexBuffer* indexStream; // +30; actual owned wrapper.
	std::list<void*> devicePayloads; // +34..3F; owns nodes, borrows opaque payloads.
	DWORD vertexBytes, indexBytes; // +40/+44; wrapped totals, even on Create failure.
	DWORD vertexHighWater, indexHighWater; // +48/+4C; unsigned comparison after accumulation.

private:
	Gfx_BufferManager(const Gfx_BufferManager&);
	Gfx_BufferManager& operator=(const Gfx_BufferManager&);
};

inline Gfx_BufferManagerPublication::Gfx_BufferManagerPublication()
{
	g_BufferManager = static_cast<Gfx_BufferManager*>(this);
}

inline Gfx_BufferManagerPublication::~Gfx_BufferManagerPublication()
{
	g_BufferManager = NULL;
}

// Observed404B70 is a receiver-unused one-byte RET. Only pointer transport is
// established; neither a payload class nor its producer/lifetime is invented.
void __fastcall Gfx_BufferDeviceChange(void* payload);
