#include "OptionalAudio.hpp"
#include "SoundSystem.hpp"

namespace JE
{
	CDAudio::CDAudio() //53A510.
	{
		deviceID = 0;
		mode = MCI_MODE_NOT_READY;
		trackCount = 0;
		seconds = 0;
		currentPosition = 1;
		baseVolume = 50;
		effectiveVolume = Volume();
		lastVolume = 50;
		mixer = NULL;
		previousClock = 0;
		//The mixer query supplies maximum, minimum and volumeControl later.
	}

	LONG CDAudio::Volume() const //5C0190.
	{
		return baseVolume;
	}

	DWORD CDAudio::ReadMixerVolume(LONG& left, LONG& right) //53A220.
	{
		left = 0;
		right = 0;
		MIXERCONTROLDETAILS_UNSIGNED values[2];
		MIXERCONTROLDETAILS details;
		details.dwControlID = volumeControl;
		details.cbStruct = sizeof(details);
		details.cChannels = 2;
		details.cMultipleItems = 0;
		details.cbDetails = sizeof(values[0]);
		details.paDetails = values;
		const MMRESULT result = mixerGetControlDetailsA(
			reinterpret_cast<HMIXEROBJ>(mixer), &details, MIXER_OBJECTF_HMIXER);
		if (result) return result;

		//Both products wrap as DWORDs before unsigned floating conversion.
		//The original divides in x87, adds0.5, then truncates through __ftol2.
		const DWORD leftMinimum = minimum;
		const DWORD leftNumerator = 100u * (values[0].dwValue - leftMinimum);
		left = LONG(static_cast<__int64>(float(leftNumerator) /
			float(maximum - leftMinimum) + 0.5));
		const DWORD rightMinimum = minimum; //Reload after the first output store.
		const DWORD rightNumerator = 100u * (values[1].dwValue - rightMinimum);
		const DWORD rightValue = DWORD(static_cast<__int64>(float(rightNumerator) /
			float(maximum - rightMinimum) + 0.5));
		right = LONG(rightValue);
		return rightValue;
	}

