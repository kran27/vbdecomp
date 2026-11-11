#pragma once
#include <windows.h>
namespace JE
{
 // Parameter kinds7/6; native constructors establish the two-float prefix.
 struct VFX_Vector2
 {
  float values[2];
  VFX_Vector2(); //560490; folded with other two-word zero initializers.
  VFX_Vector2(float x, float y); //52F780.
 };
 struct VFX_Vector
 {
  VFX_Vector2 xy;
  float z;
  VFX_Vector(); //523B00.
  VFX_Vector(float x, float y, float z); //523B20.
  bool operator!=(const VFX_Vector& input) const; //501150.
 };
 // Descriptive angular-value owner; original nominal spelling is unresolved.
 struct VFX_Rotation
 {
  float values[3];
  VFX_Rotation(); //52C260, zero angular components.
  VFX_Rotation(float x, float y, float z); //52C270.
 };
 struct VFX_Target
 {
  DWORD selector;
  VFX_Target(); //502110.
  explicit VFX_Target(DWORD value); //401450, scalar construction; folded aliases remain unresolved.
  VFX_Target& operator=(DWORD value); //401450, update the already constructed result word.
  DWORD Value() const; //502120, raw selector.
 };
#if defined(_MSC_VER) && _MSC_VER == 1300
 typedef char VfxVector2OriginalSize[(sizeof(VFX_Vector2) == 8) ? 1 : -1];
 typedef char VfxVectorOriginalSize[(sizeof(VFX_Vector) == 12) ? 1 : -1];
 typedef char VfxRotationOriginalSize[(sizeof(VFX_Rotation) == 12) ? 1 : -1];
 typedef char VfxTargetOriginalSize[(sizeof(VFX_Target) == 4) ? 1 : -1];
#endif
 struct VFX_ColorBytes
 {
  BYTE values[3];
  VFX_ColorBytes(); //52E850, three zero bytes.
  operator BYTE*() { return values; }
  operator const BYTE*() const { return values; }
 };
 struct VFX_TextureCoordinates
 {
  float values[4];
  VFX_TextureCoordinates(); //52EA80, defaults0,1,1,0.
  VFX_TextureCoordinates(float x, float y, float z, float w); //52EAA0.
  operator float*() { return values; }
  operator const float*() const { return values; }
 };
}
