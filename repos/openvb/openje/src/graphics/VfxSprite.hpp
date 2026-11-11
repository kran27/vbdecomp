#pragma once

#include "VfxEffects.hpp"
#include "VfxValueOwners.hpp"

class Gfx_Camera;
namespace JE
{
	struct VFX_LightningMesh;
	class VFX_SpriteData : public VFX_BaseData
	{
	public:
		VFX_SpriteData(); //4F4FE0.
		void Read(const VFX_ParameterHolder& parameters); //52FF00 case2.
		BYTE rotatable; //+70.
		VFX_ResourceData texture; //+74/+90.
		VFX_TextureCoordinates textureUVs; //+AC, default(0,1,1,0).
		float width, height; //+BC/+C0.
		LONG renderPhase; //+C4.
		BYTE modulate2x, disableZBuffer; //+C8/+C9.
		VFX_ColorBytes color; BYTE adjustColor, adjustColorOverLifetime; VFX_ColorBytes finalColor; //+CA..D1.
		LONG colorWaveform; //+D4.
		float colorFreq, colorPhase; //+D8/+DC.
		BYTE alpha, adjustAlpha, adjustAlphaOverLifetime, finalAlpha; //+E0..E3.
		LONG alphaWaveform; //+E4.
		float alphaFreq, alphaPhase; //+E8/+EC.
		BYTE adjustScale, adjustScaleOverLifetime; //+F0/+F1.
		float initialScaleX, initialScaleY, finalScaleX, finalScaleY; //+F4..100.
		LONG scaleWaveform; //+104.
		float scaleFreq, scalePhase; //+108/+10C.
	};

	class VFX_Sprite : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_Sprite(); //4F6490.
		VFX_Sprite& CopyFrom(const VFX_Sprite& other); //0x4F6390.
		virtual ~VFX_Sprite();
		virtual void SetData(const VFX_BaseData* input); //4F6410.
		virtual void Reset(); //4F4FC0.
		virtual BYTE GetFlag() const; //4080B0.
		virtual BYTE SkipBounds() const; //549020.
		virtual void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; //4F4CA0.
		void Update(float time); //4F4C50, absentoptionalcontext.
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //4F4C50.
		void Draw(); //4F6190, defaultviewport.
		virtual void Draw(Gfx_Viewport* viewport); //4F6190.
		VFX_SpriteData data;
		DWORD GetTexture() const { return texture; }
	private:
		BYTE dirty;
		D3DXMATRIX lastCameraTransform; //+1D8, unwrittenuntilfirstDraw.
		float lastFieldOfView;
		DWORD texture;
		BYTE ownsTexture;
		float time;
		Gfx_Viewport* lastViewport; //+228, originalconstructorleavesunwritten.
		std::vector<VFX_LightningMesh*> meshes; //+22C, exactlytwoownedrecords.
		DWORD currentBucket;
		BYTE occlusionVisible;
		float occlusionChangedTime;
		void BuildGeometry(Gfx_Camera* camera); //4F5400.
		BYTE TestOcclusion(Gfx_Camera* camera) const; //4F4EB0.
		void GetScale(float& x, float& y) const;
	};
}
