#pragma once

#include "VfxEffects.hpp"
#include "VfxValueOwners.hpp"

namespace JE
{
	class VFX_RainData : public VFX_BaseData
	{
	public:
		VFX_RainData(); //4E5F20.
		void Read(const VFX_ParameterHolder& parameters); //533D92, case10.
		float dropsPerSecond;
		LONG maxDrops;
		VFX_Vector minVelocity, maxVelocity; float minLength, maxLength;
		VFX_ColorBytes color; BYTE minBeginAlpha, maxBeginAlpha, minEndAlpha, maxEndAlpha;
		LONG renderPhase;
		BYTE modulate2x;
		float emitterSize, emitterYOffset, groundYOffset; VFX_Vector offsetFromCamera;
		BYTE affectedByWind;
		float windScale;
	};
	class VFX_SnowData : public VFX_BaseData
	{
	public:
		VFX_SnowData(); //4E3FE0.
		void Read(const VFX_ParameterHolder& parameters); //534044, case11.
		VFX_ResourceData texture;
		VFX_TextureCoordinates textureUVs; float flakesPerSecond;
		LONG maxFlakes;
		VFX_Vector minVelocity, maxVelocity; float minSize, maxSize;
		VFX_ColorBytes color; BYTE minAlpha, maxAlpha;
		LONG renderPhase;
		BYTE modulate2x;
		float emitterSize, emitterYOffset, groundYOffset; VFX_Vector offsetFromCamera;
		float minXSinAmplitude, maxXSinAmplitude, minXSinFrequency, maxXSinFrequency;
		float minXSinPhase, maxXSinPhase, minZSinAmplitude, maxZSinAmplitude;
		float minZSinFrequency, maxZSinFrequency, minZSinPhase, maxZSinPhase;
		BYTE affectedByWind;
		float windScale;
	};
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char RainDataNativeSize[(sizeof(VFX_RainData) == 0xC8) ? 1 : -1];
	typedef char RainDataNativeVectorOffsets[(offsetof(VFX_RainData, minVelocity) == 0x78
		&& offsetof(VFX_RainData, maxVelocity) == 0x84
		&& offsetof(VFX_RainData, offsetFromCamera) == 0xB4) ? 1 : -1];
	typedef char SnowDataNativeSize[(sizeof(VFX_SnowData) == 0x140) ? 1 : -1];
	typedef char SnowDataNativeVectorOffsets[(offsetof(VFX_SnowData, minVelocity) == 0xC0
		&& offsetof(VFX_SnowData, maxVelocity) == 0xCC
		&& offsetof(VFX_SnowData, offsetFromCamera) == 0xFC) ? 1 : -1];
#endif
	class VFX_WindData : public VFX_BaseData
	{
	public:
		VFX_WindData(); //4E36E0.
		void Read(const VFX_ParameterHolder& parameters); //5344DF, case12.
		LONG windType;
		VFX_Vector direction; //Native+74, actual523B00 value owner.
		float velocity;
		BYTE adjustVelocity, adjustVelocityOverLifetime;
		float finalVelocity;
		LONG velocityWaveform;
		float velocityFreq, velocityPhase, minGustVelocity, maxGustVelocity;
		float minGustDuration, maxGustDuration, minGustAttack, maxGustAttack;
		float minGustDecay, maxGustDecay, minGustDelay, maxGustDelay;
	};
	struct VFX_WeatherMesh;
	struct VFX_RainDrop
	{
		D3DXVECTOR3 position, origin, velocity, wind;
		float born, value34, length, beginAlpha, endAlpha; //value34 remains original-unwritten.
	};
	struct VFX_SnowFlake
	{
		D3DXVECTOR3 position, origin, velocity;
		float born, value28, size, alpha; //value28 remains original-unwritten.
		float xAmplitude, xFrequency, xPhase, zAmplitude, zFrequency, zPhase;
	};
	class VFX_Rain : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_Rain(); //4E5F90.
		VFX_Rain& CopyFrom(const VFX_Rain& other); //0x4E6460.
		virtual ~VFX_Rain(); //4E61E0.
		virtual void SetData(const VFX_BaseData* input); //4E6A40.
		virtual void Reset(); //4E1890.
		virtual BYTE GetFlag() const;
		virtual BYTE SkipBounds() const;
		virtual BYTE Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const; //4E5EC0, always1.
		virtual void GetBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const; //4E3F10.
		void Update(float time);
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //4E5E60.
		void Draw();
		virtual void Draw(Gfx_Viewport* viewport); //4E7330.
		size_t GetDropCount() const { return drops.size(); }
	private:
		VFX_RainData data;
		float time, delta; //+18C/+190.
		BYTE simulationDirty; //+194.
		D3DXMATRIX lastCamera; //Original constructor-unwritten.
		float emissionRemainder;
		std::list<VFX_RainDrop> drops; //+1DC.
		VFX_WeatherMesh* mesh;
		BYTE geometryDirty, emitting;
		void Simulate(const D3DXVECTOR3& cameraDelta); //4E64A0.
		void BuildGeometry(Gfx_Viewport* viewport); //4E6B20.
	};
	class VFX_Snow : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_Snow(); //4E4100.
		VFX_Snow& CopyFrom(const VFX_Snow& other); //0x4E4680.
		virtual ~VFX_Snow(); //4E4310.
		virtual void SetData(const VFX_BaseData* input); //4E4DD0.
		virtual void Reset();
		virtual BYTE GetFlag() const;
		virtual BYTE SkipBounds() const;
		virtual BYTE Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const;
		virtual void GetBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const;
		void Update(float time);
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //4E3EB0.
		void Draw();
		virtual void Draw(Gfx_Viewport* viewport); //4E5D20.
		size_t GetFlakeCount() const { return flakes.size(); }
	private:
		VFX_SnowData data;
		float time, delta; //+204/+208.
		BYTE simulationDirty; //+20C.
		D3DXMATRIX lastCamera; //Original constructor-unwritten.
		float lastFieldOfView;
		DWORD texture;
		BYTE ownsTexture;
		float emissionRemainder;
		std::list<VFX_SnowFlake> flakes; //+260.
		VFX_WeatherMesh* mesh;
		BYTE geometryDirty, emitting;
		void Simulate(const D3DXVECTOR3& cameraDelta); //4E4700.
		void BuildGeometry(Gfx_Viewport* viewport); //4E4F00.
	};
	class VFX_Wind : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_Wind(); //4E37D0.
		VFX_Wind& CopyFrom(const VFX_Wind& other); //0x4E3E10.
		virtual ~VFX_Wind();
		virtual void SetData(const VFX_BaseData* input); //4E3E60.
		virtual void Reset(); //4E3790.
		virtual BYTE GetFlag() const;
		virtual BYTE SkipBounds() const;
		virtual BYTE Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const;
		virtual void GetBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const;
		void Update(float time);
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //4E3980.
		void Draw(); //5A9320, actual empty draw.
	private:
		struct Gust { float velocity,duration,attack,decay,born; };
		VFX_WindData data;
		BYTE stopped;
		std::list<Gust> gusts;
		float gustStart, gustDelay;
	};
}
