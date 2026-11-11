#pragma once

#include "VfxEffects.hpp"
#include "VfxValueOwners.hpp"

namespace JE
{
	class VFX_TrailData : public VFX_BaseData
	{
	public:
		VFX_TrailData(); //4F16D0.
		void Read(const VFX_ParameterHolder& parameters); //53221B..532532, case4.
		VFX_ColorBytes color1; BYTE alpha1; //+70/+73.
		float widthScale1; //+74.
		VFX_ColorBytes color2; BYTE alpha2; //+78/+7B.
		float widthScale2; //+7C.
		VFX_Target endPointTarget; //+80.
		LONG endPointSocket; //+84.
		std::string endPointSnapBone; //+88.
		BYTE endPointTargetBodyPart; //+A4.
		VFX_Vector endPointOffset; //+A8.
		float segmentLifetime; //+B4.
		LONG alignment; //+B8, start0/center1/end2.
		VFX_ResourceData texture; //+BC/+D8.
		VFX_TextureCoordinates textureUVs; //+F4, default(0,1,1,0).
		LONG renderPhase; //+104.
		BYTE modulate2x; //+108.
	};

	struct VFX_TrailSample
	{
		D3DXVECTOR3 start, end;
		float birthTime, age;
		BYTE fresh;
	};
	struct VFX_TrailMesh;
	class VFX_Trail : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_Trail(); //4F2DD0.
		VFX_Trail& CopyFrom(const VFX_Trail& other); //0x4F2CB0.
		virtual ~VFX_Trail(); //4F2550.
		virtual void SetData(const VFX_BaseData* input); //4F2D50.
		virtual void Reset(); //4E1890, only active=0.
		virtual void SetSocket(DWORD socket); //4F19F0, existing slot60.
		virtual BYTE GetFlag() const; //549020.
		virtual BYTE SkipBounds() const; //549020.
		virtual void SetTransform(const D3DXMATRIX& input); //4F1180.
		virtual void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; //4F1470.
		virtual BYTE Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const; //4F1830.
		void Update(float time);
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //4F26E0.
		void Draw();
		virtual void Draw(Gfx_Viewport* viewport); //4F2BE0.
		const std::list<VFX_TrailSample>& GetSamples() const { return samples; }
		VFX_TrailData data;
	private:
		std::list<VFX_TrailSample> samples; //+1D0, newest first.
		BYTE dirty; //+1DC.
		DWORD texture; //+1E0.
		BYTE ownsTexture; //+1E4.
		float lastTime; //+1E8.
		Gfx_Viewport* lastViewport; //+1EC, constructor-unwritten until Draw.
		std::vector<VFX_TrailMesh*> meshes; //+1F0, exactly two owners.
		DWORD currentBucket; //+200.
		void GetSampleEndpoints(const VFX_TrailSample& sample, D3DXVECTOR3& start,
			D3DXVECTOR3& end) const; //4F11B0.
		void BuildGeometry(Gfx_Camera* camera); //4F1D80.
	};
}
