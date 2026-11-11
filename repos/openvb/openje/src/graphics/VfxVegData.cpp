#include "VfxVegData.hpp"
#include "VfxEffectData.hpp"
#include <string.h>
#include <stdlib.h>

namespace
{
	template <class T> T ReadValue(const BYTE* buffer, DWORD& offset)
	{
		T result;
		memcpy(&result, buffer + offset, sizeof(result));
		offset += sizeof(result);
		return result;
	}

	bool ScaleLegacyVector(JE::VFX_Parameter& parameter, bool compareStoredFirst)
	{
		const JE::VFX_Vector before = parameter.GetVector3();
		double first = double(before.xy.values[0]) * 0.1f;
		// Original x87 keeps the first product for some comparisons, but
		// compares stored floats for the other components and 501150.
		volatile float stored[3];
		stored[0] = float(first);
		stored[1] = float(double(before.xy.values[1]) * 0.1f);
		stored[2] = float(double(before.z) * 0.1f);
		JE::VFX_Vector after(stored[0], stored[1], stored[2]);
		if (compareStoredFirst ? before != after
			: (first != before.xy.values[0] || after.xy.values[1] != before.xy.values[1] || after.z != before.z))
		{
			parameter.SetVector3(after);
			return true;
		}
		return false;
	}

	bool ScaleLegacyFloat(JE::VFX_Parameter& parameter, float factor)
	{
		double before = parameter.GetFloat();
		volatile float after = float(before * factor);
		if (before == after) return false;
		parameter.SetFloat(after);
		return true;
	}
}

namespace JE
{
	VFX_Effect::~VFX_Effect()
	{
		Cleanup();
	}

	void VFX_Effect::Cleanup()
	{
		if (data != NULL)
		{
			delete data;
			data = NULL;
		}
	}

	BYTE VFX_Effect::Read(const BYTE* buffer, DWORD& offset)
	{
		DWORD tag;
		ReadVfxBuffer(&tag, 4, 1, buffer, offset);
		if (tag != 0x20584656) return 0; // "VFX ".
		DWORD version;
		ReadVfxBuffer(&version, 4, 1, buffer, offset);
		if (version != 0x302E3156) return 0; // "V1.0".
		BYTE reserved[8];
		ReadVfxBuffer(reserved, 8, 1, buffer, offset);
		return VFX_ParameterHolder::Read(buffer, offset);
	}

	BYTE VFX_Effect::Write(BYTE* buffer, DWORD& offset) const
	{
		BYTE reserved[8];
		memset(reserved, 0, sizeof(reserved));
		WriteVfxBuffer("VFX ", 4, 1, buffer, offset);
		WriteVfxBuffer("V1.0", 4, 1, buffer, offset);
		WriteVfxBuffer(reserved, 8, 1, buffer, offset);
		return VFX_ParameterHolder::Write(buffer, offset);
	}

	VFX_VEG::VFX_VEG()
		: settings(NULL), currentVersion(0), loadedVersion(0)
	{
	}

	VFX_VEG::~VFX_VEG()
	{
		//501A60: raw settings free, then natural value-vector and holder destruction.
		if (settings != NULL) free(settings);
	}

	BYTE VFX_VEG::Read(const BYTE* buffer, DWORD& offset)
	{
		Cleanup();
		LONG count = 0;
		DWORD tag;
		ReadVfxBuffer(&tag, 4, 1, buffer, offset);
		if (tag != 0x20474556) return 0; // "VEG ".
		DWORD version;
		ReadVfxBuffer(&version, 4, 1, buffer, offset);
		if (version == 0x302E3156)
		{
			loadedVersion = 1.0f;
		}
		else if (version == 0x312E3156)
		{
			loadedVersion = 1.1f;
		}
		else
		{
			loadedVersion = 0;
			currentVersion = 0;
			return 0;
		}
		currentVersion = 1.1f;
		ReadVfxBuffer(&count, 4, 1, buffer, offset);
		BYTE reserved[12];
		ReadVfxBuffer(reserved, 12, 1, buffer, offset);
		for (LONG index = 0; index < count; ++index)
		{
			VFX_Effect effect;
			if (!effect.Read(buffer, offset)) return 0;
			effects.push_back(effect);
		}
		const BYTE result = VFX_ParameterHolder::Read(buffer, offset);
		ConvertLegacy(); // Called even when the final parameter read fails.
		return result;
	}

