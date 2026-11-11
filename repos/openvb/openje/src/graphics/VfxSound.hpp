#pragma once
#include "VfxEffects.hpp"
namespace JE
{
	class VFX_SoundData : public VFX_BaseData
	{
	public:
		VFX_SoundData(); // 0x4E3000, four normal strings; flags remain unwritten.
		void Read(const VFX_ParameterHolder& parameters); //534712..534836.
		VFX_ResourceData preset,wav; //Native resource pairs +70/+A8.
		BYTE looping,fireAndForget; //+E0/+E1.
	};
	class VfxSoundInstance; // Actual52E2E0 owner, not SDK CSound layout.
	class VFX_Sound : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_Sound(); // 0x4E31D0.
		VFX_Sound& CopyFrom(const VFX_Sound& other); //0x4E34B0.
		virtual ~VFX_Sound(); // 0x4E3140.
		virtual void SetData(const VFX_BaseData* input); // 0x4E3530.
		virtual void Reset(); // 0x4E3290.
		virtual BYTE GetFlag() const;
		virtual BYTE SkipBounds() const;
		virtual void GetBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const; //4E0D70.
		void Update(float time);
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //4E2D10.
		virtual void OnStart(); //4E2E40, original slot50; Draw slot08 remains the base no-op.
		virtual void OnStop(); //4E2F00.
		virtual BYTE IsFinished(float time) const; //4E2F50, caches observed completion time.
		VFX_SoundData data;
		DWORD value1A8;
		VfxSoundInstance* sound; //+1AC, owning.
		BYTE paused,fired; //+1B0/+1B1.
		mutable float* finishedTime; //+1B4, actual allocated/free ownership.
	};
	namespace VfxServices
	{
		extern BYTE g_AudioEnabled; //Original6FAFEC initialized1;4B82D0 returns it.
		BYTE AudioEnabled();
	}
}
