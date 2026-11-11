#pragma once

struct D3DXMATRIX;
struct D3DXVECTOR3;

//512D90; shared matrix-row output boundary, with implementation credit unresolved.
void GfxGetMatrixAxes(const D3DXMATRIX& matrix, D3DXVECTOR3& right,
	D3DXVECTOR3& up, D3DXVECTOR3& forward);

// Three-float value receiver recovered from5220A0 and its shape callers.
// Historical nominal spelling and ignored return type are not established.
struct Gfx_Vector3f
{
	float value[3];
	void RotateXZ(float cosine, float sine, Gfx_Vector3f& output) const;
	float Dot(const float* other) const; //522100; borrowed3floats, rounded float ST0, RET4; support0.
	double Length() const; //522040; observed wide ST0 return.
	double SquaredDistance(const float* other) const; //412AA0; borrowed3floats, wide ST0, RET4.
	double Distance(const float* other) const; //401070; borrowed3floats, wide ST0, RET4.
	void Normalize(); //522000; unconditional reciprocal, Y/Z/X stores.
	Gfx_Vector3f* NormalizeDirection(); //401030; unconditional reciprocal, X/Y/Z stores, return this; support0.
	Gfx_Vector3f Transform(const D3DXMATRIX& matrix) const; //522140; owned affine result.
	Gfx_Vector3f Scale(float scalar) const; //5221C0; owned rounded component result.
	Gfx_Vector3f Subtract(const Gfx_Vector3f& other) const; //5222A0; hidden12B result, RET8.
};

//521FA0; descriptive wide scalar ABI; original float/double nominal identity unresolved.
double __cdecl GfxDistanceSquared3D(const float* first, const float* second);
