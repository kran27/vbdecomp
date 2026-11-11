#include "VfxFormats.hpp"
#include <mbstring.h>

namespace
{
	//VC7 elides owner copies through const-reference casts. A reference-returning
	//call preserves the lvalue input and the getter temporary's full expression.
	const JE::VFX_ResourceData& ResourceDataReference(const JE::VFX_ResourceData& value)
	{
		return value;
	}

	//Ordered original pointer streams; control prefixes and NULL boundaries are data.
	const char* const commonDefinitions[] =
	{
		"\001General", //0x6fc968
		"std::string", //0x6fc96c
		"Name", //0x6fc970
		"", //0x6fc974
		"enum VFX_EffectType", //0x6fc978
		"EffectType", //0x6fc97c
		"VFXEFFECTTYPE_NONE", //0x6fc980
		"float", //0x6fc984
		"Delay", //0x6fc988
		"0.0f", //0x6fc98c
		"float", //0x6fc990
		"Duration", //0x6fc994
		"-1", //0x6fc998
		"VFX_Target", //0x6fc99c
		"Target", //0x6fc9a0
		"VFXTARGET_SELF", //0x6fc9a4
		"enum GFX_BONE_ID", //0x6fc9a8
		"Socket", //0x6fc9ac
		"BONE_NONE", //0x6fc9b0
		"\002SnapBone empty && TargetBodyPart = false || Socket != BONE_NONE", //0x6fc9b4
		"VFX_SnapBone", //0x6fc9b8
		"SnapBone", //0x6fc9bc
		"", //0x6fc9c0
		"\002Socket = BONE_NONE && TargetBodyPart = false || SnapBone !empty", //0x6fc9c4
		"bool", //0x6fc9c8
		"TargetBodyPart", //0x6fc9cc
		"false", //0x6fc9d0
		"\002SnapBone empty && Socket = BONE_NONE || TargetBodyPart != false", //0x6fc9d4
		"bool", //0x6fc9d8
		"HideIfNoTarget", //0x6fc9dc
		"true", //0x6fc9e0
		"VFX_Vector", //0x6fc9e4
		"Offset", //0x6fc9e8
		"0 0 0", //0x6fc9ec
		"VFX_Rotation", //0x6fc9f0
		"Rotation", //0x6fc9f4
		"0 0 0", //0x6fc9f8
		"bool", //0x6fc9fc
		"RotateWithTarget", //0x6fca00
		"true", //0x6fca04
		"bool", //0x6fca08
		"TriggeredEffect", //0x6fca0c
		"false", //0x6fca10
	};

