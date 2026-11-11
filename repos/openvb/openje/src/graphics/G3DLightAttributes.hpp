#pragma once

#include "G3DModel.hpp"

//The attribute hierarchy is ordinary single inheritance. These colors are the
//real SDK four-float value; the double fields retain their natural alignment.
class G3D_BaseLight : public G3D_BaseAttribute
{
public:
	G3D_BaseLight(); //4CB520.
	virtual void ReadText(); //404B70; the original base implementation is empty.
	virtual void WriteText(FILE* stream); //5A9320; the original base implementation is empty.
	D3DXCOLOR ambient, diffuse, specular; //+24/+34/+44.
	double intensity; //+58; bytes54..57 are natural alignment, not stored data.
};

class G3D_DirectionalLight : public G3D_BaseLight
{
public:
	G3D_DirectionalLight(); //4CB5E0.
	virtual void ReadText(); //4D5EA0.
	virtual void WriteText(FILE* stream); //4CB6A0.
	virtual void Read(FILE* stream); //4D1AC0.
	virtual void Read(const BYTE* data, DWORD& offset); //4D1B60.
	virtual void Write(FILE* stream); //4CEC30.
	G3D_Vector3d direction; //+60; the original serializers omit this field.
};

class G3D_PointLight : public G3D_BaseLight
{
public:
	G3D_PointLight(); //4CB740.
	virtual void ReadText(); //4D6150.
	virtual void WriteText(FILE* stream); //4CB780.
	virtual void Read(FILE* stream); //4D1C10.
	virtual void Read(const BYTE* data, DWORD& offset); //4D1D30.
	virtual void Write(FILE* stream); //4CECA0.
	double attenuation[3]; //+60/+68/+70.
	double radius; //+78.
	LONG falloff; //+80; serialized through a float, including float-to-LONG conversion.
};

class G3D_SpotLight : public G3D_PointLight
{
public:
	G3D_SpotLight(); //4CB890.
	virtual void ReadText(); //4D64E0.
	virtual void WriteText(FILE* stream); //4CB950.
	virtual void Read(FILE* stream); //4D1E50.
	virtual void Read(const BYTE* data, DWORD& offset); //4D1FF0.
	virtual void Write(FILE* stream); //4CED80.
	G3D_Vector3d direction; //+88.
	double spotAngle, spotSoftAngle; //+A0/+A8; the constructor leaves both unwritten.
	double valueB0, valueB8; //Real +B0/+B8 values initialized45/0; their semantic names remain open.
};

class G3D_Camera : public G3D_BaseAttribute
{
public:
	G3D_Camera(); //4CBAB0.
	virtual void ReadText(); //4D6940.
	virtual void WriteText(FILE* stream); //4CBAF0.
	virtual void Read(FILE* stream); //4D21A0.
	virtual void Read(const BYTE* data, DWORD& offset); //4D2200.
	virtual void Write(FILE* stream); //4CEEB0.
	double nearDistance, farDistance, fieldOfView; //+28/+30/+38; no member at24.
};
