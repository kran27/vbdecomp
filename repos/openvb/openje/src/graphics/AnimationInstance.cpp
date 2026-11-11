#include "AnimationInstance.hpp"
#include "D3DState.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <string.h>
#include <stdexcept>

namespace Granny21
{
	extern "C"
	{
		__declspec(dllimport) float __stdcall GrannyGetControlDuration(const Control* control);
		__declspec(dllimport) float __stdcall GrannyGetControlDurationLeft(Control* control);
		__declspec(dllimport) bool __stdcall GrannyControlIsComplete(const Control* control);
		__declspec(dllimport) float __stdcall GrannyGetControlClock(const Control* control);
		__declspec(dllimport) void __stdcall GrannySetControlLoopCount(Control* control, int count);
		__declspec(dllimport) void __stdcall GrannySetControlSpeed(Control* control, float speed);
		__declspec(dllimport) float __stdcall GrannyEaseControlIn(Control* control, float duration, BYTE fromCurrent);
		__declspec(dllimport) float __stdcall GrannyEaseControlOut(Control* control, float duration);
		__declspec(dllimport) void __stdcall GrannyCompleteControlAt(Control* control, float time);
		__declspec(dllimport) void __stdcall GrannyFreeControlOnceUnused(Control* control);
	}
}

LONG Gfx_AnimationInstance::GetSemanticBoneIndex(LONG bone) const //49C9E0.
{
	if (bone < 212) return animationObject->semanticBoneIndices[bone];
	return 0; //No negative guard in the original.
}

D3DXMATRIX* Gfx_AnimationInstance::GetBoneTransform(LONG bone) const //49CA00.
{
	return worldMatrices + GetSemanticBoneIndex(bone);
}

void Gfx_AnimationInstance::SetScale(float x, float y, float z)
{
	scale[0] = x; scale[1] = y; scale[2] = z;
	scaling = !(x == 1.0f && y == 1.0f && z == 1.0f);
}

Gfx_AnimationInstance::Gfx_AnimationInstance(Gfx_AnimationObject* object)
	: currentTime(0.0f), timeOffset(0.0f), playing(0), animationObject(object),
	parent(NULL), parentBone(1), scale(1.0f, 1.0f, 1.0f), scaling(0), control(NULL), nextControl(NULL),
	sourceCompositeMatrices(NULL), group(DWORD(-1)), animationIndex(DWORD(-1)),
	value54(DWORD(-1)), value58(DWORD(-1)), value5C(DWORD(-1)), value60(DWORD(-1)),
	value64(0.0f), flag68(0), value6C(0), pendingTransition(0), nextGroup(DWORD(-1)),
	nextAnimationIndex(DWORD(-1)), value7C(DWORD(-1)), value80(0.0f), flag84(0),
	nextLoopState(0), speed(1.0f), reverse(0), remaining(0.0f), nextRemaining(0.0f),
	loopState(0), nextCuePosition(NULL), collisionBindingFlags(NULL), flagC4(0)
{
	modelInstance = animationObject->CreateModelInstance();
	boneCount = Granny21::GrannyGetSourceSkeleton(modelInstance)->boneCount;
	worldPose = Granny21::GrannyNewWorldPose(boneCount);
	trackMask = Granny21::GrannyNewTrackMask(1.0f, boneCount);
	worldMatrices = new D3DXMATRIX[boneCount];
	compositeMatrices = new D3DXMATRIX[boneCount];
	Granny21::Mesh* collision = animationObject->collisionMesh;
	if (collision)
	{
		// Public Granny2.1 Mesh prefix field+1C is BoneBindingCount. The original
		//4A5D33 allocates exactly this many zeroed bytes, not vertexCount bytes.
		DWORD count;
		memcpy(&count, reinterpret_cast<const BYTE*>(collision) + 0x1C, 4);
		collisionBindingFlags = new BYTE[count];
		memcpy(&count, reinterpret_cast<const BYTE*>(collision) + 0x1C, 4);
		memset(collisionBindingFlags, 0, count);
	}
}