	void CDAudio::SetMixerVolume(LONG left, LONG right) //53A320.
	{
		const HMIXER capturedMixer = mixer;
		if (!capturedMixer) return;
		if (left < 0) left = 0;
		else if (left > 100) left = 100;
		if (right < 0) right = 0;
		else if (right > 100) right = 100;
		const DWORD range = maximum - minimum;
		MIXERCONTROLDETAILS_UNSIGNED values[2];
		values[0].dwValue = (DWORD(left) * range) / 100u;
		values[1].dwValue = (DWORD(right) * range) / 100u;
		MIXERCONTROLDETAILS details;
		details.cbStruct = sizeof(details);
		details.dwControlID = volumeControl;
		details.cChannels = 2;
		details.cMultipleItems = 0;
		details.cbDetails = sizeof(values[0]);
		details.paDetails = values;
		mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(capturedMixer),
			&details, MIXER_OBJECTF_HMIXER);
	}

	int CDAudio::OpenMixer(DWORD_PTR callback) //53A3E0.
	{
		if (mixerOpen(&mixer, 0, callback, 0, CALLBACK_WINDOW))
		{
			mixer = NULL;
			return 11;
		}
		MIXERLINEA line;
		line.cbStruct = sizeof(line);
		line.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;
		if (mixerGetLineInfoA(reinterpret_cast<HMIXEROBJ>(mixer), &line,
			MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE))
		{
			mixerClose(mixer);
			mixer = NULL;
			return 11;
		}
		MIXERCONTROLA control;
		MIXERLINECONTROLSA controls;
		controls.dwLineID = line.dwLineID;
		controls.cbStruct = sizeof(controls);
		controls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
		controls.cControls = 1;
		controls.cbmxctrl = sizeof(control);
		controls.pamxctrl = &control;
		if (mixerGetLineControlsA(reinterpret_cast<HMIXEROBJ>(mixer), &controls,
			MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE))
		{
			mixerClose(mixer);
			mixer = NULL;
			return 11;
		}
		minimum = control.Bounds.dwMinimum;
		maximum = control.Bounds.dwMaximum;
		volumeControl = control.dwControlID;
		return 0;
	}

	MMRESULT CDAudio::CloseMixer()
	{
		MMRESULT result = reinterpret_cast<UINT_PTR>(mixer);
		if (result)
		{
			result = mixerClose(mixer);
			mixer = NULL;
		}
		return result;
	}
	MCIERROR CDAudio::Initialize() //53A550.
	{
		//Only the fields selected by each MCI command's flags are initialized.
		MCI_OPEN_PARMSA open;
		open.lpstrDeviceType = "cdaudio";
		MCIERROR result = mciSendCommandA(0, MCI_OPEN, MCI_OPEN_TYPE,
			reinterpret_cast<DWORD_PTR>(&open));
		if (result) return result;
		deviceID = open.wDeviceID;
		MCI_SET_PARMS set;
		set.dwTimeFormat = MCI_FORMAT_TMSF;
		result = mciSendCommandA(deviceID, MCI_SET, MCI_SET_TIME_FORMAT,
			reinterpret_cast<DWORD_PTR>(&set));
		if (result)
		{
			mciSendCommandA(deviceID, MCI_CLOSE, 0, 0);
			deviceID = 0;
			return result;
		}
		MCI_STATUS_PARMS status;
		status.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
		if (!mciSendCommandA(deviceID, MCI_STATUS, MCI_STATUS_ITEM,
			reinterpret_cast<DWORD_PTR>(&status)))
			trackCount = status.dwReturn;
		OpenMixer(0); //Its error is deliberately ignored before the volume query.
		LONG left, right;
		ReadMixerVolume(left, right);
		baseVolume = right;
		if (baseVolume < 0) baseVolume = 0;
		if (baseVolume > 100) baseVolume = 100;
		effectiveVolume = Volume();
		return 0;
	}

	MCIERROR CDAudio::Stop() //53A640.
	{
		MCIDEVICEID captured = deviceID;
		if (!captured)
		{
			Initialize();
			captured = deviceID;
		}
		if (captured) captured = mciSendCommandA(captured, MCI_STOP, 0, 0);
		return captured;
	}

	int CDAudio::SetVolume(LONG volume, BYTE force) //53A670.
	{
		baseVolume = volume;
		const LONG value = Volume();
		const bool changed = value != lastVolume;
		effectiveVolume = value;
		if (changed || force == 1)
		{
			SetMixerVolume(value, value);
			lastVolume = effectiveVolume;
		}
		return 0;
	}

	int CDAudio::RefreshVolume(BYTE force) //53A6B0.
	{
		return SetVolume(baseVolume, force);
	}

	MCIERROR CDAudio::Close() //53A6D0.
	{
		MCIERROR result = Stop(); //The original may reopen a closed device here.
		if (deviceID)
		{
			CloseMixer();
			result = mciSendCommandA(deviceID, MCI_CLOSE, 0, 0);
			deviceID = 0;
		}
		mode = MCI_MODE_NOT_READY;
		trackCount = 0;
		seconds = 0;
		currentPosition = 1;
		return result;
	}

	void CDAudio::Update() //53A720.
	{
		if (!deviceID) return;
		RefreshVolume(0);
		SoundSystem* const system = AudioBackend::g_System;
		const DWORD previous = previousClock;
		const DWORD now = system->now;
		if (now > previous + 500u)
		{
			MCI_STATUS_PARMS status;
			status.dwItem = MCI_STATUS_MODE;
			if (!mciSendCommandA(deviceID, MCI_STATUS, MCI_STATUS_ITEM,
				reinterpret_cast<DWORD_PTR>(&status)))
				mode = status.dwReturn;
			status.dwItem = MCI_STATUS_POSITION;
			if (!mciSendCommandA(deviceID, MCI_STATUS, MCI_STATUS_ITEM,
				reinterpret_cast<DWORD_PTR>(&status)))
			{
				const DWORD position = status.dwReturn;
				seconds = DWORD(MCI_TMSF_MINUTE(position)) * 60u + DWORD(MCI_TMSF_SECOND(position));
				currentPosition = MCI_TMSF_TRACK(position); //53A7AD zero-extends AL before the+10 store.
			}
			previousClock = now;
		}
	}
}
