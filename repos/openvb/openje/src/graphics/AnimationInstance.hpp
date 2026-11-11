#pragma once

#include "AnimationObject.hpp"
#include <d3dx8.h>
#include <list>
#include <string>

// Original list value: five words (group,index,cue-time/occurrence/requested
// float bits) and a real string, produced by Data4A3360.
struct Gfx_AnimationCue
{
	DWORD values[5];
	std::string name;
};

// Actual nonvirtual200-byte VC7 animation instance4A5BE0. It borrows its
// AnimationObject; this instance must be destroyed before that object's file.
class Gfx_AnimationInstance
{
public:
	explicit Gfx_AnimationInstance(Gfx_AnimationObject* object); // 0x4A5BE0.
	Gfx_AnimationInstance(const Gfx_AnimationInstance& source); //0x4A6880; fresh runtime owners.
	~Gfx_AnimationInstance(); // 0x4A5D70.
	void Update(float seconds); // 0x4A62B0; original callers ignore EAX.
	void SetSpeed(float value); // 0x548D10; real Granny controls.
	void SetScale(float x, float y, float z); //507980; scale values and nonidentity flag.
	void ClearCues(); //4A5F20, destroy the current value cue list.
	void ProcessCuesUntil(float seconds); // 0x4A5F60.
	BYTE SelectAnimation(DWORD group, DWORD index, DWORD subAnimation,
		float blendSeconds, BYTE fromCurrent, int loopCount); // 0x4A5640; schedules transition.
	void CompleteCurrentControl(float value, BYTE ease); //4A4C90, also used by actual VFX_Model.
	int GetLoopCount() const; //0x49CA30; current control, next control, or-1.
	Granny21::Skeleton* GetSkeleton() const; //0x4A4CD0; borrowed animation-object skeleton.
	LONG GetSemanticBoneIndex(LONG bone) const; //49C9E0; raw mapped index, signed upper bound only.
	D3DXMATRIX* GetBoneTransform(LONG bone) const; //49CA00; borrowed pose, >=212 selects root.
	BYTE RayIntersection(const D3DXMATRIX& transform, const D3DXVECTOR3& origin,
		const D3DXVECTOR3& direction, float& distance); //0x4A4CE0; real Granny bone OBBs.
	void RenderOBB(const D3DXMATRIX& transform); //0x4A4F10; one-frame collision-box debug request.

	float currentTime, timeOffset; // +00/+04.
	BYTE playing; // +08; padding unwritten.
	Gfx_AnimationObject* animationObject; // +0C; borrowed.
	Granny21::ModelInstance* modelInstance; // +10; owned.
	Granny21::TrackMask* trackMask; // +14; owned.
	Gfx_AnimationInstance* parent; // +18; borrowed parent, sampled before this instance.
	DWORD parentBone; // +1C; semantic index, default1.
	D3DXVECTOR3 scale; // +20..28; ordinary three-float scale value.
	BYTE scaling; // +2C.
	Granny21::Control* control; // +30; owned current control.
	Granny21::Control* nextControl; // +34; owned next control.
	DWORD boneCount; // +38.
	Granny21::WorldPose* worldPose; // +3C; owned runtime pose.
	D3DXMATRIX* worldMatrices; // +40; owned boneCount matrices.
	const float* sourceCompositeMatrices; // +44; borrowed from worldPose.
	D3DXMATRIX* compositeMatrices; // +48; owned boneCount matrices.
	DWORD group, animationIndex, value54, value58, value5C, value60;
	float value64;
	BYTE flag68;
	DWORD value6C;
	BYTE pendingTransition; // +70; consumed by actual transition4A6070.
	DWORD nextGroup, nextAnimationIndex, value7C;
	float value80;
	BYTE flag84;
	DWORD nextLoopState;
	float speed;
	BYTE reverse;
	float remaining, nextRemaining;
	DWORD loopState;
	std::vector<float*> cuePositions; // +A0; owns previous cue-time cursors.
	std::list<Gfx_AnimationCue> cues; // +B0; actual value list, not pointer list.
	float* nextCuePosition; // +BC; separately owned previous cue-time cursor.
	BYTE* collisionBindingFlags; // +C0; optional collision mesh bone-binding count.
	BYTE flagC4;

private:
	void ResetCuePosition(DWORD index); // 0x4A55D0.
	float** GetCuePosition(DWORD index); // 0x4A5EA0.
	void PromoteNextControl(); // Ordinary source factoring of4A641C..4A65C8.
	void PrepareTransition(); // 0x4A6070.
	void ProcessAnimationCues(DWORD group, DWORD index, float seconds, float** position);
	Gfx_AnimationInstance& operator=(const Gfx_AnimationInstance&);
};
