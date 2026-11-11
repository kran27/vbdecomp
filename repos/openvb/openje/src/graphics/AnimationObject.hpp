#pragma once

#include "GrannyRuntime21.hpp"
#include <windows.h>
#include <map>
#include <vector>
#include <list>

class Gfx_AnimationData;
struct Gfx_AnimationCue;

// Original semantic-name table6FC0C0. The helpers are source adapters over that
// table; they do not map a B3D ordinal to a Granny ordinal.
extern const char* const g_GfxSemanticBoneNames[212];
DWORD Gfx_FindSemanticBone(const char* name); // Exact comparison;212 means absent.

class Gfx_AnimationObject
{
public:
	Gfx_AnimationObject(); // 0x4F45E0; real map and zeroed semantic indices.
	~Gfx_AnimationObject(); // 0x4A4C10; groups before skeleton file.
	BYTE LoadSkeletonFromBuffer(int size, const void* buffer, float scale); // 0x4A2280.
	void Unload(); // 0x4A1F30; does not clear animation groups or semantic indices.
	void ClearAnimationGroups(); // 0x4A4BA0.
	void DestroyAnimationGroup(DWORD group); // 0x4A49D0; leaves the empty group entry.
	BYTE LoadAnimationFileFromBuffer(DWORD group, int size, const void* buffer, float scale); // 0x4A4A70.
	void ClearAnimationCues(DWORD group, DWORD index); // 0x4A4820.
	void AddAnimationCue(DWORD group, DWORD index, float time, const char* text); // 0x4A4880.
	BYTE HasAnimation(DWORD group, DWORD index, DWORD subAnimation) const; // 0x467780.
	float GetDuration(DWORD group, DWORD index, DWORD subAnimation); // 0x4A26D0, ST0.
	Granny21::Control* ApplyAnimation(Granny21::ModelInstance* instance, Granny21::TrackMask* mask,
		DWORD group, DWORD index, DWORD subAnimation, float startTime); // 0x4A2750.
	BYTE ProcessAnimationCues(DWORD group, DWORD index, float seconds, BYTE reverse,
		float** position, std::list<Gfx_AnimationCue>& output); //0x4A39B0 ->Data4A3360.
	void ResolveGrannyBoneIndices(); // 0x4A1FA0.
	Granny21::ModelInstance* CreateModelInstance(); // 0x4A1F80.
	void UpdateWorldPose(Granny21::ModelInstance* instance, Granny21::WorldPose* result,
		const float* offset); // 0x4A2070; uses this object's shared local pose.

	Granny21::File* file; // +00; owned runtime file.
	Granny21::FileInfo* fileInfo; // +04; borrowed from file.
	Granny21::Model* model; // +08; borrowed from file.
	Granny21::Skeleton* skeleton; // +0C; borrowed from file.
	Granny21::Mesh* collisionMesh; // +10; borrowed, optional.
	Granny21::MeshBinding* meshBinding; // +14; owned runtime binding.
	float scale; // +18.
	Granny21::LocalPose* localPose; // +1C; owned, shared sampling scratch.
	int semanticBoneIndices[212]; // +20..36F; initialized0, not-1.
	std::map<DWORD, std::vector<Gfx_AnimationData*> > animationGroups; // +370 VC7.

private:
	Gfx_AnimationObject(const Gfx_AnimationObject&);
	Gfx_AnimationObject& operator=(const Gfx_AnimationObject&);
};
