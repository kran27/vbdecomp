#pragma once

#include "VfxEffectData.hpp"

namespace JE
{
	class VFX_ModelData : public VFX_BaseData
	{
	public:
		VFX_ModelData(); // 0x4F3830; original400-byte value.
		void Read(const VFX_ParameterHolder& parameters); //52FF00 case3,531BE0..53221B.

		VFX_ResourceData modelResource; // +70 type, +8C name.
		VFX_ResourceData skeletonResource; // +A8 type, +C4 name.
		VFX_ResourceData animationResource; // +E0 type, +FC name.
		LONG loopCount; // +118.
		float scaleX, scaleY, scaleZ; // +11C/+120/+124.
		BYTE useShadowVolume, ignoreLighting, useRenderPhase; // +128/+129/+12A.
		LONG renderPhase; // +12C.
		BYTE useColor; VFX_ColorBytes color; // +130/+131.
		BYTE adjustColor, adjustColorOverLifetime; VFX_ColorBytes finalColor; // +134/+135/+136.
		LONG colorWaveform; // +13C.
		float colorFreq, colorPhase; // +140/+144.
		BYTE useEmissive; VFX_ColorBytes emissive; // +148/+149.
		BYTE adjustEmissive, adjustEmissiveOverLifetime; VFX_ColorBytes finalEmissive; // +14C/+14D/+14E.
		LONG emissiveWaveform; // +154.
		float emissiveFreq, emissivePhase; // +158/+15C.
		BYTE useAlpha, alpha, adjustAlpha, adjustAlphaOverLifetime, finalAlpha; // +160..164.
		LONG alphaWaveform; // +168.
		float alphaFreq, alphaPhase; // +16C/+170.
		BYTE adjustScale, adjustScaleOverLifetime; // +174/+175.
		float finalScaleX, finalScaleY, finalScaleZ; // +178/+17C/+180.
		LONG scaleWaveform; // +184.
		float scaleFreq, scalePhase; // +188/+18C.
	};
}
