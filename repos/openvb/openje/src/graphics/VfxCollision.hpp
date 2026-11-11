#pragma once
#include "VfxEffects.hpp"
#include "VfxValueOwners.hpp"
#include "LineSystem.hpp"
#include "GfxVector3.hpp"
class Gfx_ParticleSystem;
class Gfx_ParticleInfo;

namespace JE
{
	class VFX_ParticleSystem;
	class VFX_CollisionData : public VFX_BaseData
	{
	public:
		VFX_CollisionData(); // 0x4F0E90; dimensions constructs, other scalars stay unwritten.
		void Read(const VFX_ParameterHolder& parameters); //52FF00 case5,532537.
		LONG collisionShape,collisionType; // +70/+74.
		VFX_Vector dimensions; // +78; actual523B00 owner and owned vector input.
		float radius,power; // +84,+88.
		BYTE showShape; // +8C.
	};
	class VFX_Collision : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_Collision(); // 0x4F0F90.
		VFX_Collision& CopyFrom(const VFX_Collision& other); //0x4F1120.
		virtual ~VFX_Collision(); // 0x4F0EF0.
		virtual void SetData(const VFX_BaseData* input); // 0x4F1160.
		virtual void Reset(); // 0x4E1890.
		virtual BYTE GetFlag() const; //549020 constant0.
		virtual BYTE SkipBounds() const; //549020 constant0.
		virtual void GetBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const; //4EEA10.
		void Update(float time); // 0x4EE8B0 with absent optional frame inputs.
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); // 0x4EE8B0.
		BYTE IntersectsParticle(float x, float y, float z, float diameter) const; //4EEEB0; actual shape dispatch.
		BYTE ApplyParticle(Gfx_ParticleSystem& particles, Gfx_ParticleInfo& particle,
			float diameter, float time, VFX_VEGInstance& group, D3DXVECTOR3& outputPosition,
			BYTE faceNormal, BYTE trigger); //4EEFB0; ordinary8stackargument response.
		void ApplyParticles(VFX_ParticleSystem& particles, float time, VFX_VEGInstance& group); // 0x4F0DB0; per-particle response is separate.
		void Draw();
		VFX_CollisionData data;
		Gfx_Vector3f* previousPosition; //+154; actual5222A0 three-float argument.
		D3DXVECTOR3 velocity; //+158.
		float* previousTime; //+164.
		float* value168; //Owned raw allocation; populated producer/type unrecovered.
		Gfx_LineSystem debugShape; //+16C, actual existing owner.
	};
}
