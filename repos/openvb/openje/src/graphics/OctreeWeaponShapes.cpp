#include "OctreeShape.hpp"
#include <d3dx8math.h>
#include <math.h>
#include <stdexcept>
#include <cstring>
#include <cstddef>

D3DXVECTOR3 GfxAddVectors(const float* first, const float* second);
D3DXVECTOR3 GfxSubtractVectors(const float* first, const float* second);

namespace
{
	typedef char VectorSize[(sizeof(Gfx_Vector3f) == 12) ? 1 : -1];
	typedef char VectorAlign[(__alignof(Gfx_Vector3f) == 4) ? 1 : -1];
	typedef char VectorData[(offsetof(Gfx_Vector3f, value) == 0) ? 1 : -1];
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char TriOffsets[(offsetof(OCTREE_Tri2D, localEnd) == 0x28 && offsetof(OCTREE_Tri2D, rotatedEnd) == 0x34 && offsetof(OCTREE_Tri2D, localNormals) == 0x40 && offsetof(OCTREE_Tri2D, worldNormals) == 0x70 && offsetof(OCTREE_Tri2D, normalSigns) == 0xA0) ? 1 : -1];
	typedef char PieOffsets[(offsetof(OCTREE_Pie2D, localNormals) == 0x30 && offsetof(OCTREE_Pie2D, worldNormals) == 0x54 && offsetof(OCTREE_Pie2D, normalSigns) == 0x78) ? 1 : -1];
	typedef char RectOffsets[(offsetof(OCTREE_Rect2D, localStart) == 0x28 && offsetof(OCTREE_Rect2D, rotatedStart) == 0x34 && offsetof(OCTREE_Rect2D, localEnd) == 0x40 && offsetof(OCTREE_Rect2D, rotatedEnd) == 0x4C && offsetof(OCTREE_Rect2D, localNormals) == 0x58 && offsetof(OCTREE_Rect2D, worldNormals) == 0x88 && offsetof(OCTREE_Rect2D, normalSigns) == 0xB8) ? 1 : -1];
#endif
	//Original vector helpers5220A0/521EB0/521F00/522100, used by the shape
	//half-space decisions. The caller's matrices are real D3DX matrices.
	DWORD NormalSigns(const float* normal)
	{
		return (normal[0]>0.0f?0:4)|(normal[1]>0.0f?0:2)|(normal[2]>0.0f?0:1);
	}
	float RelativeDot(const float* point,const float* origin,const float* normal)
	{
		float relative[3];
		for (int axis=0;axis<3;++axis) relative[axis]=point[axis]-origin[axis];
		//522100 returns through FSTP32/FLD32. Keep that API rounding even if
		//the compiler inlines this shared Tri/Pie/Rect half-space calculation.
		volatile float result = float(double(relative[0])*normal[0]
			+double(relative[1])*normal[1]+double(relative[2])*normal[2]);
		return result;
	}
	void BoxExtremes(const float* bounds,DWORD signs,float* farthest,float* nearest)
	{
		for (int axis=0;axis<3;++axis)
		{
			DWORD mask=DWORD(4)>>axis;
			farthest[axis]=bounds[axis+((signs&mask)?0:3)];
			nearest[axis]=bounds[axis+((signs&mask)?3:0)];
		}
	}

}

