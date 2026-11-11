#include "AnimationInstance.hpp"
#include <string.h>

namespace Granny21
{
	extern "C" __declspec(dllimport) int __stdcall GrannyGetControlLoopCount(Control* control);
}

Gfx_AnimationInstance::Gfx_AnimationInstance(const Gfx_AnimationInstance& source)
	: currentTime(0), timeOffset(0), playing(0), animationObject(source.animationObject),
	parent(NULL), parentBone(1), scale(1.0f, 1.0f, 1.0f), scaling(0), control(NULL), nextControl(NULL),
	sourceCompositeMatrices(NULL), group(DWORD(-1)), animationIndex(DWORD(-1)),
	value54(DWORD(-1)), value58(DWORD(-1)), value5C(DWORD(-1)), value60(DWORD(-1)),
	value64(0), flag68(0), value6C(0), pendingTransition(0), nextGroup(DWORD(-1)),
	nextAnimationIndex(DWORD(-1)), value7C(DWORD(-1)), value80(0), flag84(0),
	nextLoopState(0), speed(1), reverse(0), remaining(0), nextRemaining(0), loopState(0),
	cues(source.cues), nextCuePosition(NULL), collisionBindingFlags(NULL), flagC4(source.flagC4)
{
	//0x4A6880 copies only the borrowed AnimationObject, value cue list and flagC4.
	//Controls, pose buffers, scale and cue cursors are fresh; no sampled matrices are shared.
	modelInstance = animationObject->CreateModelInstance();
	boneCount = Granny21::GrannyGetSourceSkeleton(modelInstance)->boneCount;
	worldPose = Granny21::GrannyNewWorldPose(boneCount);
	trackMask = Granny21::GrannyNewTrackMask(1.0f, boneCount);
	worldMatrices = new D3DXMATRIX[boneCount];
	compositeMatrices = new D3DXMATRIX[boneCount];
	Granny21::Mesh* collision = animationObject->collisionMesh;
	if (collision)
	{
		DWORD count;
		memcpy(&count, reinterpret_cast<const BYTE*>(collision) + 0x1C, 4);
		collisionBindingFlags = new BYTE[count];
		memcpy(&count, reinterpret_cast<const BYTE*>(collision) + 0x1C, 4);
		memset(collisionBindingFlags, 0, count);
	}
}

int Gfx_AnimationInstance::GetLoopCount() const
{
	int result = -1;
	if (control) result = Granny21::GrannyGetControlLoopCount(control);
	else if (nextControl) result = Granny21::GrannyGetControlLoopCount(nextControl);
	return result;
}
