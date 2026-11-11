#include "VfxParticles.hpp"
#include "VfxVeg.hpp"
#include "VfxCollision.hpp"
#include "ParticleSystem.hpp"
#include "ParticleConfiguration.hpp"
#include "SystemInterface.hpp"
#include "TextureResource.hpp"
#include "ModelData.hpp"
#include "ModelInstance.hpp"
#include "GfxVector3.hpp"
#include "VfxFrame.hpp"
#include "../resources/ResourceMemoryStream.hpp"
#include <mbstring.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <stdexcept>

namespace JE
{
	VFX_ParticleData::VFX_ParticleData()
	{
		// Native vector/rotation, UV and RGB value owners initialize in member order.
		// Other scalar fields remain unwritten until the actual format merge.
	}

	void VFX_ParticleData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input = parameters.Find("BoundingBoxType")) != NULL)
		{ boundingBoxType = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("_OptimizedAABBMin")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); optimizedAABBMin = converted; } }
		if ((input = parameters.Find("_OptimizedAABBMax")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); optimizedAABBMax = converted; } }
		if ((input = parameters.Find("MinBoundingBox")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); minBoundingBox = converted; } }
		if ((input = parameters.Find("MaxBoundingBox")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); maxBoundingBox = converted; } }
		if ((input = parameters.Find("ShowBoundingBox")) != NULL)
		{ showBoundingBox = input->GetBool(); }
		if ((input = parameters.Find("EmitterShape")) != NULL)
		{ emitterShape = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("MinDimensions")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); minDimensions = converted; } }
		if ((input = parameters.Find("MaxDimensions")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); maxDimensions = converted; } }
		if ((input = parameters.Find("ShowEmitterShape")) != NULL)
		{ showEmitterShape = input->GetBool(); }
		if ((input = parameters.Find("Acceleration1")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); acceleration1 = converted; } }
		if ((input = parameters.Find("RotateAccel1WithTarget")) != NULL)
		{ rotateAccel1WithTarget = input->GetBool(); }
		if ((input = parameters.Find("Acceleration2")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); acceleration2 = converted; } }
		if ((input = parameters.Find("RotateAccel2WithTarget")) != NULL)
		{ rotateAccel2WithTarget = input->GetBool(); }
		if ((input = parameters.Find("UseEmitterLifetime")) != NULL)
		{ useEmitterLifetime = input->GetBool(); }
		if ((input = parameters.Find("EmitterLifetime")) != NULL)
		{ emitterLifetime = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AutoAdd")) != NULL)
		{ autoAdd = input->GetBool(); }
		if ((input = parameters.Find("MinAddRate")) != NULL)
		{ minAddRate = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxAddRate")) != NULL)
		{ maxAddRate = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinParticlesPerAdd")) != NULL)
		{ minParticlesPerAdd = input->GetInteger(); }
		if ((input = parameters.Find("MaxParticlesPerAdd")) != NULL)
		{ maxParticlesPerAdd = input->GetInteger(); }
		if ((input = parameters.Find("UseMaxParticles")) != NULL)
		{ useMaxParticles = input->GetBool(); }
		if ((input = parameters.Find("MaxParticles")) != NULL)
		{ maxParticles = input->GetInteger(); }
		if ((input = parameters.Find("Texture")) != NULL)
		{ texture.type = input->GetResourceType(); texture.name = input->GetResourceName(); }
		if ((input = parameters.Find("TextureUVs")) != NULL)
		{ { const JE::VFX_TextureCoordinates converted = input->GetTextureUVs(); memcpy(textureUVs, &converted, sizeof(converted)); } }
		if ((input = parameters.Find("Model")) != NULL)
		{ model.type = input->GetResourceType(); model.name = input->GetResourceName(); }
		if ((input = parameters.Find("Model_UseRenderPhase")) != NULL)
		{ model_UseRenderPhase = input->GetBool(); }
		if ((input = parameters.Find("Model_UseColor")) != NULL)
		{ model_UseColor = input->GetBool(); }
		if ((input = parameters.Find("Model_UseAlpha")) != NULL)
		{ model_UseAlpha = input->GetBool(); }
		if ((input = parameters.Find("Model_IgnoreLighting")) != NULL)
		{ model_IgnoreLighting = input->GetBool(); }
		if ((input = parameters.Find("RenderPhase")) != NULL)
		{ renderPhase = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("Modulate2x")) != NULL)
		{ modulate2x = input->GetBool(); }
		if ((input = parameters.Find("MinParticleAlpha")) != NULL)
		{ minParticleAlpha = static_cast<BYTE>(input->GetEnum()); }
		if ((input = parameters.Find("MaxParticleAlpha")) != NULL)
		{ maxParticleAlpha = static_cast<BYTE>(input->GetEnum()); }
		if ((input = parameters.Find("MinParticleLifetime")) != NULL)
		{ minParticleLifetime = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxParticleLifetime")) != NULL)
		{ maxParticleLifetime = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinParticleSize")) != NULL)
		{ minParticleSize = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxParticleSize")) != NULL)
		{ maxParticleSize = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinParticleMass")) != NULL)
		{ minParticleMass = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxParticleMass")) != NULL)
		{ maxParticleMass = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinParticleResistance")) != NULL)
		{ minParticleResistance = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxParticleResistance")) != NULL)
		{ maxParticleResistance = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinInitialVelocity")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); minInitialVelocity = converted; } }
		if ((input = parameters.Find("MaxInitialVelocity")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); maxInitialVelocity = converted; } }
		if ((input = parameters.Find("MinInitialVelocityScale")) != NULL)
		{ minInitialVelocityScale = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxInitialVelocityScale")) != NULL)
		{ maxInitialVelocityScale = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("InitialVelocityTarget")) != NULL)
		{
			const VFX_Target target = input->GetTarget();
			initialVelocityTarget = target;
		}
		if ((input = parameters.Find("InitialVelocitySocket")) != NULL)
		{ initialVelocitySocket = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("InitialVelocitySnapBone")) != NULL)
		{ initialVelocitySnapBone = input->GetString(); }
		if ((input = parameters.Find("InitialVelocityTargetBodyPart")) != NULL)
		{ initialVelocityTargetBodyPart = input->GetBool(); }
		if ((input = parameters.Find("MinExplosion")) != NULL)
		{ minExplosion = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxExplosion")) != NULL)
		{ maxExplosion = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinVibration")) != NULL)
		{ minVibration = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxVibration")) != NULL)
		{ maxVibration = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("RotatableParticles")) != NULL)
		{ rotatableParticles = input->GetBool(); }
		if ((input = parameters.Find("MinParticleRotation")) != NULL)
		{
			const VFX_Rotation rotation = input->GetRotation();
			minParticleRotation = rotation;
		}
		if ((input = parameters.Find("MaxParticleRotation")) != NULL)
		{
			const VFX_Rotation rotation = input->GetRotation();
			maxParticleRotation = rotation;
		}
		if ((input = parameters.Find("MinParticleSpin")) != NULL)
		{
			const VFX_Rotation rotation = input->GetRotation();
			minParticleSpin = rotation;
		}
		if ((input = parameters.Find("MaxParticleSpin")) != NULL)
		{
			const VFX_Rotation rotation = input->GetRotation();
			maxParticleSpin = rotation;
		}
		if ((input = parameters.Find("FaceVelocityVector")) != NULL)
		{ faceVelocityVector = input->GetBool(); }
		if ((input = parameters.Find("ParticleDeathEffect")) != NULL)
		{ particleDeathEffect = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("NumColorsUsed")) != NULL)
		{ numColorsUsed = input->GetInteger(); }
		if ((input = parameters.Find("Color1")) != NULL)
		{ { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color1, converted.values, sizeof(converted.values)); } }
		if ((input = parameters.Find("ColorTime1")) != NULL)
		{ colorTime1 = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("Color2")) != NULL)
		{ { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color2, converted.values, sizeof(converted.values)); } }
		if ((input = parameters.Find("ColorTime2")) != NULL)
		{ colorTime2 = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("Color3")) != NULL)
		{ { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color3, converted.values, sizeof(converted.values)); } }
		if ((input = parameters.Find("ColorTime3")) != NULL)
		{ colorTime3 = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("Color4")) != NULL)
		{ { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color4, converted.values, sizeof(converted.values)); } }
		if ((input = parameters.Find("ColorTime4")) != NULL)
		{ colorTime4 = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("Color5")) != NULL)
		{ { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color5, converted.values, sizeof(converted.values)); } }
		if ((input = parameters.Find("ColorTime5")) != NULL)
		{ colorTime5 = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("Color6")) != NULL)
		{ { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color6, converted.values, sizeof(converted.values)); } }
		if ((input = parameters.Find("ColorTime6")) != NULL)
		{ colorTime6 = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("Color7")) != NULL)
		{ { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color7, converted.values, sizeof(converted.values)); } }
		if ((input = parameters.Find("ColorTime7")) != NULL)
		{ colorTime7 = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("Color8")) != NULL)
		{ { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color8, converted.values, sizeof(converted.values)); } }
		if ((input = parameters.Find("ColorTime8")) != NULL)
		{ colorTime8 = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AdjustColor")) != NULL)
		{ adjustColor = input->GetBool(); }
		if ((input = parameters.Find("AdjustColorOverLifetime")) != NULL)
		{ adjustColorOverLifetime = input->GetBool(); }
		if ((input = parameters.Find("AdjustSystemColor")) != NULL)
		{ adjustSystemColor = input->GetBool(); }
		if ((input = parameters.Find("ColorWaveform")) != NULL)
		{ colorWaveform = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("ColorFreq")) != NULL)
		{ colorFreq = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("ColorPhase")) != NULL)
		{ colorPhase = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AdjustParticleAlpha")) != NULL)
		{ adjustParticleAlpha = input->GetBool(); }
		if ((input = parameters.Find("AdjustParticleAlphaOverLifetime")) != NULL)
		{ adjustParticleAlphaOverLifetime = input->GetBool(); }
		if ((input = parameters.Find("StartParticleAlpha")) != NULL)
		{ startParticleAlpha = static_cast<BYTE>(input->GetEnum()); }
		if ((input = parameters.Find("EndParticleAlpha")) != NULL)
		{ endParticleAlpha = static_cast<BYTE>(input->GetEnum()); }
		if ((input = parameters.Find("ParticleAlphaWaveform")) != NULL)
		{ particleAlphaWaveform = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("ParticleAlphaFreq")) != NULL)
		{ particleAlphaFreq = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("ParticleAlphaPhase")) != NULL)
		{ particleAlphaPhase = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AdjustSystemAlpha")) != NULL)
		{ adjustSystemAlpha = input->GetBool(); }
		if ((input = parameters.Find("AdjustSystemAlphaOverLifetime")) != NULL)
		{ adjustSystemAlphaOverLifetime = input->GetBool(); }
		if ((input = parameters.Find("StartSystemAlpha")) != NULL)
		{ startSystemAlpha = static_cast<BYTE>(input->GetEnum()); }
		if ((input = parameters.Find("EndSystemAlpha")) != NULL)
		{ endSystemAlpha = static_cast<BYTE>(input->GetEnum()); }
		if ((input = parameters.Find("SystemAlphaWaveform")) != NULL)
		{ systemAlphaWaveform = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("SystemAlphaFreq")) != NULL)
		{ systemAlphaFreq = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("SystemAlphaPhase")) != NULL)
		{ systemAlphaPhase = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AdjustParticleScale")) != NULL)
		{ adjustParticleScale = input->GetBool(); }
		if ((input = parameters.Find("AdjustParticleScaleOverLifetime")) != NULL)
		{ adjustParticleScaleOverLifetime = input->GetBool(); }
		if ((input = parameters.Find("StartParticleScale")) != NULL)
		{ startParticleScale = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("EndParticleScale")) != NULL)
		{ endParticleScale = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("ParticleScaleWaveform")) != NULL)
		{ particleScaleWaveform = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("ParticleScaleFreq")) != NULL)
		{ particleScaleFreq = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("ParticleScalePhase")) != NULL)
		{ particleScalePhase = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AdjustSystemScale")) != NULL)
		{ adjustSystemScale = input->GetBool(); }
		if ((input = parameters.Find("AdjustSystemScaleOverLifetime")) != NULL)
		{ adjustSystemScaleOverLifetime = input->GetBool(); }
		if ((input = parameters.Find("StartSystemScale")) != NULL)
		{ startSystemScale = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("EndSystemScale")) != NULL)
		{ endSystemScale = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("SystemScaleWaveform")) != NULL)
		{ systemScaleWaveform = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("SystemScaleFreq")) != NULL)
		{ systemScaleFreq = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("SystemScalePhase")) != NULL)
		{ systemScalePhase = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AdjustMinDimX")) != NULL)
		{ adjustMinDimX = input->GetBool(); }
		if ((input = parameters.Find("AdjustMinDimXOverLifetime")) != NULL)
		{ adjustMinDimXOverLifetime = input->GetBool(); }
		if ((input = parameters.Find("FinalMinDimX")) != NULL)
		{ finalMinDimX = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinDimXWaveform")) != NULL)
		{ minDimXWaveform = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("MinDimXFreq")) != NULL)
		{ minDimXFreq = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinDimXPhase")) != NULL)
		{ minDimXPhase = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AdjustMinDimY")) != NULL)
		{ adjustMinDimY = input->GetBool(); }
		if ((input = parameters.Find("AdjustMinDimYOverLifetime")) != NULL)
		{ adjustMinDimYOverLifetime = input->GetBool(); }
		if ((input = parameters.Find("FinalMinDimY")) != NULL)
		{ finalMinDimY = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinDimYWaveform")) != NULL)
		{ minDimYWaveform = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("MinDimYFreq")) != NULL)
		{ minDimYFreq = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinDimYPhase")) != NULL)
		{ minDimYPhase = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AdjustMinDimZ")) != NULL)
		{ adjustMinDimZ = input->GetBool(); }
		if ((input = parameters.Find("AdjustMinDimZOverLifetime")) != NULL)
		{ adjustMinDimZOverLifetime = input->GetBool(); }
		if ((input = parameters.Find("FinalMinDimZ")) != NULL)
		{ finalMinDimZ = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinDimZWaveform")) != NULL)
		{ minDimZWaveform = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("MinDimZFreq")) != NULL)
		{ minDimZFreq = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MinDimZPhase")) != NULL)
		{ minDimZPhase = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AdjustMaxDimX")) != NULL)
		{ adjustMaxDimX = input->GetBool(); }
		if ((input = parameters.Find("AdjustMaxDimXOverLifetime")) != NULL)
		{ adjustMaxDimXOverLifetime = input->GetBool(); }
		if ((input = parameters.Find("FinalMaxDimX")) != NULL)
		{ finalMaxDimX = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxDimXWaveform")) != NULL)
		{ maxDimXWaveform = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("MaxDimXFreq")) != NULL)
		{ maxDimXFreq = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxDimXPhase")) != NULL)
		{ maxDimXPhase = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AdjustMaxDimY")) != NULL)
		{ adjustMaxDimY = input->GetBool(); }
		if ((input = parameters.Find("AdjustMaxDimYOverLifetime")) != NULL)
		{ adjustMaxDimYOverLifetime = input->GetBool(); }
		if ((input = parameters.Find("FinalMaxDimY")) != NULL)
		{ finalMaxDimY = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxDimYWaveform")) != NULL)
		{ maxDimYWaveform = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("MaxDimYFreq")) != NULL)
		{ maxDimYFreq = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxDimYPhase")) != NULL)
		{ maxDimYPhase = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("AdjustMaxDimZ")) != NULL)
		{ adjustMaxDimZ = input->GetBool(); }
		if ((input = parameters.Find("AdjustMaxDimZOverLifetime")) != NULL)
		{ adjustMaxDimZOverLifetime = input->GetBool(); }
		if ((input = parameters.Find("FinalMaxDimZ")) != NULL)
		{ finalMaxDimZ = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxDimZWaveform")) != NULL)
		{ maxDimZWaveform = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("MaxDimZFreq")) != NULL)
		{ maxDimZFreq = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("MaxDimZPhase")) != NULL)
		{ maxDimZPhase = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("ParticleTrailType")) != NULL)
		{ particleTrailType = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("TrailInterval")) != NULL)
		{ trailInterval = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("TrailLifetime")) != NULL)
		{ trailLifetime = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("TrailRenderPhase")) != NULL)
		{ trailRenderPhase = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("TrailModulate2x")) != NULL)
		{ trailModulate2x = input->GetBool(); }
		if ((input = parameters.Find("BeginTrailColor")) != NULL)
		{ { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(beginTrailColor, converted.values, sizeof(converted.values)); } }
		if ((input = parameters.Find("BeginTrailAlpha")) != NULL)
		{ beginTrailAlpha = static_cast<BYTE>(input->GetEnum()); }
		if ((input = parameters.Find("BeginTrailScale")) != NULL)
		{ beginTrailScale = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("EndTrailColor")) != NULL)
		{ { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(endTrailColor, converted.values, sizeof(converted.values)); } }
		if ((input = parameters.Find("EndTrailAlpha")) != NULL)
		{ endTrailAlpha = static_cast<BYTE>(input->GetEnum()); }
		if ((input = parameters.Find("EndTrailScale")) != NULL)
		{ endTrailScale = static_cast<float>(input->GetFloat()); }
		if ((input = parameters.Find("RibbonTexture")) != NULL)
		{ ribbonTexture.type = input->GetResourceType(); ribbonTexture.name = input->GetResourceName(); }
		if ((input = parameters.Find("RibbonTextureUVs")) != NULL)
		{ { const JE::VFX_TextureCoordinates converted = input->GetTextureUVs(); memcpy(ribbonTextureUVs, &converted, sizeof(converted)); } }
		if ((input = parameters.Find("RibbonOrigin")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); ribbonOrigin = converted; } }
		if ((input = parameters.Find("RibbonEndPoint")) != NULL)
		{ { const JE::VFX_Vector converted = input->GetVector3(); ribbonEndPoint = converted; } }
		if ((input = parameters.Find("RibbonAlignment")) != NULL)
		{ ribbonAlignment = static_cast<LONG>(input->GetEnum()); }
		if ((input = parameters.Find("PerformCollision")) != NULL)
		{ performCollision = input->GetBool(); }
		if ((input = parameters.Find("FaceNormal")) != NULL)
		{ faceNormal = input->GetBool(); }
		if ((input = parameters.Find("CollisionEffect")) != NULL)
		{ collisionEffect = static_cast<LONG>(input->GetEnum()); }
	}
}

