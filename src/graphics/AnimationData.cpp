#include "AnimationData.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <string.h>

Gfx_AnimationData::Gfx_AnimationData()
	: file(NULL), fileInfo(NULL), trackGroupIndex(0), scale(1.0f)
{
}

Gfx_AnimationData::~Gfx_AnimationData()
{
	fileInfo = NULL;
	if (file)
	{
		Granny28::GrannyFreeFile(file);
		file = NULL;
	}
	fileBytes.clear();
	// Real cue map/list/string members are destroyed after the runtime file.
}

void Gfx_AnimationData::ClearAnimationCues() //4A4730.
{
	cues.clear();
}

void Gfx_AnimationData::AddAnimationCue(float time, const char* text) //4A4760.
{
	std::list<std::string>& values = cues[time];
	values.push_back(std::string(text));
}

BYTE Gfx_AnimationData::LoadAnimationDataFromBuffer(int size, const void* buffer, float value)
{
	if (!size)
	{
		JE::LogDebugString("Gfx_AnimationData::LoadAnimationDataFromBuffer - 0 buffersize!\n");
		return 0;
	}
	if (!buffer)
	{
		JE::LogDebugString("Gfx_AnimationData::LoadAnimationDataFromBuffer - NULL buffer\n");
		return 0;
	}
	if (value <= 0.0f)
	{
		JE::LogDebugString("Gfx_AnimationData::LoadAnimationDataFromBuffer - invalid scale: %f\n", value);
		return 0;
	}
	if (file)
	{
		JE::LogDebugString("Gfx_AnimationData::LoadAnimationDataFromBuffer - This object already contains animation data; unload it first!\n");
		return 0;
	}
	// Granny 2.8 accepts mutable memory. Keep the copy alive with the file.
	fileBytes.assign(static_cast<const BYTE*>(buffer), static_cast<const BYTE*>(buffer) + size);
	file = Granny28::GrannyReadEntireFileFromMemory(size, fileBytes.data());
	if (!file)
	{
    	fileBytes.clear();
		JE::LogDebugString("Gfx_AnimationData::LoadAnimationDataFromBuffer - Failed!");
		return 0;
	}
	fileInfo = Granny28::GrannyGetFileInfo(file);
	Gfx_TransformGrannyFileToLHS(fileInfo, value);
	scale = value;
	if (!fileInfo->AnimationCount || !fileInfo->Animations || !fileInfo->TrackGroupCount)
	{
		JE::LogDebugString("Gfx_AnimationData::LoadAnimationDataFromBuffer - NO ANIMATION DATA (AnimCount = 0 or Animations = NULL or TrackGroupCount = 0)!\n");
		return 0;
	}
	trackGroupIndex = 0;
	while (trackGroupIndex < static_cast<DWORD>(fileInfo->TrackGroupCount))
	{
		const char* name = fileInfo->TrackGroups[trackGroupIndex]->Name;
		DWORD excluded = 0;
		for (; excluded < 4; ++excluded)
			if (!g_GfxReservedAnimationNames[excluded].compare(0,
				g_GfxReservedAnimationNames[excluded].size(), name)) break;
		if (excluded == 4) return 1;
		++trackGroupIndex;
	}
	JE::LogDebugString("Gfx_AnimationData::LoadAnimationDataFromBuffer - NO VALID ANIMATION DATA!");
	return 0;
}

#if defined(_MSC_VER) && _MSC_VER == 1300
typedef char AnimationDataSize[(sizeof(Gfx_AnimationData) == 28) ? 1 : -1];
#endif
