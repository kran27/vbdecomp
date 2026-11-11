#pragma once

#include "VfxEffectData.hpp"
#include <d3dx8.h>
#include <list>
#include <vector>
#include <stddef.h>

class Gfx_ModelInstance;
class Gfx_Viewport;
class Gfx_Camera;
class Gfx_Light;

namespace JE
{
	class VFX_VEGInstance;
	class VFX_Effect;
	void GetVfxModelTransform(Gfx_ModelInstance* model, LONG socket, std::string snapBone,
		D3DXMATRIX& output, const float* offset, BYTE rotate, float scale); //4E0F70; offset may beNULL.
	struct VFX_UpdateContext
	{
		VFX_UpdateContext() : source(NULL), target(NULL), group(NULL), sourceTransform(NULL), targetTransform(NULL) {}
		VFX_UpdateContext(Gfx_ModelInstance* sourceValue, Gfx_ModelInstance* targetValue,
			VFX_VEGInstance* groupValue, const D3DXMATRIX* sourceMatrix, const D3DXMATRIX* targetMatrix)
			: source(sourceValue), target(targetValue), group(groupValue),
			sourceTransform(sourceMatrix), targetTransform(targetMatrix) {}
		Gfx_ModelInstance* source;
		Gfx_ModelInstance* target;
		VFX_VEGInstance* group;
		const D3DXMATRIX* sourceTransform;
		const D3DXMATRIX* targetTransform;
	};
	Gfx_ModelInstance* __cdecl ResolveVfxModel(VFX_Target selectedTarget, Gfx_ModelInstance* source,
		Gfx_ModelInstance* target, VFX_VEGInstance* group); //4E0B20, four physical words.
	Gfx_ModelInstance* ResolveVfxModel(DWORD target, const VFX_UpdateContext& context); //Compatibility context adapter.
	void GetVfxTransform(const D3DXMATRIX* source, D3DXMATRIX& output,
		const float* offset, BYTE rotate, float scale); //4E0B90.

	class VFX_EffectInstance
	{
	public:
		VFX_EffectInstance(); // 0x4E17C0.
		virtual ~VFX_EffectInstance();
		void Load(const VFX_BaseData& input, float value); // 0x4E7450.
		void LoadBaseData(const VFX_BaseData& input, float value); //4E1D50, no virtual SetData.
		VFX_EffectInstance& CopyFrom(const VFX_EffectInstance& other); //0x4E1D20.
		//Original25 slots. Nonvirtual conveniences below never add vtable entries.
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target,
			VFX_VEGInstance* group, const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //04,4E19B0.
		virtual void Draw(Gfx_Viewport* viewport); //08, shared5A9320.
		virtual BYTE AddLight(Gfx_Light* light); //0C, shared591690.
		virtual void RemoveLight(Gfx_Light* light); //10, shared5A9320.
		virtual void ClearLights(); //14, shared404B70.
		virtual void FinishLights(); //18, shared404B70.
		virtual BYTE GetFlag() const; //1C, shared549020.
		virtual void SetTransform(const D3DXMATRIX& input); //20,4E1850.
		virtual void GetTransform(D3DXMATRIX& output) const; //24,4E1870.
		virtual void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; //28,4E0D70.
		virtual void SetData(const VFX_BaseData* input); //2C,4E1D00: NULL is a no-op here only.
		virtual Gfx_ModelInstance* GetTargetModel() const; //30, shared4080B0.
		virtual void Reset(); //34,4E1890, active0 only.
		virtual BYTE Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target,
			VFX_VEGInstance* group, const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const; //38,4E1930.
		virtual void Stop(); //3C,4E18A0.
		virtual BYTE IsFinished(float time) const; //40,4E0DC0.
		virtual BYTE AppliesToExtraSources() const; //44, shared549020.
		virtual BYTE SkipBounds() const; //48, shared549020.
		virtual BYTE IsVisible() const; //4C,4E0E20.
		virtual void OnStart(); //50, shared404B70.
		virtual void OnStop(); //54, shared404B70.
		virtual void SetEnabled(BYTE enabled); //58,4E18B0, rawBYTE.
		virtual BYTE GetEnabled() const; //5C,4E18C0, rawBYTE.
		virtual void SetSocket(DWORD socket); //60,4E1B80.
		void SetData(const VFX_BaseData& input) { SetData(&input); }
		void Update(float time) { Update(time, NULL, NULL, NULL, NULL, NULL); }
		void Update(float time, const VFX_UpdateContext& context)
		{ Update(time, context.source, context.target, context.group, context.sourceTransform, context.targetTransform); }
		void Draw() { Draw(NULL); }
		BYTE Prepare(const VFX_UpdateContext& context) const
		{ return Prepare(context.source, context.target, context.group, context.sourceTransform, context.targetTransform); }
		void UpdateTransform(const VFX_UpdateContext& context); //Qualified base Update, never derived redispatch.
		BYTE PrepareTarget(Gfx_ModelInstance* source, Gfx_ModelInstance* target,
			VFX_VEGInstance* group, VFX_Target selectedTarget, LONG socket, std::string snapBone,
			const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const; //4E1580, nonvirtual.
		VFX_BaseData baseData;
		float scale;
		DWORD effectIndex; // Original+78; published by VEG Update48F080, not an entity ID.
		BYTE active;
		D3DXMATRIX transform;
		BYTE flagC0, flagC1;
	};

#if defined(_MSC_VER) && _MSC_VER == 1300
	typedef char VfxInstanceOriginalSize[(sizeof(VFX_EffectInstance) == 0xC4) ? 1 : -1];
	typedef char VfxInstanceOriginalData[(offsetof(VFX_EffectInstance, baseData) == 4) ? 1 : -1];
	typedef char VfxInstanceOriginalScale[(offsetof(VFX_EffectInstance, scale) == 0x74) ? 1 : -1];
	typedef char VfxInstanceOriginalIndex[(offsetof(VFX_EffectInstance, effectIndex) == 0x78) ? 1 : -1];
	typedef char VfxInstanceOriginalActive[(offsetof(VFX_EffectInstance, active) == 0x7C) ? 1 : -1];
	typedef char VfxInstanceOriginalTransform[(offsetof(VFX_EffectInstance, transform) == 0x80) ? 1 : -1];
	typedef char VfxInstanceOriginalStopped[(offsetof(VFX_EffectInstance, flagC0) == 0xC0) ? 1 : -1];
	typedef char VfxInstanceOriginalEnabled[(offsetof(VFX_EffectInstance, flagC1) == 0xC1) ? 1 : -1];
#endif