Gfx_ParticleConfiguration::Gfx_ParticleConfiguration()
{
	beginTrailColorZ = 0;
	beginTrailColorY = 0;
	beginTrailColorX = 0;
	endTrailColorZ = 0;
	endTrailColorY = 0;
	endTrailColorX = 0;
	minDimensionsZ = 0.0f;
	minDimensionsY = 0.0f;
	minDimensionsX = 0.0f;
	maxDimensionsZ = 0.0f;
	maxDimensionsY = 0.0f;
	maxDimensionsX = 0.0f;
	rotationZ = 0.0f;
	rotationY = 0.0f;
	rotationX = 0.0f;
	acceleration1Z = 0.0f;
	acceleration1Y = 0.0f;
	acceleration1X = 0.0f;
	rotateAccel1WithTarget = 0;
	acceleration2Z = 0.0f;
	acceleration2Y = 0.0f;
	acceleration2X = 0.0f;
	rotateAccel2WithTarget = 1;
	value250 = 0;
	minInitialVelocityX = -1.0f;
	minInitialVelocityY = -1.0f;
	minInitialVelocityZ = -1.0f;
	initialVelocityTargetUsed = 0;
	maxInitialVelocityX = 1.0f;
	maxInitialVelocityY = 1.0f;
	maxInitialVelocityZ = 1.0f;
	minParticleAlpha = 1.0f;
	maxParticleAlpha = 1.0f;
	minParticleLifetime = 1.0f;
	maxParticleLifetime = 2.0f;
	minParticleSize = 1.0f;
	maxParticleSize = 10.0f;
	minParticlesPerAdd = 1;
	maxParticlesPerAdd = 1;
	maxParticles = 50;
	minAddRate = 100.0f;
	maxAddRate = 100.0f;
	emitterLifetime = 0.0f;
	startSystemScale = 1.0f;
	endSystemScale = 1.0f;
	adjustSystemScaleOverLifetime = 0;
	systemScaleWaveform = 1;
	systemScaleFreq = 1.0f;
	systemScalePhase = 0.0f;
	startParticleScale = 1.0f;
	endParticleScale = 1.0f;
	adjustParticleScaleOverLifetime = 0;
	particleScaleWaveform = 1;
	particleScaleFreq = 1.0f;
	particleScalePhase = 0.0f;
	startSystemAlpha = 1.0f;
	endSystemAlpha = 1.0f;
	adjustSystemAlphaOverLifetime = 0;
	systemAlphaWaveform = 1;
	systemAlphaFreq = 1.0f;
	systemAlphaPhase = 0.0f;
	startParticleAlpha = 1.0f;
	endParticleAlpha = 1.0f;
	adjustParticleAlphaOverLifetime = 0;
	particleAlphaWaveform = 1;
	particleAlphaFreq = 1.0f;
	particleAlphaPhase = 0.0f;
	adjustSystemColor = 0;
	adjustColorOverLifetime = 0;
	colorWaveform = 1;
	minDimXWaveform = 1;
	maxDimXWaveform = 1;
	minDimYWaveform = 1;
	maxDimYWaveform = 1;
	minDimZWaveform = 1;
	maxDimZWaveform = 1;
	colorFreq = 1.0f;
	colorPhase = 0.0f;
	adjustMinDimXOverLifetime = 0;
	minDimXFreq = 1.0f;
	minDimXPhase = 0.0f;
	finalMinDimX = 0.0f;
	adjustMaxDimXOverLifetime = 0;
	maxDimXFreq = 1.0f;
	maxDimXPhase = 0.0f;
	finalMaxDimX = 0.0f;
	adjustMinDimYOverLifetime = 0;
	minDimYFreq = 1.0f;
	minDimYPhase = 0.0f;
	finalMinDimY = 0.0f;
	adjustMaxDimYOverLifetime = 0;
	maxDimYFreq = 1.0f;
	maxDimYPhase = 0.0f;
	finalMaxDimY = 0.0f;
	adjustMinDimZOverLifetime = 0;
	minDimZFreq = 1.0f;
	minDimZPhase = 0.0f;
	finalMinDimZ = 0.0f;
	adjustMaxDimZOverLifetime = 0;
	maxDimZFreq = 1.0f;
	maxDimZPhase = 0.0f;
	finalMaxDimZ = 0.0f;
	numColorsUsed = 1;
	minParticleResistance = 1.0f;
	maxParticleResistance = 1.0f;
	minParticleMass = 1.0f;
	maxParticleMass = 1.0f;
	textureUVsX = 0.0f;
	textureUVsW = 0.0f;
	textureUVsZ = 1.0f;
	textureUVsY = 1.0f;
	emitterShape = 0;
	minInitialVelocityScale = 1.0f;
	maxInitialVelocityScale = 1.0f;
	minExplosion = 0.0f;
	maxExplosion = 0.0f;
	minVibration = 0.0f;
	maxVibration = 0.0f;
	particleTrailType = 0;
	ribbonOriginX = 0.0f;
	ribbonOriginY = 0.0f;
	ribbonOriginZ = 0.0f;
	ribbonEndPointX = 0.0f;
	ribbonEndPointY = 0.0f;
	ribbonEndPointZ = 0.0f;
	ribbonAlignment = 0;
	beginTrailColorX = 0;
	beginTrailColorY = 0;
	beginTrailColorZ = 0;
	beginTrailAlpha = -1;
	beginTrailScale = 1.0f;
	endTrailColorX = 0;
	endTrailColorY = 0;
	endTrailColorZ = 0;
	endTrailAlpha = 0;
	endTrailScale = 0.0f;
	ribbonTextureUVsX = 0.0f;
	ribbonTextureUVsY = 0.0f;
	ribbonTextureUVsZ = 1.0f;
	ribbonTextureUVsW = 1.0f;
	renderPhase = 0;
	flags = 0;
	memset(textureName, 0, sizeof(textureName));
	memset(modelName, 0, sizeof(modelName));
	ribbonTextureName[0] = 0; // Original initializes only this byte.
	colors[0][0] = colors[0][1] = colors[0][2] = 255; colorTimes[0] = 0.0f * 0.14285715f;
	colors[1][0] = colors[1][1] = colors[1][2] = 255; colorTimes[1] = 1.0f * 0.14285715f;
	colors[2][0] = colors[2][1] = colors[2][2] = 255; colorTimes[2] = 2.0f * 0.14285715f;
	colors[3][0] = colors[3][1] = colors[3][2] = 255; colorTimes[3] = 3.0f * 0.14285715f;
	colors[4][0] = colors[4][1] = colors[4][2] = 255; colorTimes[4] = 4.0f * 0.14285715f;
	colors[5][0] = colors[5][1] = colors[5][2] = 255; colorTimes[5] = 5.0f * 0.14285715f;
	colors[6][0] = colors[6][1] = colors[6][2] = 255; colorTimes[6] = 6.0f * 0.14285715f;
	colors[7][0] = colors[7][1] = colors[7][2] = 255; colorTimes[7] = 7.0f * 0.14285715f;
}