	const char* const effectDefinitions[] =
	{
		"VFXEFFECTTYPE_PARTICLESYSTEM", //0x6fca18
		"\001Emitter", //0x6fca1c
		"enum VFX_AABBType", //0x6fca20
		"BoundingBoxType", //0x6fca24
		"VFXAABB_OPTIMIZED", //0x6fca28
		"VFX_Vector", //0x6fca2c
		"_OptimizedAABBMin", //0x6fca30
		"0 0 0", //0x6fca34
		"VFX_Vector", //0x6fca38
		"_OptimizedAABBMax", //0x6fca3c
		"0 0 0", //0x6fca40
		"VFX_Vector", //0x6fca44
		"MinBoundingBox", //0x6fca48
		"0 0 0", //0x6fca4c
		"\002BoundingBoxType == VFXAABB_FIXED", //0x6fca50
		"VFX_Vector", //0x6fca54
		"MaxBoundingBox", //0x6fca58
		"0 0 0", //0x6fca5c
		"\002BoundingBoxType == VFXAABB_FIXED", //0x6fca60
		"bool", //0x6fca64
		"ShowBoundingBox", //0x6fca68
		"0", //0x6fca6c
		"enum VFX_EmitterShape", //0x6fca70
		"EmitterShape", //0x6fca74
		"VFXEMITTERSHAPE_BOX", //0x6fca78
		"VFX_Vector", //0x6fca7c
		"MinDimensions", //0x6fca80
		"0 0 0", //0x6fca84
		"VFX_Vector", //0x6fca88
		"MaxDimensions", //0x6fca8c
		"0 0 0", //0x6fca90
		"bool", //0x6fca94
		"ShowEmitterShape", //0x6fca98
		"0", //0x6fca9c
		"VFX_Vector", //0x6fcaa0
		"Acceleration1", //0x6fcaa4
		"0 0 0", //0x6fcaa8
		"bool", //0x6fcaac
		"RotateAccel1WithTarget", //0x6fcab0
		"false", //0x6fcab4
		"VFX_Vector", //0x6fcab8
		"Acceleration2", //0x6fcabc
		"0 0 0", //0x6fcac0
		"bool", //0x6fcac4
		"RotateAccel2WithTarget", //0x6fcac8
		"true", //0x6fcacc
		"bool", //0x6fcad0
		"UseEmitterLifetime", //0x6fcad4
		"0", //0x6fcad8
		"float", //0x6fcadc
		"EmitterLifetime", //0x6fcae0
		"0", //0x6fcae4
		"\002UseEmitterLifetime != 0", //0x6fcae8
		"bool", //0x6fcaec
		"AutoAdd", //0x6fcaf0
		"1", //0x6fcaf4
		"float", //0x6fcaf8
		"MinAddRate", //0x6fcafc
		"100", //0x6fcb00
		"\002AutoAdd != 0", //0x6fcb04
		"float", //0x6fcb08
		"MaxAddRate", //0x6fcb0c
		"100", //0x6fcb10
		"\002AutoAdd != 0", //0x6fcb14
		"int", //0x6fcb18
		"MinParticlesPerAdd", //0x6fcb1c
		"1", //0x6fcb20
		"\002AutoAdd != 0", //0x6fcb24
		"int", //0x6fcb28
		"MaxParticlesPerAdd", //0x6fcb2c
		"1", //0x6fcb30
		"\002AutoAdd != 0", //0x6fcb34
		"bool", //0x6fcb38
		"UseMaxParticles", //0x6fcb3c
		"0", //0x6fcb40
		"int", //0x6fcb44
		"MaxParticles", //0x6fcb48
		"50", //0x6fcb4c
		"\002UseMaxParticles != 0", //0x6fcb50
		"\001Particle", //0x6fcb54
		"VFX_Resource", //0x6fcb58
		"Texture", //0x6fcb5c
		"tga", //0x6fcb60
		"\002Model empty || Texture !empty", //0x6fcb64
		"VFX_TextureUVs", //0x6fcb68
		"TextureUVs", //0x6fcb6c
		"0 0 1 1", //0x6fcb70
		"VFX_Resource", //0x6fcb74
		"Model", //0x6fcb78
		"b3d", //0x6fcb7c
		"\002Texture empty || Model !empty", //0x6fcb80
		"bool", //0x6fcb84
		"Model_UseRenderPhase", //0x6fcb88
		"0", //0x6fcb8c
		"\002Model !empty", //0x6fcb90
		"bool", //0x6fcb94
		"Model_UseColor", //0x6fcb98
		"0", //0x6fcb9c
		"\002Model !empty", //0x6fcba0
		"bool", //0x6fcba4
		"Model_UseAlpha", //0x6fcba8
		"0", //0x6fcbac
		"\002Model !empty", //0x6fcbb0
		"bool", //0x6fcbb4
		"Model_IgnoreLighting", //0x6fcbb8
		"0", //0x6fcbbc
		"\002Model !empty", //0x6fcbc0
		"enum GFX_RENDER_PHASE", //0x6fcbc4
		"RenderPhase", //0x6fcbc8
		"RP_TRANSLUSCENT", //0x6fcbcc
		"bool", //0x6fcbd0
		"Modulate2x", //0x6fcbd4
		"0", //0x6fcbd8
		"VFX_Byte", //0x6fcbdc
		"MinParticleAlpha", //0x6fcbe0
		"255", //0x6fcbe4
		"VFX_Byte", //0x6fcbe8
		"MaxParticleAlpha", //0x6fcbec
		"255", //0x6fcbf0
		"float", //0x6fcbf4
		"MinParticleLifetime", //0x6fcbf8
		"1", //0x6fcbfc
		"float", //0x6fcc00
		"MaxParticleLifetime", //0x6fcc04
		"1", //0x6fcc08
		"float", //0x6fcc0c
		"MinParticleSize", //0x6fcc10
		"0.1", //0x6fcc14
		"float", //0x6fcc18
		"MaxParticleSize", //0x6fcc1c
		"0.1", //0x6fcc20
		"float", //0x6fcc24
		"MinParticleMass", //0x6fcc28
		"1", //0x6fcc2c
		"float", //0x6fcc30
		"MaxParticleMass", //0x6fcc34
		"1", //0x6fcc38
		"float", //0x6fcc3c
		"MinParticleResistance", //0x6fcc40
		"1", //0x6fcc44
		"float", //0x6fcc48
		"MaxParticleResistance", //0x6fcc4c
		"1", //0x6fcc50
		"VFX_Vector", //0x6fcc54
		"MinInitialVelocity", //0x6fcc58
		"-.2 -.2 -.2", //0x6fcc5c
		"VFX_Vector", //0x6fcc60
		"MaxInitialVelocity", //0x6fcc64
		".2 .2 .2", //0x6fcc68
		"float", //0x6fcc6c
		"MinInitialVelocityScale", //0x6fcc70
		"1.0", //0x6fcc74
		"float", //0x6fcc78
		"MaxInitialVelocityScale", //0x6fcc7c
		"1.0", //0x6fcc80
		"VFX_Target", //0x6fcc84
		"InitialVelocityTarget", //0x6fcc88
		"VFXTARGET_NONE", //0x6fcc8c
		"enum GFX_BONE_ID", //0x6fcc90
		"InitialVelocitySocket", //0x6fcc94
		"BONE_NONE", //0x6fcc98
		"\002InitialVelocitySnapBone empty && InitialVelocityTargetBodyPart = false || InitialVelocitySocket != BONE_NONE", //0x6fcc9c
		"VFX_SnapBone", //0x6fcca0
		"InitialVelocitySnapBone", //0x6fcca4
		"", //0x6fcca8
		"\002InitialVelocitySocket = BONE_NONE && InitialVelocityTargetBodyPart = false || InitialVelocitySnapBone !empty", //0x6fccac
		"bool", //0x6fccb0
		"InitialVelocityTargetBodyPart", //0x6fccb4
		"false", //0x6fccb8
		"\002SnapBone empty && Socket = BONE_NONE || InitialVelocityTargetBodyPart != false", //0x6fccbc
		"float", //0x6fccc0
		"MinExplosion", //0x6fccc4
		"0", //0x6fccc8
		"float", //0x6fcccc
		"MaxExplosion", //0x6fccd0
		"0", //0x6fccd4
		"float", //0x6fccd8
		"MinVibration", //0x6fccdc
		"0", //0x6fcce0
		"float", //0x6fcce4
		"MaxVibration", //0x6fcce8
		"0", //0x6fccec
		"bool", //0x6fccf0
		"RotatableParticles", //0x6fccf4
		"0", //0x6fccf8
		"VFX_Rotation", //0x6fccfc
		"MinParticleRotation", //0x6fcd00
		"0 0 0", //0x6fcd04
		"VFX_Rotation", //0x6fcd08
		"MaxParticleRotation", //0x6fcd0c
		"0 0 0", //0x6fcd10
		"VFX_Rotation", //0x6fcd14
		"MinParticleSpin", //0x6fcd18
		"0 0 0", //0x6fcd1c
		"VFX_Rotation", //0x6fcd20
		"MaxParticleSpin", //0x6fcd24
		"0 0 0", //0x6fcd28
		"bool", //0x6fcd2c
		"FaceVelocityVector", //0x6fcd30
		"0", //0x6fcd34
		"VFX_TriggeredEffect", //0x6fcd38
		"ParticleDeathEffect", //0x6fcd3c
		"-1", //0x6fcd40
		"\001Colors", //0x6fcd44
		"int", //0x6fcd48
		"NumColorsUsed", //0x6fcd4c
		"0", //0x6fcd50
		"\0030", //0x6fcd54
		"\0048", //0x6fcd58
		"VFX_Color", //0x6fcd5c
		"Color1", //0x6fcd60
		"0 0 255", //0x6fcd64
		"\002NumColorsUsed > 0", //0x6fcd68
		"float", //0x6fcd6c
		"ColorTime1", //0x6fcd70
		"0.000", //0x6fcd74
		"\002NumColorsUsed > 0", //0x6fcd78
		"VFX_Color", //0x6fcd7c
		"Color2", //0x6fcd80
		"0 0 255", //0x6fcd84
		"\002NumColorsUsed > 1", //0x6fcd88
		"float", //0x6fcd8c
		"ColorTime2", //0x6fcd90
		"0.125", //0x6fcd94
		"\002NumColorsUsed > 1", //0x6fcd98
		"VFX_Color", //0x6fcd9c
		"Color3", //0x6fcda0
		"0 0 255", //0x6fcda4
		"\002NumColorsUsed > 2", //0x6fcda8
		"float", //0x6fcdac
		"ColorTime3", //0x6fcdb0
		"0.250", //0x6fcdb4
		"\002NumColorsUsed > 2", //0x6fcdb8
		"VFX_Color", //0x6fcdbc
		"Color4", //0x6fcdc0
		"0 0 255", //0x6fcdc4
		"\002NumColorsUsed > 3", //0x6fcdc8
		"float", //0x6fcdcc
		"ColorTime4", //0x6fcdd0
		"0.375", //0x6fcdd4
		"\002NumColorsUsed > 3", //0x6fcdd8
		"VFX_Color", //0x6fcddc
		"Color5", //0x6fcde0
		"0 0 255", //0x6fcde4
		"\002NumColorsUsed > 4", //0x6fcde8
		"float", //0x6fcdec
		"ColorTime5", //0x6fcdf0
		"0.500", //0x6fcdf4
		"\002NumColorsUsed > 4", //0x6fcdf8
		"VFX_Color", //0x6fcdfc
		"Color6", //0x6fce00
		"0 0 255", //0x6fce04
		"\002NumColorsUsed > 5", //0x6fce08
		"float", //0x6fce0c
		"ColorTime6", //0x6fce10
		"0.625", //0x6fce14
		"\002NumColorsUsed > 5", //0x6fce18
		"VFX_Color", //0x6fce1c
		"Color7", //0x6fce20
		"0 0 255", //0x6fce24
		"\002NumColorsUsed > 6", //0x6fce28
		"float", //0x6fce2c
		"ColorTime7", //0x6fce30
		"0.750", //0x6fce34
		"\002NumColorsUsed > 6", //0x6fce38
		"VFX_Color", //0x6fce3c
		"Color8", //0x6fce40
		"0 0 255", //0x6fce44
		"\002NumColorsUsed > 7", //0x6fce48
		"float", //0x6fce4c
		"ColorTime8", //0x6fce50
		"0.875", //0x6fce54
		"\002NumColorsUsed > 7", //0x6fce58
		"bool", //0x6fce5c
		"AdjustColor", //0x6fce60
		"0", //0x6fce64
		"\002NumColorsUsed > 0", //0x6fce68
		"bool", //0x6fce6c
		"AdjustColorOverLifetime", //0x6fce70
		"1", //0x6fce74
		"\002NumColorsUsed > 0 && AdjustColor != 0", //0x6fce78
		"bool", //0x6fce7c
		"AdjustSystemColor", //0x6fce80
		"0", //0x6fce84
		"\002NumColorsUsed > 0 && AdjustColor != 0", //0x6fce88
		"enum VFX_WaveType", //0x6fce8c
		"ColorWaveform", //0x6fce90
		"VFXWAVE_LINEAR", //0x6fce94
		"\002NumColorsUsed > 0 && AdjustColor != 0", //0x6fce98
		"float", //0x6fce9c
		"ColorFreq", //0x6fcea0
		"1", //0x6fcea4
		"\002NumColorsUsed > 0 && AdjustColor != 0", //0x6fcea8
		"float", //0x6fceac
		"ColorPhase", //0x6fceb0
		"0", //0x6fceb4
		"\002NumColorsUsed > 0 && AdjustColor != 0", //0x6fceb8
		"\001Alpha", //0x6fcebc
		"bool", //0x6fcec0
		"AdjustParticleAlpha", //0x6fcec4
		"0", //0x6fcec8
		"bool", //0x6fcecc
		"AdjustParticleAlphaOverLifetime", //0x6fced0
		"1", //0x6fced4
		"\002AdjustParticleAlpha != 0", //0x6fced8
		"VFX_Byte", //0x6fcedc
		"StartParticleAlpha", //0x6fcee0
		"0", //0x6fcee4
		"\002AdjustParticleAlpha != 0", //0x6fcee8
		"VFX_Byte", //0x6fceec
		"EndParticleAlpha", //0x6fcef0
		"0", //0x6fcef4
		"\002AdjustParticleAlpha != 0", //0x6fcef8
		"enum VFX_WaveType", //0x6fcefc
		"ParticleAlphaWaveform", //0x6fcf00
		"VFXWAVE_LINEAR", //0x6fcf04
		"\002AdjustParticleAlpha != 0", //0x6fcf08
		"float", //0x6fcf0c
		"ParticleAlphaFreq", //0x6fcf10
		"1", //0x6fcf14
		"\002AdjustParticleAlpha != 0", //0x6fcf18
		"float", //0x6fcf1c
		"ParticleAlphaPhase", //0x6fcf20
		"0", //0x6fcf24
		"\002AdjustParticleAlpha != 0", //0x6fcf28
		"bool", //0x6fcf2c
		"AdjustSystemAlpha", //0x6fcf30
		"0", //0x6fcf34
		"bool", //0x6fcf38
		"AdjustSystemAlphaOverLifetime", //0x6fcf3c
		"1", //0x6fcf40
		"\002AdjustSystemAlpha != 0", //0x6fcf44
		"VFX_Byte", //0x6fcf48
		"StartSystemAlpha", //0x6fcf4c
		"0", //0x6fcf50
		"\002AdjustSystemAlpha != 0", //0x6fcf54
		"VFX_Byte", //0x6fcf58
		"EndSystemAlpha", //0x6fcf5c
		"0", //0x6fcf60
		"\002AdjustSystemAlpha != 0", //0x6fcf64
		"enum VFX_WaveType", //0x6fcf68
		"SystemAlphaWaveform", //0x6fcf6c
		"VFXWAVE_LINEAR", //0x6fcf70
		"\002AdjustSystemAlpha != 0", //0x6fcf74
		"float", //0x6fcf78
		"SystemAlphaFreq", //0x6fcf7c
		"1", //0x6fcf80
		"\002AdjustSystemAlpha != 0", //0x6fcf84
		"float", //0x6fcf88
		"SystemAlphaPhase", //0x6fcf8c
		"0", //0x6fcf90
		"\002AdjustSystemAlpha != 0", //0x6fcf94
		"\001Scale", //0x6fcf98
		"bool", //0x6fcf9c
		"AdjustParticleScale", //0x6fcfa0
		"0", //0x6fcfa4
		"bool", //0x6fcfa8
		"AdjustParticleScaleOverLifetime", //0x6fcfac
		"1", //0x6fcfb0
		"\002AdjustParticleScale != 0", //0x6fcfb4
		"float", //0x6fcfb8
		"StartParticleScale", //0x6fcfbc
		"0", //0x6fcfc0
		"\002AdjustParticleScale != 0", //0x6fcfc4
		"float", //0x6fcfc8
		"EndParticleScale", //0x6fcfcc
		"0", //0x6fcfd0
		"\002AdjustParticleScale != 0", //0x6fcfd4
		"enum VFX_WaveType", //0x6fcfd8
		"ParticleScaleWaveform", //0x6fcfdc
		"VFXWAVE_LINEAR", //0x6fcfe0
		"\002AdjustParticleScale != 0", //0x6fcfe4
		"float", //0x6fcfe8
		"ParticleScaleFreq", //0x6fcfec
		"1", //0x6fcff0
		"\002AdjustParticleScale != 0", //0x6fcff4
		"float", //0x6fcff8
		"ParticleScalePhase", //0x6fcffc
		"0", //0x6fd000
		"\002AdjustParticleScale != 0", //0x6fd004
		"bool", //0x6fd008
		"AdjustSystemScale", //0x6fd00c
		"0", //0x6fd010
		"bool", //0x6fd014
		"AdjustSystemScaleOverLifetime", //0x6fd018
		"1", //0x6fd01c
		"\002AdjustSystemScale != 0", //0x6fd020
		"float", //0x6fd024
		"StartSystemScale", //0x6fd028
		"1", //0x6fd02c
		"\002AdjustSystemScale != 0", //0x6fd030
		"float", //0x6fd034
		"EndSystemScale", //0x6fd038
		"1", //0x6fd03c
		"\002AdjustSystemScale != 0", //0x6fd040
		"enum VFX_WaveType", //0x6fd044
		"SystemScaleWaveform", //0x6fd048
		"VFXWAVE_LINEAR", //0x6fd04c
		"\002AdjustSystemScale != 0", //0x6fd050
		"float", //0x6fd054
		"SystemScaleFreq", //0x6fd058
		"1", //0x6fd05c
		"\002AdjustSystemScale != 0", //0x6fd060
		"float", //0x6fd064
		"SystemScalePhase", //0x6fd068
		"0", //0x6fd06c
		"\002AdjustSystemScale != 0", //0x6fd070
		"\001Dimensions", //0x6fd074
		"bool", //0x6fd078
		"AdjustMinDimX", //0x6fd07c
		"0", //0x6fd080
		"bool", //0x6fd084
		"AdjustMinDimXOverLifetime", //0x6fd088
		"1", //0x6fd08c
		"\002AdjustMinDimX != 0", //0x6fd090
		"float", //0x6fd094
		"FinalMinDimX", //0x6fd098
		"0", //0x6fd09c
		"\002AdjustMinDimX != 0", //0x6fd0a0
		"enum VFX_WaveType", //0x6fd0a4
		"MinDimXWaveform", //0x6fd0a8
		"VFXWAVE_LINEAR", //0x6fd0ac
		"\002AdjustMinDimX != 0", //0x6fd0b0
		"float", //0x6fd0b4
		"MinDimXFreq", //0x6fd0b8
		"1", //0x6fd0bc
		"\002AdjustMinDimX != 0", //0x6fd0c0
		"float", //0x6fd0c4
		"MinDimXPhase", //0x6fd0c8
		"0", //0x6fd0cc
		"\002AdjustMinDimX != 0", //0x6fd0d0
		"bool", //0x6fd0d4
		"AdjustMinDimY", //0x6fd0d8
		"0", //0x6fd0dc
		"bool", //0x6fd0e0
		"AdjustMinDimYOverLifetime", //0x6fd0e4
		"1", //0x6fd0e8
		"\002AdjustMinDimY != 0", //0x6fd0ec
		"float", //0x6fd0f0
		"FinalMinDimY", //0x6fd0f4
		"0", //0x6fd0f8
		"\002AdjustMinDimY != 0", //0x6fd0fc
		"enum VFX_WaveType", //0x6fd100
		"MinDimYWaveform", //0x6fd104
		"VFXWAVE_LINEAR", //0x6fd108
		"\002AdjustMinDimY != 0", //0x6fd10c
		"float", //0x6fd110
		"MinDimYFreq", //0x6fd114
		"1", //0x6fd118
		"\002AdjustMinDimY != 0", //0x6fd11c
		"float", //0x6fd120
		"MinDimYPhase", //0x6fd124
		"0", //0x6fd128
		"\002AdjustMinDimY != 0", //0x6fd12c
		"bool", //0x6fd130
		"AdjustMinDimZ", //0x6fd134
		"0", //0x6fd138
		"bool", //0x6fd13c
		"AdjustMinDimZOverLifetime", //0x6fd140
		"1", //0x6fd144
		"\002AdjustMinDimZ != 0", //0x6fd148
		"float", //0x6fd14c
		"FinalMinDimZ", //0x6fd150
		"0", //0x6fd154
		"\002AdjustMinDimZ != 0", //0x6fd158
		"enum VFX_WaveType", //0x6fd15c
		"MinDimZWaveform", //0x6fd160
		"VFXWAVE_LINEAR", //0x6fd164
		"\002AdjustMinDimZ != 0", //0x6fd168
		"float", //0x6fd16c
		"MinDimZFreq", //0x6fd170
		"1", //0x6fd174
		"\002AdjustMinDimZ != 0", //0x6fd178
		"float", //0x6fd17c
		"MinDimZPhase", //0x6fd180
		"0", //0x6fd184
		"\002AdjustMinDimZ != 0", //0x6fd188
		"bool", //0x6fd18c
		"AdjustMaxDimX", //0x6fd190
		"0", //0x6fd194
		"bool", //0x6fd198
		"AdjustMaxDimXOverLifetime", //0x6fd19c
		"1", //0x6fd1a0
		"\002AdjustMaxDimX != 0", //0x6fd1a4
		"float", //0x6fd1a8
		"FinalMaxDimX", //0x6fd1ac
		"0", //0x6fd1b0
		"\002AdjustMaxDimX != 0", //0x6fd1b4
		"enum VFX_WaveType", //0x6fd1b8
		"MaxDimXWaveform", //0x6fd1bc
		"VFXWAVE_LINEAR", //0x6fd1c0
		"\002AdjustMaxDimX != 0", //0x6fd1c4
		"float", //0x6fd1c8
		"MaxDimXFreq", //0x6fd1cc
		"1", //0x6fd1d0
		"\002AdjustMaxDimX != 0", //0x6fd1d4
		"float", //0x6fd1d8
		"MaxDimXPhase", //0x6fd1dc
		"0", //0x6fd1e0
		"\002AdjustMaxDimX != 0", //0x6fd1e4
		"bool", //0x6fd1e8
		"AdjustMaxDimY", //0x6fd1ec
		"0", //0x6fd1f0
		"bool", //0x6fd1f4
		"AdjustMaxDimYOverLifetime", //0x6fd1f8
		"1", //0x6fd1fc
		"\002AdjustMaxDimY != 0", //0x6fd200
		"float", //0x6fd204
		"FinalMaxDimY", //0x6fd208
		"0", //0x6fd20c
		"\002AdjustMaxDimY != 0", //0x6fd210
		"enum VFX_WaveType", //0x6fd214
		"MaxDimYWaveform", //0x6fd218
		"VFXWAVE_LINEAR", //0x6fd21c
		"\002AdjustMaxDimY != 0", //0x6fd220
		"float", //0x6fd224
		"MaxDimYFreq", //0x6fd228
		"1", //0x6fd22c
		"\002AdjustMaxDimY != 0", //0x6fd230
		"float", //0x6fd234
		"MaxDimYPhase", //0x6fd238
		"0", //0x6fd23c
		"\002AdjustMaxDimY != 0", //0x6fd240
		"bool", //0x6fd244
		"AdjustMaxDimZ", //0x6fd248
		"0", //0x6fd24c
		"bool", //0x6fd250
		"AdjustMaxDimZOverLifetime", //0x6fd254
		"1", //0x6fd258
		"\002AdjustMaxDimZ != 0", //0x6fd25c
		"float", //0x6fd260
		"FinalMaxDimZ", //0x6fd264
		"0", //0x6fd268
		"\002AdjustMaxDimZ != 0", //0x6fd26c
		"enum VFX_WaveType", //0x6fd270
		"MaxDimZWaveform", //0x6fd274
		"VFXWAVE_LINEAR", //0x6fd278
		"\002AdjustMaxDimZ != 0", //0x6fd27c
		"float", //0x6fd280
		"MaxDimZFreq", //0x6fd284
		"1", //0x6fd288
		"\002AdjustMaxDimZ != 0", //0x6fd28c
		"float", //0x6fd290
		"MaxDimZPhase", //0x6fd294
		"0", //0x6fd298
		"\002AdjustMaxDimZ != 0", //0x6fd29c
		"\001Particle Trails", //0x6fd2a0
		"enum VFX_ParticleTrailType", //0x6fd2a4
		"ParticleTrailType", //0x6fd2a8
		"VFXPARTICLETRAILTYPE_NONE", //0x6fd2ac
		"float", //0x6fd2b0
		"TrailInterval", //0x6fd2b4
		"0.02f", //0x6fd2b8
		"\002ParticleTrailType != VFXPARTICLETRAILTYPE_NONE", //0x6fd2bc
		"\0050.01f", //0x6fd2c0
		"float", //0x6fd2c4
		"TrailLifetime", //0x6fd2c8
		"0.1f", //0x6fd2cc
		"\002ParticleTrailType != VFXPARTICLETRAILTYPE_NONE", //0x6fd2d0
		"enum GFX_RENDER_PHASE", //0x6fd2d4
		"TrailRenderPhase", //0x6fd2d8
		"RP_TRANSLUSCENT", //0x6fd2dc
		"\002ParticleTrailType != VFXPARTICLETRAILTYPE_NONE", //0x6fd2e0
		"bool", //0x6fd2e4
		"TrailModulate2x", //0x6fd2e8
		"0", //0x6fd2ec
		"\002ParticleTrailType != VFXPARTICLETRAILTYPE_NONE", //0x6fd2f0
		"VFX_Color", //0x6fd2f4
		"BeginTrailColor", //0x6fd2f8
		"255 255 255", //0x6fd2fc
		"\002ParticleTrailType != VFXPARTICLETRAILTYPE_NONE", //0x6fd300
		"VFX_Byte", //0x6fd304
		"BeginTrailAlpha", //0x6fd308
		"255", //0x6fd30c
		"\002ParticleTrailType != VFXPARTICLETRAILTYPE_NONE", //0x6fd310
		"float", //0x6fd314
		"BeginTrailScale", //0x6fd318
		"1.0", //0x6fd31c
		"\002ParticleTrailType == VFXPARTICLETRAILTYPE_AFTERIMAGE || ParticleTrailType == VFXPARTICLETRAILTYPE_RIBBON", //0x6fd320
		"VFX_Color", //0x6fd324
		"EndTrailColor", //0x6fd328
		"255 255 255", //0x6fd32c
		"\002ParticleTrailType != VFXPARTICLETRAILTYPE_NONE", //0x6fd330
		"VFX_Byte", //0x6fd334
		"EndTrailAlpha", //0x6fd338
		"0", //0x6fd33c
		"\002ParticleTrailType != VFXPARTICLETRAILTYPE_NONE", //0x6fd340
		"float", //0x6fd344
		"EndTrailScale", //0x6fd348
		"1.0", //0x6fd34c
		"\002ParticleTrailType == VFXPARTICLETRAILTYPE_AFTERIMAGE || ParticleTrailType == VFXPARTICLETRAILTYPE_RIBBON", //0x6fd350
		"VFX_Resource", //0x6fd354
		"RibbonTexture", //0x6fd358
		"tga", //0x6fd35c
		"\002ParticleTrailType == VFXPARTICLETRAILTYPE_RIBBON", //0x6fd360
		"VFX_TextureUVs", //0x6fd364
		"RibbonTextureUVs", //0x6fd368
		"0 0 1 1", //0x6fd36c
		"\002ParticleTrailType == VFXPARTICLETRAILTYPE_RIBBON", //0x6fd370
		"VFX_Vector", //0x6fd374
		"RibbonOrigin", //0x6fd378
		"0 -0.05 0", //0x6fd37c
		"\002ParticleTrailType == VFXPARTICLETRAILTYPE_RIBBON", //0x6fd380
		"VFX_Vector", //0x6fd384
		"RibbonEndPoint", //0x6fd388
		"0 0.05 0", //0x6fd38c
		"\002ParticleTrailType == VFXPARTICLETRAILTYPE_RIBBON", //0x6fd390
		"enum VFX_AlignmentType", //0x6fd394
		"RibbonAlignment", //0x6fd398
		"VFXALIGN_CENTER", //0x6fd39c
		"\002ParticleTrailType == VFXPARTICLETRAILTYPE_RIBBON", //0x6fd3a0
		"\001Collision", //0x6fd3a4
		"bool", //0x6fd3a8
		"PerformCollision", //0x6fd3ac
		"true", //0x6fd3b0
		"bool", //0x6fd3b4
		"FaceNormal", //0x6fd3b8
		"0", //0x6fd3bc
		"\002PerformCollision = true", //0x6fd3c0
		"VFX_TriggeredEffect", //0x6fd3c4
		"CollisionEffect", //0x6fd3c8
		"-1", //0x6fd3cc
		"\002PerformCollision = true", //0x6fd3d0
		NULL, //0x6fd3d4
		"VFXEFFECTTYPE_MOTIONBLUR", //0x6fd3d8
		"\001Motion Blur", //0x6fd3dc
		"float", //0x6fd3e0
		"BlurDuration", //0x6fd3e4
		"0.5", //0x6fd3e8
		"float", //0x6fd3ec
		"BlurDelay", //0x6fd3f0
		"0.04f", //0x6fd3f4
		"\0050.01f", //0x6fd3f8
		"enum GFX_RENDER_PHASE", //0x6fd3fc
		"RenderPhase", //0x6fd400
		"RP_TRANSLUSCENT", //0x6fd404
		"bool", //0x6fd408
		"ApplyToAttachments", //0x6fd40c
		"1", //0x6fd410
		NULL, //0x6fd414
		"VFXEFFECTTYPE_SPRITE", //0x6fd418
		"\001Sprite", //0x6fd41c
		"bool", //0x6fd420
		"Rotatable", //0x6fd424
		"0", //0x6fd428
		"VFX_Resource", //0x6fd42c
		"Texture", //0x6fd430
		"tga", //0x6fd434
		"VFX_TextureUVs", //0x6fd438
		"TextureUVs", //0x6fd43c
		"0 0 1 1", //0x6fd440
		"float", //0x6fd444
		"Width", //0x6fd448
		"0.1f", //0x6fd44c
		"float", //0x6fd450
		"Height", //0x6fd454
		"0.1f", //0x6fd458
		"enum GFX_RENDER_PHASE", //0x6fd45c
		"RenderPhase", //0x6fd460
		"RP_TRANSLUSCENT", //0x6fd464
		"bool", //0x6fd468
		"Modulate2x", //0x6fd46c
		"0", //0x6fd470
		"bool", //0x6fd474
		"_DisableZBuffer", //0x6fd478
		"0", //0x6fd47c
		"\001Color", //0x6fd480
		"VFX_Color", //0x6fd484
		"Color", //0x6fd488
		"255 255 255", //0x6fd48c
		"bool", //0x6fd490
		"AdjustColor", //0x6fd494
		"0", //0x6fd498
		"bool", //0x6fd49c
		"AdjustColorOverLifetime", //0x6fd4a0
		"1", //0x6fd4a4
		"\002AdjustColor != 0", //0x6fd4a8
		"VFX_Color", //0x6fd4ac
		"FinalColor", //0x6fd4b0
		"0 0 0", //0x6fd4b4
		"\002AdjustColor != 0", //0x6fd4b8
		"enum VFX_WaveType", //0x6fd4bc
		"ColorWaveform", //0x6fd4c0
		"VFXWAVE_LINEAR", //0x6fd4c4
		"\002AdjustColor != 0", //0x6fd4c8
		"float", //0x6fd4cc
		"ColorFreq", //0x6fd4d0
		"1", //0x6fd4d4
		"\002AdjustColor != 0", //0x6fd4d8
		"float", //0x6fd4dc
		"ColorPhase", //0x6fd4e0
		"0", //0x6fd4e4
		"\002AdjustColor != 0", //0x6fd4e8
		"\001Alpha", //0x6fd4ec
		"VFX_Byte", //0x6fd4f0
		"Alpha", //0x6fd4f4
		"255", //0x6fd4f8
		"bool", //0x6fd4fc
		"AdjustAlpha", //0x6fd500
		"0", //0x6fd504
		"bool", //0x6fd508
		"AdjustAlphaOverLifetime", //0x6fd50c
		"1", //0x6fd510
		"\002AdjustAlpha != 0", //0x6fd514
		"VFX_Byte", //0x6fd518
		"FinalAlpha", //0x6fd51c
		"0", //0x6fd520
		"\002AdjustAlpha != 0", //0x6fd524
		"enum VFX_WaveType", //0x6fd528
		"AlphaWaveform", //0x6fd52c
		"VFXWAVE_LINEAR", //0x6fd530
		"\002AdjustAlpha != 0", //0x6fd534
		"float", //0x6fd538
		"AlphaFreq", //0x6fd53c
		"1", //0x6fd540
		"\002AdjustAlpha != 0", //0x6fd544
		"float", //0x6fd548
		"AlphaPhase", //0x6fd54c
		"0", //0x6fd550
		"\002AdjustAlpha != 0", //0x6fd554
		"\001Scale", //0x6fd558
		"bool", //0x6fd55c
		"AdjustScale", //0x6fd560
		"0", //0x6fd564
		"bool", //0x6fd568
		"AdjustScaleOverLifetime", //0x6fd56c
		"1", //0x6fd570
		"\002AdjustScale != 0", //0x6fd574
		"float", //0x6fd578
		"InitialScaleX", //0x6fd57c
		"0", //0x6fd580
		"\002AdjustScale != 0", //0x6fd584
		"float", //0x6fd588
		"InitialScaleY", //0x6fd58c
		"0", //0x6fd590
		"\002AdjustScale != 0", //0x6fd594
		"float", //0x6fd598
		"FinalScaleX", //0x6fd59c
		"1", //0x6fd5a0
		"\002AdjustScale != 0", //0x6fd5a4
		"float", //0x6fd5a8
		"FinalScaleY", //0x6fd5ac
		"1", //0x6fd5b0
		"\002AdjustScale != 0", //0x6fd5b4
		"enum VFX_WaveType", //0x6fd5b8
		"ScaleWaveform", //0x6fd5bc
		"VFXWAVE_LINEAR", //0x6fd5c0
		"\002AdjustScale != 0", //0x6fd5c4
		"float", //0x6fd5c8
		"ScaleFreq", //0x6fd5cc
		"1", //0x6fd5d0
		"\002AdjustScale != 0", //0x6fd5d4
		"float", //0x6fd5d8
		"ScalePhase", //0x6fd5dc
		"0", //0x6fd5e0
		"\002AdjustScale != 0", //0x6fd5e4
		NULL, //0x6fd5e8
		"VFXEFFECTTYPE_MODEL", //0x6fd5ec
		"\001Model", //0x6fd5f0
		"VFX_Resource", //0x6fd5f4
		"Model", //0x6fd5f8
		"b3d", //0x6fd5fc
		"VFX_Resource", //0x6fd600
		"Skeleton", //0x6fd604
		"skl", //0x6fd608
		"VFX_Resource", //0x6fd60c
		"Animation", //0x6fd610
		"gr2", //0x6fd614
		"int", //0x6fd618
		"LoopCount", //0x6fd61c
		"0", //0x6fd620
		"float", //0x6fd624
		"ScaleX", //0x6fd628
		"1", //0x6fd62c
		"float", //0x6fd630
		"ScaleY", //0x6fd634
		"1", //0x6fd638
		"float", //0x6fd63c
		"ScaleZ", //0x6fd640
		"1", //0x6fd644
		"bool", //0x6fd648
		"UseShadowVolume", //0x6fd64c
		"0", //0x6fd650
		"bool", //0x6fd654
		"IgnoreLighting", //0x6fd658
		"0", //0x6fd65c
		"bool", //0x6fd660
		"UseRenderPhase", //0x6fd664
		"0", //0x6fd668
		"enum GFX_RENDER_PHASE", //0x6fd66c
		"RenderPhase", //0x6fd670
		"RP_TRANSLUSCENT", //0x6fd674
		"\002UseRenderPhase != 0", //0x6fd678
		"\001Color", //0x6fd67c
		"bool", //0x6fd680
		"UseColor", //0x6fd684
		"0", //0x6fd688
		"VFX_Color", //0x6fd68c
		"Color", //0x6fd690
		"255 255 255", //0x6fd694
		"\002UseColor != 0 || AdjustColor != 0", //0x6fd698
		"bool", //0x6fd69c
		"AdjustColor", //0x6fd6a0
		"0", //0x6fd6a4
		"bool", //0x6fd6a8
		"AdjustColorOverLifetime", //0x6fd6ac
		"1", //0x6fd6b0
		"\002AdjustColor != 0", //0x6fd6b4
		"VFX_Color", //0x6fd6b8
		"FinalColor", //0x6fd6bc
		"0 0 0", //0x6fd6c0
		"\002AdjustColor != 0", //0x6fd6c4
		"enum VFX_WaveType", //0x6fd6c8
		"ColorWaveform", //0x6fd6cc
		"VFXWAVE_LINEAR", //0x6fd6d0
		"\002AdjustColor != 0", //0x6fd6d4
		"float", //0x6fd6d8
		"ColorFreq", //0x6fd6dc
		"1", //0x6fd6e0
		"\002AdjustColor != 0", //0x6fd6e4
		"float", //0x6fd6e8
		"ColorPhase", //0x6fd6ec
		"0", //0x6fd6f0
		"\002AdjustColor != 0", //0x6fd6f4
		"bool", //0x6fd6f8
		"UseEmissive", //0x6fd6fc
		"0", //0x6fd700
		"VFX_Color", //0x6fd704
		"Emissive", //0x6fd708
		"255 255 255", //0x6fd70c
		"\002UseEmissive != 0 || AdjustEmissive != 0", //0x6fd710
		"bool", //0x6fd714
		"AdjustEmissive", //0x6fd718
		"0", //0x6fd71c
		"bool", //0x6fd720
		"AdjustEmissiveOverLifetime", //0x6fd724
		"1", //0x6fd728
		"\002AdjustEmissive != 0", //0x6fd72c
		"VFX_Color", //0x6fd730
		"FinalEmissive", //0x6fd734
		"0 0 0", //0x6fd738
		"\002AdjustEmissive != 0", //0x6fd73c
		"enum VFX_WaveType", //0x6fd740
		"EmissiveWaveform", //0x6fd744
		"VFXWAVE_LINEAR", //0x6fd748
		"\002AdjustEmissive != 0", //0x6fd74c
		"float", //0x6fd750
		"EmissiveFreq", //0x6fd754
		"1", //0x6fd758
		"\002AdjustEmissive != 0", //0x6fd75c
		"float", //0x6fd760
		"EmissivePhase", //0x6fd764
		"0", //0x6fd768
		"\002AdjustEmissive != 0", //0x6fd76c
		"\001Alpha", //0x6fd770
		"bool", //0x6fd774
		"UseAlpha", //0x6fd778
		"0", //0x6fd77c
		"VFX_Byte", //0x6fd780
		"Alpha", //0x6fd784
		"255", //0x6fd788
		"\002UseAlpha != 0 || AdjustAlpha != 0", //0x6fd78c
		"bool", //0x6fd790
		"AdjustAlpha", //0x6fd794
		"0", //0x6fd798
		"bool", //0x6fd79c
		"AdjustAlphaOverLifetime", //0x6fd7a0
		"1", //0x6fd7a4
		"\002AdjustAlpha != 0", //0x6fd7a8
		"VFX_Byte", //0x6fd7ac
		"FinalAlpha", //0x6fd7b0
		"0", //0x6fd7b4
		"\002AdjustAlpha != 0", //0x6fd7b8
		"enum VFX_WaveType", //0x6fd7bc
		"AlphaWaveform", //0x6fd7c0
		"VFXWAVE_LINEAR", //0x6fd7c4
		"\002AdjustAlpha != 0", //0x6fd7c8
		"float", //0x6fd7cc
		"AlphaFreq", //0x6fd7d0
		"1", //0x6fd7d4
		"\002AdjustAlpha != 0", //0x6fd7d8
		"float", //0x6fd7dc
		"AlphaPhase", //0x6fd7e0
		"0", //0x6fd7e4
		"\002AdjustAlpha != 0", //0x6fd7e8
		"\001Scale", //0x6fd7ec
		"bool", //0x6fd7f0
		"AdjustScale", //0x6fd7f4
		"0", //0x6fd7f8
		"bool", //0x6fd7fc
		"AdjustScaleOverLifetime", //0x6fd800
		"1", //0x6fd804
		"\002AdjustScale != 0", //0x6fd808
		"float", //0x6fd80c
		"FinalScaleX", //0x6fd810
		"0", //0x6fd814
		"\002AdjustScale != 0", //0x6fd818
		"float", //0x6fd81c
		"FinalScaleY", //0x6fd820
		"0", //0x6fd824
		"\002AdjustScale != 0", //0x6fd828
		"float", //0x6fd82c
		"FinalScaleZ", //0x6fd830
		"0", //0x6fd834
		"\002AdjustScale != 0", //0x6fd838
		"enum VFX_WaveType", //0x6fd83c
		"ScaleWaveform", //0x6fd840
		"VFXWAVE_LINEAR", //0x6fd844
		"\002AdjustScale != 0", //0x6fd848
		"float", //0x6fd84c
		"ScaleFreq", //0x6fd850
		"1", //0x6fd854
		"\002AdjustScale != 0", //0x6fd858
		"float", //0x6fd85c
		"ScalePhase", //0x6fd860
		"0", //0x6fd864
		"\002AdjustScale != 0", //0x6fd868
		NULL, //0x6fd86c
		"VFXEFFECTTYPE_TRAIL", //0x6fd870
		"\001Trail", //0x6fd874
		"VFX_Color", //0x6fd878
		"Color1", //0x6fd87c
		"255 255 255", //0x6fd880
		"VFX_Byte", //0x6fd884
		"Alpha1", //0x6fd888
		"255", //0x6fd88c
		"float", //0x6fd890
		"WidthScale1", //0x6fd894
		"1.0", //0x6fd898
		"VFX_Color", //0x6fd89c
		"Color2", //0x6fd8a0
		"255 255 255", //0x6fd8a4
		"VFX_Byte", //0x6fd8a8
		"Alpha2", //0x6fd8ac
		"255", //0x6fd8b0
		"float", //0x6fd8b4
		"WidthScale2", //0x6fd8b8
		"0", //0x6fd8bc
		"VFX_Target", //0x6fd8c0
		"EndPointTarget", //0x6fd8c4
		"VFXTARGET_SELF", //0x6fd8c8
		"enum GFX_BONE_ID", //0x6fd8cc
		"EndPointSocket", //0x6fd8d0
		"BONE_NONE", //0x6fd8d4
		"\002EndPointSnapBone empty && EndPointTargetBodyPart = false || EndPointSocket != BONE_NONE", //0x6fd8d8
		"VFX_SnapBone", //0x6fd8dc
		"EndPointSnapBone", //0x6fd8e0
		"", //0x6fd8e4
		"\002EndPointSocket = BONE_NONE && EndPointTargetBodyPart = false || EndPointSnapBone !empty", //0x6fd8e8
		"bool", //0x6fd8ec
		"EndPointTargetBodyPart", //0x6fd8f0
		"false", //0x6fd8f4
		"\002SnapBone empty && Socket = BONE_NONE || EndPointTargetBodyPart != false", //0x6fd8f8
		"VFX_Vector", //0x6fd8fc
		"EndPointOffset", //0x6fd900
		"0 0 0", //0x6fd904
		"float", //0x6fd908
		"SegmentLifetime", //0x6fd90c
		"0.1f", //0x6fd910
		"enum VFX_AlignmentType", //0x6fd914
		"Alignment", //0x6fd918
		"VFXALIGN_CENTER", //0x6fd91c
		"VFX_Resource", //0x6fd920
		"Texture", //0x6fd924
		"tga", //0x6fd928
		"VFX_TextureUVs", //0x6fd92c
		"TextureUVs", //0x6fd930
		"0 0 1 1", //0x6fd934
		"enum GFX_RENDER_PHASE", //0x6fd938
		"RenderPhase", //0x6fd93c
		"RP_TRANSLUSCENT", //0x6fd940
		"bool", //0x6fd944
		"Modulate2x", //0x6fd948
		"0", //0x6fd94c
		NULL, //0x6fd950
		"VFXEFFECTTYPE_COLLISION", //0x6fd954
		"\001Collision", //0x6fd958
		"enum VFX_CollisionShape", //0x6fd95c
		"CollisionShape", //0x6fd960
		"VFXCOLLISIONSHAPE_BOX", //0x6fd964
		"enum VFX_CollisionType", //0x6fd968
		"CollisionType", //0x6fd96c
		"VFXCOLLISIONTYPE_BOUNCE", //0x6fd970
		"VFX_Vector", //0x6fd974
		"Dimensions", //0x6fd978
		".1 .1 .1", //0x6fd97c
		"float", //0x6fd980
		"Radius", //0x6fd984
		"1", //0x6fd988
		"float", //0x6fd98c
		"Power", //0x6fd990
		"100", //0x6fd994
		"bool", //0x6fd998
		"ShowShape", //0x6fd99c
		"0", //0x6fd9a0
		NULL, //0x6fd9a4
		"VFXEFFECTTYPE_WATER", //0x6fd9a8
		"\001Water", //0x6fd9ac
		"VFX_Resource", //0x6fd9b0
		"Texture", //0x6fd9b4
		"tga", //0x6fd9b8
		"VFX_TextureUVs", //0x6fd9bc
		"TextureUVs", //0x6fd9c0
		"0 0 1 1", //0x6fd9c4
		"VFX_Resource", //0x6fd9c8
		"LightMap", //0x6fd9cc
		"tga", //0x6fd9d0
		"VFX_TextureUVs", //0x6fd9d4
		"LightMapUVs", //0x6fd9d8
		"0 0 1 1", //0x6fd9dc
		"int", //0x6fd9e0
		"Width", //0x6fd9e4
		"10", //0x6fd9e8
		"int", //0x6fd9ec
		"Height", //0x6fd9f0
		"10", //0x6fd9f4
		"float", //0x6fd9f8
		"GridSize", //0x6fd9fc
		"1.0f", //0x6fda00
		"VFX_Color", //0x6fda04
		"Color", //0x6fda08
		"255 255 255", //0x6fda0c
		"enum GFX_RENDER_PHASE", //0x6fda10
		"RenderPhase", //0x6fda14
		"RP_TRANSLUSCENT", //0x6fda18
		"bool", //0x6fda1c
		"DoubleSided", //0x6fda20
		"1", //0x6fda24
		"bool", //0x6fda28
		"Modulate2x", //0x6fda2c
		"1", //0x6fda30
		"bool", //0x6fda34
		"IgnoreLighting", //0x6fda38
		"1", //0x6fda3c
		"float", //0x6fda40
		"UpdateInterval", //0x6fda44
		"0.025f", //0x6fda48
		"\0050.005f", //0x6fda4c
		"\001Alpha", //0x6fda50
		"VFX_Byte", //0x6fda54
		"Alpha", //0x6fda58
		"255", //0x6fda5c
		"bool", //0x6fda60
		"AdjustAlpha", //0x6fda64
		"0", //0x6fda68
		"bool", //0x6fda6c
		"AdjustAlphaOverLifetime", //0x6fda70
		"1", //0x6fda74
		"\002AdjustAlpha != 0", //0x6fda78
		"VFX_Byte", //0x6fda7c
		"FinalAlpha", //0x6fda80
		"0", //0x6fda84
		"\002AdjustAlpha != 0", //0x6fda88
		"enum VFX_WaveType", //0x6fda8c
		"AlphaWaveform", //0x6fda90
		"VFXWAVE_LINEAR", //0x6fda94
		"\002AdjustAlpha != 0", //0x6fda98
		"float", //0x6fda9c
		"AlphaFreq", //0x6fdaa0
		"1", //0x6fdaa4
		"\002AdjustAlpha != 0", //0x6fdaa8
		"float", //0x6fdaac
		"AlphaPhase", //0x6fdab0
		"0", //0x6fdab4
		"\002AdjustAlpha != 0", //0x6fdab8
		"\001Motion", //0x6fdabc
		"enum VFX_WaterMotionType", //0x6fdac0
		"WaterMotionType", //0x6fdac4
		"VFXWATERMOTIONTYPE_NONE", //0x6fdac8
		"VFX_Vector2", //0x6fdacc
		"MinDropLocation", //0x6fdad0
		"0 0 0", //0x6fdad4
		"\002WaterMotionType = VFXWATERMOTIONTYPE_RANDOMDROPS", //0x6fdad8
		"VFX_Vector2", //0x6fdadc
		"MaxDropLocation", //0x6fdae0
		"1 1 1", //0x6fdae4
		"\002WaterMotionType = VFXWATERMOTIONTYPE_RANDOMDROPS", //0x6fdae8
		"float", //0x6fdaec
		"MinDropAmount", //0x6fdaf0
		"0.1f", //0x6fdaf4
		"\002WaterMotionType = VFXWATERMOTIONTYPE_RANDOMDROPS", //0x6fdaf8
		"float", //0x6fdafc
		"MaxDropAmount", //0x6fdb00
		"0.2f", //0x6fdb04
		"\002WaterMotionType = VFXWATERMOTIONTYPE_RANDOMDROPS", //0x6fdb08
		"float", //0x6fdb0c
		"MinDropInterval", //0x6fdb10
		"1.0f", //0x6fdb14
		"\002WaterMotionType = VFXWATERMOTIONTYPE_RANDOMDROPS", //0x6fdb18
		"float", //0x6fdb1c
		"MaxDropInterval", //0x6fdb20
		"2.0f", //0x6fdb24
		"\002WaterMotionType = VFXWATERMOTIONTYPE_RANDOMDROPS", //0x6fdb28
		"VFX_Vector2", //0x6fdb2c
		"MinFlowOrigin", //0x6fdb30
		"0.1f 0.5f", //0x6fdb34
		"\002WaterMotionType = VFXWATERMOTIONTYPE_FLOW", //0x6fdb38
		"VFX_Vector2", //0x6fdb3c
		"MaxFlowOrigin", //0x6fdb40
		"0.1f 0.5f", //0x6fdb44
		"\002WaterMotionType = VFXWATERMOTIONTYPE_FLOW", //0x6fdb48
		"VFX_Vector2", //0x6fdb4c
		"MinFlowDirection", //0x6fdb50
		"1 0", //0x6fdb54
		"\002WaterMotionType = VFXWATERMOTIONTYPE_FLOW", //0x6fdb58
		"VFX_Vector2", //0x6fdb5c
		"MaxFlowDirection", //0x6fdb60
		"1 0", //0x6fdb64
		"\002WaterMotionType = VFXWATERMOTIONTYPE_FLOW", //0x6fdb68
		"float", //0x6fdb6c
		"MinFlowAmount", //0x6fdb70
		"0.1f", //0x6fdb74
		"\002WaterMotionType = VFXWATERMOTIONTYPE_FLOW", //0x6fdb78
		"float", //0x6fdb7c
		"MaxFlowAmount", //0x6fdb80
		"0.1f", //0x6fdb84
		"\002WaterMotionType = VFXWATERMOTIONTYPE_FLOW", //0x6fdb88
		"float", //0x6fdb8c
		"MinFlowInterval", //0x6fdb90
		"1.0f", //0x6fdb94
		"\002WaterMotionType = VFXWATERMOTIONTYPE_FLOW", //0x6fdb98
		"float", //0x6fdb9c
		"MaxFlowInterval", //0x6fdba0
		"1.0f", //0x6fdba4
		"\002WaterMotionType = VFXWATERMOTIONTYPE_FLOW", //0x6fdba8
		"float", //0x6fdbac
		"MinSwirlRadius", //0x6fdbb0
		"0.2f", //0x6fdbb4
		"\002WaterMotionType = VFXWATERMOTIONTYPE_SWIRL", //0x6fdbb8
		"float", //0x6fdbbc
		"MaxSwirlRadius", //0x6fdbc0
		"0.2f", //0x6fdbc4
		"\002WaterMotionType = VFXWATERMOTIONTYPE_SWIRL", //0x6fdbc8
		"float", //0x6fdbcc
		"MinSwirlSpeed", //0x6fdbd0
		"360.0f", //0x6fdbd4
		"\002WaterMotionType = VFXWATERMOTIONTYPE_SWIRL", //0x6fdbd8
		"float", //0x6fdbdc
		"MaxSwirlSpeed", //0x6fdbe0
		"360.0f", //0x6fdbe4
		"\002WaterMotionType = VFXWATERMOTIONTYPE_SWIRL", //0x6fdbe8
		"float", //0x6fdbec
		"MinSwirlAmount", //0x6fdbf0
		"0.1f", //0x6fdbf4
		"\002WaterMotionType = VFXWATERMOTIONTYPE_SWIRL", //0x6fdbf8
		"float", //0x6fdbfc
		"MaxSwirlAmount", //0x6fdc00
		"0.1f", //0x6fdc04
		"\002WaterMotionType = VFXWATERMOTIONTYPE_SWIRL", //0x6fdc08
		"float", //0x6fdc0c
		"MinSwirlInterval", //0x6fdc10
		"0.025f", //0x6fdc14
		"\0050.005f", //0x6fdc18
		"\002WaterMotionType = VFXWATERMOTIONTYPE_SWIRL", //0x6fdc1c
		"float", //0x6fdc20
		"MaxSwirlInterval", //0x6fdc24
		"0.025f", //0x6fdc28
		"\0050.005f", //0x6fdc2c
		"\002WaterMotionType = VFXWATERMOTIONTYPE_SWIRL", //0x6fdc30
		NULL, //0x6fdc34
		"VFXEFFECTTYPE_LIGHTNING", //0x6fdc38
		"\001Lightning", //0x6fdc3c
		"enum GFX_RENDER_PHASE", //0x6fdc40
		"RenderPhase", //0x6fdc44
		"RP_BLEND_ADD", //0x6fdc48
		"bool", //0x6fdc4c
		"Modulate2x", //0x6fdc50
		"1", //0x6fdc54
		"bool", //0x6fdc58
		"InnerGlow", //0x6fdc5c
		"true", //0x6fdc60
		"float", //0x6fdc64
		"MinLifetime", //0x6fdc68
		"0.0f", //0x6fdc6c
		"float", //0x6fdc70
		"MaxLifetime", //0x6fdc74
		"0.01f", //0x6fdc78
		"int", //0x6fdc7c
		"RepeatCount", //0x6fdc80
		"-1", //0x6fdc84
		"float", //0x6fdc88
		"MinRepeatDelay", //0x6fdc8c
		"0.0f", //0x6fdc90
		"float", //0x6fdc94
		"MaxRepeatDelay", //0x6fdc98
		"0.1f", //0x6fdc9c
		"VFX_Target", //0x6fdca0
		"EndPointTarget", //0x6fdca4
		"VFXTARGET_SELF", //0x6fdca8
		"enum GFX_BONE_ID", //0x6fdcac
		"EndPointSocket", //0x6fdcb0
		"BONE_NONE", //0x6fdcb4
		"\002EndPointSnapBone empty && EndPointTargetBodyPart = false || EndPointSocket != BONE_NONE", //0x6fdcb8
		"VFX_SnapBone", //0x6fdcbc
		"EndPointSnapBone", //0x6fdcc0
		"", //0x6fdcc4
		"\002EndPointSocket = BONE_NONE && EndPointTargetBodyPart = false || EndPointSnapBone !empty", //0x6fdcc8
		"bool", //0x6fdccc
		"EndPointTargetBodyPart", //0x6fdcd0
		"false", //0x6fdcd4
		"\002SnapBone empty && Socket = BONE_NONE || EndPointTargetBodyPart != false", //0x6fdcd8
		"VFX_Vector", //0x6fdcdc
		"EndPointOffset", //0x6fdce0
		"0 0 0", //0x6fdce4
		"float", //0x6fdce8
		"MinSegmentsPerFoot", //0x6fdcec
		"1.0f", //0x6fdcf0
		"float", //0x6fdcf4
		"MaxSegmentsPerFoot", //0x6fdcf8
		"1.0f", //0x6fdcfc
		"float", //0x6fdd00
		"MinAdditionalSegments", //0x6fdd04
		"0.0f", //0x6fdd08
		"float", //0x6fdd0c
		"MaxAdditionalSegments", //0x6fdd10
		"0.0f", //0x6fdd14
		"float", //0x6fdd18
		"MinJaggedness", //0x6fdd1c
		"0.0f", //0x6fdd20
		"float", //0x6fdd24
		"MaxJaggedness", //0x6fdd28
		"50.0f", //0x6fdd2c
		"float", //0x6fdd30
		"MinMeander", //0x6fdd34
		"100.0f", //0x6fdd38
		"\0030", //0x6fdd3c
		"\004100", //0x6fdd40
		"\0051.0f", //0x6fdd44
		"float", //0x6fdd48
		"MaxMeander", //0x6fdd4c
		"100.0f", //0x6fdd50
		"\0030", //0x6fdd54
		"\004100", //0x6fdd58
		"\0051.0f", //0x6fdd5c
		"bool", //0x6fdd60
		"AlwaysStrikeTarget", //0x6fdd64
		"true", //0x6fdd68
		"\002MinMeander > 0 || MaxMeander > 0", //0x6fdd6c
		"float", //0x6fdd70
		"MinWidth", //0x6fdd74
		"1.0f", //0x6fdd78
		"float", //0x6fdd7c
		"MaxWidth", //0x6fdd80
		"1.0f", //0x6fdd84
		"enum VFX_LightningWidthStyle", //0x6fdd88
		"WidthStyle", //0x6fdd8c
		"VFXLIGHTNINGWIDTHSTYLE_MINTOMAX", //0x6fdd90
		"float", //0x6fdd94
		"TipWidthScale", //0x6fdd98
		"0.0f", //0x6fdd9c
		"\001Branching", //0x6fdda0
		"float", //0x6fdda4
		"BranchProbability", //0x6fdda8
		"10.0f", //0x6fddac
		"float", //0x6fddb0
		"MinBranchAngle", //0x6fddb4
		"20.0f", //0x6fddb8
		"\002BranchProbability > 0.0f", //0x6fddbc
		"float", //0x6fddc0
		"MaxBranchAngle", //0x6fddc4
		"40.0f", //0x6fddc8
		"\002BranchProbability > 0.0f", //0x6fddcc
		"int", //0x6fddd0
		"MaxBranches", //0x6fddd4
		"20", //0x6fddd8
		"\002BranchProbability > 0.0f", //0x6fdddc
		"int", //0x6fdde0
		"MaxBranchLevels", //0x6fdde4
		"3", //0x6fdde8
		"\002BranchProbability > 0.0f", //0x6fddec
		"float", //0x6fddf0
		"MinBranchWidthScale", //0x6fddf4
		"0.5f", //0x6fddf8
		"\002BranchProbability > 0.0f", //0x6fddfc
		"float", //0x6fde00
		"MaxBranchWidthScale", //0x6fde04
		"0.5f", //0x6fde08
		"\002BranchProbability > 0.0f", //0x6fde0c
		"float", //0x6fde10
		"MinBranchLengthScale", //0x6fde14
		"0.8f", //0x6fde18
		"\002BranchProbability > 0.0f", //0x6fde1c
		"float", //0x6fde20
		"MaxBranchLengthScale", //0x6fde24
		"0.8f", //0x6fde28
		"\002BranchProbability > 0.0f", //0x6fde2c
		"\001Color", //0x6fde30
		"VFX_Color", //0x6fde34
		"OuterColor", //0x6fde38
		"0 0 255", //0x6fde3c
		"VFX_Color", //0x6fde40
		"InnerColor", //0x6fde44
		"255 255 255", //0x6fde48
		"\002InnerGlow != 0", //0x6fde4c
		"bool", //0x6fde50
		"AdjustOuterColor", //0x6fde54
		"0", //0x6fde58
		"bool", //0x6fde5c
		"AdjustOuterColorOverLifetime", //0x6fde60
		"1", //0x6fde64
		"\002AdjustOuterColor != 0", //0x6fde68
		"VFX_Color", //0x6fde6c
		"FinalOuterColor", //0x6fde70
		"0 0 0", //0x6fde74
		"\002AdjustOuterColor != 0", //0x6fde78
		"enum VFX_WaveType", //0x6fde7c
		"OuterColorWaveform", //0x6fde80
		"VFXWAVE_LINEAR", //0x6fde84
		"\002AdjustOuterColor != 0", //0x6fde88
		"float", //0x6fde8c
		"OuterColorFreq", //0x6fde90
		"1", //0x6fde94
		"\002AdjustOuterColor != 0", //0x6fde98
		"float", //0x6fde9c
		"OuterColorPhase", //0x6fdea0
		"0", //0x6fdea4
		"\002AdjustOuterColor != 0", //0x6fdea8
		"bool", //0x6fdeac
		"AdjustInnerColor", //0x6fdeb0
		"0", //0x6fdeb4
		"\002InnerGlow != 0", //0x6fdeb8
		"bool", //0x6fdebc
		"AdjustInnerColorOverLifetime", //0x6fdec0
		"1", //0x6fdec4
		"\002InnerGlow != 0 && AdjustInnerColor != 0", //0x6fdec8
		"VFX_Color", //0x6fdecc
		"FinalInnerColor", //0x6fded0
		"0 0 0", //0x6fded4
		"\002InnerGlow != 0 && AdjustInnerColor != 0", //0x6fded8
		"enum VFX_WaveType", //0x6fdedc
		"InnerColorWaveform", //0x6fdee0
		"VFXWAVE_LINEAR", //0x6fdee4
		"\002InnerGlow != 0 && AdjustInnerColor != 0", //0x6fdee8
		"float", //0x6fdeec
		"InnerColorFreq", //0x6fdef0
		"1", //0x6fdef4
		"\002InnerGlow != 0 && AdjustInnerColor != 0", //0x6fdef8
		"float", //0x6fdefc
		"InnerColorPhase", //0x6fdf00
		"0", //0x6fdf04
		"\002InnerGlow != 0 && AdjustInnerColor != 0", //0x6fdf08
		"\001Alpha", //0x6fdf0c
		"VFX_Byte", //0x6fdf10
		"OuterAlpha", //0x6fdf14
		"0", //0x6fdf18
		"VFX_Byte", //0x6fdf1c
		"InnerAlpha", //0x6fdf20
		"128", //0x6fdf24
		"\002InnerGlow != 0", //0x6fdf28
		"bool", //0x6fdf2c
		"AdjustOuterAlpha", //0x6fdf30
		"0", //0x6fdf34
		"bool", //0x6fdf38
		"AdjustOuterAlphaOverLifetime", //0x6fdf3c
		"1", //0x6fdf40
		"\002AdjustOuterAlpha != 0", //0x6fdf44
		"VFX_Byte", //0x6fdf48
		"FinalOuterAlpha", //0x6fdf4c
		"0", //0x6fdf50
		"\002AdjustOuterAlpha != 0", //0x6fdf54
		"enum VFX_WaveType", //0x6fdf58
		"OuterAlphaWaveform", //0x6fdf5c
		"VFXWAVE_LINEAR", //0x6fdf60
		"\002AdjustOuterAlpha != 0", //0x6fdf64
		"float", //0x6fdf68
		"OuterAlphaFreq", //0x6fdf6c
		"1", //0x6fdf70
		"\002AdjustOuterAlpha != 0", //0x6fdf74
		"float", //0x6fdf78
		"OuterAlphaPhase", //0x6fdf7c
		"0", //0x6fdf80
		"\002AdjustOuterAlpha != 0", //0x6fdf84
		"bool", //0x6fdf88
		"AdjustInnerAlpha", //0x6fdf8c
		"0", //0x6fdf90
		"\002InnerGlow != 0", //0x6fdf94
		"bool", //0x6fdf98
		"AdjustInnerAlphaOverLifetime", //0x6fdf9c
		"1", //0x6fdfa0
		"\002InnerGlow != 0 && AdjustInnerAlpha != 0", //0x6fdfa4
		"VFX_Byte", //0x6fdfa8
		"FinalInnerAlpha", //0x6fdfac
		"0", //0x6fdfb0
		"\002InnerGlow != 0 && AdjustInnerAlpha != 0", //0x6fdfb4
		"enum VFX_WaveType", //0x6fdfb8
		"InnerAlphaWaveform", //0x6fdfbc
		"VFXWAVE_LINEAR", //0x6fdfc0
		"\002InnerGlow != 0 && AdjustInnerAlpha != 0", //0x6fdfc4
		"float", //0x6fdfc8
		"InnerAlphaFreq", //0x6fdfcc
		"1", //0x6fdfd0
		"\002InnerGlow != 0 && AdjustInnerAlpha != 0", //0x6fdfd4
		"float", //0x6fdfd8
		"InnerAlphaPhase", //0x6fdfdc
		"0", //0x6fdfe0
		"\002InnerGlow != 0 && AdjustInnerAlpha != 0", //0x6fdfe4
		NULL, //0x6fdfe8
		"VFXEFFECTTYPE_MODELEFFECT", //0x6fdfec
		"\001Model", //0x6fdff0
		"bool", //0x6fdff4
		"IgnoreLighting", //0x6fdff8
		"0", //0x6fdffc
		"bool", //0x6fe000
		"UseRenderPhase", //0x6fe004
		"0", //0x6fe008
		"enum GFX_RENDER_PHASE", //0x6fe00c
		"RenderPhase", //0x6fe010
		"RP_TRANSLUSCENT", //0x6fe014
		"\002UseRenderPhase != 0", //0x6fe018
		"enum VFX_ShadowVolumeType", //0x6fe01c
		"ShadowVolume", //0x6fe020
		"VFXSHADOWVOLUME_DEFAULT", //0x6fe024
		"bool", //0x6fe028
		"ApplyToAttachments", //0x6fe02c
		"1", //0x6fe030
		"\001Color", //0x6fe034
		"bool", //0x6fe038
		"UseColor", //0x6fe03c
		"0", //0x6fe040
		"VFX_Color", //0x6fe044
		"Color", //0x6fe048
		"255 255 255", //0x6fe04c
		"\002UseColor != 0 || AdjustColor != 0", //0x6fe050
		"bool", //0x6fe054
		"AdjustColor", //0x6fe058
		"0", //0x6fe05c
		"bool", //0x6fe060
		"AdjustColorOverLifetime", //0x6fe064
		"1", //0x6fe068
		"\002AdjustColor != 0", //0x6fe06c
		"VFX_Color", //0x6fe070
		"FinalColor", //0x6fe074
		"0 0 0", //0x6fe078
		"\002AdjustColor != 0", //0x6fe07c
		"enum VFX_WaveType", //0x6fe080
		"ColorWaveform", //0x6fe084
		"VFXWAVE_LINEAR", //0x6fe088
		"\002AdjustColor != 0", //0x6fe08c
		"float", //0x6fe090
		"ColorFreq", //0x6fe094
		"1", //0x6fe098
		"\002AdjustColor != 0", //0x6fe09c
		"float", //0x6fe0a0
		"ColorPhase", //0x6fe0a4
		"0", //0x6fe0a8
		"\002AdjustColor != 0", //0x6fe0ac
		"bool", //0x6fe0b0
		"UseEmissive", //0x6fe0b4
		"0", //0x6fe0b8
		"VFX_Color", //0x6fe0bc
		"Emissive", //0x6fe0c0
		"255 255 255", //0x6fe0c4
		"\002UseEmissive != 0 || AdjustEmissive != 0", //0x6fe0c8
		"bool", //0x6fe0cc
		"AdjustEmissive", //0x6fe0d0
		"0", //0x6fe0d4
		"bool", //0x6fe0d8
		"AdjustEmissiveOverLifetime", //0x6fe0dc
		"1", //0x6fe0e0
		"\002AdjustEmissive != 0", //0x6fe0e4
		"VFX_Color", //0x6fe0e8
		"FinalEmissive", //0x6fe0ec
		"0 0 0", //0x6fe0f0
		"\002AdjustEmissive != 0", //0x6fe0f4
		"enum VFX_WaveType", //0x6fe0f8
		"EmissiveWaveform", //0x6fe0fc
		"VFXWAVE_LINEAR", //0x6fe100
		"\002AdjustEmissive != 0", //0x6fe104
		"float", //0x6fe108
		"EmissiveFreq", //0x6fe10c
		"1", //0x6fe110
		"\002AdjustEmissive != 0", //0x6fe114
		"float", //0x6fe118
		"EmissivePhase", //0x6fe11c
		"0", //0x6fe120
		"\002AdjustEmissive != 0", //0x6fe124
		"\001Alpha", //0x6fe128
		"bool", //0x6fe12c
		"UseAlpha", //0x6fe130
		"0", //0x6fe134
		"VFX_Byte", //0x6fe138
		"Alpha", //0x6fe13c
		"255", //0x6fe140
		"\002UseAlpha != 0 || AdjustAlpha != 0", //0x6fe144
		"bool", //0x6fe148
		"AdjustAlpha", //0x6fe14c
		"0", //0x6fe150
		"bool", //0x6fe154
		"AdjustAlphaOverLifetime", //0x6fe158
		"1", //0x6fe15c
		"\002AdjustAlpha != 0", //0x6fe160
		"VFX_Byte", //0x6fe164
		"FinalAlpha", //0x6fe168
		"0", //0x6fe16c
		"\002AdjustAlpha != 0", //0x6fe170
		"enum VFX_WaveType", //0x6fe174
		"AlphaWaveform", //0x6fe178
		"VFXWAVE_LINEAR", //0x6fe17c
		"\002AdjustAlpha != 0", //0x6fe180
		"float", //0x6fe184
		"AlphaFreq", //0x6fe188
		"1", //0x6fe18c
		"\002AdjustAlpha != 0", //0x6fe190
		"float", //0x6fe194
		"AlphaPhase", //0x6fe198
		"0", //0x6fe19c
		"\002AdjustAlpha != 0", //0x6fe1a0
		NULL, //0x6fe1a4
		"VFXEFFECTTYPE_LIGHT", //0x6fe1a8
		"\001Light", //0x6fe1ac
		"enum VFX_LightType", //0x6fe1b0
		"LightType", //0x6fe1b4
		"VFXLIGHTTYPE_POINT", //0x6fe1b8
		"VFX_Vector", //0x6fe1bc
		"Direction", //0x6fe1c0
		"1 1 1", //0x6fe1c4
		"\002LightType != VFXLIGHTTYPE_POINT", //0x6fe1c8
		"VFX_Color", //0x6fe1cc
		"Diffuse", //0x6fe1d0
		"255 255 255", //0x6fe1d4
		"float", //0x6fe1d8
		"DiffusePercent", //0x6fe1dc
		"100.0", //0x6fe1e0
		"VFX_Color", //0x6fe1e4
		"Specular", //0x6fe1e8
		"0 0 0", //0x6fe1ec
		"VFX_Color", //0x6fe1f0
		"Ambient", //0x6fe1f4
		"0 0 0", //0x6fe1f8
		"float", //0x6fe1fc
		"Range", //0x6fe200
		"1.0", //0x6fe204
		"float", //0x6fe208
		"Falloff", //0x6fe20c
		"1.0", //0x6fe210
		"float", //0x6fe214
		"Attenuation0", //0x6fe218
		"1.0", //0x6fe21c
		"float", //0x6fe220
		"Attenuation1", //0x6fe224
		"0.0", //0x6fe228
		"float", //0x6fe22c
		"Attenuation2", //0x6fe230
		"0.0", //0x6fe234
		"float", //0x6fe238
		"Theta", //0x6fe23c
		"0.0", //0x6fe240
		"float", //0x6fe244
		"Phi", //0x6fe248
		"0.0", //0x6fe24c
		"int", //0x6fe250
		"Priority", //0x6fe254
		"5", //0x6fe258
		"\0030", //0x6fe25c
		"\00410", //0x6fe260
		"bool", //0x6fe264
		"CastShadow", //0x6fe268
		"0", //0x6fe26c
		"bool", //0x6fe270
		"ShowShape", //0x6fe274
		"0", //0x6fe278
		"\001Color", //0x6fe27c
		"bool", //0x6fe280
		"AdjustDiffuse", //0x6fe284
		"0", //0x6fe288
		"bool", //0x6fe28c
		"AdjustDiffuseOverLifetime", //0x6fe290
		"1", //0x6fe294
		"\002AdjustDiffuse != 0", //0x6fe298
		"VFX_Color", //0x6fe29c
		"FinalDiffuse", //0x6fe2a0
		"0 0 0", //0x6fe2a4
		"\002AdjustDiffuse != 0", //0x6fe2a8
		"enum VFX_WaveType", //0x6fe2ac
		"DiffuseWaveform", //0x6fe2b0
		"VFXWAVE_LINEAR", //0x6fe2b4
		"\002AdjustDiffuse != 0", //0x6fe2b8
		"float", //0x6fe2bc
		"DiffuseFreq", //0x6fe2c0
		"1", //0x6fe2c4
		"\002AdjustDiffuse != 0", //0x6fe2c8
		"float", //0x6fe2cc
		"DiffusePhase", //0x6fe2d0
		"0", //0x6fe2d4
		"\002AdjustDiffuse != 0", //0x6fe2d8
		"bool", //0x6fe2dc
		"AdjustSpecular", //0x6fe2e0
		"0", //0x6fe2e4
		"bool", //0x6fe2e8
		"AdjustSpecularOverLifetime", //0x6fe2ec
		"1", //0x6fe2f0
		"\002AdjustSpecular != 0", //0x6fe2f4
		"VFX_Color", //0x6fe2f8
		"FinalSpecular", //0x6fe2fc
		"0 0 0", //0x6fe300
		"\002AdjustSpecular != 0", //0x6fe304
		"enum VFX_WaveType", //0x6fe308
		"SpecularWaveform", //0x6fe30c
		"VFXWAVE_LINEAR", //0x6fe310
		"\002AdjustSpecular != 0", //0x6fe314
		"float", //0x6fe318
		"SpecularFreq", //0x6fe31c
		"1", //0x6fe320
		"\002AdjustSpecular != 0", //0x6fe324
		"float", //0x6fe328
		"SpecularPhase", //0x6fe32c
		"0", //0x6fe330
		"\002AdjustSpecular != 0", //0x6fe334
		"bool", //0x6fe338
		"AdjustAmbient", //0x6fe33c
		"0", //0x6fe340
		"bool", //0x6fe344
		"AdjustAmbientOverLifetime", //0x6fe348
		"1", //0x6fe34c
		"\002AdjustAmbient != 0", //0x6fe350
		"VFX_Color", //0x6fe354
		"FinalAmbient", //0x6fe358
		"0 0 0", //0x6fe35c
		"\002AdjustAmbient != 0", //0x6fe360
		"enum VFX_WaveType", //0x6fe364
		"AmbientWaveform", //0x6fe368
		"VFXWAVE_LINEAR", //0x6fe36c
		"\002AdjustAmbient != 0", //0x6fe370
		"float", //0x6fe374
		"AmbientFreq", //0x6fe378
		"1", //0x6fe37c
		"\002AdjustAmbient != 0", //0x6fe380
		"float", //0x6fe384
		"AmbientPhase", //0x6fe388
		"0", //0x6fe38c
		"\002AdjustAmbient != 0", //0x6fe390
		"\001Adjust", //0x6fe394
		"bool", //0x6fe398
		"AdjustRange", //0x6fe39c
		"0", //0x6fe3a0
		"bool", //0x6fe3a4
		"AdjustRangeOverLifetime", //0x6fe3a8
		"1", //0x6fe3ac
		"\002AdjustRange != 0", //0x6fe3b0
		"float", //0x6fe3b4
		"FinalRange", //0x6fe3b8
		"0", //0x6fe3bc
		"\002AdjustScale != 0", //0x6fe3c0
		"enum VFX_WaveType", //0x6fe3c4
		"RangeWaveform", //0x6fe3c8
		"VFXWAVE_LINEAR", //0x6fe3cc
		"\002AdjustRange != 0", //0x6fe3d0
		"float", //0x6fe3d4
		"RangeFreq", //0x6fe3d8
		"1", //0x6fe3dc
		"\002AdjustRange != 0", //0x6fe3e0
		"float", //0x6fe3e4
		"RangePhase", //0x6fe3e8
		"0", //0x6fe3ec
		"\002AdjustRange != 0", //0x6fe3f0
		NULL, //0x6fe3f4
		"VFXEFFECTTYPE_RAIN", //0x6fe3f8
		"\001Rain", //0x6fe3fc
		"float", //0x6fe400
		"RainDropsPerSecond", //0x6fe404
		"2000", //0x6fe408
		"int", //0x6fe40c
		"MaxRainDrops", //0x6fe410
		"1500", //0x6fe414
		"VFX_Vector", //0x6fe418
		"MinVelocity", //0x6fe41c
		"-0.01, -90, -0.01", //0x6fe420
		"VFX_Vector", //0x6fe424
		"MaxVelocity", //0x6fe428
		"0.01, -60, 0.01", //0x6fe42c
		"float", //0x6fe430
		"MinLength", //0x6fe434
		"0.005", //0x6fe438
		"float", //0x6fe43c
		"MaxLength", //0x6fe440
		"0.010", //0x6fe444
		"VFX_Color", //0x6fe448
		"Color", //0x6fe44c
		"128 128 128", //0x6fe450
		"VFX_Byte", //0x6fe454
		"MinBeginAlpha", //0x6fe458
		"96", //0x6fe45c
		"VFX_Byte", //0x6fe460
		"MaxBeginAlpha", //0x6fe464
		"128", //0x6fe468
		"VFX_Byte", //0x6fe46c
		"MinEndAlpha", //0x6fe470
		"0", //0x6fe474
		"VFX_Byte", //0x6fe478
		"MaxEndAlpha", //0x6fe47c
		"0", //0x6fe480
		"enum GFX_RENDER_PHASE", //0x6fe484
		"RenderPhase", //0x6fe488
		"RP_TRANSLUSCENT", //0x6fe48c
		"bool", //0x6fe490
		"Modulate2x", //0x6fe494
		"0", //0x6fe498
		"float", //0x6fe49c
		"EmitterSize", //0x6fe4a0
		"16", //0x6fe4a4
		"float", //0x6fe4a8
		"EmitterYOffset", //0x6fe4ac
		"5", //0x6fe4b0
		"float", //0x6fe4b4
		"GroundYOffset", //0x6fe4b8
		"-5", //0x6fe4bc
		"VFX_Vector", //0x6fe4c0
		"OffsetFromCamera", //0x6fe4c4
		"0 0 16", //0x6fe4c8
		"bool", //0x6fe4cc
		"AffectedByWind", //0x6fe4d0
		"1", //0x6fe4d4
		"float", //0x6fe4d8
		"WindScale", //0x6fe4dc
		"10.0", //0x6fe4e0
		"\002AffectedByWind != 0", //0x6fe4e4
		NULL, //0x6fe4e8
		"VFXEFFECTTYPE_SNOW", //0x6fe4ec
		"\001Snow", //0x6fe4f0
		"VFX_Resource", //0x6fe4f4
		"SnowTexture", //0x6fe4f8
		"tga", //0x6fe4fc
		"VFX_TextureUVs", //0x6fe500
		"SnowTextureUVs", //0x6fe504
		"0 0 1 1", //0x6fe508
		"float", //0x6fe50c
		"SnowFlakesPerSecond", //0x6fe510
		"100", //0x6fe514
		"int", //0x6fe518
		"MaxSnowFlakes", //0x6fe51c
		"2000", //0x6fe520
		"VFX_Vector", //0x6fe524
		"MinVelocity", //0x6fe528
		"-0.01, -0.2, -0.01", //0x6fe52c
		"VFX_Vector", //0x6fe530
		"MaxVelocity", //0x6fe534
		"0.01, -0.3, 0.01", //0x6fe538
		"float", //0x6fe53c
		"MinSize", //0x6fe540
		"0.001", //0x6fe544
		"float", //0x6fe548
		"MaxSize", //0x6fe54c
		"0.002", //0x6fe550
		"VFX_Color", //0x6fe554
		"Color", //0x6fe558
		"255 255 255", //0x6fe55c
		"VFX_Byte", //0x6fe560
		"MinAlpha", //0x6fe564
		"32", //0x6fe568
		"VFX_Byte", //0x6fe56c
		"MaxAlpha", //0x6fe570
		"128", //0x6fe574
		"enum GFX_RENDER_PHASE", //0x6fe578
		"RenderPhase", //0x6fe57c
		"RP_TRANSLUSCENT", //0x6fe580
		"bool", //0x6fe584
		"Modulate2x", //0x6fe588
		"0", //0x6fe58c
		"float", //0x6fe590
		"EmitterSize", //0x6fe594
		"6", //0x6fe598
		"float", //0x6fe59c
		"EmitterYOffset", //0x6fe5a0
		"2", //0x6fe5a4
		"float", //0x6fe5a8
		"GroundYOffset", //0x6fe5ac
		"-2", //0x6fe5b0
		"VFX_Vector", //0x6fe5b4
		"OffsetFromCamera", //0x6fe5b8
		"0 0 3", //0x6fe5bc
		"float", //0x6fe5c0
		"MinXSinAmplitude", //0x6fe5c4
		"0", //0x6fe5c8
		"float", //0x6fe5cc
		"MaxXSinAmplitude", //0x6fe5d0
		"0.2", //0x6fe5d4
		"float", //0x6fe5d8
		"MinXSinFrequency", //0x6fe5dc
		"0.05", //0x6fe5e0
		"float", //0x6fe5e4
		"MaxXSinFrequency", //0x6fe5e8
		"0.1", //0x6fe5ec
		"float", //0x6fe5f0
		"MinXSinPhase", //0x6fe5f4
		"0", //0x6fe5f8
		"float", //0x6fe5fc
		"MaxXSinPhase", //0x6fe600
		"100", //0x6fe604
		"float", //0x6fe608
		"MinZSinAmplitude", //0x6fe60c
		"0", //0x6fe610
		"float", //0x6fe614
		"MaxZSinAmplitude", //0x6fe618
		"0.2", //0x6fe61c
		"float", //0x6fe620
		"MinZSinFrequency", //0x6fe624
		"0.05", //0x6fe628
		"float", //0x6fe62c
		"MaxZSinFrequency", //0x6fe630
		"0.1", //0x6fe634
		"float", //0x6fe638
		"MinZSinPhase", //0x6fe63c
		"0", //0x6fe640
		"float", //0x6fe644
		"MaxZSinPhase", //0x6fe648
		"100", //0x6fe64c
		"bool", //0x6fe650
		"AffectedByWind", //0x6fe654
		"1", //0x6fe658
		"float", //0x6fe65c
		"WindScale", //0x6fe660
		"1.0", //0x6fe664
		"\002AffectedByWind != 0", //0x6fe668
		NULL, //0x6fe66c
		"VFXEFFECTTYPE_WIND", //0x6fe670
		"\001Wind", //0x6fe674
		"enum VFX_WindType", //0x6fe678
		"WindType", //0x6fe67c
		"VFXWIND_CONSTANT", //0x6fe680
		"VFX_Vector", //0x6fe684
		"Direction", //0x6fe688
		"1 0 0", //0x6fe68c
		"float", //0x6fe690
		"Velocity", //0x6fe694
		"5.0", //0x6fe698
		"\002WindType == VFXWIND_CONSTANT", //0x6fe69c
		"bool", //0x6fe6a0
		"AdjustVelocity", //0x6fe6a4
		"0", //0x6fe6a8
		"\002WindType == VFXWIND_CONSTANT", //0x6fe6ac
		"bool", //0x6fe6b0
		"AdjustVelocityOverLifetime", //0x6fe6b4
		"1", //0x6fe6b8
		"\002WindType == VFXWIND_CONSTANT && AdjustVelocity != 0", //0x6fe6bc
		"float", //0x6fe6c0
		"FinalVelocity", //0x6fe6c4
		"0", //0x6fe6c8
		"\002WindType == VFXWIND_CONSTANT && AdjustVelocity != 0", //0x6fe6cc
		"enum VFX_WaveType", //0x6fe6d0
		"VelocityWaveform", //0x6fe6d4
		"VFXWAVE_LINEAR", //0x6fe6d8
		"\002WindType == VFXWIND_CONSTANT && AdjustVelocity != 0", //0x6fe6dc
		"float", //0x6fe6e0
		"VelocityFreq", //0x6fe6e4
		"1", //0x6fe6e8
		"\002WindType == VFXWIND_CONSTANT && AdjustVelocity != 0", //0x6fe6ec
		"float", //0x6fe6f0
		"VelocityPhase", //0x6fe6f4
		"0", //0x6fe6f8
		"\002WindType == VFXWIND_CONSTANT && AdjustVelocity != 0", //0x6fe6fc
		"float", //0x6fe700
		"MinGustVelocity", //0x6fe704
		"1.0", //0x6fe708
		"\002WindType == VFXWIND_RANDOMGUSTS", //0x6fe70c
		"float", //0x6fe710
		"MaxGustVelocity", //0x6fe714
		"5.0", //0x6fe718
		"\002WindType == VFXWIND_RANDOMGUSTS", //0x6fe71c
		"float", //0x6fe720
		"MinGustDuration", //0x6fe724
		"0.5", //0x6fe728
		"\002WindType == VFXWIND_RANDOMGUSTS", //0x6fe72c
		"float", //0x6fe730
		"MaxGustDuration", //0x6fe734
		"3.0", //0x6fe738
		"\002WindType == VFXWIND_RANDOMGUSTS", //0x6fe73c
		"float", //0x6fe740
		"MinGustAttack", //0x6fe744
		"0.1", //0x6fe748
		"\002WindType == VFXWIND_RANDOMGUSTS", //0x6fe74c
		"float", //0x6fe750
		"MaxGustAttack", //0x6fe754
		"0.2", //0x6fe758
		"\002WindType == VFXWIND_RANDOMGUSTS", //0x6fe75c
		"float", //0x6fe760
		"MinGustDecay", //0x6fe764
		"0.4", //0x6fe768
		"\002WindType == VFXWIND_RANDOMGUSTS", //0x6fe76c
		"float", //0x6fe770
		"MaxGustDecay", //0x6fe774
		"0.5", //0x6fe778
		"\002WindType == VFXWIND_RANDOMGUSTS", //0x6fe77c
		"float", //0x6fe780
		"MinGustDelay", //0x6fe784
		"2.0", //0x6fe788
		"\002WindType == VFXWIND_RANDOMGUSTS", //0x6fe78c
		"float", //0x6fe790
		"MaxGustDelay", //0x6fe794
		"5.0", //0x6fe798
		"\002WindType == VFXWIND_RANDOMGUSTS", //0x6fe79c
		NULL, //0x6fe7a0
		"VFXEFFECTTYPE_SOUND", //0x6fe7a4
		"\001Sound", //0x6fe7a8
		"VFX_Resource", //0x6fe7ac
		"PresetFile", //0x6fe7b0
		"psf", //0x6fe7b4
		"\002WAVFile empty || PresetFile !empty", //0x6fe7b8
		"VFX_Resource", //0x6fe7bc
		"WAVFile", //0x6fe7c0
		"wav", //0x6fe7c4
		"\002PresetFile empty || WAVFile !empty", //0x6fe7c8
		"bool", //0x6fe7cc
		"Looping", //0x6fe7d0
		"1", //0x6fe7d4
		"\002FireAndForget != true || Looping != false", //0x6fe7d8
		"bool", //0x6fe7dc
		"FireAndForget", //0x6fe7e0
		"0", //0x6fe7e4
		"\002Looping != true || FireAndForget != false", //0x6fe7e8
		NULL, //0x6fe7ec
	};

