#pragma once

#include "VfxVegData.hpp"
#include "VfxEffects.hpp"
#include <map>
#include "configuration/ConfigurationTypes.hpp"

namespace JE
{
	class VFX_VEGInstance
	{
	public:
		VFX_VEGInstance(); // 0x48ED60.
		virtual ~VFX_VEGInstance();
		void Create(VFX_VEG* input, float value, BYTE clone); // 0x48EFB0.
		VFX_EffectInstance* CreateEffect(VFX_Effect* definition); //48CDA0; group receiver and live scale.
		LONG Refresh(LONG index); // 0x48EE30.
		void Reset(); // 0x48DBE0.
		void Stop(); // 0x48DCC0.
		void SetEnabled(BYTE enabled); //48DC60, original live-end child dispatch.
		void SetSocket(DWORD socket); //48DC90, actual virtual child dispatch.
		float GetDuration(BYTE includeInfinite) const; // 0x48D9B0.
		BYTE IsFinished() const; // 0x48DC10.
		VFX_EffectInstance* GetEffect(LONG index) const; // 0x48DDC0.
		void Update(float time, Gfx_ModelInstance* source = NULL, Gfx_ModelInstance* target = NULL,
			const std::list<Gfx_ModelInstance*>* extraSources = NULL,
			const D3DXMATRIX* sourceTransform = NULL, const D3DXMATRIX* targetTransform = NULL); // 0x48F080.
		void Draw(Gfx_Viewport* viewport); // 0x48D850.
		BYTE AddLight(Gfx_Light* light); //48D8A0, raw BYTE OR across all children.
		void RemoveLight(Gfx_Light* light); //48D8E0, shared with59F810.
		void ClearLights(); //48D910, shared with59F830.
		void FinishLights(); //48D940, shared with59F840.
		BYTE GetFlag() const; // 0x48D970.
		void GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const; // 0x48DA40.
		void Cleanup(); // 0x48E5F0.
		void ClearEffects(); // 0x48DF00.
		VFX_VEG* data;
		BYTE ownsData;
		float scale;
		std::string name;
		float startTime;
		std::vector<VFX_EffectInstance*> effects;
	private:
		std::list<VFX_EffectInstance*> values40; // Original+40, borrowed frame draw order.
		float* value4C; // Original+4C, owned previous group time.
		VFX_VEGInstance(const VFX_VEGInstance&);
		VFX_VEGInstance& operator=(const VFX_VEGInstance&);
	};

	class VFX_VEGCache
	{
	public:
		struct Entry { VFX_VEG* data; DWORD references; };
		typedef std::map<std::string,Entry,Configuration::CaseInsensitiveLess> Entries;
		VFX_VEGCache(); // 0x4E2A70.
		virtual ~VFX_VEGCache(); // 0x4E2920.
		VFX_VEGInstance* Acquire(const char* name, float scale); // 0x4E2AD0.
		void Release(VFX_VEGInstance* instance); // 0x4E20F0, retains cached data at0 refs.
		const Entries& GetEntries() const { return entries; }
	private:
		Entries entries;
		VFX_VEGCache(const VFX_VEGCache&);
		VFX_VEGCache& operator=(const VFX_VEGCache&);
	};
	VFX_VEGCache& GetVfxVegCache(); // 0x4E1DE0, singleton7080A0.
}
