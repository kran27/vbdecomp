#include "GameUsableObject.hpp"
#include "entity/SpatialFootprint.hpp"
#include <stdlib.h>
#include <new>

namespace F3
{
	JE::RLEPrimitive<JE::Entity*>* GameUsableObject::CreateSpatialFootprint()
	{
		D3DXVECTOR3 low = minimum, high = maximum;
		float footprintAngle = rotation.x; //412924..412942, before allocation.
		if (high.x - low.x < 4.0f) { low.x = 0; high.x = 4; }
		if (high.z - low.z < 4.0f) { low.z = 0; high.z = 4; }
		LONG absX = abs(static_cast<LONG>(low.x));
		float absZ = static_cast<float>(abs(static_cast<LONG>(low.z)));
		LONG first = static_cast<LONG>(double(absX) + low.x);
		LONG last = static_cast<LONG>(double(absX) + high.x);
		LONG row = static_cast<LONG>(double(absZ) + low.z);
		LONG lastRow = static_cast<LONG>(double(absZ) + high.z);
		return new JE::RLEConvexQuad<JE::Entity*>(first, row, first, lastRow,
			last, lastRow, last, row, footprintAngle, this);
	}
}
