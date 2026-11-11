#include "GameScriptNatives.hpp"
#include "GameScriptMapContext.hpp"
#include "graphics/GfxVector3.hpp"
#include "entity/Entity.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <math.h>
#include <cstring>

namespace
{
	void NormalizeDirection(D3DXVECTOR3& value)
	{
		// Actual401030 has no zero-length guard; zero vectors propagate NaNs.
		double reciprocal = 1.0 / sqrt(double(value.x)*value.x
			+ double(value.y)*value.y + double(value.z)*value.z);
		value.x = float(reciprocal * value.x);
		value.y = float(reciprocal * value.y);
		value.z = float(reciprocal * value.z);
	}
	double FacingDirection(D3DXVECTOR3 direction)
	{
		NormalizeDirection(direction);
		double angle = acos(double(direction.y)*0.0 + double(direction.x)*0.0 + direction.z) * 57.295776f;
		return direction.x < 0.0f ? -angle : angle;
	}
	cell TruncateScalar(double value)
	{
		// Originals tail-call the official __ftol2 and return its low DWORD,
		// rather than returning AMX float bits or a rounded integer.
		return cell(static_cast<__int64>(value));
	}
}

namespace F3
{
	cell AMX_NATIVE_CALL NativeGetPosition(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetPosition")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetPosition() was unable to find the requested entity.");
			return 0;
		}
		D3DXVECTOR3 position;
		position.x = entity->position.x;
		position.y = entity->position.y;
		position.z = entity->position.z;
		if (!WriteScriptVector(vm, parameters[2], position))
		{
			LogScriptError(vm, "GetPosition() was unable to set the vector parameter.");
			return 0;
		}
		return 1;
	}

	cell AMX_NATIVE_CALL NativeSetPosition(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "SetPosition")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "SetPosition was unable to find the requested entity.");
			return 0;
		}
		D3DXVECTOR3 point(0,0,0);
		if (!JE::ScriptServices::ReadNativeVector(vm, parameters[2], point))
		{
			LogScriptError(vm, "SetPosition() was unable to retrieve the position point.");
			return 0;
		}
		JE::LogDebugString("Trying to SetPosition on entity.");
		entity->SetPosition(point); //Actual54A8A0; the old IDA Network label is misleading.
		return 1;
	}

	cell AMX_NATIVE_CALL NativeGetFacing(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetFacing")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetFacing() was unable to find the requested entity.");
			return 0;
		}
		return TruncateScalar(entity->rotation.x);
	}

	cell AMX_NATIVE_CALL NativeSetFacing(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "SetFacing")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "SetFacing() was unable to find the requested entity.");
			return 0;
		}
		D3DXVECTOR3 rotation;
		rotation.x = entity->rotation.x;
		rotation.y = entity->rotation.y;
		rotation.z = entity->rotation.z;
		rotation.x = float(parameters[2]); //Original FILD consumes signed integer degrees.
		entity->SetRotation(rotation);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeSetFacingTarget(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "SetFacingTarget")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &source, 1))
		{
			LogScriptError(vm, "SetFacingTarget() was unable to find the source entity.");
			return 0;
		}
		JE::Entity* target;
		if (!ResolveScriptEntity(vm, DWORD(parameters[2]), &target, 1))
		{
			LogScriptError(vm, "SetFacingTarget() was unable to find the target entity.");
			return 0;
		}
		JE::Entity* entity = source;
		Gfx_Vector3f direction;
		direction.value[0] = target->position.x - entity->position.x;
		direction.value[1] = target->position.y - entity->position.y;
		direction.value[2] = target->position.z - entity->position.z;
		direction.NormalizeDirection();
		D3DXVECTOR3 rotation;
		rotation.x = entity->rotation.x;
		rotation.y = entity->rotation.y;
		rotation.z = entity->rotation.z;
		float dot = direction.value[1] * 0.0f;
		dot += direction.value[0] * 0.0f;
		dot += direction.value[2];
		rotation.x = float(acos(dot));
		rotation.x *= 57.295776f;
		if (direction.value[0] < 0.0f) rotation.x = -rotation.x;
		entity->SetRotation(rotation);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeSetFacingPoint(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "SetFacingPoint")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &source, 1))
		{
			LogScriptError(vm, "SetFacingPoint() was unable to find the source entity.");
			return 0;
		}
		D3DXVECTOR3 point(0,0,0);
		if (!JE::ScriptServices::ReadNativeVector(vm, parameters[2], point))
		{
			LogScriptError(vm, "SetFacingPoint() was unable to retreive the point.");
			return 0;
		}
		JE::Entity* entity = source;
		Gfx_Vector3f direction;
		direction.value[0] = point.x - entity->position.x;
		direction.value[1] = point.y - entity->position.y;
		direction.value[2] = point.z - entity->position.z;
		direction.NormalizeDirection();
		D3DXVECTOR3 rotation;
		rotation.x = entity->rotation.x;
		rotation.y = entity->rotation.y;
		rotation.z = entity->rotation.z;
		float dot = direction.value[1] * 0.0f;
		dot += direction.value[0] * 0.0f;
		dot += direction.value[2];
		rotation.x = float(acos(dot));
		rotation.x *= 57.295776f;
		if (direction.value[0] < 0.0f) rotation.x = -rotation.x;
		entity->SetRotation(rotation);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeGetDirectionToEntity(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetDirectionToEntity")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &source, 1))
		{
			LogScriptError(vm, "GetDirectionToEntity() was unable to find the source entity.");
			return 0;
		}
		JE::Entity* target;
		if (!ResolveScriptEntity(vm, DWORD(parameters[2]), &target, 1))
		{
			LogScriptError(vm, "GetDirectionToEntity() was unable to find the target entity.");
			return 0;
		}
		Gfx_Vector3f direction;
		direction.value[0] = target->position.x - source->position.x;
		direction.value[1] = target->position.y - source->position.y;
		direction.value[2] = target->position.z - source->position.z;
		direction.NormalizeDirection();
		float dot = direction.value[1] * 0.0f;
		dot += direction.value[0] * 0.0f;
		dot += direction.value[2];
		float angle = float(acos(dot));
		angle *= 57.295776f;
		return TruncateScalar(direction.value[0] < 0.0f ? -angle : angle);
	}

	cell AMX_NATIVE_CALL NativeGetDirectionToPoint(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetDirectionToPoint")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetDirectionToPoint() was unable to find the requested entity.");
			return 0;
		}
		D3DXVECTOR3 point(0,0,0);
		if (!JE::ScriptServices::ReadNativeVector(vm, parameters[2], point))
		{
			LogScriptError(vm, "GetDirectionToPoint() was unable to retrieve the point.");
			return 0;
		}
		Gfx_Vector3f direction;
		direction.value[0] = point.x - entity->position.x;
		direction.value[1] = point.y - entity->position.y;
		direction.value[2] = point.z - entity->position.z;
		direction.NormalizeDirection();
		float dot = direction.value[1] * 0.0f;
		dot += direction.value[0] * 0.0f;
		dot += direction.value[2];
		float angle = float(acos(dot));
		angle *= 57.295776f;
		return TruncateScalar(direction.value[0] < 0.0f ? -angle : angle);
	}

	cell AMX_NATIVE_CALL NativeDistance(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "Distance")) return 0;
		JE::Entity* first;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &first, 1))
		{
			LogScriptError(vm, "Distance() is unable to find the first entity.");
			return 0;
		}
		JE::Entity* second;
		if (!ResolveScriptEntity(vm, DWORD(parameters[2]), &second, 1))
		{
			LogScriptError(vm, "Distance() is unable to find the second entity.");
			return 0;
		}
		D3DXVECTOR3 firstPosition = first->position;
		D3DXVECTOR3 secondPosition = second->position;
		float x = float(double(firstPosition.x) - secondPosition.x);
		double y = double(firstPosition.y) - secondPosition.y;
		double z = double(firstPosition.z) - secondPosition.z;
		return TruncateScalar(sqrt(z*z + y*y + double(x)*x));
	}
}
