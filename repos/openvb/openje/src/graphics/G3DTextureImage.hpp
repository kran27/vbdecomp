#pragma once
#include <windows.h>
struct G3D_Texture;

//Caller convenience for actual G3D_Texture::LoadImage4CA2B0 with flag0.
//The Texture object owns its original raw four-byte pixel array.
BYTE G3D_ReadTga(G3D_Texture& texture,const BYTE* data);