Gfx_AnimationInstance::~Gfx_AnimationInstance()
{
	if (collisionBindingFlags)
	{
		delete[] collisionBindingFlags;
		collisionBindingFlags = NULL;
	}
	if (worldMatrices)
	{
		delete[] worldMatrices;
		worldMatrices = NULL;
	}
	if (compositeMatrices)
	{
		delete[] compositeMatrices;
		compositeMatrices = NULL;
	}
	if (control) Granny21::GrannyFreeControl(control);
	if (nextControl) Granny21::GrannyFreeControl(nextControl);
	if (trackMask) Granny21::GrannyFreeTrackMask(trackMask);
	if (worldPose) Granny21::GrannyFreeWorldPose(worldPose);
	if (modelInstance) Granny21::GrannyFreeModelInstance(modelInstance);
	for (std::vector<float*>::iterator position = cuePositions.begin();
		position != cuePositions.end(); ++position)
	{
		if (*position) delete *position;
	}
	if (nextCuePosition) delete nextCuePosition;
	// Normal reverse member destruction clears the real cue list and vector.
}

void Gfx_AnimationInstance::ResetCuePosition(DWORD index)
{
	if (index < cuePositions.size())
	{
		float*& position = cuePositions[index];
		if (position)
		{
			delete position;
			position = NULL;
		}
	}
}

float** Gfx_AnimationInstance::GetCuePosition(DWORD index)
{
	if (index >= cuePositions.size())
	{
		DWORD first = cuePositions.size();
		cuePositions.resize(index + 1, NULL);
		for (; first < cuePositions.size(); ++first) cuePositions[first] = NULL;
	}
	return &cuePositions[index];
}

void Gfx_AnimationInstance::PromoteNextControl()
{
	if (nextCuePosition)
	{
		float** position = GetCuePosition(nextGroup);
		if (!*position) *position = new float;
		memcpy(*position, nextCuePosition, 4);
		delete nextCuePosition;
		nextCuePosition = NULL;
	}
	else
	{
		ResetCuePosition(nextGroup);
	}
}

BYTE Gfx_AnimationInstance::SelectAnimation(DWORD requestedGroup, DWORD index,
	DWORD subAnimation, float blendSeconds, BYTE fromCurrent, int loopCount)
{
	if (animationObject->HasAnimation(requestedGroup, index, subAnimation))
	{
		value64 = blendSeconds;
		value60 = subAnimation;
		value58 = requestedGroup;
		value5C = index;
		flag68 = fromCurrent;
		value6C = loopCount;
		pendingTransition = 1;
		return 1;
	}
	return 0;
}

void Gfx_AnimationInstance::SetSpeed(float value)
{
	if (control) Granny21::GrannySetControlSpeed(control, value);
	if (nextControl) Granny21::GrannySetControlSpeed(nextControl, value);
	speed = value;
}

void Gfx_AnimationInstance::ProcessCuesUntil(float seconds)
{
	Granny21::Control* capturedCurrent = control;
	float offset = timeOffset, current = seconds;
	Granny21::Control* capturedNext = nextControl;
	DWORD capturedIndex = animationIndex, capturedGroup = group;
	if (capturedCurrent && (!loopState || remaining > 0.0f))
	{
		if (!reverse) current = offset + seconds;
		else current = Granny21::GrannyGetControlDuration(capturedCurrent) - (offset + seconds);
		Granny21::GrannyControlIsComplete(control); // Original call, result ignored.
	}
	float next = seconds;
	if (capturedNext && reverse) next = Granny21::GrannyGetControlDuration(capturedNext) - seconds;
	if (control)
	{
		float** position = GetCuePosition(capturedGroup);
		animationObject->ProcessAnimationCues(capturedGroup, capturedIndex, current, reverse, position, cues);
	}
	if (nextControl)
		animationObject->ProcessAnimationCues(nextGroup, nextAnimationIndex, next, reverse, &nextCuePosition, cues);
}

void Gfx_AnimationInstance::CompleteCurrentControl(float value, BYTE ease)
{
	if (control)
	{
		if (ease)
		{
			Granny21::GrannyCompleteControlAt(control, Granny21::GrannyEaseControlOut(control, value));
		}
		else
		{
			Granny21::GrannyCompleteControlAt(control, value);
		}
	}
}

