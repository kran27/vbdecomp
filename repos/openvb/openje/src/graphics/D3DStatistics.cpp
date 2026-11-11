#include "D3DState.hpp"
#include "../diagnostics/DebugOutput.hpp"

DWORD Gfx_D3D::GetFreeTextureMemory()
{
	if (device)
	{
		DWORD available = device->GetAvailableTextureMem();
		statistics[14] = available;
		JE::LogDebugString("Free Texture Memory: %d\n", available);
	}
	return statistics[14];
}

void Gfx_D3D::UpdateStatistics()
{
	++statistics[2];
	++statistics[1];
	QueryPerformanceCounter(&performanceCounter); // BOOL ignored, original state output.
	const float current = static_cast<float>(
		inverseFrequency * static_cast<double>(performanceCounter.QuadPart));
	currentStatisticsTime = static_cast<float>(current);
	const float elapsed = current - previousStatisticsTime;
	if (elapsed > 0.5f)
	{
		framesPerSecond = static_cast<float>(statistics[1]) / elapsed;
		statistics[1] = 0;
		previousStatisticsTime = static_cast<float>(current);
	}
	statistics[3] = 0;
	statistics[4] = 0;
	statistics[5] = 0;
	statistics[6] = 0;
	statistics[7] = 0;
	statistics[8] = 0;
	statistics[9] = 0;
	statistics[10] = 0;
	statistics[11] = 0;
	statistics[12] = 0;
	statistics[13] = 0;
}