	const char* const effectNames[] =
	{
		"VFXEFFECTTYPE_PARTICLESYSTEM", //0x6fe82c
		"VFXEFFECTTYPE_MOTIONBLUR", //0x6fe830
		"VFXEFFECTTYPE_SPRITE", //0x6fe834
		"VFXEFFECTTYPE_MODEL", //0x6fe838
		"VFXEFFECTTYPE_TRAIL", //0x6fe83c
		"VFXEFFECTTYPE_COLLISION", //0x6fe840
		"VFXEFFECTTYPE_WATER", //0x6fe844
		"VFXEFFECTTYPE_LIGHTNING", //0x6fe848
		"VFXEFFECTTYPE_MODELEFFECT", //0x6fe84c
		"VFXEFFECTTYPE_LIGHT", //0x6fe850
		"VFXEFFECTTYPE_RAIN", //0x6fe854
		"VFXEFFECTTYPE_SNOW", //0x6fe858
		"VFXEFFECTTYPE_WIND", //0x6fe85c
		"VFXEFFECTTYPE_SOUND", //0x6fe860
	};

	//52FA30: the complete effect name is compared against its real fourteen names.
	LONG EffectTypeID(const char* name)
	{
		for (LONG index = 0; index < 14; ++index)
		{
			if (_mbscmp(reinterpret_cast<const unsigned char*>(effectNames[index]),
				reinterpret_cast<const unsigned char*>(name)) == 0) return index;
		}
		return -1;
	}