void Gfx_AnimationInstance::PrepareTransition()
{
	if (nextControl)
	{
		if (control) Granny21::GrannyFreeControl(control);
		control = nextControl;
		remaining = nextRemaining;
		loopState = nextLoopState;
		animationIndex = nextAnimationIndex;
		group = nextGroup;
		value54 = value7C;
		nextLoopState = 0;
		nextRemaining = 0.0f;
		currentTime = 0.0f;
		timeOffset = 0.0f;
		nextControl = NULL;
		if (nextCuePosition)
		{
			float** position = GetCuePosition(group);
			if (!*position) *position = new float;
			memcpy(*position, nextCuePosition, 4);
			delete nextCuePosition;
			nextCuePosition = NULL;
		}
		else
		{
			ResetCuePosition(group);
		}
	}
	if (!control)
	{
		timeOffset = 0.0f;
	}
	else if (loopState && remaining <= 0.0f)
	{
		timeOffset = 0.0f;
		Granny21::GrannyFreeControl(control);
		control = NULL;
	}
	else
	{
		timeOffset = Granny21::GrannyGetControlClock(control);
		if (timeOffset < 0.0f) timeOffset = 0.0f;
	}
	nextControl = animationObject->ApplyAnimation(modelInstance, trackMask, value58, value5C, value60, 0.0f);
	nextGroup = value58;
	value80 = value64;
	nextAnimationIndex = value5C;
	value7C = value60;
	flag84 = flag68;
	nextLoopState = value6C;
	if (nextCuePosition)
	{
		delete nextCuePosition;
		nextCuePosition = NULL;
	}
	if (nextControl)
	{
		Granny21::GrannySetControlLoopCount(nextControl, nextLoopState);
		Granny21::GrannySetControlSpeed(nextControl, speed);
		if (!(value80 > 0.0f))
		{
			CompleteCurrentControl(timeOffset, 0);
		}
		else
		{
			Granny21::GrannyEaseControlIn(nextControl, value80, flag84);
			CompleteCurrentControl(value80, 1);
		}
		if (reverse)
		{
			float duration = Granny21::GrannyGetControlDuration(nextControl);
			nextRemaining = duration - Granny21::GrannyGetControlDurationLeft(nextControl);
		}
		else
		{
			nextRemaining = Granny21::GrannyGetControlDurationLeft(nextControl);
		}
		Granny21::GrannyFreeControlOnceUnused(nextControl);
	}
	else
	{
		JE::LogDebugString("Invalid Animation: Group: %d Anim: %d SubAnim: %d\n", nextGroup, nextAnimationIndex, value7C);
	}
}

void Gfx_AnimationInstance::ProcessAnimationCues(DWORD requestedGroup, DWORD index,
	float seconds, float** position)
{
	animationObject->ProcessAnimationCues(requestedGroup, index, seconds, reverse, position, cues);
}

void Gfx_AnimationInstance::ClearCues() //4A5F20.
{
	cues.clear();
}

