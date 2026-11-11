#include "AnimationData.hpp"
#include "AnimationInstance.hpp"
#include <cstring>

BYTE Gfx_AnimationData::ProcessAnimationCues(DWORD group, DWORD index, float seconds,
	BYTE reverse, float** position, std::list<Gfx_AnimationCue>& output) //4A3360.
{
	//These exits do not allocate/update the caller's cursor or clear its output.
	if (cues.empty()) return 0;
	float duration;
	std::memcpy(&duration,reinterpret_cast<const BYTE*>(fileInfo->animations[0])+4,4);
	if (duration <= 0.0f) return 0;
	float previous = *position ? **position : 0.0f;
	const LONG cycle = static_cast<LONG>(static_cast<__int64>(double(previous)/duration));
	float base = static_cast<float>(double(cycle)*duration);
	DWORD passes = 0;
	BYTE emitted = 0;
	if (reverse)
	{
		for (;;)
		{
			if ((*position || passes) && !(previous >= seconds)) break;
			const float nextBase = base-duration;
			float end = nextBase;
			const bool last = nextBase <= seconds;
			if (last) end = seconds;
			for (std::map<float,std::list<std::string> >::reverse_iterator entry = cues.rbegin();
				entry != cues.rend(); ++entry)
			{
				//One original FSTP after both subtractions, preserving reverse loop policy.
				const float occurrence = static_cast<float>(double(base)-(double(duration)-entry->first));
				if (occurrence < previous && occurrence >= end)
				{
					std::list<std::string>& names = entry->second;
					for (std::list<std::string>::iterator name = names.begin(); name != names.end(); ++name)
					{
						emitted = 1;
						const std::string cueName(*name);
						Gfx_AnimationCue cue;
						cue.values[0] = group;
						cue.values[1] = index;
						cue.name = cueName;
						const float cueTime = entry->first;
						std::memcpy(cue.values+2,&cueTime,4);
						std::memcpy(cue.values+3,&occurrence,4);
						std::memcpy(cue.values+4,&seconds,4);
						output.push_back(cue);
					}
				}
			}
			base = nextBase;
			previous = end;
			++passes;
			if (last) break;
		}
	}
	else
	{
		while ((!*position && !passes) || previous <= seconds)
		{
			const float nextBase = base+duration;
			float end = nextBase;
			const bool last = nextBase >= seconds;
			if (last) end = seconds;
			for (std::map<float,std::list<std::string> >::iterator entry = cues.begin();
				entry != cues.end(); ++entry)
			{
				const float occurrence = base+entry->first;
				if (*position)
				{
					if (!passes ? occurrence <= previous : occurrence < previous) continue;
				}
				if (occurrence > end) continue;
				for (std::list<std::string>::iterator name = entry->second.begin(); name != entry->second.end(); ++name)
				{
					emitted = 1;
					const std::string cueName(*name);
					Gfx_AnimationCue cue;
					cue.values[0] = group;
					cue.values[1] = index;
					cue.name = cueName;
					const float cueTime = entry->first;
					std::memcpy(cue.values+2,&cueTime,4);
					std::memcpy(cue.values+4,&seconds,4);
					std::memcpy(cue.values+3,&occurrence,4);
					output.push_back(cue);
				}
			}
			++passes;
			previous = end;
			base = nextBase;
			if (last) break;
		}
	}
	if (!*position) *position = new float;
	**position = seconds;
	return emitted;
}