	//4FCD00: every invocation reparses the common stream with a fresh category.
	void __stdcall PopulateCommonFormat(JE::VFX_FormatManager::Format& format)
	{
		LONG index = 0;
		const LONG count = sizeof(commonDefinitions) / sizeof(commonDefinitions[0]);
		std::string category;
		while (index < count)
		{
			const char* token = commonDefinitions[index++];
			if (index >= count) break;
			if (*token == 1)
			{
				category.assign(token + 1);
			}
			else
			{
				JE::VFX_ParameterInfo info;
				const LONG type = JE::VFX_Parameter::GetTypeID(token);
				const LONG subtype = JE::VFX_Parameter::GetSubtypeID(token);
				info.type = type;
				info.subtype = subtype;
				{
					const std::string copy(category);
					info.category = copy;
				}
				const char* name = commonDefinitions[index++];
				if (index >= count)
				{
					//The common parser publishes this partial entry; the effect parser does not.
					format.parameters.push_back(info);
					break;
				}
				{
					const std::string copy(name);
					info.name = copy;
				}
				info.SetValueLimit(-1);
				info.SetText(commonDefinitions[index], type, subtype);
				++index;
				while (index < count)
				{
					token = commonDefinitions[index];
					if (token == NULL) break;
					if (*token == 2)
					{
						{
							const std::string copy(token + 1);
							info.condition = copy;
						}
						++index;
					}
					else if (*token == 3)
					{
						JE::VFX_Parameter input;
						input.SetText(token + 1, type, subtype);
						info.minimum = input;
						info.hasMinimum = 1;
						++index;
					}
					else if (*token == 4)
					{
						JE::VFX_Parameter input;
						input.SetText(token + 1, type, subtype);
						info.maximum = input;
						info.hasMaximum = 1;
						++index;
					}
					else if (*token == 5)
					{
						JE::VFX_Parameter input;
						input.SetText(token + 1, type, subtype);
						info.increment = input;
						info.hasIncrement = 1;
						++index;
					}
					else break; //Leave the next category/type token for the outer loop.
				}
				format.parameters.push_back(info);
				if (index >= count) break;
			}
		}
	}