OCTREE_Tri2D::OCTREE_Tri2D()
{
	for (int axis=0;axis<3;++axis) localEnd.value[axis]=rotatedEnd.value[axis]=0;
	for (int normal=0;normal<4;++normal)
		for (int axis=0;axis<3;++axis) localNormals[normal].value[axis]=worldNormals[normal].value[axis]=0;
}
void OCTREE_Tri2D::Configure(float angle,float length)
{
	localEnd.value[0]=0;
	localEnd.value[1]=0;
	localEnd.value[2]=length;
	rotatedEnd.value[1]=0;
	localNormals[3].value[0]=0;
	localNormals[3].value[1]=0;
	localNormals[3].value[2]=-1;
	worldNormals[3].value[1]=0;
	localNormals[0].value[0]=0;
	localNormals[0].value[1]=0;
	localNormals[0].value[2]=1;
	double radians=(90.0f-angle*0.5f)*0.017453f; //Actual float operands.
	float cosine=float(cos(radians)),sine=float(sin(radians));
	localNormals[0].RotateXZ(cosine,-sine,localNormals[2]);
	localNormals[0].RotateXZ(cosine,sine,localNormals[1]);
}
int OCTREE_Tri2D::SetTransform(const float* matrix)
{
	const float cosine = matrix[0], sine = matrix[8];
	position[0]=matrix[12]; position[1]=0; position[2]=matrix[14];
	localNormals[0].RotateXZ(cosine,sine,worldNormals[0]);
	localNormals[1].RotateXZ(cosine,sine,worldNormals[1]);
	localNormals[2].RotateXZ(cosine,sine,worldNormals[2]);
	localNormals[3].RotateXZ(cosine,sine,worldNormals[3]);
	localEnd.RotateXZ(cosine,sine,rotatedEnd);
	for (int normal=0;normal<4;++normal)
	{
		const float* value = worldNormals[normal].value;
		if (value[0] > 0.0f)
		{
			if (value[1] > 0.0f)
			{
				if (value[2] > 0.0f) normalSigns[normal] = 0;
				else normalSigns[normal] = 1;
			}
			else
			{
				if (value[2] > 0.0f) normalSigns[normal] = 2;
				else normalSigns[normal] = 3;
			}
		}
		else
		{
			if (value[1] > 0.0f)
			{
				if (value[2] > 0.0f) normalSigns[normal] = 4;
				else normalSigns[normal] = 5;
			}
			else
			{
				if (value[2] > 0.0f) normalSigns[normal] = 6;
				else normalSigns[normal] = 7;
			}
		}
	}
	if (cachedNodes)
	{
		cachedNodes->count = 0;
		cachedNodes->blocks[DWORD(0) >> cachedNodes->shift][DWORD(0) & cachedNodes->mask] = NULL;
	}
	return 0;
}
int OCTREE_Tri2D::ContainsPoint(const float* point) const
{
	Gfx_Vector3f difference = {{0.0f, 0.0f, 0.0f}};
	std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(point, position)), sizeof difference.value);
	if (difference.Dot(worldNormals[1].value) < 0.0f) return 0;
	if (difference.Dot(worldNormals[2].value) < 0.0f) return 0;
	D3DXVECTOR3 origin;
	std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(point,
		static_cast<const float*>(origin = GfxAddVectors(position, rotatedEnd.value)))), sizeof difference.value);
	if (difference.Dot(worldNormals[3].value) < 0.0f) return 0;
	return 1;
}
int OCTREE_Tri2D::ClassifyObject(const float* center,const float* extent) const
{
	float maximum[3],minimum[3];
	Gfx_Vector3f difference = {{0.0f, 0.0f, 0.0f}};
	std::memcpy(maximum, static_cast<const float*>(GfxAddVectors(center, extent)), sizeof maximum);
	std::memcpy(minimum, static_cast<const float*>(GfxSubtractVectors(center, extent)), sizeof minimum);
	float farthest[3],nearest[3]; //Native switch default retains these locals.
	BYTE inside=1;
	for (int normal=0;normal<4;++normal)
	{
		switch (normalSigns[normal])
		{
		case 0:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 1:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		case 2:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 3:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		case 4:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 5:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		case 6:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 7:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		default:
			break; //Original leaves preceding/unwritten corner words unchanged.
		}
		if (normal < 3)
		{
			std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(farthest, position)), sizeof difference.value);
		}
		else
		{
			D3DXVECTOR3 origin;
			std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(farthest,
				static_cast<const float*>(origin = GfxAddVectors(position, rotatedEnd.value)))), sizeof difference.value);
		}
		if (difference.Dot(worldNormals[normal].value) < 0.0f) return 0;
		if (inside)
		{
			if (normal < 3)
			{
				std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(nearest, position)), sizeof difference.value);
			}
			else
			{
				D3DXVECTOR3 origin;
				std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(nearest,
					static_cast<const float*>(origin = GfxAddVectors(position, rotatedEnd.value)))), sizeof difference.value);
			}
			if (difference.Dot(worldNormals[normal].value) < 0.0f) inside = 0;
		}
	}
	return inside ? 2 : 1;
}
int OCTREE_Tri2D::ClassifyBox(const float* bounds) const
{
	float farthest[3],nearest[3]; //Native switch default retains these locals.
	Gfx_Vector3f difference = {{0.0f, 0.0f, 0.0f}};
	BYTE inside=1;
	for (int normal=0;normal<4;++normal)
	{
		switch (normalSigns[normal])
		{
		case 0:
			std::memcpy(farthest, bounds+3, sizeof farthest);
			std::memcpy(nearest, bounds, sizeof nearest);
			break;
		case 1:
			std::memcpy(farthest+0, bounds+3+0, sizeof(float));
			std::memcpy(nearest+0, bounds+0, sizeof(float));
			std::memcpy(farthest+1, bounds+3+1, sizeof(float));
			std::memcpy(nearest+1, bounds+1, sizeof(float));
			std::memcpy(farthest+2, bounds+2, sizeof(float));
			std::memcpy(nearest+2, bounds+3+2, sizeof(float));
			break;
		case 2:
			std::memcpy(farthest+0, bounds+3+0, sizeof(float));
			std::memcpy(nearest+0, bounds+0, sizeof(float));
			std::memcpy(farthest+1, bounds+1, sizeof(float));
			std::memcpy(nearest+1, bounds+3+1, sizeof(float));
			std::memcpy(farthest+2, bounds+3+2, sizeof(float));
			std::memcpy(nearest+2, bounds+2, sizeof(float));
			break;
		case 3:
			std::memcpy(farthest+0, bounds+3+0, sizeof(float));
			std::memcpy(nearest+0, bounds+0, sizeof(float));
			std::memcpy(farthest+1, bounds+1, sizeof(float));
			std::memcpy(nearest+1, bounds+3+1, sizeof(float));
			std::memcpy(farthest+2, bounds+2, sizeof(float));
			std::memcpy(nearest+2, bounds+3+2, sizeof(float));
			break;
		case 4:
			std::memcpy(farthest+0, bounds+0, sizeof(float));
			std::memcpy(nearest+0, bounds+3+0, sizeof(float));
			std::memcpy(farthest+1, bounds+3+1, sizeof(float));
			std::memcpy(nearest+1, bounds+1, sizeof(float));
			std::memcpy(farthest+2, bounds+3+2, sizeof(float));
			std::memcpy(nearest+2, bounds+2, sizeof(float));
			break;
		case 5:
			std::memcpy(farthest+0, bounds+0, sizeof(float));
			std::memcpy(nearest+0, bounds+3+0, sizeof(float));
			std::memcpy(farthest+1, bounds+3+1, sizeof(float));
			std::memcpy(nearest+1, bounds+1, sizeof(float));
			std::memcpy(farthest+2, bounds+2, sizeof(float));
			std::memcpy(nearest+2, bounds+3+2, sizeof(float));
			break;
		case 6:
			std::memcpy(farthest+0, bounds+0, sizeof(float));
			std::memcpy(nearest+0, bounds+3+0, sizeof(float));
			std::memcpy(farthest+1, bounds+1, sizeof(float));
			std::memcpy(nearest+1, bounds+3+1, sizeof(float));
			std::memcpy(farthest+2, bounds+3+2, sizeof(float));
			std::memcpy(nearest+2, bounds+2, sizeof(float));
			break;
		case 7:
			std::memcpy(farthest, bounds, sizeof farthest);
			std::memcpy(nearest, bounds+3, sizeof nearest);
			break;
		default:
			break; //Original leaves preceding/unwritten corner words unchanged.
		}
		if (normal < 3)
		{
			std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(farthest, position)), sizeof difference.value);
		}
		else
		{
			D3DXVECTOR3 origin;
			std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(farthest,
				static_cast<const float*>(origin = GfxAddVectors(position, rotatedEnd.value)))), sizeof difference.value);
		}
		if (difference.Dot(worldNormals[normal].value) < 0.0f) return 0;
		if (inside)
		{
			if (normal < 3)
			{
				std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(nearest, position)), sizeof difference.value);
			}
			else
			{
				D3DXVECTOR3 origin;
				std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(nearest,
					static_cast<const float*>(origin = GfxAddVectors(position, rotatedEnd.value)))), sizeof difference.value);
			}
			if (difference.Dot(worldNormals[normal].value) < 0.0f) inside = 0;
		}
	}
	return inside ? 2 : 1;
}

