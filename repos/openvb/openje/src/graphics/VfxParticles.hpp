#pragma once

#include "VfxEffects.hpp"
#include "LineSystem.hpp"

class Gfx_ParticleSystem;

namespace JE
{
	class VFX_ParticleData : public VFX_BaseData
	{
	public:
		VFX_ParticleData(); // 0x4F6D80.
		void Read(const VFX_ParameterHolder& parameters); // 0x52FF00 case0.
		LONG boundingBoxType; // Original +70.
		VFX_Vector optimizedAABBMin; // Original +74.
		VFX_Vector optimizedAABBMax; // Original +80.
		VFX_Vector minBoundingBox; // Original +8C.
		VFX_Vector maxBoundingBox; // Original +98.
		BYTE showBoundingBox; // Original +0A4.
		LONG emitterShape; // Original +0A8.
		VFX_Vector minDimensions; // Original +0AC.
		VFX_Vector maxDimensions; // Original +0B8.
		BYTE showEmitterShape; // Original +0C4.
		VFX_Vector acceleration1; // Original +0C8.
		BYTE rotateAccel1WithTarget; // Original +0D4.
		VFX_Vector acceleration2; // Original +0D8.
		BYTE rotateAccel2WithTarget; // Original +0E4.
		BYTE useEmitterLifetime; // Original +0E5.
		float emitterLifetime; // Original +0E8.
		BYTE autoAdd; // Original +0EC.
		float minAddRate; // Original +0F0.
		float maxAddRate; // Original +0F4.
		LONG minParticlesPerAdd; // Original +0F8.
		LONG maxParticlesPerAdd; // Original +0FC.
		BYTE useMaxParticles; // Original +100.
		LONG maxParticles; // Original +104.
		VFX_ResourceData texture; // Original +108.
		VFX_TextureCoordinates textureUVs; // Original +140.
		VFX_ResourceData model; // Original +150.
		BYTE model_UseRenderPhase; // Original +188.
		BYTE model_UseColor; // Original +189.
		BYTE model_UseAlpha; // Original +18A.
		BYTE model_IgnoreLighting; // Original +18B.
		LONG renderPhase; // Original +18C.
		BYTE modulate2x; // Original +190.
		BYTE minParticleAlpha; // Original +191.
		BYTE maxParticleAlpha; // Original +192.
		float minParticleLifetime; // Original +194.
		float maxParticleLifetime; // Original +198.
		float minParticleSize; // Original +19C.
		float maxParticleSize; // Original +1A0.
		float minParticleMass; // Original +1A4.
		float maxParticleMass; // Original +1A8.
		float minParticleResistance; // Original +1AC.
		float maxParticleResistance; // Original +1B0.
		VFX_Vector minInitialVelocity; // Original +1B4.
		VFX_Vector maxInitialVelocity; // Original +1C0.
		float minInitialVelocityScale; // Original +1CC.
		float maxInitialVelocityScale; // Original +1D0.
		VFX_Target initialVelocityTarget; // Original +1D4.
		LONG initialVelocitySocket; // Original +1D8.
		std::string initialVelocitySnapBone; // Original +1DC.
		BYTE initialVelocityTargetBodyPart; // Original +1F8.
		float minExplosion; // Original +1FC.
		float maxExplosion; // Original +200.
		float minVibration; // Original +204.
		float maxVibration; // Original +208.
		BYTE rotatableParticles; // Original +20C.
		VFX_Rotation minParticleRotation; // Original +210.
		VFX_Rotation maxParticleRotation; // Original +21C.
		VFX_Rotation minParticleSpin; // Original +228.
		VFX_Rotation maxParticleSpin; // Original +234.
		BYTE faceVelocityVector; // Original +240.
		LONG particleDeathEffect; // Original +244.
		LONG numColorsUsed; // Original +248.
		VFX_ColorBytes color1; // Original +24C.
		float colorTime1; // Original +250.
		VFX_ColorBytes color2; // Original +254.
		float colorTime2; // Original +258.
		VFX_ColorBytes color3; // Original +25C.
		float colorTime3; // Original +260.
		VFX_ColorBytes color4; // Original +264.
		float colorTime4; // Original +268.
		VFX_ColorBytes color5; // Original +26C.
		float colorTime5; // Original +270.
		VFX_ColorBytes color6; // Original +274.
		float colorTime6; // Original +278.
		VFX_ColorBytes color7; // Original +27C.
		float colorTime7; // Original +280.
		VFX_ColorBytes color8; // Original +284.
		float colorTime8; // Original +288.
		BYTE adjustColor; // Original +28C.
		BYTE adjustColorOverLifetime; // Original +28D.
		BYTE adjustSystemColor; // Original +28E.
		LONG colorWaveform; // Original +290.
		float colorFreq; // Original +294.
		float colorPhase; // Original +298.
		BYTE adjustParticleAlpha; // Original +29C.
		BYTE adjustParticleAlphaOverLifetime; // Original +29D.
		BYTE startParticleAlpha; // Original +29E.
		BYTE endParticleAlpha; // Original +29F.
		LONG particleAlphaWaveform; // Original +2A0.
		float particleAlphaFreq; // Original +2A4.
		float particleAlphaPhase; // Original +2A8.
		BYTE adjustSystemAlpha; // Original +2AC.
		BYTE adjustSystemAlphaOverLifetime; // Original +2AD.
		BYTE startSystemAlpha; // Original +2AE.
		BYTE endSystemAlpha; // Original +2AF.
		LONG systemAlphaWaveform; // Original +2B0.
		float systemAlphaFreq; // Original +2B4.
		float systemAlphaPhase; // Original +2B8.
		BYTE adjustParticleScale; // Original +2BC.
		BYTE adjustParticleScaleOverLifetime; // Original +2BD.
		float startParticleScale; // Original +2C0.
		float endParticleScale; // Original +2C4.
		LONG particleScaleWaveform; // Original +2C8.
		float particleScaleFreq; // Original +2CC.
		float particleScalePhase; // Original +2D0.
		BYTE adjustSystemScale; // Original +2D4.
		BYTE adjustSystemScaleOverLifetime; // Original +2D5.
		float startSystemScale; // Original +2D8.
		float endSystemScale; // Original +2DC.
		LONG systemScaleWaveform; // Original +2E0.
		float systemScaleFreq; // Original +2E4.
		float systemScalePhase; // Original +2E8.
		BYTE adjustMinDimX; // Original +2EC.
		BYTE adjustMinDimXOverLifetime; // Original +2ED.
		float finalMinDimX; // Original +2F0.
		LONG minDimXWaveform; // Original +2F4.
		float minDimXFreq; // Original +2F8.
		float minDimXPhase; // Original +2FC.
		BYTE adjustMinDimY; // Original +300.
		BYTE adjustMinDimYOverLifetime; // Original +301.
		float finalMinDimY; // Original +304.
		LONG minDimYWaveform; // Original +308.
		float minDimYFreq; // Original +30C.
		float minDimYPhase; // Original +310.
		BYTE adjustMinDimZ; // Original +314.
		BYTE adjustMinDimZOverLifetime; // Original +315.
		float finalMinDimZ; // Original +318.
		LONG minDimZWaveform; // Original +31C.
		float minDimZFreq; // Original +320.
		float minDimZPhase; // Original +324.
		BYTE adjustMaxDimX; // Original +328.
		BYTE adjustMaxDimXOverLifetime; // Original +329.
		float finalMaxDimX; // Original +32C.
		LONG maxDimXWaveform; // Original +330.
		float maxDimXFreq; // Original +334.
		float maxDimXPhase; // Original +338.
		BYTE adjustMaxDimY; // Original +33C.
		BYTE adjustMaxDimYOverLifetime; // Original +33D.
		float finalMaxDimY; // Original +340.
		LONG maxDimYWaveform; // Original +344.
		float maxDimYFreq; // Original +348.
		float maxDimYPhase; // Original +34C.
		BYTE adjustMaxDimZ; // Original +350.
		BYTE adjustMaxDimZOverLifetime; // Original +351.
		float finalMaxDimZ; // Original +354.
		LONG maxDimZWaveform; // Original +358.
		float maxDimZFreq; // Original +35C.
		float maxDimZPhase; // Original +360.
		LONG particleTrailType; // Original +364.
		float trailInterval; // Original +368.
		float trailLifetime; // Original +36C.
		LONG trailRenderPhase; // Original +370.
		BYTE trailModulate2x; // Original +374.
		VFX_ColorBytes beginTrailColor; // Original +375.
		BYTE beginTrailAlpha; // Original +378.
		float beginTrailScale; // Original +37C.
		VFX_ColorBytes endTrailColor; // Original +380.
		BYTE endTrailAlpha; // Original +383.
		float endTrailScale; // Original +384.
		VFX_ResourceData ribbonTexture; // Original +388.
		VFX_TextureCoordinates ribbonTextureUVs; // Original +3C0.
		VFX_Vector ribbonOrigin; // Original +3D0.
		VFX_Vector ribbonEndPoint; // Original +3DC.
		LONG ribbonAlignment; // Original +3E8.
		BYTE performCollision; // Original +3EC.
		BYTE faceNormal; // Original +3ED.
		LONG collisionEffect; // Original +3F0.
	};

