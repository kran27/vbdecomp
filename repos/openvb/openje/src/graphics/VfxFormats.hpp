#pragma once

#include "VfxParameters.hpp"

namespace JE
{
	class VFX_ParameterInfo : public VFX_Parameter
	{
	public:
		VFX_ParameterInfo(); // 0x4FB2D0.
		~VFX_ParameterInfo(); //4FB370, existing virtual override.
		std::string category; // Original +34.
		std::string condition; // Original +50.
		VFX_Parameter minimum; //Original +6C.
		BYTE hasMinimum; //Original +A0.
		VFX_Parameter maximum; //Original +A4, ordinary alignment after the byte.
		BYTE hasMaximum; //Original +D8.
		VFX_Parameter increment; //Original +DC.
		BYTE hasIncrement; //Original +110.
	};

#if defined(_MSC_VER) && _MSC_VER == 1300
	typedef char VfxParameterInfoOriginalSize[(sizeof(VFX_ParameterInfo) == 0x114) ? 1 : -1];
	typedef char VfxParameterInfoOriginalCategory[(offsetof(VFX_ParameterInfo, category) == 0x34) ? 1 : -1];
	typedef char VfxParameterInfoOriginalCondition[(offsetof(VFX_ParameterInfo, condition) == 0x50) ? 1 : -1];
	typedef char VfxParameterInfoOriginalMinimum[(offsetof(VFX_ParameterInfo, minimum) == 0x6C) ? 1 : -1];
	typedef char VfxParameterInfoOriginalMinimumFlag[(offsetof(VFX_ParameterInfo, hasMinimum) == 0xA0) ? 1 : -1];
	typedef char VfxParameterInfoOriginalMaximum[(offsetof(VFX_ParameterInfo, maximum) == 0xA4) ? 1 : -1];
	typedef char VfxParameterInfoOriginalMaximumFlag[(offsetof(VFX_ParameterInfo, hasMaximum) == 0xD8) ? 1 : -1];
	typedef char VfxParameterInfoOriginalIncrement[(offsetof(VFX_ParameterInfo, increment) == 0xDC) ? 1 : -1];
	typedef char VfxParameterInfoOriginalIncrementFlag[(offsetof(VFX_ParameterInfo, hasIncrement) == 0x110) ? 1 : -1];
#endif

	class VFX_FormatManager
	{
	public:
		struct Format
		{
			LONG effectType;
			std::vector<VFX_ParameterInfo> parameters;
		};

		VFX_FormatManager(); // 0x4FD780, tables 0x6FCA18/0x6FC968.
		virtual ~VFX_FormatManager();
		const Format* Find(LONG effectType) const; // 0x4FB1E0.
		void Merge(VFX_ParameterHolder& holder, LONG effectType) const; // 0x4FB980.
		const Format& GetCommonFormat() const { return common; }
		const std::vector<VFX_Parameter>& GetGlobalDefaults() const { return globalDefaults; }

	private:
		void InitializeGlobalDefaults(); //0x4FC7B0.
		std::vector<Format> formats;
		Format common;
		std::vector<VFX_Parameter> globalDefaults; // 0x4FC7B0: Loop/start-time controls.
	};

	VFX_FormatManager& GetVfxFormatManager(); // Original static owner 0x70811C.
}