OCTREE_Pie2D::OCTREE_Pie2D()
{
	for (int normal=0;normal<3;++normal)
		for (int axis=0;axis<3;++axis) localNormals[normal].value[axis]=worldNormals[normal].value[axis]=0;
}
void OCTREE_Pie2D::Configure(float angle,float value)
{
	radius=value;
	radiusSquared=value*value;
	double radians=(90.0f-angle*0.5f)*0.017453f; //Original float operands, not the SDK degree constant.
	localNormals[0].value[0]=0;
	localNormals[0].value[1]=0;
	localNormals[0].value[2]=1;
	float cosine=float(cos(radians)),sine=float(sin(radians));
	localNormals[0].RotateXZ(cosine,-sine,localNormals[2]);
	worldNormals[2].value[1]=0;
	localNormals[0].RotateXZ(cosine,sine,localNormals[1]);
	worldNormals[1].value[1]=0;
}
int OCTREE_Pie2D::SetTransform(const float* matrix)
{
	position[0]=matrix[12]; position[1]=0; position[2]=matrix[14];
	for (int normal=0;normal<3;++normal)
	{
		localNormals[normal].RotateXZ(matrix[0],matrix[8],worldNormals[normal]);
		normalSigns[normal]=NormalSigns(worldNormals[normal].value);
	}
	if (cachedNodes) cachedNodes->Invalidate();
	return 0;
}
int OCTREE_Pie2D::ContainsPoint(const float* point) const
{
	Gfx_Vector3f difference = {{0.0f, 0.0f, 0.0f}};
	std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(point, position)), sizeof difference.value);
	if (difference.Dot(worldNormals[1].value) < 0.0f) return 0;
	if (difference.Dot(worldNormals[2].value) < 0.0f) return 0;
	if (difference.value[2] * difference.value[2]
		+ difference.value[0] * difference.value[0] > radiusSquared) return 0;
	return 1;
}
int OCTREE_Pie2D::ClassifyObject(const float* center,const float* extent) const
{
	float maximum[3],minimum[3];
	Gfx_Vector3f difference = {{0.0f, 0.0f, 0.0f}};
	std::memcpy(maximum, static_cast<const float*>(GfxAddVectors(center, extent)), sizeof maximum);
	std::memcpy(minimum, static_cast<const float*>(GfxSubtractVectors(center, extent)), sizeof minimum);
	float farthest[3],nearest[3]; //Native switch default retains these locals.
	BYTE inside=1;
	for (int normal=0;normal<3;++normal)
	{
		switch (normalSigns[normal])
		{
		case 0:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 1:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		case 2:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 3:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		case 4:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 5:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		case 6:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 7:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		default:
			break; //Original leaves preceding/unwritten corner words unchanged.
		}
		std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(farthest, position)), sizeof difference.value);
		if (difference.Dot(worldNormals[normal].value) < 0.0f) return 0;
		if (inside)
		{
			std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(nearest, position)), sizeof difference.value);
			if (difference.Dot(worldNormals[normal].value) < 0.0f) inside=0;
		}
	}
	float closestSquared=0;
	float farthestSquared=0;
	if (position[0]<minimum[0])
	{
		float nearest=minimum[0]-position[0];
		closestSquared=nearest*nearest;
		if (inside)
		{
			float farthest=position[0]-maximum[0];
			farthestSquared=float(farthest*farthest);
		}
	}
	else if (position[0]>maximum[0])
	{
		float nearest=position[0]-maximum[0];
		closestSquared=nearest*nearest;
		if (inside)
		{
			float farthest=minimum[0]-position[0];
			farthestSquared=float(farthest*farthest);
		}
	}
	else if (position[0]-minimum[0]>radius
		|| maximum[0]-position[0]>radius) inside=0;
	if (position[2]<minimum[2])
	{
		float nearest=minimum[2]-position[2];
		closestSquared+=nearest*nearest;
		if (inside)
		{
			float farthest=position[2]-maximum[2];
			farthestSquared=float(farthest*farthest+farthestSquared);
		}
	}
	else if (position[2]>maximum[2])
	{
		float nearest=position[2]-maximum[2];
		closestSquared+=nearest*nearest;
		if (inside)
		{
			float farthest=minimum[2]-position[2];
			farthestSquared=float(farthest*farthest+farthestSquared);
		}
	}
	else if (position[2]-minimum[2]>radius
		|| maximum[2]-position[2]>radius) inside=0;
	if (closestSquared>radiusSquared) return 0;
	if (!inside) return 1;
	if (!(farthestSquared<=radiusSquared)) return 1; //5BAEA1 rejects unordered.
	return 2;
}
int OCTREE_Pie2D::ClassifyBox(const float* bounds) const
{
	float farthest[3],nearest[3]; //Native switch default retains these locals.
	Gfx_Vector3f difference = {{0.0f, 0.0f, 0.0f}};
	BYTE inside=1;
	for (int normal=0;normal<3;++normal)
	{
		switch (normalSigns[normal])
		{
		case 0:
			std::memcpy(farthest, bounds+3, sizeof farthest);
			std::memcpy(nearest, bounds, sizeof nearest);
			break;
		case 1:
			std::memcpy(farthest+0, bounds+3+0, sizeof(float));
			std::memcpy(nearest+0, bounds+0, sizeof(float));
			std::memcpy(farthest+1, bounds+3+1, sizeof(float));
			std::memcpy(nearest+1, bounds+1, sizeof(float));
			std::memcpy(farthest+2, bounds+2, sizeof(float));
			std::memcpy(nearest+2, bounds+3+2, sizeof(float));
			break;
		case 2:
			std::memcpy(farthest+0, bounds+3+0, sizeof(float));
			std::memcpy(nearest+0, bounds+0, sizeof(float));
			std::memcpy(farthest+1, bounds+1, sizeof(float));
			std::memcpy(nearest+1, bounds+3+1, sizeof(float));
			std::memcpy(farthest+2, bounds+3+2, sizeof(float));
			std::memcpy(nearest+2, bounds+2, sizeof(float));
			break;
		case 3:
			std::memcpy(farthest+0, bounds+3+0, sizeof(float));
			std::memcpy(nearest+0, bounds+0, sizeof(float));
			std::memcpy(farthest+1, bounds+1, sizeof(float));
			std::memcpy(nearest+1, bounds+3+1, sizeof(float));
			std::memcpy(farthest+2, bounds+2, sizeof(float));
			std::memcpy(nearest+2, bounds+3+2, sizeof(float));
			break;
		case 4:
			std::memcpy(farthest+0, bounds+0, sizeof(float));
			std::memcpy(nearest+0, bounds+3+0, sizeof(float));
			std::memcpy(farthest+1, bounds+3+1, sizeof(float));
			std::memcpy(nearest+1, bounds+1, sizeof(float));
			std::memcpy(farthest+2, bounds+3+2, sizeof(float));
			std::memcpy(nearest+2, bounds+2, sizeof(float));
			break;
		case 5:
			std::memcpy(farthest+0, bounds+0, sizeof(float));
			std::memcpy(nearest+0, bounds+3+0, sizeof(float));
			std::memcpy(farthest+1, bounds+3+1, sizeof(float));
			std::memcpy(nearest+1, bounds+1, sizeof(float));
			std::memcpy(farthest+2, bounds+2, sizeof(float));
			std::memcpy(nearest+2, bounds+3+2, sizeof(float));
			break;
		case 6:
			std::memcpy(farthest+0, bounds+0, sizeof(float));
			std::memcpy(nearest+0, bounds+3+0, sizeof(float));
			std::memcpy(farthest+1, bounds+1, sizeof(float));
			std::memcpy(nearest+1, bounds+3+1, sizeof(float));
			std::memcpy(farthest+2, bounds+3+2, sizeof(float));
			std::memcpy(nearest+2, bounds+2, sizeof(float));
			break;
		case 7:
			std::memcpy(farthest, bounds, sizeof farthest);
			std::memcpy(nearest, bounds+3, sizeof nearest);
			break;
		default:
			break; //Original leaves preceding/unwritten corner words unchanged.
		}
		std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(farthest, position)), sizeof difference.value);
		if (difference.Dot(worldNormals[normal].value) < 0.0f) return 0;
		if (inside)
		{
			std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(nearest, position)), sizeof difference.value);
			if (difference.Dot(worldNormals[normal].value) < 0.0f) inside=0;
		}
	}
	float closestSquared=0;
	float farthestSquared=0;
	if (position[0]<bounds[0])
	{
		float nearest=bounds[0]-position[0];
		closestSquared=nearest*nearest;
		if (inside)
		{
			float farthest=position[0]-bounds[3];
			farthestSquared=float(farthest*farthest);
		}
	}
	else if (position[0]>bounds[3])
	{
		float nearest=position[0]-bounds[3];
		closestSquared=nearest*nearest;
		if (inside)
		{
			float farthest=bounds[0]-position[0];
			farthestSquared=float(farthest*farthest);
		}
	}
	else if (position[0]-bounds[0]>radius
		|| bounds[3]-position[0]>radius) inside=0;
	if (position[2]<bounds[2])
	{
		float nearest=bounds[2]-position[2];
		closestSquared+=nearest*nearest;
		if (inside)
		{
			float farthest=position[2]-bounds[5];
			farthestSquared=float(farthest*farthest+farthestSquared);
		}
	}
	else if (position[2]>bounds[5])
	{
		float nearest=position[2]-bounds[5];
		closestSquared+=nearest*nearest;
		if (inside)
		{
			float farthest=bounds[2]-position[2];
			farthestSquared=float(farthest*farthest+farthestSquared);
		}
	}
	else if (position[2]-bounds[2]>radius
		|| bounds[5]-position[2]>radius) inside=0;
	if (closestSquared>radiusSquared) return 0;
	if (!inside) return 1;
	if (!(farthestSquared<=radiusSquared)) return 1; //5BAA39 rejects unordered.
	return 2;
}