namespace
{
	JE::VFX_ParticleSystem* currentEffect = NULL; // Original708118.
	Gfx_ModelInstance* currentSource = NULL; //708104.
	Gfx_ModelInstance* currentTarget = NULL; //708110.
	JE::VFX_VEGInstance* currentGroup = NULL; //708108.
	const D3DXMATRIX* currentSourceTransform = NULL; //708114.
	const D3DXMATRIX* currentTargetTransform = NULL; //70810C.
	void __cdecl ParticleSpawn(float x, float y, float z, int useTarget,
		float* vx, float* vy, float* vz) //4F8750; absent owner leaves outputs unwritten.
	{
		if (currentEffect)
			currentEffect->ComputeInitialVelocity(x, y, z, currentSource, currentTarget,
				currentGroup, useTarget, currentSourceTransform, currentTargetTransform, vx, vy, vz);
	}
	void __cdecl ParticleDeath(float x, float y, float z, float time) //4F87B0.
	{
		if (currentEffect) currentEffect->OnParticleDeath(x, y, z, time);
	}


}

namespace JE
{
	Gfx_ParticleSystem* VFX_ParticleSystem::CreateParticleSystem(const VFX_ParticleData& input)
	{
		Gfx_ParticleSystem* result = new Gfx_ParticleSystem;
		result->scale = this->scale;
		result->SetSpawnCallback(ParticleSpawn);
		result->SetDeathCallback(input.particleDeathEffect == -1 ? NULL : ParticleDeath);
		Gfx_ParticleConfiguration config;
		config.flags = static_cast<DWORD>(0);
		config.minDimensionsX = static_cast<float>(input.minDimensions.xy.values[0]);
		config.minDimensionsY = static_cast<float>(input.minDimensions.xy.values[1]);
		config.minDimensionsZ = static_cast<float>(input.minDimensions.z);
		config.maxDimensionsX = static_cast<float>(input.maxDimensions.xy.values[0]);
		config.maxDimensionsY = static_cast<float>(input.maxDimensions.xy.values[1]);
		config.maxDimensionsZ = static_cast<float>(input.maxDimensions.z);
		config.rotationX = static_cast<float>(input.rotation.values[0] * 0.017453292f);
		config.rotationY = static_cast<float>(input.rotation.values[1] * 0.017453292f);
		config.rotationZ = static_cast<float>(input.rotation.values[2] * 0.017453292f);
		config.acceleration1X = static_cast<float>(input.acceleration1.xy.values[0]);
		config.acceleration1Y = static_cast<float>(input.acceleration1.xy.values[1]);
		config.acceleration1Z = static_cast<float>(input.acceleration1.z);
		config.rotateAccel1WithTarget = static_cast<BYTE>(input.rotateAccel1WithTarget);
		config.acceleration2X = static_cast<float>(input.acceleration2.xy.values[0]);
		config.acceleration2Y = static_cast<float>(input.acceleration2.xy.values[1]);
		config.acceleration2Z = static_cast<float>(input.acceleration2.z);
		config.rotateAccel2WithTarget = static_cast<BYTE>(input.rotateAccel2WithTarget);
		config.emitterLifetime = static_cast<float>(input.emitterLifetime);
		config.numColorsUsed = static_cast<LONG>(input.numColorsUsed);
		config.colors[0][0] = static_cast<LONG>(input.color1[0]);
		config.colors[0][1] = static_cast<LONG>(input.color1[1]);
		config.colors[0][2] = static_cast<LONG>(input.color1[2]);
		config.colors[1][0] = static_cast<LONG>(input.color2[0]);
		config.colors[1][1] = static_cast<LONG>(input.color2[1]);
		config.colors[1][2] = static_cast<LONG>(input.color2[2]);
		config.colors[2][0] = static_cast<LONG>(input.color3[0]);
		config.colors[2][1] = static_cast<LONG>(input.color3[1]);
		config.colors[2][2] = static_cast<LONG>(input.color3[2]);
		config.colors[3][0] = static_cast<LONG>(input.color4[0]);
		config.colors[3][1] = static_cast<LONG>(input.color4[1]);
		config.colors[3][2] = static_cast<LONG>(input.color4[2]);
		config.colors[4][0] = static_cast<LONG>(input.color5[0]);
		config.colors[4][1] = static_cast<LONG>(input.color5[1]);
		config.colors[4][2] = static_cast<LONG>(input.color5[2]);
		config.colors[5][0] = static_cast<LONG>(input.color6[0]);
		config.colors[5][1] = static_cast<LONG>(input.color6[1]);
		config.colors[5][2] = static_cast<LONG>(input.color6[2]);
		config.colors[6][0] = static_cast<LONG>(input.color7[0]);
		config.colors[6][1] = static_cast<LONG>(input.color7[1]);
		config.colors[6][2] = static_cast<LONG>(input.color7[2]);
		config.colors[7][0] = static_cast<LONG>(input.color8[0]);
		config.colors[7][1] = static_cast<LONG>(input.color8[1]);
		config.colors[7][2] = static_cast<LONG>(input.color8[2]);
		config.colorTimes[0] = static_cast<float>(input.colorTime1);
		config.colorTimes[1] = static_cast<float>(input.colorTime2);
		config.colorTimes[2] = static_cast<float>(input.colorTime3);
		config.colorTimes[3] = static_cast<float>(input.colorTime4);
		config.colorTimes[4] = static_cast<float>(input.colorTime5);
		config.colorTimes[5] = static_cast<float>(input.colorTime6);
		config.colorTimes[6] = static_cast<float>(input.colorTime7);
		config.colorTimes[7] = static_cast<float>(input.colorTime8);
		config.minParticlesPerAdd = static_cast<LONG>(input.minParticlesPerAdd);
		config.maxParticlesPerAdd = static_cast<LONG>(input.maxParticlesPerAdd);
		config.maxParticles = static_cast<LONG>(input.useMaxParticles ? input.maxParticles : -1);
		config.minAddRate = static_cast<float>(input.minAddRate);
		config.maxAddRate = static_cast<float>(input.maxAddRate);
		config.initialVelocityTargetUsed = static_cast<BOOL>(input.initialVelocityTarget.Value() != 0);
		config.minInitialVelocityX = static_cast<float>(input.minInitialVelocity.xy.values[0]);
		config.minInitialVelocityY = static_cast<float>(input.minInitialVelocity.xy.values[1]);
		config.minInitialVelocityZ = static_cast<float>(input.minInitialVelocity.z);
		config.maxInitialVelocityX = static_cast<float>(input.maxInitialVelocity.xy.values[0]);
		config.maxInitialVelocityY = static_cast<float>(input.maxInitialVelocity.xy.values[1]);
		config.maxInitialVelocityZ = static_cast<float>(input.maxInitialVelocity.z);
		config.minParticleAlpha = static_cast<float>(input.minParticleAlpha * 0.0039215689f);
		config.maxParticleAlpha = static_cast<float>(input.maxParticleAlpha * 0.0039215689f);
		config.minParticleLifetime = static_cast<float>(input.minParticleLifetime);
		config.maxParticleLifetime = static_cast<float>(input.maxParticleLifetime);
		config.minParticleSize = static_cast<float>(input.minParticleSize);
		config.maxParticleSize = static_cast<float>(input.maxParticleSize);
		config.startSystemScale = static_cast<float>(input.startSystemScale);
		config.endSystemScale = static_cast<float>(input.endSystemScale);
		config.adjustSystemScaleOverLifetime = static_cast<BYTE>(input.adjustSystemScaleOverLifetime);
		config.systemScaleWaveform = static_cast<LONG>(input.systemScaleWaveform);
		config.systemScaleFreq = static_cast<float>(input.systemScaleFreq);
		config.systemScalePhase = static_cast<float>(input.systemScalePhase);
		config.startParticleScale = static_cast<float>(input.startParticleScale);
		config.endParticleScale = static_cast<float>(input.endParticleScale);
		config.adjustParticleScaleOverLifetime = static_cast<BYTE>(input.adjustParticleScaleOverLifetime);
		config.particleScaleWaveform = static_cast<LONG>(input.particleScaleWaveform);
		config.particleScaleFreq = static_cast<float>(input.particleScaleFreq);
		config.particleScalePhase = static_cast<float>(input.particleScalePhase);
		config.startSystemAlpha = static_cast<float>(input.startSystemAlpha * 0.0039215689f);
		config.endSystemAlpha = static_cast<float>(input.endSystemAlpha * 0.0039215689f);
		config.adjustSystemAlphaOverLifetime = static_cast<BYTE>(input.adjustSystemAlphaOverLifetime);
		config.systemAlphaWaveform = static_cast<LONG>(input.systemAlphaWaveform);
		config.systemAlphaFreq = static_cast<float>(input.systemAlphaFreq);
		config.systemAlphaPhase = static_cast<float>(input.systemAlphaPhase);
		config.startParticleAlpha = static_cast<float>(input.startParticleAlpha * 0.0039215689f);
		config.endParticleAlpha = static_cast<float>(input.endParticleAlpha * 0.0039215689f);
		config.adjustParticleAlphaOverLifetime = static_cast<BYTE>(input.adjustParticleAlphaOverLifetime);
		config.particleAlphaWaveform = static_cast<LONG>(input.particleAlphaWaveform);
		config.particleAlphaFreq = static_cast<float>(input.particleAlphaFreq);
		config.particleAlphaPhase = static_cast<float>(input.particleAlphaPhase);
		config.adjustSystemColor = static_cast<BYTE>(input.adjustSystemColor);
		config.adjustColorOverLifetime = static_cast<BYTE>(input.adjustColorOverLifetime);
		config.colorWaveform = static_cast<LONG>(input.colorWaveform);
		config.colorFreq = static_cast<float>(input.colorFreq);
		config.colorPhase = static_cast<float>(input.colorPhase);
		config.adjustMinDimXOverLifetime = static_cast<BYTE>(input.adjustMinDimXOverLifetime);
		config.minDimXWaveform = static_cast<LONG>(input.minDimXWaveform);
		config.minDimXFreq = static_cast<float>(input.minDimXFreq);
		config.minDimXPhase = static_cast<float>(input.minDimXPhase);
		config.finalMinDimX = static_cast<float>(input.finalMinDimX);
		config.adjustMaxDimXOverLifetime = static_cast<BYTE>(input.adjustMaxDimXOverLifetime);
		config.maxDimXWaveform = static_cast<LONG>(input.maxDimXWaveform);
		config.maxDimXFreq = static_cast<float>(input.maxDimXFreq);
		config.maxDimXPhase = static_cast<float>(input.maxDimXPhase);
		config.finalMaxDimX = static_cast<float>(input.finalMaxDimX);
		config.adjustMinDimYOverLifetime = static_cast<BYTE>(input.adjustMinDimYOverLifetime);
		config.minDimYWaveform = static_cast<LONG>(input.minDimYWaveform);
		config.minDimYFreq = static_cast<float>(input.minDimYFreq);
		config.minDimYPhase = static_cast<float>(input.minDimYPhase);
		config.finalMinDimY = static_cast<float>(input.finalMinDimY);
		config.adjustMaxDimYOverLifetime = static_cast<BYTE>(input.adjustMaxDimYOverLifetime);
		config.maxDimYWaveform = static_cast<LONG>(input.maxDimYWaveform);
		config.maxDimYFreq = static_cast<float>(input.maxDimYFreq);
		config.maxDimYPhase = static_cast<float>(input.maxDimYPhase);
		config.finalMaxDimY = static_cast<float>(input.finalMaxDimY);
		config.adjustMinDimZOverLifetime = static_cast<BYTE>(input.adjustMinDimZOverLifetime);
		config.minDimZWaveform = static_cast<LONG>(input.minDimZWaveform);
		config.minDimZFreq = static_cast<float>(input.minDimZFreq);
		config.minDimZPhase = static_cast<float>(input.minDimZPhase);
		config.finalMinDimZ = static_cast<float>(input.finalMinDimZ);
		config.adjustMaxDimZOverLifetime = static_cast<BYTE>(input.adjustMaxDimZOverLifetime);
		config.maxDimZWaveform = static_cast<LONG>(input.maxDimZWaveform);
		config.maxDimZFreq = static_cast<float>(input.maxDimZFreq);
		config.maxDimZPhase = static_cast<float>(input.maxDimZPhase);
		config.finalMaxDimZ = static_cast<float>(input.finalMaxDimZ);
		config.minParticleResistance = static_cast<float>(input.minParticleResistance);
		config.maxParticleResistance = static_cast<float>(input.maxParticleResistance);
		config.minParticleMass = static_cast<float>(input.minParticleMass);
		config.maxParticleMass = static_cast<float>(input.maxParticleMass);
		config.textureUVsX = static_cast<float>(input.textureUVs[0]);
		config.textureUVsW = static_cast<float>(input.textureUVs[3]);
		config.textureUVsZ = static_cast<float>(input.textureUVs[2]);
		config.textureUVsY = static_cast<float>(input.textureUVs[1]);
		config.emitterShape = static_cast<LONG>(input.emitterShape);
		config.minInitialVelocityScale = static_cast<float>(input.minInitialVelocityScale);
		config.maxInitialVelocityScale = static_cast<float>(input.maxInitialVelocityScale);
		config.minExplosion = static_cast<float>(input.minExplosion);
		config.maxExplosion = static_cast<float>(input.maxExplosion);
		config.minVibration = static_cast<float>(input.minVibration);
		config.maxVibration = static_cast<float>(input.maxVibration);
		config.particleTrailType = static_cast<LONG>(input.particleTrailType);
		config.trailInterval = static_cast<float>(input.trailInterval);
		config.trailLifetime = static_cast<float>(input.trailLifetime);
		config.ribbonOriginX = static_cast<float>(input.ribbonOrigin.xy.values[0]);
		config.ribbonOriginY = static_cast<float>(input.ribbonOrigin.xy.values[1]);
		config.ribbonOriginZ = static_cast<float>(input.ribbonOrigin.z);
		config.ribbonEndPointX = static_cast<float>(input.ribbonEndPoint.xy.values[0]);
		config.ribbonEndPointY = static_cast<float>(input.ribbonEndPoint.xy.values[1]);
		config.ribbonEndPointZ = static_cast<float>(input.ribbonEndPoint.z);
		config.ribbonAlignment = static_cast<LONG>(input.ribbonAlignment);
		config.beginTrailColorX = static_cast<LONG>(input.beginTrailColor[0]);
		config.beginTrailColorY = static_cast<LONG>(input.beginTrailColor[1]);
		config.beginTrailColorZ = static_cast<LONG>(input.beginTrailColor[2]);
		config.beginTrailAlpha = static_cast<BYTE>(input.beginTrailAlpha);
		config.beginTrailScale = static_cast<float>(input.beginTrailScale);
		config.endTrailColorX = static_cast<LONG>(input.endTrailColor[0]);
		config.endTrailColorY = static_cast<LONG>(input.endTrailColor[1]);
		config.endTrailColorZ = static_cast<LONG>(input.endTrailColor[2]);
		config.endTrailAlpha = static_cast<BYTE>(input.endTrailAlpha);
		config.endTrailScale = static_cast<float>(input.endTrailScale);
		config.ribbonTextureUVsX = static_cast<float>(input.ribbonTextureUVs[0]);
		config.ribbonTextureUVsW = static_cast<float>(input.ribbonTextureUVs[3]);
		config.ribbonTextureUVsZ = static_cast<float>(input.ribbonTextureUVs[2]);
		config.ribbonTextureUVsY = static_cast<float>(input.ribbonTextureUVs[1]);
		config.renderPhase = static_cast<LONG>(input.renderPhase);
		config.trailRenderPhase = static_cast<LONG>(input.trailRenderPhase);
		config.model_UseRenderPhase = static_cast<BYTE>(input.model_UseRenderPhase);
		config.model_UseColor = static_cast<BYTE>(input.model_UseColor);
		config.model_UseAlpha = static_cast<BYTE>(input.model_UseAlpha);
		config.model_IgnoreLighting = static_cast<BYTE>(input.model_IgnoreLighting);
		config.rotatableParticles = static_cast<BYTE>(input.rotatableParticles);
		config.minParticleRotationX = static_cast<float>(input.minParticleRotation.values[0] * 0.017453292f);
		config.minParticleRotationY = static_cast<float>(input.minParticleRotation.values[1] * 0.017453292f);
		config.minParticleRotationZ = static_cast<float>(input.minParticleRotation.values[2] * 0.017453292f);
		config.maxParticleRotationX = static_cast<float>(input.maxParticleRotation.values[0] * 0.017453292f);
		config.maxParticleRotationY = static_cast<float>(input.maxParticleRotation.values[1] * 0.017453292f);
		config.maxParticleRotationZ = static_cast<float>(input.maxParticleRotation.values[2] * 0.017453292f);
		config.minParticleSpinX = static_cast<float>(input.minParticleSpin.values[0] * 0.017453292f);
		config.minParticleSpinY = static_cast<float>(input.minParticleSpin.values[1] * 0.017453292f);
		config.minParticleSpinZ = static_cast<float>(input.minParticleSpin.values[2] * 0.017453292f);
		config.maxParticleSpinX = static_cast<float>(input.maxParticleSpin.values[0] * 0.017453292f);
		config.maxParticleSpinY = static_cast<float>(input.maxParticleSpin.values[1] * 0.017453292f);
		config.maxParticleSpinZ = static_cast<float>(input.maxParticleSpin.values[2] * 0.017453292f);
		config.faceVelocityVectorUsed = static_cast<BYTE>(input.faceVelocityVector != 0);
		config.modulate2x = static_cast<BYTE>(input.modulate2x);
		config.trailModulate2x = static_cast<BYTE>(input.trailModulate2x);
		config.flags = input.model.name.empty() ? 0 : 2;
		if (input.useEmitterLifetime) config.flags |= 4;
		if (input.autoAdd) config.flags |= 8;
		if (input.adjustSystemScale) config.flags |= 0x20;
		if (input.adjustParticleScale) config.flags |= 0x40;
		if (input.adjustSystemAlpha) config.flags |= 0x80;
		if (input.adjustParticleAlpha) config.flags |= 0x100;
		if (input.adjustColor) config.flags |= 0x200;
		if (input.adjustMinDimX) config.flags |= 0x400;
		if (input.adjustMinDimY) config.flags |= 0x800;
		if (input.adjustMinDimZ) config.flags |= 0x1000;
		if (input.adjustMaxDimX) config.flags |= 0x2000;
		if (input.adjustMaxDimY) config.flags |= 0x4000;
		if (input.adjustMaxDimZ) config.flags |= 0x8000;
		_mbsnbcpy(reinterpret_cast<unsigned char*>(config.textureName),
			reinterpret_cast<const unsigned char*>(input.texture.name.c_str()), 260);
		_mbsnbcpy(reinterpret_cast<unsigned char*>(config.modelName),
			reinterpret_cast<const unsigned char*>(input.model.name.c_str()), 260);
		_mbsnbcpy(reinterpret_cast<unsigned char*>(config.ribbonTextureName),
			reinterpret_cast<const unsigned char*>(input.ribbonTexture.name.c_str()), 260);
		result->Configure(config); // 0x513070 replaces configuration/cache only.
		result->SetTexture(0, 1, 1);
		result->SetModel(0, 1, 1);
		return result;
	}
	VFX_ParticleSystem::VFX_ParticleSystem()
	{
		particles = NULL;
		lastTime = 0;
	}
	VFX_ParticleSystem::~VFX_ParticleSystem()
	{
		delete particles;
		ClearSpawnedEffects();
	}
	void VFX_ParticleSystem::ClearSpawnedEffects()
	{
		for (std::list<TriggeredEffect>::iterator i = spawnedEffects.begin(); i != spawnedEffects.end(); ++i)
			delete i->child;
		spawnedEffects.clear();
	}
	void VFX_ParticleSystem::QueueTriggeredEffect(LONG index, D3DXVECTOR3 position, float time) //4F8680.
	{
		TriggeredEffect effect;
		effect.effectIndex = index;
		effect.child = NULL;
		effect.position = position;
		effect.time = time;
		spawnedEffects.push_back(effect);
	}
	void VFX_ParticleSystem::OnParticleDeath(float x, float y, float z, float time) //4F86F0.
	{
		if (data.particleDeathEffect != -1)
			QueueTriggeredEffect(data.particleDeathEffect, D3DXVECTOR3(x, y, z), time);
	}
	VFX_ParticleSystem& VFX_ParticleSystem::CopyFrom(const VFX_ParticleSystem& input) //4F9850.
	{
		if (particles) delete particles;
		VFX_EffectInstance::CopyFrom(input);
		data = input.data;
		particles = CreateParticleSystem(data);
		if (input.particles->texture)
			particles->SetTexture(input.particles->texture, 0, 1);
		return *this;
	}
	void VFX_ParticleSystem::Load(const VFX_BaseData* input, float value) //4F69B0.
	{
		LoadBaseData(*input, value);
		if (input) SetData(input);
	}
	void VFX_ParticleSystem::SetData(const VFX_BaseData* input)
	{
		VFX_EffectInstance::SetData(input);
		data = *static_cast<const VFX_ParticleData*>(input);
		if (particles) { delete particles; particles = NULL; }
		particles = CreateParticleSystem(data); // 0x4F87E0, actual owner/config.
		if (!particles) return;
		if (!data.texture.name.empty())
		{
			const DWORD texture = Gfx_RequestTextureResource(data.texture.name.c_str(), UINT(-1), 3, 5, 0);
			particles->SetTexture(texture, 1, 1); //4F9563 reloads the owner after the callback.
		}
		if (!data.model.name.empty())
		{
			const DWORD model = Gfx_LoadModelData(data.model.name.c_str(), 1, 1, 0, 1.0f, 0x252);
			particles->SetModel(model, 1, 1); //4F95AC likewise reloads after model loading.
		}
		if (data.boundingBoxType == 0
			&& data.optimizedAABBMin.xy.values[0] == 0 && data.optimizedAABBMin.xy.values[1] == 0 && data.optimizedAABBMin.z == 0
			&& data.optimizedAABBMax.xy.values[0] == 0 && data.optimizedAABBMax.xy.values[1] == 0 && data.optimizedAABBMax.z == 0)
		{
			// Unordered duration uses the native ten-second fallback.
			float duration = data.duration >= 0 && data.duration < 10 ? data.duration : 10.0f;
			const float step = duration * 0.050000001f;
			D3DXVECTOR3 low(FLT_MAX, FLT_MAX, FLT_MAX), high(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			for (float time = 0; time < duration; time += step)
			{
				Update(time, NULL, NULL, NULL, NULL, NULL);
				D3DXVECTOR3 a, b;
				particles->GetBounds(a, b);
				for (int axis = 0; axis < 3; ++axis)
					if (!(low[axis] < a[axis])) low[axis] = a[axis];
				for (int axis = 0; axis < 3; ++axis)
					if (!(high[axis] > b[axis])) high[axis] = b[axis];
			}
			const double inverseScale = 1.0 / scale;
			data.optimizedAABBMin.xy.values[0] = float(double(low.x) * inverseScale);
			data.optimizedAABBMin.xy.values[1] = float(double(low.y) * inverseScale);
			data.optimizedAABBMin.z = float(double(low.z) * inverseScale);
			data.optimizedAABBMax.xy.values[0] = float(double(high.x) * inverseScale);
			data.optimizedAABBMax.xy.values[1] = float(double(high.y) * inverseScale);
			data.optimizedAABBMax.z = float(double(high.z) * inverseScale);
		}
	}
	void VFX_ParticleSystem::Reset()
	{
		active = 0;
		if (particles) particles->Reset();
		ClearSpawnedEffects();
	}
	BYTE VFX_ParticleSystem::GetFlag() const { return 0; }
	BYTE VFX_ParticleSystem::SkipBounds() const { return 0; }
	void VFX_ParticleSystem::SetEnabled(BYTE enabled)
	{
		flagC1 = enabled;
		if (particles) particles->SetEnabled(enabled);
	}
	void VFX_ParticleSystem::SetTransform(const D3DXMATRIX& input)
	{
		Gfx_ParticleSystem* target = particles;
		transform = input;
		if (target) target->secondTransform = input;
	}
	void VFX_ParticleSystem::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const
	{
		switch (data.boundingBoxType)
		{
		case 2:
		{
			const float x = transform._41, y = transform._42, z = transform._43;
			low.x = data.minBoundingBox.xy.values[0] * scale + x;
			low.y = data.minBoundingBox.xy.values[1] * scale + y;
			low.z = data.minBoundingBox.z * scale + z;
			high.x = data.maxBoundingBox.xy.values[0] * scale + x;
			high.y = data.maxBoundingBox.xy.values[1] * scale + y;
			high.z = data.maxBoundingBox.z * scale + z;
			return;
		}
		case 1:
			if (particles) particles->GetBounds(low, high);
			else VFX_EffectInstance::GetBounds(low, high);
			return;
		case 0:
		{
			const float x = transform._41, y = transform._42, z = transform._43;
			low.x = data.optimizedAABBMin.xy.values[0] * scale + x;
			low.y = data.optimizedAABBMin.xy.values[1] * scale + y;
			low.z = data.optimizedAABBMin.z * scale + z;
			high.x = data.optimizedAABBMax.xy.values[0] * scale + x;
			high.y = data.optimizedAABBMax.xy.values[1] * scale + y;
			high.z = data.optimizedAABBMax.z * scale + z;
			break;
		}
		}
	}
	void VFX_ParticleSystem::GetTransform(D3DXMATRIX& output) const
	{
		if (particles) output = particles->secondTransform;
		else output = transform;
	}
	BYTE VFX_ParticleSystem::IsFinished(float time) const
	{
		if (!VFX_EffectInstance::IsFinished(time)) return 0;
		if (data.duration > 0 && data.delay + data.duration < time) return 1;
		const Gfx_ParticleSystem* target = particles;
		if (!target || (data.minParticleLifetime < 0 && data.maxParticleLifetime < 0)) return 1;
		if (!(data.minParticleLifetime < 0) && !(data.maxParticleLifetime < 0))
			return static_cast<LONG>(target->particles.size()) <= 0 ? 1 : 0;
		typedef std::list<Gfx_ParticleInfo> ParticleList;
		const ParticleList& live = target->particles;
		const ParticleList::const_iterator end = live.end();
		for (ParticleList::const_iterator i = live.begin(); i != end; ++i)
			if (i->lifetime > 0) return 0;
		return 1;
	}
	void VFX_ParticleSystem::Stop()
	{
		flagC0 = 1;
		data.autoAdd = 0;
		if (particles) particles->configuration.flags &= ~8u;
	}
	BYTE VFX_ParticleSystem::GetInitialVelocityTransform(D3DXMATRIX& output, VFX_Target target,
		Gfx_ModelInstance* sourceModel, Gfx_ModelInstance* targetModel,
		VFX_VEGInstance* group, const D3DXMATRIX* sourceTransform,
		const D3DXMATRIX* targetTransform) const //4F7390.
	{
		const VFX_Vector offset = data.offset;
		const BYTE rotate = data.rotateWithTarget;
		//Original tests the effect's target selector for these explicit matrices,
		//then uses the distinct initial-velocity target for model/effect lookup.
		if (data.target.Value() == 1 && sourceTransform)
		{
			GetVfxTransform(sourceTransform, output, offset.xy.values, rotate, scale);
			return 1;
		}
		if (data.target.Value() == 2 && targetTransform)
		{
			GetVfxTransform(targetTransform, output, offset.xy.values, rotate, scale);
			return 1;
		}
		Gfx_ModelInstance* model = ResolveVfxModel(target, sourceModel, targetModel, group);
		if (model)
		{
			GetVfxModelTransform(model, data.initialVelocitySocket, data.initialVelocitySnapBone,
				output, NULL, 0, scale);
			return 1;
		}
		if (static_cast<LONG>(target.Value()) >= 3)
		{
			VFX_EffectInstance* effect = group->GetEffect(LONG(target.Value() - 3));
			if (effect)
			{
				effect->GetTransform(output);
				return 1;
			}
		}
		return 0;
	}
	BYTE VFX_ParticleSystem::GetInitialVelocityPosition(float* x, float* y, float* z,
		VFX_Target target, Gfx_ModelInstance* sourceModel, Gfx_ModelInstance* targetModel,
		VFX_VEGInstance* group, const D3DXMATRIX* sourceTransform,
		const D3DXMATRIX* targetTransform) const //4F75D0.
	{
		D3DXMATRIX matrix;
		if (GetInitialVelocityTransform(matrix, target, sourceModel, targetModel,
			group, sourceTransform, targetTransform))
		{
			*x = matrix._41;
			*y = matrix._42;
			*z = matrix._43;
			return 1;
		}
		return 0;
	}
	BYTE VFX_ParticleSystem::ComputeInitialVelocity(float x, float y, float z,
		Gfx_ModelInstance* sourceModel, Gfx_ModelInstance* targetModel,
		VFX_VEGInstance* group, int useTarget, const D3DXMATRIX* sourceTransform,
		const D3DXMATRIX* targetTransform, float* vx, float* vy, float* vz) const //4F84D0.
	{
		if (useTarget)
		{
			if (useTarget != 1) return BYTE(useTarget - 1);
			float targetX, targetY, targetZ;
			if (!GetInitialVelocityPosition(&targetX, &targetY, &targetZ,
				data.initialVelocityTarget, sourceModel, targetModel, group,
				sourceTransform, targetTransform)) return 0;
			D3DXMATRIX current, inverse;
			GetTransform(current);
			if (!D3DXMatrixInverse(&inverse, NULL, &current)) return 0;
			D3DXVECTOR3 position(targetX, targetY, targetZ);
			D3DXVec3TransformCoord(&position, &position, &inverse);
			Gfx_Vector3f direction = {{position.x - x, position.y - y, position.z - z}};
			direction.Normalize(); //522000; same actual12B receiver and opaque call.
			*vx = direction.value[0];
			*vy = direction.value[1];
			*vz = direction.value[2];
		}
		else
		{
			float extent = data.maxInitialVelocity.xy.values[0] - data.minInitialVelocity.xy.values[0];
			const double randomX = VfxFrame::RandomRange(0.0f, extent);
			*vx = float(randomX + data.minInitialVelocity.xy.values[0]);
			extent = data.maxInitialVelocity.xy.values[1] - data.minInitialVelocity.xy.values[1];
			const double randomY = VfxFrame::RandomRange(0.0f, extent);
			*vy = float(randomY + data.minInitialVelocity.xy.values[1]);
			extent = data.maxInitialVelocity.z - data.minInitialVelocity.z;
			const double randomZ = VfxFrame::RandomRange(0.0f, extent);
			*vz = float(randomZ + data.minInitialVelocity.z);
		}
		return BYTE(reinterpret_cast<ULONG_PTR>(vz)); //Original callback result is unused.
	}
	void VFX_ParticleSystem::Update(float time)
	{
		Update(time, VFX_UpdateContext());
	}
	void VFX_ParticleSystem::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_UpdateContext context(source, target, group, sourceTransform, targetTransform);
		if (!particles) return;
		lastTime = time;
		UpdateTransform(context);
		if (!data.boundingBoxType && !IsVisible()) return;
		visibleEffects.clear();
		if (context.group && context.group->data)
		{
			for (std::list<TriggeredEffect>::iterator i = spawnedEffects.begin(); i != spawnedEffects.end(); )
			{
				if (i->effectIndex < 0 || size_t(i->effectIndex) >= context.group->data->effects.size())
				{
					++i; //Original leaves invalid-index records pending until cleanup.
					continue;
				}
				VFX_Effect& definition = context.group->data->effects[i->effectIndex];
				if (!definition.data) definition.Prepare();
				VFX_EffectInstance* child = NULL;
				if (!i->child)
				{
					const VFX_BaseData& parameters = *definition.data;
					if (parameters.effectType != -1 && parameters.triggeredEffect)
					{
						const float start = parameters.delay + i->time;
						if (time >= start
							&& (parameters.duration <= 0 || parameters.duration + parameters.delay + i->time > time)
							&& parameters.duration != 0 && time >= start)
						{
							child = CloneVfxEffectInstance(&definition, context.group->GetEffect(i->effectIndex));
							if (child)
							{
								i->child = child;
								if (!child->baseData.target.selector)
								{
									child->baseData.offset.xy.values[0] += i->position.x / scale;
									child->baseData.offset.xy.values[1] += i->position.y / scale;
									child->baseData.offset.z += i->position.z / scale;
								}
							}
						}
					}
				}
				else
				{
					const VFX_BaseData& parameters = *definition.data;
					if (parameters.delay + i->time <= time
						&& (parameters.duration <= 0 || parameters.duration + parameters.delay + i->time > time))
						child = i->child;
					else
					{
						delete i->child;
						i = spawnedEffects.erase(i);
						continue;
					}
				}
				const TriggeredEffect& effect = *i++;
				if (child)
				{
					if (child->GetTargetModel()) child->GetTargetModel()->afterimagesEnabled = 0;
					if (child->baseData.effectType == 1) visibleEffects.push_front(effect);
					else visibleEffects.push_back(effect);
				}
			}
		}
		VFX_UpdateContext childContext = context;
		childContext.sourceTransform = childContext.targetTransform = NULL;
		for (std::list<TriggeredEffect>::iterator i = visibleEffects.begin(); i != visibleEffects.end(); ++i)
			i->child->Update(time - i->time - i->child->baseData.delay, childContext);
		currentSource = context.source;
		currentTarget = context.target;
		currentTargetTransform = context.targetTransform;
		currentSourceTransform = context.sourceTransform;
		currentEffect = this;
		currentGroup = context.group;
		particles->Update(time);
		currentEffect = NULL;
		currentSource = NULL;
		currentTarget = NULL;
		currentGroup = NULL;
		currentSourceTransform = NULL;
		currentTargetTransform = NULL;
		if (data.performCollision && context.group)
		{
			for (size_t i = 0; i < context.group->effects.size(); ++i)
			{
				VFX_Collision* collision = dynamic_cast<VFX_Collision*>(context.group->GetEffect(LONG(i)));
				if (collision) collision->ApplyParticles(*this, time, *context.group);
			}
		}
	}
	void VFX_ParticleSystem::Draw()
	{
		Draw(NULL);
	}
	void VFX_ParticleSystem::Draw(Gfx_Viewport* viewport)
	{
		if (!particles) return;
		particles->Draw(viewport);
		for (std::list<TriggeredEffect>::iterator i = visibleEffects.begin(); i != visibleEffects.end(); ++i)
			i->child->Draw(viewport);
	}
}