void Gfx_AnimationInstance::Update(float seconds)
{
	if (pendingTransition)
	{
		PrepareTransition();
		pendingTransition = 0;
	}
	float controlTime = seconds;
	currentTime = seconds;
	bool complete = false;
	if (control && (!loopState || remaining > 0.0f))
	{
		if (reverse)
		{
			controlTime = Granny21::GrannyGetControlDuration(control) - (seconds + timeOffset);
			Granny21::GrannySetControlClock(control, controlTime);
			remaining = controlTime;
		}
		else
		{
			controlTime = seconds + timeOffset;
			Granny21::GrannySetControlClock(control, controlTime);
			remaining = Granny21::GrannyGetControlDurationLeft(control);
		}
		complete = Granny21::GrannyControlIsComplete(control);
	}
	float nextTime = seconds;
	if (nextControl)
	{
		float duration = Granny21::GrannyGetControlDuration(nextControl);
		if (reverse) nextTime = duration - seconds;
		Granny21::GrannySetControlClock(nextControl, nextTime);
		if (reverse)
		{
			nextRemaining = duration - Granny21::GrannyGetControlDurationLeft(nextControl);
		}
		else
		{
			nextRemaining = Granny21::GrannyGetControlDurationLeft(nextControl);
		}
	}
	if (control && (complete || (loopState && remaining <= 0.0f)))
	{
		if (complete || nextControl)
		{
			Granny21::GrannyFreeControl(control);
			control = nextControl;
			remaining = nextRemaining;
			loopState = nextLoopState;
			nextLoopState = 0;
			nextRemaining = 0.0f;
			nextControl = NULL;
			//4A6416 jumps directly to4A64A3 when a finite control holds its
			//last pose without a successor. Only actual replacement transfers
			//or resets the cue cursor at4A6456..4A649E.
			PromoteNextControl();
		}
		group = nextGroup;
		animationIndex = nextAnimationIndex;
		value54 = value7C;
		currentTime = 0.0f;
		timeOffset = 0.0f;
		if (reverse)
		{
			controlTime = Granny21::GrannyGetControlDuration(control) - (seconds + timeOffset);
		}
		else
		{
			controlTime = seconds;
		}
	}
	else if (nextControl && !control)
	{
		control = nextControl;
		remaining = nextRemaining;
		loopState = nextLoopState;
		nextLoopState = 0;
		nextRemaining = 0.0f;
		nextControl = NULL;
		group = nextGroup;
		animationIndex = nextAnimationIndex;
		value54 = value7C;
		currentTime = 0.0f;
		timeOffset = 0.0f;
		if (reverse)
		{
			controlTime = Granny21::GrannyGetControlDuration(control) - (seconds + timeOffset);
		}
		else
		{
			controlTime = seconds;
		}
		// In this no-current-control arm, the original publishes the group and
		// computes the clock before allocating/transferring the saved cue cursor.
		if (nextCuePosition)
		{
			float** position = GetCuePosition(nextGroup);
			if (!*position) *position = new float;
			memcpy(*position, nextCuePosition, 4);
			delete nextCuePosition;
			nextCuePosition = NULL;
		}
		else
		{
			ResetCuePosition(nextGroup);
		}
	}
	const float* parentTransform = NULL;
	if (parent)
	{
		// Original4A65D9 uses a signed comparison; out-of-range positive semantic
		// indices select bone0. The valid domain retains the skeleton's mapping.
		int semantic = static_cast<int>(parentBone);
		int bone = semantic < 212 ? parent->animationObject->semanticBoneIndices[semantic] : 0;
		parentTransform = reinterpret_cast<const float*>(parent->worldMatrices + bone);
	}
	animationObject->UpdateWorldPose(modelInstance, worldPose, parentTransform);
	sourceCompositeMatrices = Granny21::GrannyGetWorldPoseComposite4x4Array(worldPose);
	if (scaling)
	{
		D3DXMATRIX scaleMatrix = g_D3D.identity;
		scaleMatrix._11 = scale[0];
		scaleMatrix._22 = scale[1];
		scaleMatrix._33 = scale[2];
		const float* source = Granny21::GrannyGetWorldPose4x4Array(worldPose);
		for (DWORD bone = 0; bone < boneCount; ++bone)
		{
			D3DXMATRIX& matrix = worldMatrices[bone];
			for (DWORD row = 0; row < 3; ++row)
			{
				for (DWORD column = 0; column < 3; ++column) matrix.m[row][column] = source[row * 4 + column];
				matrix.m[row][3] = 0.0f;
			}
			matrix._41 = source[12] * scale[0];
			matrix._42 = source[13] * scale[1];
			matrix._43 = source[14] * scale[2];
			matrix._44 = 1.0f;
			D3DXMATRIX composite;
			memcpy(&composite, sourceCompositeMatrices + bone * 16, 64);
			D3DXMatrixMultiply(compositeMatrices + bone, &composite, &scaleMatrix);
			source += 16;
		}
	}
	else
	{
		const float* source = Granny21::GrannyGetWorldPose4x4Array(worldPose);
		memcpy(worldMatrices, source, boneCount * 64);
		memcpy(compositeMatrices, sourceCompositeMatrices, boneCount * 64);
	}
	playing = control != NULL;
	ClearCues();
	if (control)
	{
		float** position = GetCuePosition(group);
		ProcessAnimationCues(group, animationIndex, controlTime, position);
	}
	if (nextControl)
	{
		ProcessAnimationCues(nextGroup, nextAnimationIndex, nextTime, &nextCuePosition);
	}
}

