#pragma once

#include "G3DModel.hpp"

class G3D_WaterTile : public G3D_Mesh
{
public:
	G3D_WaterTile(); //4D98F0.
	virtual void ReadText(); //4DD060.
	virtual void WriteText(FILE* stream); //4D6BC0.
	virtual void Read(FILE* stream); //4DD0C0.
	virtual void Read(const BYTE* data, DWORD& offset); //4DD120, actual two-argument memory ABI.
	virtual void Write(FILE* stream); //4D6C10.
	LONG offsetX, offsetZ, columns, rows; //Original+9C..A8.
	DWORD sourceVersion; //Original+AC, distinct from G3D_Mesh::converted.
};
