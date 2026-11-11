#include "Camera.hpp"
#include "GfxVector3.hpp"
#include "G3DModel.hpp"
#include <string.h>
#include <math.h>

// The original read-only float at0x649C50 has bits3C8EFA35.
const float g_GfxDegreesToRadians = D3DX_PI / 180.0f;

// Original OctreeCamera inputs at6ADB70/6ADB6C are loaded as separate floats.
extern const float g_GfxCameraDegreeFraction = 0.0055555557f;
extern const float g_GfxCameraPi = 3.1415901f;

//48F660; actual scalar call boundary, real sqrt primitive; support0.
double G3D_Sqrt(double value)
{
	return sqrt(value);
}

double G3D_InverseSqrt(double value) //48F670; actual shared math boundary.
{
	return value == 0.0 ? 0.0 : 1.0 / sqrt(value);
}

// Ordinary observed boundaries; ownership is unresolved and receives no code credit.
double G3D_Sin(double value) //48F6A0.
{
	return sin(value);
}

double G3D_Cos(double value) //48F6B0.
{
	return cos(value);
}

void G3D_Quaternion::SetAxisAngle(double angle, const double* axis) //48F7F0.
{
	double x = axis[0], y = axis[1], z = axis[2];
	double half = angle * 0.5;
	double inverse = G3D_InverseSqrt(x * x + y * y + z * z);
	double factor = inverse * G3D_Sin(half);
	value[0] = factor * axis[0];
	value[1] = factor * axis[1];
	value[2] = factor * axis[2];
	value[3] = G3D_Cos(half);
}

//48F960: preserve the existing products, grouping and row-publication order.
void G3D_LinearMatrix::SetQuaternion(const double* input)
{
	double xx = input[0] * input[0], yy = input[1] * input[1], zz = input[2] * input[2];
	double xy = input[1] * input[0], xz = input[2] * input[0], xw = input[3] * input[0];
	double yz = input[2] * input[1], yw = input[1] * input[3], zw = input[2] * input[3];
	double row[3];
	row[0] = 1.0 - (zz + yy + zz + yy);
	row[1] = xy - zw + xy - zw;
	row[2] = yw + xz + yw + xz;
	memcpy(value, row, sizeof(row));
	row[0] = zw + xy + zw + xy;
	row[1] = 1.0 - (zz + xx + zz + xx);
	row[2] = yz - xw + yz - xw;
	memcpy(value + 3, row, sizeof(row));
	row[0] = xz - yw + xz - yw;
	row[1] = yz + xw + yz + xw;
	row[2] = 1.0 - (yy + xx + yy + xx);
	memcpy(value + 6, row, sizeof(row));
}

G3D_AffineMatrix::G3D_AffineMatrix() //48FCE0; initial value state precedes the reset API.
{
	for (DWORD row = 0; row < 3; ++row)
		linear.value[row * 3] = linear.value[row * 3 + 1] = linear.value[row * 3 + 2] = 0.0;
	translation[0] = translation[1] = translation[2] = 0.0;
	SetZero();
}

G3D_AffineMatrix::G3D_AffineMatrix(double diagonal) //490010.
{
	for (DWORD row = 0; row < 3; ++row)
		linear.value[row * 3] = linear.value[row * 3 + 1] = linear.value[row * 3 + 2] = 0.0;
	translation[0] = translation[1] = translation[2] = 0.0;
	SetDiagonal(diagonal);
}

G3D_AffineMatrix::G3D_AffineMatrix(const double* quaternion) //48FE90.
{
	for (DWORD row = 0; row < 3; ++row)
		linear.value[row * 3] = linear.value[row * 3 + 1] = linear.value[row * 3 + 2] = 0.0;
	translation[0] = translation[1] = translation[2] = 0.0;
	SetQuaternion(quaternion);
}

void G3D_LinearMatrix::SetZero() //48F870.
{
	{
		double row[3] = {0.0, 0.0, 0.0};
		memcpy(value, row, sizeof(row));
	}
	{
		double row[3] = {0.0, 0.0, 0.0};
		memcpy(value + 3, row, sizeof(row));
	}
	{
		double row[3] = {0.0, 0.0, 0.0};
		memcpy(value + 6, row, sizeof(row));
	}
}

void G3D_LinearMatrix::SetDiagonal(double diagonal) //48F940.
{
	for (DWORD index = 0; index < 3; ++index) value[index * 4] = diagonal;
}

void G3D_LinearMatrix::SetIdentity() //48FE50.
{
	SetZero();
	SetDiagonal(1.0);
}

void G3D_LinearMatrix::SetIdentity(double diagonal) //48FE70.
{
	SetZero();
	SetDiagonal(diagonal);
}

