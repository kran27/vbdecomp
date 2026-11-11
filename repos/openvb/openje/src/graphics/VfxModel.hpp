#pragma once

#include "VfxEffects.hpp"
#include "VfxModelData.hpp"

class G3D_Model;
class Gfx_AnimationObject;
class Gfx_AnimationInstance;

namespace JE
{
	class VFX_Model : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_Model(); //4F4650.
		VFX_Model& CopyFrom(const VFX_Model& other); //4F4000.
		virtual ~VFX_Model();
		virtual void SetData(const VFX_BaseData* input); //4F4960.
		virtual void Reset(); //Shared4E1890.
		virtual BYTE GetFlag() const; //Shared408D30, light-capable even without a model.
		virtual BYTE AddLight(Gfx_Light* light); //4F3720.
		virtual void RemoveLight(Gfx_Light* light); //4F3740.
		virtual void ClearLights(); //4F3760.
		virtual void FinishLights(); //4F3770.
		virtual BYTE SkipBounds() const; //549020.
		virtual void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; //4F3780.
		void Update(float time); //4F30C0, absent optional context.
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //4F30C0.
		void Draw();
		virtual void Draw(Gfx_Viewport* viewport); //4F36F0.
		virtual void SetTransform(const D3DXMATRIX& input); //4F3690.
		virtual void GetTransform(D3DXMATRIX& output) const; //4F36C0.
		virtual Gfx_ModelInstance* GetTargetModel() const; //4F43D0.
		VFX_ModelData data;
	private:
		void LoadModel(const std::string& name, float units); //4F3A00.
		BYTE LoadSkeleton(const std::string& name, float units); //4F47B0.
		void LoadAnimation(const std::string& name, float units); //4F43E0.
		float currentTime; //+254, zero in constructor.
		std::string modelName, skeletonName, animationName; //+258/+274/+290.
		LONG loopCount; //+2AC, zero.
		BYTE loadedShadow; //+2B0, constructor-unwritten.
		G3D_Model* modelSource; //+2B4.
		DWORD modelHandle; //+2B8.
		Gfx_ModelInstance* model; //+2BC, owned.
		Gfx_AnimationObject* animationObject; //+2C0, owned by flag2DA.
		Gfx_AnimationInstance* animation; //+2C4, owned.
		DWORD clockSnapshot[4]; //+2C8; actual sixteen-byte Granny clock snapshot.
		BYTE ownsSource, ownsHandle, ownsAnimationObject; //+2D8/+2D9/+2DA.
	};
}
