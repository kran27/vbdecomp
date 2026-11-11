#pragma once

#include "VfxParticles.hpp"
#include "VfxValueOwners.hpp"

class Gfx_Water;
class Gfx_WaterTile;
class Gfx_Light;

namespace JE
{
	class VFX_WaterData : public VFX_BaseData
	{
	public:
		VFX_WaterData(); //4EE150; only constructed child values are initialized.
		void Read(const VFX_ParameterHolder& parameters); //52FF00, Water case532CC4.
		VFX_ResourceData texture; //+70, two real strings.
		VFX_TextureCoordinates textureUVs; //+A8, default(0,1,1,0).
		VFX_ResourceData lightMap; //+B8.
		VFX_TextureCoordinates lightMapUVs; //+F0.
		LONG width, height; //+100/+104.
		float gridSize; //+108.
		VFX_ColorBytes color; //+10C.
		LONG renderPhase; //+110.
		BYTE doubleSided, modulate2x, ignoreLighting; //+114..116.
		float updateInterval; //+118.
		BYTE alpha, adjustAlpha, adjustAlphaOverLifetime, finalAlpha; //+11C..11F.
		LONG alphaWaveform; //+120.
		float alphaFreq, alphaPhase; //+124/+128.
		LONG waterMotionType; //+12C.
		VFX_Vector2 minDropLocation, maxDropLocation; //+130/+138, actual560490 construction.
		float minDropAmount, maxDropAmount, minDropInterval, maxDropInterval; //+140..14C.
		VFX_Vector2 minFlowOrigin, maxFlowOrigin; //+150/+158.
		VFX_Vector2 minFlowDirection, maxFlowDirection; //+160/+168.
		float minFlowAmount, maxFlowAmount, minFlowInterval, maxFlowInterval; //+170..17C.
		float minSwirlRadius, maxSwirlRadius, minSwirlSpeed, maxSwirlSpeed; //+180..18C.
		float minSwirlAmount, maxSwirlAmount, minSwirlInterval, maxSwirlInterval; //+190..19C.
	};

	void __stdcall BuildVfxWater(Gfx_Water** water, Gfx_WaterTile** tile,
		const VFX_WaterData& data); //4EDD20; publishes each child before configuring either.

	class VFX_Water : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_Water(); //4EE390, original allocation628B.
		virtual ~VFX_Water(); //4EE300.
		void Load(const VFX_BaseData* input, float value); //4ED0E0, publishes value270 before SetData.
		VFX_Water& CopyFrom(const VFX_Water& other); //4EE6F0; rebuilds owners, borrows textures.
		virtual void SetData(const VFX_BaseData* input); //4EE7C0.
		virtual void Reset(); //4EDCE0, shared lower reset51CC60 retained separately.
		virtual BYTE GetFlag() const;
		virtual BYTE SkipBounds() const;
		virtual void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; //4ED260.
		void Update(float time);
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //4ED110.
		void Draw();
		virtual void Draw(Gfx_Viewport* viewport); //4ED1C0.
		virtual void SetTransform(const D3DXMATRIX& input); //4ED1E0.
		virtual void GetTransform(D3DXMATRIX& output) const; //4ED230.
		virtual BYTE AddLight(Gfx_Light* light); //4EDC80.
		virtual void RemoveLight(Gfx_Light* light); //4EDCA0.
		virtual void ClearLights(); //4EDCC0.
		virtual void FinishLights(); //4EDCD0.

		Gfx_Water* water; //+C4, owned simulation.
		Gfx_WaterTile* tile; //+C8, owned drawable tile.
		VFX_WaterData data; //+CC..26B, a member, not a second base.
		float* lastTime; //+26C, new float in Update, ordinary scalar delete in Reset/destruction.
		float value270; //+270, initialized1 and copied by4EE6F0.
	};
}
