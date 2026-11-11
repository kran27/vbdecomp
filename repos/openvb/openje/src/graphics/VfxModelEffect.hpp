#pragma once

#include "VfxEffects.hpp"
#include "VfxValueOwners.hpp"

namespace JE
{
	class VFX_ModelEffectData : public VFX_BaseData
	{
	public:
		VFX_ModelEffectData(); //4E8B70, actual0xC0 value.
		void Read(const VFX_ParameterHolder& parameters); //52FF00 case8,5334C5..53385A.
		BYTE ignoreLighting, useRenderPhase; //+70/+71.
		LONG renderPhase, shadowVolume; //+74/+78; shadow default0/off1/on2.
		BYTE applyToAttachments, useColor; //+7C/+7D.
		VFX_ColorBytes color; //+7E.
		BYTE adjustColor, adjustColorOverLifetime; //+81/+82.
		VFX_ColorBytes finalColor; //+83.
		LONG colorWaveform; //+88.
		float colorFreq, colorPhase; //+8C/+90.
		BYTE useEmissive; //+94.
		VFX_ColorBytes emissive; //+95.
		BYTE adjustEmissive, adjustEmissiveOverLifetime; //+98/+99.
		VFX_ColorBytes finalEmissive; //+9A.
		LONG emissiveWaveform; //+A0.
		float emissiveFreq, emissivePhase; //+A4/+A8.
		BYTE useAlpha, alpha, adjustAlpha, adjustAlphaOverLifetime, finalAlpha; //+AC..B0.
		LONG alphaWaveform; //+B4.
		float alphaFreq, alphaPhase; //+B8/+BC.
	};

	class VFX_ModelEffect : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_ModelEffect(); //4E8C00, embedded data plus two scalar members.
		VFX_ModelEffect& CopyFrom(const VFX_ModelEffect& other); //0x4E8E00.
		virtual ~VFX_ModelEffect();
		virtual void SetData(const VFX_BaseData* input); //4E8E40/4E8C80.
		virtual void Reset(); //Shared4E1890.
		virtual BYTE GetFlag() const; //549020.
		virtual BYTE SkipBounds() const; //549020.
		virtual void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; //Shared4E0D70.
		void Update(float time);
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //4E8650.
		void Draw();
		virtual void Draw(Gfx_Viewport* viewport); //Shared5A9320 no-op.
		virtual BYTE Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const; //Shared4F6800.
		virtual BYTE AppliesToExtraSources() const; //Shared4F6880, raw+140.
		VFX_ModelEffectData data; //+C4.
	private:
		BYTE flag184; //Original constructor0; no later producer recovered.
		float currentTime; //+188, constructor0.
	};
}
