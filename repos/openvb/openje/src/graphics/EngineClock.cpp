#include "EngineClock.hpp"
#include "Engine.hpp"
#include "WaterSampling.hpp"
#include <cstring>

Gfx_EngineClockState::Gfx_EngineClockState() : deltaMilliseconds(0), milliseconds(0), secondsWord(0) {}
float Gfx_EngineClockState::Seconds() const { return Gfx_EngineClockSeconds(secondsWord); }

float Gfx_EngineClockSeconds(DWORD word)
{
	float seconds;
	std::memcpy(&seconds, &word, sizeof(seconds));
	return seconds;
}
void Gfx_AdvanceEngineClock(DWORD& delta, DWORD& total, DWORD& secondsWord, DWORD milliseconds)
{
	DWORD time = total + milliseconds;
	delta = milliseconds;
	total = time;
	float seconds = static_cast<float>(milliseconds) * 0.001f + Gfx_EngineClockSeconds(secondsWord);
	std::memcpy(&secondsWord, &seconds, sizeof(seconds));
	JE::AdvanceWaterSampling(milliseconds);
}
void Gfx_SetEngineClock(DWORD& total, DWORD& secondsWord, DWORD milliseconds)
{
	total = milliseconds;
	float seconds = static_cast<float>(milliseconds) * 0.001f;
	std::memcpy(&secondsWord, &seconds, sizeof(seconds));
}

void Gfx_Engine::AdvanceTime(DWORD milliseconds)
{
	DWORD time = value0C + milliseconds;
	value08 = milliseconds;
	value0C = time;
	value10 += static_cast<float>(milliseconds) * 0.001f;
	JE::AdvanceWaterSampling(milliseconds);
}
void Gfx_Engine::SetTimeMilliseconds(DWORD milliseconds)
{
	value0C = milliseconds;
	value10 = static_cast<float>(milliseconds) * 0.001f;
}
DWORD Gfx_Engine::DeltaMilliseconds() { return value08; }
DWORD Gfx_Engine::TimeMilliseconds() { return value0C; }
float Gfx_Engine::TimeSeconds() { return value10; }
