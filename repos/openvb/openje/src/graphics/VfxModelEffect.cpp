#include "VfxModelEffect.hpp"
#include <string.h>
#include "ModelInstance.hpp"
#include "ParticleSystem.hpp"
#include <stdexcept>

namespace
{
	DWORD Pack(BYTE red, BYTE green, BYTE blue, BYTE alpha)
	{
		return (DWORD(alpha) << 24) | (DWORD(red) << 16) | (DWORD(green) << 8) | blue;
	}
}

namespace JE
{
	VFX_ModelEffectData::VFX_ModelEffectData() //4E8B70.
	{
		// The four native RGB value owners construct in member order.
		// All other primitives are unwritten; real format merging supplies defaults.
	}
	void VFX_ModelEffectData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input = parameters.Find("IgnoreLighting")) != NULL) ignoreLighting = input->GetBool();
		if ((input = parameters.Find("UseRenderPhase")) != NULL) useRenderPhase = input->GetBool();
		if ((input = parameters.Find("RenderPhase")) != NULL) renderPhase = input->GetEnum();
		if ((input = parameters.Find("ShadowVolume")) != NULL) shadowVolume = input->GetEnum();
		if ((input = parameters.Find("ApplyToAttachments")) != NULL) applyToAttachments = input->GetBool();
		if ((input = parameters.Find("UseColor")) != NULL) useColor = input->GetBool();
		if ((input = parameters.Find("Color")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("AdjustColor")) != NULL) adjustColor = input->GetBool();
		if ((input = parameters.Find("AdjustColorOverLifetime")) != NULL) adjustColorOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalColor")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(finalColor, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("ColorWaveform")) != NULL) colorWaveform = input->GetEnum();
		if ((input = parameters.Find("ColorFreq")) != NULL) colorFreq = float(input->GetFloat());
		if ((input = parameters.Find("ColorPhase")) != NULL) colorPhase = float(input->GetFloat());
		if ((input = parameters.Find("UseEmissive")) != NULL) useEmissive = input->GetBool();
		if ((input = parameters.Find("Emissive")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(emissive, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("AdjustEmissive")) != NULL) adjustEmissive = input->GetBool();
		if ((input = parameters.Find("AdjustEmissiveOverLifetime")) != NULL) adjustEmissiveOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalEmissive")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(finalEmissive, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("EmissiveWaveform")) != NULL) emissiveWaveform = input->GetEnum();
		if ((input = parameters.Find("EmissiveFreq")) != NULL) emissiveFreq = float(input->GetFloat());
		if ((input = parameters.Find("EmissivePhase")) != NULL) emissivePhase = float(input->GetFloat());
		if ((input = parameters.Find("UseAlpha")) != NULL) useAlpha = input->GetBool();
		if ((input = parameters.Find("Alpha")) != NULL) alpha = BYTE(input->GetEnum());
		if ((input = parameters.Find("AdjustAlpha")) != NULL) adjustAlpha = input->GetBool();
		if ((input = parameters.Find("AdjustAlphaOverLifetime")) != NULL) adjustAlphaOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalAlpha")) != NULL) finalAlpha = BYTE(input->GetEnum());
		if ((input = parameters.Find("AlphaWaveform")) != NULL) alphaWaveform = input->GetEnum();
		if ((input = parameters.Find("AlphaFreq")) != NULL) alphaFreq = float(input->GetFloat());
		if ((input = parameters.Find("AlphaPhase")) != NULL) alphaPhase = float(input->GetFloat());
	}

	VFX_ModelEffect::VFX_ModelEffect() //4E8C00.
	{
		currentTime = 0;
		flag184 = 0;
	}
	VFX_ModelEffect::~VFX_ModelEffect()
	{} //4E8BE0: embedded data then base destruction, no borrowed target deletion.
	VFX_ModelEffect& VFX_ModelEffect::CopyFrom(const VFX_ModelEffect& other) //0x4E8E00.
	{
		VFX_EffectInstance::CopyFrom(other);
		data = other.data;
		flag184 = other.flag184;
		currentTime = other.currentTime;
		return *this;
	}
	void VFX_ModelEffect::SetData(const VFX_BaseData* input) //4E8E40.
	{
		VFX_EffectInstance::SetData(input);
		data = *static_cast<const VFX_ModelEffectData*>(input); //4E8C80 normal value copy.
	}
	void VFX_ModelEffect::Reset() { active = 0; } //Shared4E1890; leave stop state unchanged.
	BYTE VFX_ModelEffect::GetFlag() const { return 0; }
	BYTE VFX_ModelEffect::SkipBounds() const { return 0; }
	BYTE VFX_ModelEffect::AppliesToExtraSources() const { return data.applyToAttachments; } //4F6880.
	BYTE VFX_ModelEffect::Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const
	{
		return data.target.Value() == 0 || ResolveVfxModel(data.target, source, target, group) != NULL;
	}
	void VFX_ModelEffect::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const //4E0D70.
	{
		low = high = D3DXVECTOR3(transform._41, transform._42, transform._43);
	}
	void VFX_ModelEffect::Draw() {}
	void VFX_ModelEffect::Draw(Gfx_Viewport*) {} //Original5A9320 no-op.
	void VFX_ModelEffect::Update(float time)
	{
		VFX_UpdateContext context;
		Update(time, context);
	}
	void VFX_ModelEffect::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_EffectInstance::Update(time, source, target, group, sourceTransform, targetTransform);
		if (flag184) return;
		if (currentTime != time) currentTime = time;
		Gfx_ModelInstance* model = ResolveVfxModel(data.target, source, target, group);
		if (!model) return;
		float fraction = float(double(currentTime) / data.duration);
		if (fraction < 0) fraction = 0;
		else if (fraction > 1) fraction = 1;

		LONG red, green, blue;
		if (data.adjustColor)
		{
			double selected = data.adjustColorOverLifetime && data.duration > 0 ? fraction : currentTime;
			float clock = float(selected * data.colorFreq + data.colorPhase);
			double wave = Gfx_ParticleWaveform(data.colorWaveform, 0, 1, clock);
			red = static_cast<LONG>((int(data.finalColor[0]) - data.color[0]) * wave + data.color[0]);
			green = static_cast<LONG>((int(data.finalColor[1]) - data.color[1]) * wave + data.color[1]);
			blue = static_cast<LONG>((int(data.finalColor[2]) - data.color[2]) * wave + data.color[2]);
		}
		else
		{
			red = data.color[0]; green = data.color[1]; blue = data.color[2];
		}
		DWORD alpha;
		if (data.adjustAlpha)
		{
			double selected = data.adjustAlphaOverLifetime && data.duration > 0 ? fraction : currentTime;
			float clock = float(selected * data.alphaFreq + data.alphaPhase);
			double wave = Gfx_ParticleWaveform(data.alphaWaveform, 0, 1, clock);
			alpha = static_cast<DWORD>(static_cast<LONG>((int(data.finalAlpha) - data.alpha) * wave + data.alpha));
		}
		else alpha = data.alpha;
		DWORD emissive;
		const DWORD* emissiveInput = NULL;
		if (data.adjustEmissive)
		{
			double selected = data.adjustEmissiveOverLifetime && data.duration > 0 ? fraction : currentTime;
			float clock = float(selected * data.emissiveFreq + data.emissivePhase);
			double wave = Gfx_ParticleWaveform(data.emissiveWaveform, 0, 1, clock);
			const LONG baseRed = data.emissive[0], baseGreen = data.emissive[1], baseBlue = data.emissive[2];
			const LONG emissiveRed = static_cast<LONG>((int(data.finalEmissive[0]) - baseRed) * wave + baseRed);
			const LONG emissiveGreen = static_cast<LONG>((int(data.finalEmissive[1]) - baseGreen) * wave + baseGreen);
			const LONG emissiveBlue = static_cast<LONG>((int(data.finalEmissive[2]) - baseBlue) * wave + baseBlue);
			emissive = Pack(BYTE(emissiveRed), BYTE(emissiveGreen), BYTE(emissiveBlue), 255);
			emissiveInput = &emissive;
		}
		else if (data.useEmissive)
		{
			emissive = Pack(data.emissive[0], data.emissive[1], data.emissive[2], 255);
			emissiveInput = &emissive;
		}
		// Preserve the actual asymmetry: adjustEmissive is absent from this reset gate.
		if (!data.useRenderPhase && !data.adjustColor && !data.adjustAlpha && !data.useColor
			&& !data.useAlpha && !data.ignoreLighting && !data.useEmissive)
		{
			model->ResetMaterialValues();
			model->ResetMaterialStates();
		}
		if (data.adjustColor || data.adjustAlpha || data.useColor || data.useAlpha || data.adjustEmissive || data.useEmissive)
		{
			if (data.ignoreLighting && !data.useColor) red = green = blue = 128;
			model->SetMaterialColors(Pack(BYTE(red), BYTE(green), BYTE(blue), BYTE(alpha)),
				data.adjustColor || data.useColor || data.ignoreLighting,
				data.adjustAlpha || data.useAlpha, emissiveInput);
		}
		model->SetMaterialFlagBC(data.ignoreLighting == 0);
		if (data.useRenderPhase) model->SetRenderPhase(data.renderPhase);
		if (data.shadowVolume) model->SetShadow(data.shadowVolume == 2);
	}
}
