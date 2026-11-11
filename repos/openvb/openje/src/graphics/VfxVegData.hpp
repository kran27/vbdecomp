#pragma once

#include "VfxParameters.hpp"

struct VFXEFFECTTYPE_BASE_Struct;

namespace JE
{
	typedef ::VFXEFFECTTYPE_BASE_Struct VFX_BaseData;

	class VFX_Effect : public VFX_ParameterHolder
	{
	public:
		VFX_Effect(); // 0x4F98C0.
		VFX_Effect(const VFX_Effect& input); // 0x4F9AF0.
		VFX_Effect& operator=(const VFX_Effect& input); //0x4F9AB0.
		virtual ~VFX_Effect(); // 0x4F98E0.
		virtual BYTE Read(const BYTE* buffer, DWORD& offset); //0x4F9960, slot4.
		virtual BYTE Write(BYTE* buffer, DWORD& offset) const; //0x4F99F0, slot8.
		VFX_BaseData* Prepare(); // 0x4F9A90.
		void Cleanup(); // 0x4F9940.
		VFX_BaseData* data; // Original +20.

	};

	class VFX_VEG : public VFX_ParameterHolder
	{
	public:
		struct Settings
		{
			BYTE loop; //5011E1/5011E8 write only this byte; floats align naturally.
			float minStartTime, maxStartTime;
		};

		VFX_VEG(); // 0x501EC0.
		virtual ~VFX_VEG();
		virtual BYTE Read(const BYTE* buffer, DWORD& offset); //0x501FA0, slot4.
		virtual BYTE Write(BYTE* buffer, DWORD& offset) const; //0x501340, slot8.
		void Prepare(); // 0x5011A0.
		void ConvertLegacy(); // 0x501430.
		void Cleanup(); // 0x501AD0, effects only.

		std::vector<VFX_Effect> effects; //Original owning values +20, element stride24hex.
		Settings* settings; // Original +30.
		float currentVersion, loadedVersion; // Original +34/+38.

	private:
		VFX_VEG(const VFX_VEG& input);
		VFX_VEG& operator=(const VFX_VEG& input);
	};

#if defined(_MSC_VER) && _MSC_VER == 1300
	typedef char VfxHolderOriginalSize[(sizeof(VFX_ParameterHolder) == 0x20) ? 1 : -1];
	typedef char VfxEffectOriginalSize[(sizeof(VFX_Effect) == 0x24) ? 1 : -1];
	typedef char VfxEffectOriginalData[(offsetof(VFX_Effect, data) == 0x20) ? 1 : -1];
	typedef char VfxVegOriginalSize[(sizeof(VFX_VEG) == 0x3C) ? 1 : -1];
	typedef char VfxVegOriginalEffects[(offsetof(VFX_VEG, effects) == 0x20) ? 1 : -1];
	typedef char VfxVegOriginalSettings[(offsetof(VFX_VEG, settings) == 0x30) ? 1 : -1];
	typedef char VfxVegOriginalCurrentVersion[(offsetof(VFX_VEG, currentVersion) == 0x34) ? 1 : -1];
	typedef char VfxVegOriginalLoadedVersion[(offsetof(VFX_VEG, loadedVersion) == 0x38) ? 1 : -1];
	typedef char VfxVegSettingsOriginalSize[(sizeof(VFX_VEG::Settings) == 12) ? 1 : -1];
	typedef char VfxVegSettingsOriginalMinimum[(offsetof(VFX_VEG::Settings, minStartTime) == 4) ? 1 : -1];
	typedef char VfxVegSettingsOriginalMaximum[(offsetof(VFX_VEG::Settings, maxStartTime) == 8) ? 1 : -1];
#endif
}
