#pragma once

#include "MeshData.hpp"
#include <list>
#include <map>

class Gfx_Viewport;
class Gfx_VertexBuffer;
class Gfx_HeightFog;
class Gfx_Light;
class Gfx_SkinMesh;
class Gfx_SkinMeshPkt;
class Gfx_DebugSphere505880;
class Octree2Gfx;

// Actual16-byte submission payload copied by0x493CD0. English source names
// remain descriptive; padding+05..07 is not initialized by that producer.
struct Gfx_RenderSubmission16
{
	Gfx_SkinMeshPkt* packet;
	BYTE flag;
	BYTE unknown05[3];
	DWORD value08, value0C;
};

// Descriptive source name. The observed 48-byte group owns copied instances;
// its mesh pointer remains borrowed. Real list copying supplies nested lifetime.
struct Gfx_RenderGroup
{
	Gfx_RenderGroup()
	{
		mesh = 0;
		for (DWORD i = 0; i < 6; ++i)
		{
			groupCounts[i] = 0;
		}
		vertexBase = 0;
		indexBase = 0;
	}

	Gfx_MeshData* mesh; // +00.
	std::list<Gfx_MeshInstance> instances; // +04; owns copies, not pointers.
	DWORD groupCounts[6]; // +10..27; wrapped material-pass totals.
	DWORD vertexBase, indexBase; // +28/+2C; later dynamic upload results.
};

// Actual120-byte owner under matching VC7, with real container members.
// Descriptive field names do not establish unrelated render consumers/startup.
class Gfx_RenderManager
{
public:
	explicit Gfx_RenderManager(Gfx_Viewport* owner); // 0x4950D0, including compiler member EH.
	~Gfx_RenderManager(); // 0x494F30; ordinary destructor, not owner storage deletion.
	void ReleaseHeightFog(); // 0x4903E0; deletes before clearing pointer/gate.
	HRESULT CreateHeightFog(D3DCOLOR color, float baseHeight, float baseRate,
		float baseAmplitude, float totalHeight, float totalRate, float totalAmplitude,
		float positionY, float maximumDensity); //490330; retains existing owner.
	BYTE EnableHeightFog(BYTE enabled); // 0x490400; raw byte, missing-owner warning/false.
	void ReleaseResources(); // 0x491F90; fog, quad buffer, then both sphere pools.
	void DeleteDebugSpheres(); // 0x491EC0; used pool before free pool.
	HRESULT SetViewportQuad(DWORD x, DWORD y, DWORD width, DWORD height); // 0x4901D0.
	void QueueMeshInstances(Gfx_MeshInstance* instances, UINT count); // 0x4948A0; void source hypothesis.
	void QueueSkinMesh(Gfx_SkinMeshPkt* packet, BYTE flag, DWORD value08, DWORD value0C); // 0x493CD0.
	void Upload(UINT pass, BYTE shadowOnly); // 0x4911B0; static bind and dynamic upload, then actual draw API.
	void DrawGroup(UINT pass, Gfx_RenderGroup* group, BYTE shadowOnly); // 0x490510; complete draw policy, explicit child APIs.
	void Render(BYTE clear); // 0x494CA0; incidental AL is not consumed by the viewport caller.
	void RenderSkins(UINT pass); // 0x4913E0; full state policy, with undefined skin child APIs.
	void Clear(); // 0x494B40; clears submissions, retaining borrowed assets and owner policy.
	void RecycleDebugSpheres(); // 0x4931E0; Reset, append borrowed pointer to free pool, erase used node.
	Gfx_DebugSphere505880* GetDebugSphere(); //4930F0; transfer free->used or construct8/16.
	void SubmitLight(Gfx_Light* light); // 0x493430, ordinary thiscall/one stack argument.
	void RenderProjectedShadows(); //491FC0; actual stencil volume/overlay policy.
	void RenderShadowVolumes2(); //4915E0; actual static/skin volume submission.
	void DrawShadowOverlay(); //490150; actual viewport quad, unused SDK HRESULT.
	void RenderDebugLights(); //491B10; streamed point sprites, no sphere dependency.

	Gfx_Viewport* owner; // +00.
	std::map<DWORD, Gfx_RenderGroup> staticGroups[1]; // +04; actual one-element array.
	std::list<Gfx_RenderGroup> dynamicGroups[1]; // +10; actual one-element array.
	// Queue indexes both arrays unchecked by mesh.value00. They remain count1;
	// the following members are different owners, not additional categories.
	std::map<DWORD, Gfx_SkinMesh*> skinMeshes; // +1C; borrowed assets by handle.
	std::map<DWORD, std::list<Gfx_RenderSubmission16> > skinSubmissions; // +28.
	Gfx_VertexBuffer* quadBuffer; // +34; owned, lazy acquisition0x4901D0.
	BYTE projectedShadows; // +38; gates the complete projected-skin child.
	BYTE unknown39[3];
	D3DCOLOR shadowColor; // +3C; texture factor, initialized80000000 last.
	BYTE flag40; // +40; raw material-selection gate, source name unresolved.
	BYTE unknown41[3];
	Gfx_HeightFog* heightFog; // +44; owned; draw passes these pointer bits onward.
	BYTE flag48; // +48; raw material-selection gate, source name unresolved.
	BYTE flag49; // +49; gates the borrowed floor callback.
	BYTE unknown4A[2];
	Octree2Gfx* floor; // +4C; borrowed, not deleted by this owner.
	std::map<Gfx_Light*, DWORD> debugLights; // +50; borrowed light keys, four-byte flags.
	std::list<Gfx_DebugSphere505880*> freeDebugSpheres; // +5C; owns pointed objects.
	std::list<Gfx_DebugSphere505880*> usedDebugSpheres; // +68; owns pointed objects.
	BYTE flag74;
	BYTE unknown75[3];

private:
	Gfx_RenderManager(const Gfx_RenderManager&);
	Gfx_RenderManager& operator=(const Gfx_RenderManager&);
};