void G3D_AffineMatrix::SetZero() //48FB60.
{
	linear.SetZero();
	double zero[3];
	zero[0] = zero[1] = zero[2] = 0.0;
	memcpy(translation, zero, sizeof(zero));
}

void G3D_AffineMatrix::SetDiagonal(double diagonal) //48FF70.
{
	linear.SetIdentity(diagonal);
	double zero[3];
	zero[0] = zero[1] = zero[2] = 0.0;
	memcpy(translation, zero, sizeof(zero));
}

void G3D_AffineMatrix::SetQuaternion(const double* quaternion) //48FBB0.
{
	linear.SetQuaternion(quaternion);
	double zero[3];
	zero[0] = zero[1] = zero[2] = 0.0;
	memcpy(translation, zero, sizeof(zero));
}

void G3D_AffineMatrix::SetIdentity() //48FF20.
{
	linear.SetIdentity();
	double zero[3];
	zero[0] = zero[1] = zero[2] = 0.0;
	memcpy(translation, zero, sizeof(zero));
}

void G3D_AffineMatrix::Multiply(const G3D_AffineMatrix& left, const G3D_AffineMatrix& right) //48FAC0.
{
	const double* input = left.linear.value;
	double* output = linear.value;
	const double* inputTranslation = left.translation;
	double* outputTranslation = translation;
	for (DWORD rows = 3; rows; --rows)
	{
		output[0] = right.linear.value[6] * input[2] + input[1] * right.linear.value[3] + right.linear.value[0] * input[0];
		output[1] = right.linear.value[7] * input[2] + input[1] * right.linear.value[4] + right.linear.value[1] * input[0];
		output[2] = input[1] * right.linear.value[5] + right.linear.value[8] * input[2] + right.linear.value[2] * input[0];
		*outputTranslation = input[1] * right.translation[1] + right.translation[0] * input[0] + *inputTranslation + input[2] * right.translation[2];
		input += 3;
		output += 3;
		++inputTranslation;
		++outputTranslation;
	}
}

G3D_AffineMatrix G3D_AffineMatrix::operator*(const G3D_AffineMatrix& right) const //48FEE0.
{
	G3D_AffineMatrix result;
	result.Multiply(*this, right);
	return result;
}

void G3D_AffineMatrix::ApplyRotation(const double* quaternion) //48FFD0.
{
	G3D_AffineMatrix rotation(quaternion);
	*this = *this * rotation;
}

void G3D_AffineMatrix::ScaleColumns(const double* value) //48FC70.
{
	linear.ScaleColumns(value);
}

void G3D_LinearMatrix::ScaleColumns(const double* input) //48FA60.
{
	value[0] = input[0] * value[0];
	value[1] = value[1] * input[1];
	value[2] = input[2] * value[2];
	value[3] = value[3] * input[0];
	value[4] = input[1] * value[4];
	value[5] = value[5] * input[2];
	value[6] = value[6] * input[0];
	value[7] = input[1] * value[7];
	value[8] = value[8] * input[2];
}

void G3D_AffineMatrix::Translate(const double* value) //48FC10; retain all products and sums.
{
	translation[0] = linear.value[2] * value[2] + linear.value[1] * value[1] + value[0] * linear.value[0] + translation[0];
	translation[1] = linear.value[5] * value[2] + linear.value[4] * value[1] + linear.value[3] * value[0] + translation[1];
	translation[2] = linear.value[8] * value[2] + linear.value[7] * value[1] + linear.value[6] * value[0] + translation[2];
}


//521FA0; observed wide CDECL2 protocol, original nominal return type unresolved.
double __cdecl GfxDistanceSquared3D(const float* first, const float* second)
{
	const double x = double(first[0]) - second[0];
	const double y = double(first[1]) - second[1];
	const double z = double(first[2]) - second[2];
	return (z * z + y * y) + x * x;
}

float GfxDistanceSquaredXZ(const float* first, const float* second)
{
	const double x = double(first[0]) - second[0];
	const double z = double(first[2]) - second[2];
	return z * z + x * x;
}

D3DXVECTOR3 GfxAddVectors(const float* first, const float* second)
{
	D3DXVECTOR3 result;
	D3DXVec3Add(&result, reinterpret_cast<const D3DXVECTOR3*>(first),
		reinterpret_cast<const D3DXVECTOR3*>(second));
	return result;
}

