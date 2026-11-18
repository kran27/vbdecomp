#include "GrannyRuntime28.hpp"
#include "../diagnostics/DebugOutput.hpp"

void Gfx_TransformGrannyFileToLHS(Granny28::FileInfo* file, float unitsPerMeter)
{
	if (!file)
	{
		JE::LogDebugString("TransformGrannyFileToLHS - Invalid file!\n");
	}
	else if (unitsPerMeter <= 0.0f)
	{
		JE::LogDebugString("TransformGrannyFileToLHS - Invalid scale: %f\n", unitsPerMeter);
	}
	else
	{
		// Original FCOMP/TEST AH,41h/JP also enters this branch for unordered input.
		float origin[3] = {0.0f, 0.0f, 0.0f};
		float right[3] = {1.0f, 0.0f, 0.0f};
		float up[3] = {0.0f, 1.0f, 0.0f};
		float back[3] = {0.0f, 0.0f, -1.0f};
		float affine[3], linear[9], inverseLinear[9];
		if (!Granny28::GrannyComputeBasisConversion(file, unitsPerMeter, origin, right, up,
			back, affine, linear, inverseLinear))
		{
			JE::LogDebugString("TransformGrannyFileToLHS - GrannyComputeBasisConversion failed!\n");
			return;
		}
		// Granny 2.8 requires tolerances and flags (validate these against real assets).
		Granny28::GrannyTransformFile(file, affine, linear, inverseLinear,
			0.001f, 0.001f, 0);
	}
}
