#include "VfxWeather.hpp"
#include <string.h>
#include <stdexcept>

namespace JE
{
	VFX_RainData::VFX_RainData()
	{
	}
	VFX_SnowData::VFX_SnowData()
	{
	}
	VFX_WindData::VFX_WindData() {}

	void VFX_RainData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input=parameters.Find("RainDropsPerSecond"))!=NULL) dropsPerSecond=float(input->GetFloat());
		if ((input=parameters.Find("MaxRainDrops"))!=NULL) maxDrops=input->GetInteger();
		if ((input=parameters.Find("MinVelocity"))!=NULL) { const JE::VFX_Vector converted = input->GetVector3(); minVelocity = converted; }
		if ((input=parameters.Find("MaxVelocity"))!=NULL) { const JE::VFX_Vector converted = input->GetVector3(); maxVelocity = converted; }
		if ((input=parameters.Find("MinLength"))!=NULL) minLength=float(input->GetFloat());
		if ((input=parameters.Find("MaxLength"))!=NULL) maxLength=float(input->GetFloat());
		if ((input=parameters.Find("Color"))!=NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color, converted.values, sizeof(converted.values)); }
		if ((input=parameters.Find("MinBeginAlpha"))!=NULL) minBeginAlpha=BYTE(input->GetEnum());
		if ((input=parameters.Find("MaxBeginAlpha"))!=NULL) maxBeginAlpha=BYTE(input->GetEnum());
		if ((input=parameters.Find("MinEndAlpha"))!=NULL) minEndAlpha=BYTE(input->GetEnum());
		if ((input=parameters.Find("MaxEndAlpha"))!=NULL) maxEndAlpha=BYTE(input->GetEnum());
		if ((input=parameters.Find("RenderPhase"))!=NULL) renderPhase=input->GetEnum();
		if ((input=parameters.Find("Modulate2x"))!=NULL) modulate2x=input->GetBool();
		if ((input=parameters.Find("EmitterSize"))!=NULL) emitterSize=float(input->GetFloat());
		if ((input=parameters.Find("EmitterYOffset"))!=NULL) emitterYOffset=float(input->GetFloat());
		if ((input=parameters.Find("GroundYOffset"))!=NULL) groundYOffset=float(input->GetFloat());
		if ((input=parameters.Find("OffsetFromCamera"))!=NULL) { const JE::VFX_Vector converted = input->GetVector3(); offsetFromCamera = converted; }
		if ((input=parameters.Find("AffectedByWind"))!=NULL) affectedByWind=input->GetBool();
		if ((input=parameters.Find("WindScale"))!=NULL) windScale=float(input->GetFloat());
	}

	void VFX_SnowData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input=parameters.Find("SnowTexture"))!=NULL)
		{ texture.type=input->GetResourceType(); texture.name=input->GetResourceName(); }
		if ((input=parameters.Find("SnowTextureUVs"))!=NULL) { const JE::VFX_TextureCoordinates converted = input->GetTextureUVs(); memcpy(textureUVs, &converted, sizeof(converted)); }
		if ((input=parameters.Find("SnowFlakesPerSecond"))!=NULL) flakesPerSecond=float(input->GetFloat());
		if ((input=parameters.Find("MaxSnowFlakes"))!=NULL) maxFlakes=input->GetInteger();
		if ((input=parameters.Find("MinVelocity"))!=NULL) { const JE::VFX_Vector converted = input->GetVector3(); minVelocity = converted; }
		if ((input=parameters.Find("MaxVelocity"))!=NULL) { const JE::VFX_Vector converted = input->GetVector3(); maxVelocity = converted; }
		if ((input=parameters.Find("MinSize"))!=NULL) minSize=float(input->GetFloat());
		if ((input=parameters.Find("MaxSize"))!=NULL) maxSize=float(input->GetFloat());
		if ((input=parameters.Find("Color"))!=NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color, converted.values, sizeof(converted.values)); }
		if ((input=parameters.Find("MinAlpha"))!=NULL) minAlpha=BYTE(input->GetEnum());
		if ((input=parameters.Find("MaxAlpha"))!=NULL) maxAlpha=BYTE(input->GetEnum());
		if ((input=parameters.Find("RenderPhase"))!=NULL) renderPhase=input->GetEnum();
		if ((input=parameters.Find("Modulate2x"))!=NULL) modulate2x=input->GetBool();
		if ((input=parameters.Find("EmitterSize"))!=NULL) emitterSize=float(input->GetFloat());
		if ((input=parameters.Find("EmitterYOffset"))!=NULL) emitterYOffset=float(input->GetFloat());
		if ((input=parameters.Find("GroundYOffset"))!=NULL) groundYOffset=float(input->GetFloat());
		if ((input=parameters.Find("OffsetFromCamera"))!=NULL) { const JE::VFX_Vector converted = input->GetVector3(); offsetFromCamera = converted; }
		if ((input=parameters.Find("MinXSinAmplitude"))!=NULL) minXSinAmplitude=float(input->GetFloat());
		if ((input=parameters.Find("MaxXSinAmplitude"))!=NULL) maxXSinAmplitude=float(input->GetFloat());
		if ((input=parameters.Find("MinXSinFrequency"))!=NULL) minXSinFrequency=float(input->GetFloat());
		if ((input=parameters.Find("MaxXSinFrequency"))!=NULL) maxXSinFrequency=float(input->GetFloat());
		if ((input=parameters.Find("MinXSinPhase"))!=NULL) minXSinPhase=float(input->GetFloat());
		if ((input=parameters.Find("MaxXSinPhase"))!=NULL) maxXSinPhase=float(input->GetFloat());
		if ((input=parameters.Find("MinZSinAmplitude"))!=NULL) minZSinAmplitude=float(input->GetFloat());
		if ((input=parameters.Find("MaxZSinAmplitude"))!=NULL) maxZSinAmplitude=float(input->GetFloat());
		if ((input=parameters.Find("MinZSinFrequency"))!=NULL) minZSinFrequency=float(input->GetFloat());
		if ((input=parameters.Find("MaxZSinFrequency"))!=NULL) maxZSinFrequency=float(input->GetFloat());
		if ((input=parameters.Find("MinZSinPhase"))!=NULL) minZSinPhase=float(input->GetFloat());
		if ((input=parameters.Find("MaxZSinPhase"))!=NULL) maxZSinPhase=float(input->GetFloat());
		if ((input=parameters.Find("AffectedByWind"))!=NULL) affectedByWind=input->GetBool();
		if ((input=parameters.Find("WindScale"))!=NULL) windScale=float(input->GetFloat());
	}

	void VFX_WindData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input=parameters.Find("WindType"))!=NULL) windType=input->GetEnum();
		if ((input=parameters.Find("Direction"))!=NULL) { const JE::VFX_Vector converted = input->GetVector3(); direction = converted; }
		if ((input=parameters.Find("Velocity"))!=NULL) velocity=float(input->GetFloat());
		if ((input=parameters.Find("AdjustVelocity"))!=NULL) adjustVelocity=input->GetBool();
		if ((input=parameters.Find("AdjustVelocityOverLifetime"))!=NULL) adjustVelocityOverLifetime=input->GetBool();
		if ((input=parameters.Find("FinalVelocity"))!=NULL) finalVelocity=float(input->GetFloat());
		if ((input=parameters.Find("VelocityWaveform"))!=NULL) velocityWaveform=input->GetEnum();
		if ((input=parameters.Find("VelocityFreq"))!=NULL) velocityFreq=float(input->GetFloat());
		if ((input=parameters.Find("VelocityPhase"))!=NULL) velocityPhase=float(input->GetFloat());
		if ((input=parameters.Find("MinGustVelocity"))!=NULL) minGustVelocity=float(input->GetFloat());
		if ((input=parameters.Find("MaxGustVelocity"))!=NULL) maxGustVelocity=float(input->GetFloat());
		if ((input=parameters.Find("MinGustDuration"))!=NULL) minGustDuration=float(input->GetFloat());
		if ((input=parameters.Find("MaxGustDuration"))!=NULL) maxGustDuration=float(input->GetFloat());
		if ((input=parameters.Find("MinGustAttack"))!=NULL) minGustAttack=float(input->GetFloat());
		if ((input=parameters.Find("MaxGustAttack"))!=NULL) maxGustAttack=float(input->GetFloat());
		if ((input=parameters.Find("MinGustDecay"))!=NULL) minGustDecay=float(input->GetFloat());
		if ((input=parameters.Find("MaxGustDecay"))!=NULL) maxGustDecay=float(input->GetFloat());
		if ((input=parameters.Find("MinGustDelay"))!=NULL) minGustDelay=float(input->GetFloat());
		if ((input=parameters.Find("MaxGustDelay"))!=NULL) maxGustDelay=float(input->GetFloat());
	}
}