D3DXVECTOR3 GfxSubtractVectors(const float* first, const float* second)
{
	D3DXVECTOR3 result;
	D3DXVec3Subtract(&result, reinterpret_cast<const D3DXVECTOR3*>(first),
		reinterpret_cast<const D3DXVECTOR3*>(second));
	return result;
}


	void GfxGetMatrixAxes(const D3DXMATRIX& matrix, D3DXVECTOR3& right,
		D3DXVECTOR3& up, D3DXVECTOR3& forward) // 0x512D90.
	{
		right.x=matrix._11;
		right.y=matrix._12;
		right.z=matrix._13;
		up.x=matrix._21;
		up.y=matrix._22;
		up.z=matrix._23;
		forward.x=matrix._31;
		forward.y=matrix._32;
		forward.z=matrix._33;
	}

//522100; ordinary float-result member boundary; dependency implementation0.
float Gfx_Vector3f::Dot(const float* other) const
{
	const float x = value[0] * other[0];
	const float y = value[1] * other[1];
	const float z = value[2] * other[2];
	const float result = (x + y) + z;
	return result;
}

void Gfx_Vector3f::RotateXZ(float cosine, float sine, Gfx_Vector3f& output) const //5220A0, support0.
{
	output.value[1] = value[1];
	output.value[0] = sine * value[2] + cosine * value[0];
	output.value[2] = cosine * value[2] - sine * value[0];
}

// Ordinary observed vector boundaries; ownership remains unresolved/support0.
// double names the observed wide ST0 transport, not a unique historical typedef.
double Gfx_Vector3f::Length() const //522040.
{
	const double x = value[0], y = value[1], z = value[2];
	return sqrt(x * x + y * y + z * z);
}

double Gfx_Vector3f::SquaredDistance(const float* other) const //412AA0; descriptive physical member boundary.
{
	double x = double(value[0]) - other[0];
	const double y = double(value[1]) - other[1];
	const double z = double(value[2]) - other[2];
	x *= x;
	return (x + z * z) + y * y;
}

double Gfx_Vector3f::Distance(const float* other) const //401070; descriptive physical member boundary.
{
	double x = double(value[0]) - other[0];
	const double y = double(value[1]) - other[1];
	const double z = double(value[2]) - other[2];
	x *= x;
	return sqrt((x + z * z) + y * y);
}

Gfx_Vector3f* Gfx_Vector3f::NormalizeDirection() //401030; descriptive physical boundary, support0.
{
	const float length = sqrt(value[0] * value[0]
		+ value[1] * value[1] + value[2] * value[2]);
	const float reciprocal = 1.0f / length;
	value[0] = float(reciprocal * value[0]);
	value[1] = float(reciprocal * value[1]);
	value[2] = float(reciprocal * value[2]);
	return this;
}

void Gfx_Vector3f::Normalize() //522000.
{
	const double x = value[0], y = value[1], z = value[2];
	const double inverse = 1.0 / sqrt(y * y + x * x + z * z);
	value[1] = float(y * inverse);
	value[2] = float(z * inverse);
	value[0] = float(x * inverse);
}

Gfx_Vector3f Gfx_Vector3f::Transform(const D3DXMATRIX& matrix) const //522140, support0.
{
	Gfx_Vector3f result;
	//522140 accumulates Z, Y, X, translation wide, then rounds each result once.
	double component = double(matrix._31) * double(value[2]);
	component += double(matrix._21) * double(value[1]);
	component += double(matrix._11) * double(value[0]);
	component += double(matrix._41);
	result.value[0] = float(component);
	component = double(matrix._32) * double(value[2]);
	component += double(matrix._22) * double(value[1]);
	component += double(matrix._12) * double(value[0]);
	component += double(matrix._42);
	result.value[1] = float(component);
	component = double(matrix._33) * double(value[2]);
	component += double(matrix._23) * double(value[1]);
	component += double(matrix._13) * double(value[0]);
	component += double(matrix._43);
	result.value[2] = float(component);
	return result;
}

Gfx_Vector3f Gfx_Vector3f::Scale(float scalar) const //5221C0, unresolved ownership/support0.
{
	Gfx_Vector3f result;
	result.value[0] = scalar * value[0];
	result.value[1] = scalar * value[1];
	result.value[2] = scalar * value[2];
	return result;
}

Gfx_Vector3f Gfx_Vector3f::Subtract(const Gfx_Vector3f& other) const //5222A0, support0.
{
	Gfx_Vector3f result;
	result.value[0] = value[0] - other.value[0];
	result.value[1] = value[1] - other.value[1];
	result.value[2] = value[2] - other.value[2];
	return result;
}

//521F50 ordinary SDK vector-scaling boundary; ownership remains unresolved/support0.
D3DXVECTOR3 GfxScaleVector(float scalar, const D3DXVECTOR3& input)
{
	D3DXVECTOR3 result;
	D3DXVec3Scale(&result, &input, scalar);
	return result;
}
