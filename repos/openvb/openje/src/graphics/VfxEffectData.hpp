#pragma once

#include "VfxParameters.hpp"
#include "VfxValueOwners.hpp"
#include <d3dx8.h>

// Typed proprietary effect records. Original offsets document ownership and
// data decisions; normal C++ members supply the runnable host representation.
struct VFXEFFECTTYPE_BASE_Struct
{
public:
	VFXEFFECTTYPE_BASE_Struct(); // 0x4E0F00.
	virtual ~VFXEFFECTTYPE_BASE_Struct() {}

	std::string name; // +4.
	LONG effectType; // +20.
	float delay, duration; // +24/+28, unwritten until present in parameters.
	JE::VFX_Target target; //+2C, actual four-byte selector owner.
	LONG socket; // +30.
	std::string snapBone; // +34.
	BYTE targetBodyPart, hideIfNoTarget; // +50/+51.
	JE::VFX_Vector offset; //+54, actual two-component prefix and third component.
	JE::VFX_Rotation rotation; //+60, distinct angular value owner.
	BYTE rotateWithTarget, triggeredEffect; // +6C/+6D.
};

namespace JE
{
	typedef ::VFXEFFECTTYPE_BASE_Struct VFX_BaseData;

	class VFX_LightningData : public VFX_BaseData
	{
	public:
		VFX_LightningData(); // 0x4E9070.
		void Read(const VFX_ParameterHolder& parameters); // 0x532CC4 case7.

		LONG renderPhase; // Original +70.
		BYTE modulate2x; // Original +74.
		BYTE innerGlow; // Original +75.
		float minLifetime; // Original +78.
		float maxLifetime; // Original +7C.
		LONG repeatCount; // Original +80.
		float minRepeatDelay; // Original +84.
		float maxRepeatDelay; // Original +88.
		VFX_Target endPointTarget; // Original +8C, actual selector owner.
		LONG endPointSocket; // Original +90.
		std::string endPointSnapBone; // Original +94.
		BYTE endPointTargetBodyPart; // Original +0B0.
		VFX_Vector endPointOffset; // Original +0B4, actual vector owner.
		float minSegmentsPerFoot; // Original +0C0.
		float maxSegmentsPerFoot; // Original +0C4.
		float minAdditionalSegments; // Original +0C8.
		float maxAdditionalSegments; // Original +0CC.
		float minJaggedness; // Original +0D0.
		float maxJaggedness; // Original +0D4.
		float minMeander; // Original +0D8.
		float maxMeander; // Original +0DC.
		BYTE alwaysStrikeTarget; // Original +0E0.
		float minWidth; // Original +0E4.
		float maxWidth; // Original +0E8.
		LONG widthStyle; // Original +0EC.
		float tipWidthScale; // Original +0F0.
		float branchProbability; // Original +0F4.
		float minBranchAngle; // Original +0F8.
		float maxBranchAngle; // Original +0FC.
		LONG maxBranches; // Original +100.
		LONG maxBranchLevels; // Original +104.
		float minBranchWidthScale; // Original +108.
		float maxBranchWidthScale; // Original +10C.
		float minBranchLengthScale; // Original +110.
		float maxBranchLengthScale; // Original +114.
		VFX_ColorBytes outerColor; // Original +118.
		VFX_ColorBytes innerColor; // Original +11B.
		BYTE adjustOuterColor; // Original +11E.
		BYTE adjustOuterColorOverLifetime; // Original +11F.
		VFX_ColorBytes finalOuterColor; // Original +120.
		LONG outerColorWaveform; // Original +124.
		float outerColorFreq; // Original +128.
		float outerColorPhase; // Original +12C.
		BYTE adjustInnerColor; // Original +130.
		BYTE adjustInnerColorOverLifetime; // Original +131.
		VFX_ColorBytes finalInnerColor; // Original +132.
		LONG innerColorWaveform; // Original +138.
		float innerColorFreq; // Original +13C.
		float innerColorPhase; // Original +140.
		BYTE outerAlpha; // Original +144.
		BYTE innerAlpha; // Original +145.
		BYTE adjustOuterAlpha; // Original +146.
		BYTE adjustOuterAlphaOverLifetime; // Original +147.
		BYTE finalOuterAlpha; // Original +148.
		LONG outerAlphaWaveform; // Original +14C.
		float outerAlphaFreq; // Original +150.
		float outerAlphaPhase; // Original +154.
		BYTE adjustInnerAlpha; // Original +158.
		BYTE adjustInnerAlphaOverLifetime; // Original +159.
		BYTE finalInnerAlpha; // Original +15A.
		LONG innerAlphaWaveform; // Original +15C.
		float innerAlphaFreq; // Original +160.
		float innerAlphaPhase; // Original +164.
	};

	void ReadVfxBaseData(VFX_BaseData* output, const VFX_ParameterHolder& parameters); //0x52FC30, common fields.
	void ReadVfxEffectData(VFX_BaseData* output, const VFX_ParameterHolder& parameters);
	VFX_BaseData* BuildVfxEffectData(const VFX_ParameterHolder& parameters); // 0x534890.
}