OCTREE_Rect2D::OCTREE_Rect2D()
{
	localStart.value[0] = 0;
	localStart.value[1] = 0;
	localStart.value[2] = 0;
	rotatedStart.value[0] = 0;
	rotatedStart.value[1] = 0;
	rotatedStart.value[2] = 0;
	localEnd.value[0] = 0;
	localEnd.value[1] = 0;
	localEnd.value[2] = 0;
	rotatedEnd.value[0] = 0;
	rotatedEnd.value[1] = 0;
	rotatedEnd.value[2] = 0;

	localNormals[0].value[0] = 0;
	localNormals[0].value[1] = 0;
	localNormals[0].value[2] = 0;
	localNormals[1].value[0] = 0;
	localNormals[1].value[1] = 0;
	localNormals[1].value[2] = 0;
	localNormals[2].value[0] = 0;
	localNormals[2].value[1] = 0;
	localNormals[2].value[2] = 0;
	localNormals[3].value[0] = 0;
	localNormals[3].value[1] = 0;
	localNormals[3].value[2] = 0;

	worldNormals[0].value[0] = 0;
	worldNormals[0].value[1] = 0;
	worldNormals[0].value[2] = 0;
	worldNormals[1].value[0] = 0;
	worldNormals[1].value[1] = 0;
	worldNormals[1].value[2] = 0;
	worldNormals[2].value[0] = 0;
	worldNormals[2].value[1] = 0;
	worldNormals[2].value[2] = 0;
	worldNormals[3].value[0] = 0;
	worldNormals[3].value[1] = 0;
	worldNormals[3].value[2] = 0;
}
void OCTREE_Rect2D::Configure(float width,float length)
{
	const float halfWidth = width * 0.5f;
	localEnd.value[2] = length;
	localNormals[0].value[2] = 1.0f;
	localNormals[1].value[0] = 1.0f;
	localStart.value[0] = -halfWidth;
	localStart.value[1] = 0.0f;
	localEnd.value[0] = halfWidth;
	localStart.value[2] = 0.0f;
	rotatedStart.value[1] = 0.0f;
	localEnd.value[1] = 0.0f;
	rotatedEnd.value[1] = 0.0f;
	localNormals[0].value[0] = 0.0f;
	localNormals[0].value[1] = 0.0f;
	worldNormals[0].value[1] = 0.0f;
	localNormals[1].value[1] = 0.0f;
	localNormals[1].value[2] = 0.0f;
	worldNormals[1].value[1] = 0.0f;
	localNormals[2].value[0] = -1.0f;
	localNormals[2].value[1] = 0.0f;
	localNormals[2].value[2] = 0.0f;
	worldNormals[2].value[1] = 0.0f;
	localNormals[3].value[0] = 0.0f;
	localNormals[3].value[1] = 0.0f;
	localNormals[3].value[2] = -1.0f;
	worldNormals[3].value[1] = 0.0f;
}
int OCTREE_Rect2D::SetTransform(const float* matrix)
{
	for (int axis=0;axis<3;++axis) position[axis]=matrix[12+axis];
	for (int normal=0;normal<4;++normal)
	{
		localNormals[normal].RotateXZ(matrix[0],matrix[8],worldNormals[normal]);
		normalSigns[normal]=NormalSigns(worldNormals[normal].value);
	}
	localStart.RotateXZ(matrix[0],matrix[8],rotatedStart);
	localEnd.RotateXZ(matrix[0],matrix[8],rotatedEnd);
	if (cachedNodes) cachedNodes->Invalidate();
	return 0;
}
int OCTREE_Rect2D::ContainsPoint(const float* point) const
{
	Gfx_Vector3f difference = {{0.0f, 0.0f, 0.0f}};
	D3DXVECTOR3 origin;
	std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(point,
		static_cast<const float*>(origin = GfxAddVectors(position, rotatedStart.value)))), sizeof difference.value);
	if (difference.Dot(worldNormals[0].value) < 0.0f) return 0;
	if (difference.Dot(worldNormals[1].value) < 0.0f) return 0;
	std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(point,
		static_cast<const float*>(origin = GfxAddVectors(position, rotatedEnd.value)))), sizeof difference.value);
	if (difference.Dot(worldNormals[2].value) < 0.0f) return 0;
	if (difference.Dot(worldNormals[3].value) < 0.0f) return 0;
	return 1;
}
int OCTREE_Rect2D::ClassifyObject(const float* center,const float* extent) const
{
	Gfx_Vector3f difference = {{0.0f, 0.0f, 0.0f}};
	BYTE inside = 1;
	float maximum[3], minimum[3];
	std::memcpy(maximum, static_cast<const float*>(GfxAddVectors(center, extent)), sizeof maximum);
	std::memcpy(minimum, static_cast<const float*>(GfxSubtractVectors(center, extent)), sizeof minimum);
	float farthest[3], nearest[3];
	for (int normal = 0; normal < 4; ++normal)
	{
		switch (normalSigns[normal])
		{
		case 0:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 1:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		case 2:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 3:
			std::memcpy(farthest+0, maximum+0, sizeof(float));
			std::memcpy(nearest+0, minimum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		case 4:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 5:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, maximum+1, sizeof(float));
			std::memcpy(nearest+1, minimum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		case 6:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, maximum+2, sizeof(float));
			std::memcpy(nearest+2, minimum+2, sizeof(float));
			break;
		case 7:
			std::memcpy(farthest+0, minimum+0, sizeof(float));
			std::memcpy(nearest+0, maximum+0, sizeof(float));
			std::memcpy(farthest+1, minimum+1, sizeof(float));
			std::memcpy(nearest+1, maximum+1, sizeof(float));
			std::memcpy(farthest+2, minimum+2, sizeof(float));
			std::memcpy(nearest+2, maximum+2, sizeof(float));
			break;
		default:
			break; //Original leaves preceding/unwritten corner words unchanged.
		}
		if (normal < 2)
		{
			D3DXVECTOR3 origin;
			std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(farthest,
				static_cast<const float*>(origin = GfxAddVectors(position, rotatedStart.value)))), sizeof difference.value);
		}
		else
		{
			D3DXVECTOR3 origin;
			std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(farthest,
				static_cast<const float*>(origin = GfxAddVectors(position, rotatedEnd.value)))), sizeof difference.value);
		}
		if (difference.Dot(worldNormals[normal].value) < 0.0f) return 0;
		if (inside)
		{
			if (normal < 2)
			{
				D3DXVECTOR3 origin;
				std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(nearest,
					static_cast<const float*>(origin = GfxAddVectors(position, rotatedStart.value)))), sizeof difference.value);
			}
			else
			{
				D3DXVECTOR3 origin;
				std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(nearest,
					static_cast<const float*>(origin = GfxAddVectors(position, rotatedEnd.value)))), sizeof difference.value);
			}
			if (difference.Dot(worldNormals[normal].value) < 0.0f) inside = 0;
		}
	}
	return inside ? 2 : 1;
}
int OCTREE_Rect2D::ClassifyBox(const float* bounds) const
{
	Gfx_Vector3f difference = {{0.0f, 0.0f, 0.0f}};
	BYTE inside = 1;
	float farthest[3], nearest[3];
	for (int normal = 0; normal < 4; ++normal)
	{
		switch (normalSigns[normal])
		{
		case 0:
			std::memcpy(farthest+0, bounds+3, sizeof(float));
			std::memcpy(nearest+0, bounds+0, sizeof(float));
			std::memcpy(farthest+1, bounds+4, sizeof(float));
			std::memcpy(nearest+1, bounds+1, sizeof(float));
			std::memcpy(farthest+2, bounds+5, sizeof(float));
			std::memcpy(nearest+2, bounds+2, sizeof(float));
			break;
		case 1:
			std::memcpy(farthest+0, bounds+3, sizeof(float));
			std::memcpy(nearest+0, bounds+0, sizeof(float));
			std::memcpy(farthest+1, bounds+4, sizeof(float));
			std::memcpy(nearest+1, bounds+1, sizeof(float));
			std::memcpy(farthest+2, bounds+2, sizeof(float));
			std::memcpy(nearest+2, bounds+5, sizeof(float));
			break;
		case 2:
			std::memcpy(farthest+0, bounds+3, sizeof(float));
			std::memcpy(nearest+0, bounds+0, sizeof(float));
			std::memcpy(farthest+1, bounds+1, sizeof(float));
			std::memcpy(nearest+1, bounds+4, sizeof(float));
			std::memcpy(farthest+2, bounds+5, sizeof(float));
			std::memcpy(nearest+2, bounds+2, sizeof(float));
			break;
		case 3:
			std::memcpy(farthest+0, bounds+3, sizeof(float));
			std::memcpy(nearest+0, bounds+0, sizeof(float));
			std::memcpy(farthest+1, bounds+1, sizeof(float));
			std::memcpy(nearest+1, bounds+4, sizeof(float));
			std::memcpy(farthest+2, bounds+2, sizeof(float));
			std::memcpy(nearest+2, bounds+5, sizeof(float));
			break;
		case 4:
			std::memcpy(farthest+0, bounds+0, sizeof(float));
			std::memcpy(nearest+0, bounds+3, sizeof(float));
			std::memcpy(farthest+1, bounds+4, sizeof(float));
			std::memcpy(nearest+1, bounds+1, sizeof(float));
			std::memcpy(farthest+2, bounds+5, sizeof(float));
			std::memcpy(nearest+2, bounds+2, sizeof(float));
			break;
		case 5:
			std::memcpy(farthest+0, bounds+0, sizeof(float));
			std::memcpy(nearest+0, bounds+3, sizeof(float));
			std::memcpy(farthest+1, bounds+4, sizeof(float));
			std::memcpy(nearest+1, bounds+1, sizeof(float));
			std::memcpy(farthest+2, bounds+2, sizeof(float));
			std::memcpy(nearest+2, bounds+5, sizeof(float));
			break;
		case 6:
			std::memcpy(farthest+0, bounds+0, sizeof(float));
			std::memcpy(nearest+0, bounds+3, sizeof(float));
			std::memcpy(farthest+1, bounds+1, sizeof(float));
			std::memcpy(nearest+1, bounds+4, sizeof(float));
			std::memcpy(farthest+2, bounds+5, sizeof(float));
			std::memcpy(nearest+2, bounds+2, sizeof(float));
			break;
		case 7:
			std::memcpy(farthest+0, bounds+0, sizeof(float));
			std::memcpy(nearest+0, bounds+3, sizeof(float));
			std::memcpy(farthest+1, bounds+1, sizeof(float));
			std::memcpy(nearest+1, bounds+4, sizeof(float));
			std::memcpy(farthest+2, bounds+2, sizeof(float));
			std::memcpy(nearest+2, bounds+5, sizeof(float));
			break;
		default:
			break; //Original leaves preceding/unwritten corner words unchanged.
		}
		if (normal < 2)
		{
			D3DXVECTOR3 origin;
			std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(farthest,
				static_cast<const float*>(origin = GfxAddVectors(position, rotatedStart.value)))), sizeof difference.value);
		}
		else
		{
			D3DXVECTOR3 origin;
			std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(farthest,
				static_cast<const float*>(origin = GfxAddVectors(position, rotatedEnd.value)))), sizeof difference.value);
		}
		if (difference.Dot(worldNormals[normal].value) < 0.0f) return 0;
		if (inside)
		{
			if (normal < 2)
			{
				D3DXVECTOR3 origin;
				std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(nearest,
					static_cast<const float*>(origin = GfxAddVectors(position, rotatedStart.value)))), sizeof difference.value);
			}
			else
			{
				D3DXVECTOR3 origin;
				std::memcpy(difference.value, static_cast<const float*>(GfxSubtractVectors(nearest,
					static_cast<const float*>(origin = GfxAddVectors(position, rotatedEnd.value)))), sizeof difference.value);
			}
			if (difference.Dot(worldNormals[normal].value) < 0.0f) inside = 0;
		}
	}
	return inside ? 2 : 1;
}

int CreateOctreeRect2D(float width, float length, OCTREE_Shape** output) //5BA030.
{
	if (width < 0.0f || length < 0.0f) return 2;
	OCTREE_Rect2D* rectangle = new OCTREE_Rect2D;
	if (!rectangle) return 1;
	rectangle->Configure(width, length);
	*output = rectangle;
	return 0;
}
