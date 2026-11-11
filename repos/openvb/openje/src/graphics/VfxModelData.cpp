#include "VfxModelData.hpp"
#include <string.h>
#include <stdexcept>

namespace JE
{
	VFX_ModelData::VFX_ModelData() //4F3830.
	{
		// Resource and color members construct naturally; primitive fields stay unwritten.
	}

	void VFX_ModelData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input = parameters.Find("Model")) != NULL)
		{ modelResource.type = input->GetResourceType(); modelResource.name = input->GetResourceName(); }
		if ((input = parameters.Find("Skeleton")) != NULL)
		{ skeletonResource.type = input->GetResourceType(); skeletonResource.name = input->GetResourceName(); }
		if ((input = parameters.Find("Animation")) != NULL)
		{ animationResource.type = input->GetResourceType(); animationResource.name = input->GetResourceName(); }
		if ((input = parameters.Find("LoopCount")) != NULL) loopCount = input->GetInteger();
		if ((input = parameters.Find("ScaleX")) != NULL) scaleX = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("ScaleY")) != NULL) scaleY = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("ScaleZ")) != NULL) scaleZ = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("UseShadowVolume")) != NULL) useShadowVolume = input->GetBool();
		if ((input = parameters.Find("IgnoreLighting")) != NULL) ignoreLighting = input->GetBool();
		if ((input = parameters.Find("UseRenderPhase")) != NULL) useRenderPhase = input->GetBool();
		if ((input = parameters.Find("RenderPhase")) != NULL) renderPhase = input->GetEnum();
		if ((input = parameters.Find("UseColor")) != NULL) useColor = input->GetBool();
		if ((input = parameters.Find("Color")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("AdjustColor")) != NULL) adjustColor = input->GetBool();
		if ((input = parameters.Find("AdjustColorOverLifetime")) != NULL) adjustColorOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalColor")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(finalColor, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("ColorWaveform")) != NULL) colorWaveform = input->GetEnum();
		if ((input = parameters.Find("ColorFreq")) != NULL) colorFreq = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("ColorPhase")) != NULL) colorPhase = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("UseEmissive")) != NULL) useEmissive = input->GetBool();
		if ((input = parameters.Find("Emissive")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(emissive, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("AdjustEmissive")) != NULL) adjustEmissive = input->GetBool();
		if ((input = parameters.Find("AdjustEmissiveOverLifetime")) != NULL) adjustEmissiveOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalEmissive")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(finalEmissive, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("EmissiveWaveform")) != NULL) emissiveWaveform = input->GetEnum();
		if ((input = parameters.Find("EmissiveFreq")) != NULL) emissiveFreq = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("EmissivePhase")) != NULL) emissivePhase = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("UseAlpha")) != NULL) useAlpha = input->GetBool();
		if ((input = parameters.Find("Alpha")) != NULL) alpha = static_cast<BYTE>(input->GetEnum());
		if ((input = parameters.Find("AdjustAlpha")) != NULL) adjustAlpha = input->GetBool();
		if ((input = parameters.Find("AdjustAlphaOverLifetime")) != NULL) adjustAlphaOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalAlpha")) != NULL) finalAlpha = static_cast<BYTE>(input->GetEnum());
		if ((input = parameters.Find("AlphaWaveform")) != NULL) alphaWaveform = input->GetEnum();
		if ((input = parameters.Find("AlphaFreq")) != NULL) alphaFreq = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("AlphaPhase")) != NULL) alphaPhase = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("AdjustScale")) != NULL) adjustScale = input->GetBool();
		if ((input = parameters.Find("AdjustScaleOverLifetime")) != NULL) adjustScaleOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalScaleX")) != NULL) finalScaleX = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("FinalScaleY")) != NULL) finalScaleY = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("FinalScaleZ")) != NULL) finalScaleZ = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("ScaleWaveform")) != NULL) scaleWaveform = input->GetEnum();
		if ((input = parameters.Find("ScaleFreq")) != NULL) scaleFreq = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("ScalePhase")) != NULL) scalePhase = static_cast<float>(input->GetFloat());
	}
}