	JE::VFX_FormatManager g_FormatManager; // 0x70811C, initializer 0x641980.
}

namespace JE
{
	VFX_ParameterInfo::VFX_ParameterInfo()
	{
		hasIncrement = 0;
		hasMaximum = 0;
		hasMinimum = 0;
	}

	VFX_ParameterInfo::~VFX_ParameterInfo()
	{
	}

	VFX_FormatManager::VFX_FormatManager()
	{
		LONG index = 0;
		const LONG count = sizeof(effectDefinitions) / sizeof(effectDefinitions[0]);
		std::string category; //4FD780 keeps this across effect groups.
		while (index < count)
		{
			Format format;
			const LONG effectType = EffectTypeID(effectDefinitions[index]);
			++index;
			format.effectType = effectType;
			if (index >= count) break;
			PopulateCommonFormat(format);
			const char* token = effectDefinitions[index++];
			if (index < count)
			{
				while (token != NULL)
				{
					if (*token == 1)
					{
						category.assign(token + 1);
					}
					else
					{
						VFX_ParameterInfo info;
						const LONG type = VFX_Parameter::GetTypeID(token);
						const LONG subtype = VFX_Parameter::GetSubtypeID(token);
						info.type = type;
						info.subtype = subtype;
						{
							const std::string copy(category);
							info.category = copy;
						}
						const char* name = effectDefinitions[index++];
						if (index >= count) break; //Unlike4FCD00, do not publish a partial entry.
						{
							const std::string copy(name);
							info.name = copy;
						}
						info.SetValueLimit(-1);
						info.SetText(effectDefinitions[index], type, subtype);
						++index;
						while (index < count)
						{
							token = effectDefinitions[index];
							if (token == NULL) break;
							if (*token == 2)
							{
								{
									const std::string copy(token + 1);
									info.condition = copy;
								}
								++index;
							}
							else if (*token == 3)
							{
								VFX_Parameter input;
								input.SetText(token + 1, type, subtype);
								info.minimum = input;
								info.hasMinimum = 1;
								++index;
							}
							else if (*token == 4)
							{
								VFX_Parameter input;
								input.SetText(token + 1, type, subtype);
								info.maximum = input;
								info.hasMaximum = 1;
								++index;
							}
							else if (*token == 5)
							{
								VFX_Parameter input;
								input.SetText(token + 1, type, subtype);
								info.increment = input;
								info.hasIncrement = 1;
								++index;
							}
							else break;
						}
						format.parameters.push_back(info);
						if (index >= count) break;
					}
					token = effectDefinitions[index++];
					if (index >= count) break;
				}
			}
			formats.push_back(format); //The local vector dies before the next effect.
		}
		PopulateCommonFormat(common); //4FDC91, after all published effect formats.
		//The original never writes common.effectType; only its parameter vector is used.

		InitializeGlobalDefaults(); //4FDC98.
	}

