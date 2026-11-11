#include "VfxMotionBlur.hpp"
#include "ModelInstance.hpp"
#include <stdexcept>

VFXEFFECTTYPE_MOTIONBLUR_Struct::VFXEFFECTTYPE_MOTIONBLUR_Struct() {}
void VFXEFFECTTYPE_MOTIONBLUR_Struct::Read(const JE::VFX_ParameterHolder& parameters)
{
	const JE::VFX_Parameter* input;
	if ((input = parameters.Find("BlurDuration")) != NULL) blurDuration = float(input->GetFloat());
	if ((input = parameters.Find("BlurDelay")) != NULL) blurDelay = float(input->GetFloat());
	if ((input = parameters.Find("RenderPhase")) != NULL) renderPhase = input->GetEnum();
	if ((input = parameters.Find("ApplyToAttachments")) != NULL) applyToAttachments = input->GetBool();
}

namespace JE
{
	VFX_MotionBlur::VFX_MotionBlur() : flag144(0) {}
	VFX_MotionBlur::~VFX_MotionBlur()
	{
	}
	VFX_MotionBlur& VFX_MotionBlur::CopyFrom(const VFX_MotionBlur& other) //0x4F6910.
	{
		VFX_EffectInstance::CopyFrom(other);
		data = other.data;
		flag144 = other.flag144;
		return *this;
	}
	void VFX_MotionBlur::SetData(const VFX_BaseData* input)
	{
		VFX_EffectInstance::SetData(input);
		data = *static_cast<const VFX_MotionBlurData*>(input);
	}
	void VFX_MotionBlur::Reset() { active = 0; }
	BYTE VFX_MotionBlur::GetFlag() const { return 0; }
	BYTE VFX_MotionBlur::SkipBounds() const { return 0; }
	void VFX_MotionBlur::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const
	{
		low = high = D3DXVECTOR3(transform._41, transform._42, transform._43);
	}
	void VFX_MotionBlur::Draw() {}
	BYTE VFX_MotionBlur::AppliesToExtraSources() const { return data.applyToAttachments; }
	BYTE VFX_MotionBlur::Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const
	{
		return data.target.Value() == 0 || ResolveVfxModel(data.target, source, target, group) != NULL;
	}
	void VFX_MotionBlur::Update(float time) { Update(time, VFX_UpdateContext()); }
	void VFX_MotionBlur::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_EffectInstance::Update(time, source, target, group, sourceTransform, targetTransform);
		if (flag144) return;
		Gfx_ModelInstance* model = ResolveVfxModel(data.target, source, target, group);
		if (model)
		{
			model->afterimageLifetime = data.blurDuration;
			model->afterimagePeriod = data.blurDelay;
			model->value90 = DWORD(data.renderPhase);
			model->afterimagesEnabled = 1;
		}
	}
}
