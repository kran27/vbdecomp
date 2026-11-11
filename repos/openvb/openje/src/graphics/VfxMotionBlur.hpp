#pragma once

#include "VfxEffects.hpp"

struct VFXEFFECTTYPE_MOTIONBLUR_Struct : public ::VFXEFFECTTYPE_BASE_Struct
{
public:
	VFXEFFECTTYPE_MOTIONBLUR_Struct(); //0x4F6840, also inlined by4F6890; scalar fields stay unwritten.
	void Read(const JE::VFX_ParameterHolder& parameters); //0x52FF00 case1.
	float blurDuration, blurDelay; // Original+70/+74.
	LONG renderPhase; // +78.
	BYTE applyToAttachments; // +7C.
};

namespace JE
{
	typedef ::VFXEFFECTTYPE_MOTIONBLUR_Struct VFX_MotionBlurData;

	class VFX_MotionBlur : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_MotionBlur(); //0x4F6890.
		VFX_MotionBlur& CopyFrom(const VFX_MotionBlur& other); //0x4F6910.
		virtual ~VFX_MotionBlur();
		virtual void SetData(const VFX_BaseData* input); //0x4F6970.
		virtual void Reset(); //Shared4E1890.
		virtual BYTE GetFlag() const;
		virtual BYTE SkipBounds() const;
		virtual void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; //Shared4E0D70.
		void Update(float time);
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //0x4F6780.
		void Draw(); //Actual404B70 no-op; the model owns/submits snapshots.
		virtual BYTE Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const; //0x4F6800.
		virtual BYTE AppliesToExtraSources() const; //0x4F6880.
		VFX_MotionBlurData data; // Original+C4.
		BYTE flag144; // Constructor0; nonzero suppresses model afterimage configuration.
	};
}