	void VFX_FormatManager::InitializeGlobalDefaults()
	{
		// 0x4FC7B0: VEG-level controls are separate from effect-level formats.
		{
			VFX_Parameter parameter;
			const LONG type = VFX_Parameter::GetTypeID("bool");
			const LONG subtype = VFX_Parameter::GetSubtypeID("bool");
			{
				const std::string copy("Loop");
				parameter.name = copy;
			}
			parameter.type = type;
			parameter.subtype = subtype;
			parameter.SetValueLimit(-1);
			parameter.SetText("false", type, subtype);
			globalDefaults.push_back(parameter);
		}
		{
			VFX_Parameter parameter;
			const LONG type = VFX_Parameter::GetTypeID("float");
			const LONG subtype = VFX_Parameter::GetSubtypeID("float");
			{
				const std::string copy("MinStartTime");
				parameter.name = copy;
			}
			parameter.type = type;
			parameter.subtype = subtype;
			parameter.SetValueLimit(-1);
			parameter.SetText("0", type, subtype);
			globalDefaults.push_back(parameter);
		}
		{
			VFX_Parameter parameter;
			const LONG type = VFX_Parameter::GetTypeID("float");
			const LONG subtype = VFX_Parameter::GetSubtypeID("float");
			{
				const std::string copy("MaxStartTime");
				parameter.name = copy;
			}
			parameter.type = type;
			parameter.subtype = subtype;
			parameter.SetValueLimit(-1);
			parameter.SetText("0", type, subtype);
			globalDefaults.push_back(parameter);
		}
	}

