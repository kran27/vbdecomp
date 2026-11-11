#include "GrannyRuntime21.hpp"
#include "../diagnostics/DebugOutput.hpp"

void Gfx_TransformGrannyFileToLHS(Granny21::FileInfo* file, float unitsPerMeter)
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
		Granny21::GrannyComputeBasisConversion(file, unitsPerMeter, origin, right, up,
			back, affine, linear, inverseLinear);
		// Original4A1EFF does not inspect ComputeBasisConversion's return value.
		Granny21::GrannyTransformFile(file, affine, linear, inverseLinear);
	}
}
