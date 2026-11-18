#pragma once

#include "GrannyRuntime28.hpp"
#include <windows.h>
#include <map>
#include <list>
#include <string>
#include <vector>

struct Gfx_AnimationCue;

// One real shared std::string[4] owner at6FADB0; original name/linkage unproved.
extern std::string g_GfxReservedAnimationNames[4];

// Actual28-byte VC7 animation-file owner4A4970. Cue timing is a real map of
// string lists, interpreted by4A3360 into caller-owned cue records/cursor.
class Gfx_AnimationData
{
public:
	Gfx_AnimationData(); // 0x4A4970.
	~Gfx_AnimationData(); // 0x4A4910; file before cue members.
	void ClearAnimationCues(); // 0x4A4730; actual owned cue-map boundary.
	void AddAnimationCue(float time, const char* text); // 0x4A4760.
	BYTE LoadAnimationDataFromBuffer(int size, const void* buffer, float scale); // 0x4A2110.
	BYTE ProcessAnimationCues(DWORD group, DWORD index, float seconds, BYTE reverse,
		float** position, std::list<Gfx_AnimationCue>& output); //0x4A3360.

	Granny28::File* file;
	Granny28::FileInfo* fileInfo;
	DWORD trackGroupIndex;
	float scale;
	std::map<float, std::list<std::string> > cues; // +10; real owned strings.
	std::vector<BYTE> fileBytes; // Mutable input retained while Granny owns the file.

private:
	Gfx_AnimationData(const Gfx_AnimationData&);
	Gfx_AnimationData& operator=(const Gfx_AnimationData&);
};