	VFX_FormatManager::~VFX_FormatManager()
	{
	}

	const VFX_FormatManager::Format* VFX_FormatManager::Find(LONG effectType) const
	{
		std::vector<Format>::const_iterator current = formats.begin();
		const std::vector<Format>::const_iterator end = formats.end();
		for (; current != end; ++current)
		{
			if (current->effectType == effectType) return &*current;
		}
		return NULL;
	}

	void VFX_FormatManager::Merge(VFX_ParameterHolder& holder, LONG effectType) const
	{
		const Format* format = Find(effectType);
		if (format == NULL)
		{
			format = &common;
		}
		for (size_t index = 0; index < format->parameters.size(); ++index)
		{
			const VFX_ParameterInfo& expected = format->parameters[index];
			VFX_Parameter* existing = holder.Find(expected.name.c_str());
			if (existing == NULL || existing->type != expected.type ||
				(expected.type == VFX_Parameter::Enumeration &&
					existing->subtype != expected.subtype))
			{
				holder.Insert(expected, 1);
			}
			else if (expected.type == VFX_Parameter::Resource)
			{
				//Copy each returned pair, then destroy the temporary before the next
				//conversion, as at4FBA3D..4FBAAC.
				const VFX_ResourceData actual(ResourceDataReference(existing->GetTypeAndValue()));
				const VFX_ResourceData desired(ResourceDataReference(expected.GetTypeAndValue()));
				if (_mbscmp(reinterpret_cast<const unsigned char*>(actual.type.c_str()),
					reinterpret_cast<const unsigned char*>(desired.type.c_str())) != 0)
				{
					holder.Insert(expected, 1);
				}
			}
		}
	}

	VFX_FormatManager& GetVfxFormatManager()
	{
		return g_FormatManager;
	}
}
