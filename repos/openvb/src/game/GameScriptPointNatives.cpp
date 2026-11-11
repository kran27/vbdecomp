#include "GameScriptNatives.hpp"
#include "GameScriptScalarContext.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "startup/Random.hpp"
#include <math.h>

namespace F3
{
	cell AMX_NATIVE_CALL NativePointDistance(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "PointDistance")) return 0;
		D3DXVECTOR3 first(0,0,0);
		if (!JE::ScriptServices::ReadNativeVector(vm, parameters[1], first))
		{
			LogScriptError(vm, "PointDistance() is unable to retrieve the first point.");
			return 0;
		}
		D3DXVECTOR3 second(0,0,0);
		if (!JE::ScriptServices::ReadNativeVector(vm, parameters[2], second))
		{
			LogScriptError(vm, "PointDistance() is unable to retrieve the second point.");
			return 0;
		}
		//5C382A keeps all three differences in x87 until FSQRT, then uses the
		//official __ftol2 integer conversion. No float-cell result or x spill.
		double x=double(first.x)-second.x;
		double y=double(first.y)-second.y;
		double z=double(first.z)-second.z;
		return cell(sqrt(z*z+y*y+x*x));
	}

	cell AMX_NATIVE_CALL NativeRandomNumberFloat(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "RandomNumberFloat")) return 0;
		float low;
		if (!JE::ScriptServices::ReadNativeFloat(vm, parameters[1], low))
		{
			LogScriptError(vm, "RandomNumberFloat() was unable to retrieve the minimum value.");
			return 0;
		}
		float high;
		if (!JE::ScriptServices::ReadNativeFloat(vm, parameters[2], high))
		{
			LogScriptError(vm, "RandomNumberFloat() was unable to retrieve the maximum value.");
			return 0;
		}
		float result;
		if (!CopyScriptFloat(vm, result, RandomFloat(low, high)))
		{
			LogScriptError(vm, "RandomNumberFloat() was unable to set the result.");
			return 0;
		}
		return amx_ftoc(result);
	}
	cell AMX_NATIVE_CALL NativeRandomNumber(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "RandomNumber")) return 0;
		return cell(RandomInteger(parameters[1], parameters[2]));
	}

}