	BYTE VFX_VEG::Write(BYTE* buffer, DWORD& offset) const
	{
		BYTE reserved[12];
		memset(reserved, 0, sizeof(reserved));
		WriteVfxBuffer("VEG ", 4, 1, buffer, offset);
		WriteVfxBuffer("V1.1", 4, 1, buffer, offset);
		LONG count = static_cast<LONG>(effects.size());
		WriteVfxBuffer(&count, 4, 1, buffer, offset);
		WriteVfxBuffer(reserved, 12, 1, buffer, offset);
		for (LONG index = 0; index < count; ++index)
		{
			//The count is captured, but the vector's storage is reloaded for
			//each virtual child write, as in5013E7. Do not snapshot its entries.
			if (!effects[index].Write(buffer, offset)) return 0;
		}
		return VFX_ParameterHolder::Write(buffer, offset);
	}

	void VFX_VEG::Prepare()
	{
		if (settings != NULL)
		{
			free(settings);
			settings = NULL;
		}
		settings = new Settings;
		VFX_Parameter* parameter = Find("Loop");
		if (parameter != NULL) settings->loop = parameter->GetBool();
		else settings->loop = 0;
		parameter = Find("MinStartTime");
		if (parameter != NULL) settings->minStartTime = static_cast<float>(parameter->GetFloat());
		else settings->minStartTime = 0;
		parameter = Find("MaxStartTime");
		if (parameter != NULL) settings->maxStartTime = static_cast<float>(parameter->GetFloat());
		else settings->maxStartTime = 0;
	}

	void VFX_VEG::ConvertLegacy()
	{
		// 501430 compares the stored float version against a double constant.
		if (loadedVersion < 1.1)
		{
			BYTE anyChanged = 0;
			for (DWORD effectIndex = 0; effectIndex < effects.size(); ++effectIndex)
			{
				VFX_Effect* effect = &effects[effectIndex];
				BYTE changed = 0;
				if (!effect->data) effect->Prepare();
				LONG count = LONG(effect->GetParameterOrder().size());
				for (LONG index = 0; index < count; ++index)
				{
					VFX_Parameter& parameter = *effect->GetParameter(index);
					if (!_stricmp(parameter.name.c_str(), "Offset")
						&& parameter.type == VFX_Parameter::Vector
						&& ScaleLegacyVector(parameter, false)) changed = 1;
					// Reload compiled type after each parameter's generic Offset conversion.
					switch (effect->data->effectType)
					{
					case 4:
						if (!_stricmp(parameter.name.c_str(), "EndPointOffset")
							&& parameter.type == VFX_Parameter::Vector
							&& ScaleLegacyVector(parameter, false)) changed = 1;
						break;
					case 7:
						if (!_stricmp(parameter.name.c_str(), "EndPointOffset")
							&& parameter.type == VFX_Parameter::Vector)
						{
							if (ScaleLegacyVector(parameter, true)) changed = 1;
						}
						else if ((!_stricmp(parameter.name.c_str(), "MinWidth")
							|| !_stricmp(parameter.name.c_str(), "MaxWidth"))
							&& parameter.type == VFX_Parameter::Float)
						{
							if (ScaleLegacyFloat(parameter, 0.1f)) changed = 1;
						}
						else if ((!_stricmp(parameter.name.c_str(), "MinSegmentsPerFoot")
							|| !_stricmp(parameter.name.c_str(), "MaxSegmentsPerFoot"))
							&& parameter.type == VFX_Parameter::Float)
						{
							if (ScaleLegacyFloat(parameter, 10.0f)) changed = 1;
						}
						break;
					case 0:
						// Both original name comparisons use the same RibbonOrigin literal.
						if ((!_stricmp(parameter.name.c_str(), "RibbonOrigin")
							|| !_stricmp(parameter.name.c_str(), "RibbonOrigin"))
							&& parameter.type == VFX_Parameter::Vector
							&& ScaleLegacyVector(parameter, false)) changed = 1;
						break;
					}
				}
				if (changed)
				{
					effect->Prepare();
					anyChanged = 1;
				}
			}
			// A changed conversion deliberately leaves the old version published.
			if (!anyChanged) loadedVersion = currentVersion;
		}
	}
}