#if defined(_MSC_VER) && _MSC_VER == 1300
typedef char AnimationCueSize[(sizeof(Gfx_AnimationCue) == 48) ? 1 : -1];
typedef char AnimationInstanceSize[(sizeof(Gfx_AnimationInstance) == 200) ? 1 : -1];
typedef char AnimationCompositeOffset[(offsetof(Gfx_AnimationInstance, compositeMatrices) == 0x48) ? 1 : -1];
#endif

#if defined(_MSC_VER) && _MSC_VER == 1300
struct AnimationScaleAlignmentCheck { char prefix; D3DXVECTOR3 value; };
typedef char AnimationScaleExtent[(sizeof(D3DXVECTOR3) == 12) ? 1 : -1];
typedef char AnimationScaleAlignment[(offsetof(AnimationScaleAlignmentCheck, value) == 4) ? 1 : -1];
#define ANIMATION_FIELD_OFFSET(member, offset) typedef char AnimationOffset_##member[(offsetof(Gfx_AnimationInstance, member) == offset) ? 1 : -1]
ANIMATION_FIELD_OFFSET(scale, 0x20);
ANIMATION_FIELD_OFFSET(scaling, 0x2c);
ANIMATION_FIELD_OFFSET(control, 0x30);
ANIMATION_FIELD_OFFSET(nextControl, 0x34);
ANIMATION_FIELD_OFFSET(boneCount, 0x38);
ANIMATION_FIELD_OFFSET(worldPose, 0x3c);
ANIMATION_FIELD_OFFSET(worldMatrices, 0x40);
ANIMATION_FIELD_OFFSET(sourceCompositeMatrices, 0x44);
ANIMATION_FIELD_OFFSET(compositeMatrices, 0x48);
ANIMATION_FIELD_OFFSET(group, 0x4c);
ANIMATION_FIELD_OFFSET(animationIndex, 0x50);
ANIMATION_FIELD_OFFSET(value54, 0x54);
ANIMATION_FIELD_OFFSET(value58, 0x58);
ANIMATION_FIELD_OFFSET(value5C, 0x5c);
ANIMATION_FIELD_OFFSET(value60, 0x60);
ANIMATION_FIELD_OFFSET(value64, 0x64);
ANIMATION_FIELD_OFFSET(flag68, 0x68);
ANIMATION_FIELD_OFFSET(value6C, 0x6c);
ANIMATION_FIELD_OFFSET(pendingTransition, 0x70);
ANIMATION_FIELD_OFFSET(nextGroup, 0x74);
ANIMATION_FIELD_OFFSET(nextAnimationIndex, 0x78);
ANIMATION_FIELD_OFFSET(value7C, 0x7c);
ANIMATION_FIELD_OFFSET(value80, 0x80);
ANIMATION_FIELD_OFFSET(flag84, 0x84);
ANIMATION_FIELD_OFFSET(nextLoopState, 0x88);
ANIMATION_FIELD_OFFSET(speed, 0x8c);
ANIMATION_FIELD_OFFSET(reverse, 0x90);
ANIMATION_FIELD_OFFSET(remaining, 0x94);
ANIMATION_FIELD_OFFSET(nextRemaining, 0x98);
ANIMATION_FIELD_OFFSET(loopState, 0x9c);
ANIMATION_FIELD_OFFSET(cuePositions, 0xa0);
ANIMATION_FIELD_OFFSET(cues, 0xb0);
ANIMATION_FIELD_OFFSET(nextCuePosition, 0xbc);
ANIMATION_FIELD_OFFSET(collisionBindingFlags, 0xc0);
ANIMATION_FIELD_OFFSET(flagC4, 0xc4);
#undef ANIMATION_FIELD_OFFSET
#endif
