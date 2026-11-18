#include "AnimationInstance.hpp"
#include <string.h>

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
	boneCount = Granny28::GrannyGetSourceSkeleton(modelInstance)->BoneCount;
	worldPose = Granny28::GrannyNewWorldPose(boneCount);
	trackMask = Granny28::GrannyNewTrackMask(1.0f, boneCount);
	worldMatrices = new D3DXMATRIX[boneCount];
	compositeMatrices = new D3DXMATRIX[boneCount];
	Granny28::Mesh* collision = animationObject->collisionMesh;
	if (collision)
	{
		const DWORD count = static_cast<DWORD>(collision->BoneBindingCount);
		collisionBindingFlags = new BYTE[count]{};
	}
}

int Gfx_AnimationInstance::GetLoopCount() const
{
	int result = -1;
	if (control) result = Granny28::GrannyGetControlLoopCount(control);
	else if (nextControl) result = Granny28::GrannyGetControlLoopCount(nextControl);
	return result;
}