	struct VFX_LightningPoint
	{
		D3DXVECTOR3 position;
		float width;
	};
	struct VFX_LightningSegment
	{
		std::list<VFX_LightningPoint> points;
		LONG level;
	};
	struct VFX_LightningMesh;
	class VFX_Lightning : public VFX_EffectInstance
	{
	public:
		using VFX_EffectInstance::SetData;
		using VFX_EffectInstance::Update;
		using VFX_EffectInstance::Draw;
		using VFX_EffectInstance::Prepare;
		VFX_Lightning(); // 0x4ECFB0; geometry is created by Update, not construction.
		virtual ~VFX_Lightning();
		virtual void SetData(const VFX_BaseData* input); // 0x4EA2E0.
		virtual void Reset(); // 0x4E8F30.
		virtual void SetSocket(DWORD socket); //4E9530, existing slot60.
		virtual BYTE GetFlag() const; // Actual549020 policy0.
		virtual BYTE SkipBounds() const; // Actual549020 policy0.
		virtual void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; // 0x4E8F70.
		void Update(float time); //4EC990, absent optional context.
		virtual void Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform); //4EC990.
		void Draw(); //4ECEB0, default viewport entry.
		virtual void Draw(Gfx_Viewport* viewport); //4ECEB0.
		virtual BYTE Prepare(Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform) const; // 0x4E9170, checks both endpoints.
		virtual void SetTransform(const D3DXMATRIX& input); // 0x4E8E60.
		VFX_LightningData data;
		const std::list<VFX_LightningSegment>& GetSegments() const { return segments; }
	private:
		std::list<VFX_LightningSegment> segments; //Original+22C, owns point values.
		BYTE dirty; //Original+238.
		float lastTime, cycleStart, currentLifetime, currentRepeatDelay; //+23C..248.
		BYTE fresh; //+24C.
		LONG repeats; //+250.
		Gfx_Viewport* lastViewport; //Original+254, constructor-unwritten until Draw.
		std::vector<std::list<VFX_LightningMesh*> > meshes; //Original+258, owns GPU meshes.
		DWORD value268;
		float ComputeWidth(LONG index, LONG count) const; //4E8E90.
		void GenerateSegments(const D3DXVECTOR3& start, const D3DXVECTOR3& end,
			LONG level, LONG* branchCount); //4EBBB0, recursive append-copy.
		void BuildGeometry(Gfx_Camera* camera); //4EA6F0.
	};

	VFX_EffectInstance* NewVfxEffectInstance(const VFX_BaseData* data, float scale); // Compatibility factory; native48CDA0 is VFX_VEGInstance::CreateEffect.
	VFX_EffectInstance* __stdcall CloneVfxEffectInstance(VFX_Effect* definition,
		const VFX_EffectInstance* prototype); //0x48D2C0; Lightning7 is unsupported.
}
