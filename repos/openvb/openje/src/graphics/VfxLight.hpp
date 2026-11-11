#pragma once

#include "VfxEffects.hpp"
#include "VfxValueOwners.hpp"
#include "Light.hpp"
#include "LineSystem.hpp"
#include "OctreeShape.hpp"

class OCTREE;

namespace JE
{
	class VFX_LightData : public VFX_BaseData
	{
	public:
		VFX_LightData(); // 0x4E7F90.
		void Read(const VFX_ParameterHolder& parameters); // 0x52FF00 case9.

		LONG lightType; // Original+70: point0, spot1, directional2.
		VFX_Vector direction; // +74; actual523B00 construction and owned vector reads.
		VFX_ColorBytes diffuse; // +80.
		float diffusePercent; // +84.
		VFX_ColorBytes specular, ambient; // +88/+8B.
		float range, falloff, attenuation0, attenuation1, attenuation2; // +90..+A0.
		float theta, phi; // +A4/+A8, copied without an angle conversion.
		LONG priority; // +AC, not passed to the light constructor by SetData.
		BYTE castShadow, showShape; // +B0/+B1.
		BYTE adjustDiffuse, adjustDiffuseOverLifetime; // +B2/+B3.
		VFX_ColorBytes finalDiffuse; // +B4.
		LONG diffuseWaveform; // +B8.
		float diffuseFreq, diffusePhase; // +BC/+C0.
		BYTE adjustSpecular, adjustSpecularOverLifetime; // +C4/+C5.
		VFX_ColorBytes finalSpecular; // +C6.
		LONG specularWaveform; // +CC.
		float specularFreq, specularPhase; // +D0/+D4.
		BYTE adjustAmbient, adjustAmbientOverLifetime; // +D8/+D9.
		VFX_ColorBytes finalAmbient; // +DA.
		LONG ambientWaveform; // +E0.
		float ambientFreq, ambientPhase; // +E4/+E8.
		BYTE adjustRange, adjustRangeOverLifetime; // +EC/+ED.
		float finalRange; // +F0.
		LONG rangeWaveform; // +F4.
		float rangeFreq, rangePhase; // +F8/+FC.
	};

	class VFX_Light : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_Light(); // 0x4E80D0.
		VFX_Light& CopyFrom(const VFX_Light& other); //0x4E83B0.
		virtual ~VFX_Light(); // Ordinary0x4E8040.
		virtual void SetData(const VFX_BaseData* input); // 0x4E83F0.
		virtual void Reset(); // Shared0x4E1890.
		virtual BYTE GetFlag() const; // Shared zero policy.
		virtual BYTE SkipBounds() const; // Shared zero policy.
		virtual void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; // 0x4E7950.
		void Update(float time);
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); // 0x4E7470.
		void Draw(); // Actual shared0x404B70 no-op. World submits lights separately.
		void UpdateInfluence(OCTREE* map); // 0x4E7BF0, World548840 after collecting lights.
		void PositionInfluence(OCTREE* map); // 0x4E7D80, World545AA0 before renderer AddLight.
		OctreeShapeNodes* GetInfluenceNodes() const; // 0x4E7F70, NULL or shape slot+1C.

		VFX_LightData data; // Original+C4.
		float currentTime; // +1C4.
		Gfx_Light* light; // +1C8, owning.
		OCTREE_Shape* influenceShape; // +1CC, owned by the separate World light-area path.
		Gfx_LineSystem debugLines; // +1D0, real embedded owner.

	private:
		double NormalizedTime() const;
		float CurrentRange() const;
	};
}