	class VFX_ParticleSystem : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::Load;
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		struct TriggeredEffect
		{
			LONG effectIndex;
			VFX_EffectInstance* child;
			D3DXVECTOR3 position;
			float time;
		};
		VFX_ParticleSystem(); // 0x4F7110.
		void Load(const VFX_BaseData* input, float value); //4F69B0; base publication then conditional virtual SetData.
		virtual ~VFX_ParticleSystem();
		virtual void SetData(const VFX_BaseData* input); // 0x4F94E0.
		virtual void Reset(); // 0x4F72A0.
		virtual void SetSocket(DWORD socket); //4F74E0, existing slot60.
		virtual BYTE GetFlag() const; // 0x4080B0.
		virtual BYTE SkipBounds() const; // 0x549020.
		virtual void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; // 0x4F6A50.
		void Update(float time); // 0x4F76B0 with absent optional frame inputs.
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); // 0x4F76B0.
		void Draw(); // 0x4F6BD0 with default viewport.
		virtual void Draw(Gfx_Viewport* viewport); // 0x4F6BD0.
		virtual BYTE IsFinished(float time) const; // 0x4F6C20.
		virtual void Stop(); // 0x4F6B90.
		virtual void GetTransform(D3DXMATRIX& output) const; // 0x4F6A20.
		virtual void SetEnabled(BYTE enabled); // 0x4F6BB0.
		virtual void SetTransform(const D3DXMATRIX& input); // 0x4F69E0.
		void QueueTriggeredEffect(LONG index, D3DXVECTOR3 position, float time); //4F8680; actual by-value position.
		void OnParticleDeath(float x, float y, float z, float time); //4F86F0; return unused by callback.
		VFX_ParticleSystem& CopyFrom(const VFX_ParticleSystem& input); //4F9850; rebuilds lower owner, borrows texture.
	private:
		Gfx_ParticleSystem* CreateParticleSystem(const VFX_ParticleData& input); //4F87E0; ECX owner, one data reference.
		Gfx_ParticleSystem* particles; // Original +C4, before its parameter value.
	public:
		VFX_ParticleData data; // Original +C8.
		Gfx_ParticleSystem* GetParticles() const { return particles; }
		BYTE ComputeInitialVelocity(float x, float y, float z,
			Gfx_ModelInstance* sourceModel, Gfx_ModelInstance* targetModel,
			VFX_VEGInstance* group, int useTarget, const D3DXMATRIX* sourceTransform,
			const D3DXMATRIX* targetTransform, float* vx, float* vy, float* vz) const; //4F84D0.
	private:
		BYTE GetInitialVelocityTransform(D3DXMATRIX& output, VFX_Target target,
			Gfx_ModelInstance* sourceModel, Gfx_ModelInstance* targetModel,
			VFX_VEGInstance* group, const D3DXMATRIX* sourceTransform,
			const D3DXMATRIX* targetTransform) const; //4F7390.
		BYTE GetInitialVelocityPosition(float* x, float* y, float* z, VFX_Target target,
			Gfx_ModelInstance* sourceModel, Gfx_ModelInstance* targetModel,
			VFX_VEGInstance* group, const D3DXMATRIX* sourceTransform,
			const D3DXMATRIX* targetTransform) const; //4F75D0.
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
		typedef char TriggeredEffectRecordSizeCheck[sizeof(TriggeredEffect) == 24 ? 1 : -1];
#endif
		Gfx_LineSystem lines; // Original +4BC.
		float lastTime; // Original +5D8.
		std::list<TriggeredEffect> spawnedEffects; // Original +5DC, owns child pointers.
		std::list<TriggeredEffect> visibleEffects; // Original +5E8, copied frame records borrow children.
		void ClearSpawnedEffects(); // 0x4F70A0.
	};
}

