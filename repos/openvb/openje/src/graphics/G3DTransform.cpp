#include "G3DModel.hpp"
#include "TextParser.hpp"
#include <math.h>
#include <stddef.h>
#include <string.h>

//641780 initializes the shared angle value at708048 by doubling this PI value.
const double G3D_Pi = 3.14159265358979323846;
double G3D_TwoPi = G3D_Pi + G3D_Pi;

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char G3DTransformExtent[sizeof(G3D_Transform) == 0x78 ? 1 : -1];
typedef char G3DTransformNameOffset[offsetof(G3D_Transform, name) == 0x08 ? 1 : -1];
typedef char G3DTransformRotationOffset[offsetof(G3D_Transform, rotation) == 0x28 ? 1 : -1];
typedef char G3DTransformTranslationOffset[offsetof(G3D_Transform, translation) == 0x48 ? 1 : -1];
typedef char G3DTransformScaleOffset[offsetof(G3D_Transform, scale) == 0x60 ? 1 : -1];
#endif
typedef char G3DAffineExtent[sizeof(G3D_AffineMatrix) == 96 ? 1 : -1];
typedef char G3DLinearExtent[sizeof(G3D_LinearMatrix) == 72 ? 1 : -1];
typedef char G3DAffineLinearOffset[offsetof(G3D_AffineMatrix, linear) == 0 ? 1 : -1];
typedef char G3DAffineTranslationOffset[offsetof(G3D_AffineMatrix, translation) == 72 ? 1 : -1];
typedef char G3DQuaternionExtent[sizeof(G3D_Quaternion) == 32 ? 1 : -1];

namespace
{

	//4C9550; the axis uses coordinate scaling, the degree angle does not.
	void ReadTextRotation(double* output)
	{
		G3DText::Expect('(');
		double x = G3D_ReadScaledNumber(), y = G3D_ReadScaledNumber(), z = G3D_ReadScaledNumber();
		double angle = G3DText::ReadNumber() * G3D_TwoPi * 0.001388888888888889;
		G3DText::Expect(')');
		double inverse = G3D_InverseSqrt(y * y + z * z + x * x);
		double sine = G3D_Sin(angle);
		output[1] = y * inverse * sine;
		output[2] = inverse * z * sine;
		output[3] = G3D_Cos(angle);
		output[0] = x * inverse * sine;
	}

	//4C97A0. The fread result remains ignored and the float buffer is reused.
	void ReadFileRotation(FILE* stream, double* output)
	{
		float value;
		fread(&value, sizeof(value), 1, stream); double x = value;
		fread(&value, sizeof(value), 1, stream); double y = value;
		fread(&value, sizeof(value), 1, stream); double z = value;
		fread(&value, sizeof(value), 1, stream);
		double angle = value * G3D_TwoPi * 0.001388888888888889;
		double inverse = G3D_InverseSqrt(y * y + z * z + x * x);
		x *= inverse; y *= inverse;
		double sine = G3D_Sin(angle);
		y *= sine;
		z = inverse * z * sine;
		output[1] = y;
		output[2] = z;
		output[3] = G3D_Cos(angle);
		output[0] = x * sine;
	}

	//4C98B0, including the unchecked four-byte cursor advances.
	void ReadMemoryRotation(const BYTE* data, double* output, DWORD& offset)
	{
		float value;
		memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); double x = value;
		memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); double y = value;
		memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value); double z = value;
		memcpy(&value, data + offset, sizeof(value)); offset += sizeof(value);
		double angle = value * G3D_TwoPi * 0.001388888888888889;
		double inverse = G3D_InverseSqrt(y * y + z * z + x * x);
		double sine = G3D_Sin(angle);
		output[1] = y * inverse * sine;
		output[2] = inverse * z * sine;
		output[3] = G3D_Cos(angle);
		output[0] = x * inverse * sine;
	}

	void WriteTextRotation(FILE* stream, const double* rotation) //4C9630.
	{
		double axis[3];
		memcpy(axis, rotation, sizeof(axis));
		if (axis[2] * axis[2] + axis[1] * axis[1] + axis[0] * axis[0] == 0.0)
		{
			double defaultAxis[3] = {0.0, 0.0, 1.0};
			memcpy(axis, defaultAxis, sizeof(axis));
		}
		double inverse = G3D_InverseSqrt(axis[2] * axis[2] + axis[1] * axis[1] + axis[0] * axis[0]);
		axis[0] *= inverse; axis[1] *= inverse; axis[2] *= inverse;
		double angle = acos(rotation[3]) / G3D_TwoPi * 720.0;
		if (angle > 180.0) angle -= 360.0;
		fprintf(stream, "(%g %g %g %g) ", G3D_TextNumber(axis[0]), G3D_TextNumber(axis[1]), G3D_TextNumber(axis[2]), angle);
	}

	void WriteFileRotation(FILE* stream, const double* rotation) //4C99B0.
	{
		double axis[3];
		memcpy(axis, rotation, sizeof(axis));
		if (axis[2] * axis[2] + axis[1] * axis[1] + axis[0] * axis[0] == 0.0)
		{
			double defaultAxis[3] = {0.0, 0.0, 1.0};
			memcpy(axis, defaultAxis, sizeof(axis));
		}
		double inverse = G3D_InverseSqrt(axis[2] * axis[2] + axis[1] * axis[1] + axis[0] * axis[0]);
		axis[0] *= inverse; axis[1] *= inverse; axis[2] *= inverse;
		double angle = acos(rotation[3]) / G3D_TwoPi * 720.0;
		if (angle > 180.0) angle -= 360.0;
		float value = static_cast<float>(axis[0]); fwrite(&value, sizeof(value), 1, stream);
		value = static_cast<float>(axis[1]); fwrite(&value, sizeof(value), 1, stream);
		value = static_cast<float>(axis[2]); fwrite(&value, sizeof(value), 1, stream);
		value = static_cast<float>(angle); fwrite(&value, sizeof(value), 1, stream);
	}
}

G3D_Transform::G3D_Transform(const G3D_Transform& source) : name(source.name) //4D3780.
{
	memcpy(rotation, source.rotation, 32);
	memcpy(translation, source.translation, 24);
	memcpy(scale, source.scale, 24);
}

void G3D_Transform::Read(G3D_Input& input)
{
	Read(input.data, input.offset);
}

void G3D_Transform::Read(FILE* stream) //4D0A00; absent flags leave old fields unchanged.
{
	double vector[3];
	vector[0] = vector[1] = vector[2] = 0.0;
	double quaternion[4];
	quaternion[0] = quaternion[1] = quaternion[2] = 0.0;
	quaternion[3] = 1.0;
	name.Read(stream);
	BYTE flags;
	fread(&flags, sizeof(flags), 1, stream);
	bool hasRotation = (flags & 2) != 0;
	bool hasScale = (flags & 4) != 0;
	if (flags & 1) { G3D_ReadFileVector(stream, vector); memcpy(translation, vector, sizeof(translation)); }
	if (hasRotation) { ReadFileRotation(stream, quaternion); memcpy(rotation, quaternion, sizeof(rotation)); }
	if (hasScale) { G3D_ReadFileVector(stream, vector); memcpy(scale, vector, sizeof(scale)); }
}

void G3D_Transform::Read(const BYTE* data, DWORD& offset) //4D0B40.
{
	double vector[3];
	vector[0] = vector[1] = vector[2] = 0.0;
	double quaternion[4];
	quaternion[0] = quaternion[1] = quaternion[2] = 0.0;
	quaternion[3] = 1.0;
	name.Read(data, offset);
	BYTE flags = data[offset++];
	bool hasRotation = (flags & 2) != 0;
	bool hasScale = (flags & 4) != 0;
	if (flags & 1) { G3D_ReadMemoryVector(data, vector, offset); memcpy(translation, vector, sizeof(translation)); }
	if (hasRotation) { ReadMemoryRotation(data, quaternion, offset); memcpy(rotation, quaternion, sizeof(rotation)); }
	if (hasScale) { G3D_ReadMemoryVector(data, vector, offset); memcpy(scale, vector, sizeof(scale)); }
}

void G3D_Transform::ReadText() //4D4700; reset from the original default value first.
{
	{
		G3D_Transform initial;
		name = initial.name;
		memcpy(rotation, initial.rotation, sizeof(rotation));
		memcpy(translation, initial.translation, sizeof(translation));
		memcpy(scale, initial.scale, sizeof(scale));
	}
	G3DText::Expect('{');
	for (G3DText::Token token = G3DText::NextToken(); token.kind != '}'; token = G3DText::NextToken())
	{
		if (token.kind != 256) token.ParseError();
		std::string key = G3DText::TokenString(token.value.identifier);
		if (key == "Name") name.ReadText();
		else if (key == "Translation")
		{
			double value[3] = {0.0, 0.0, 0.0};
			G3D_ReadTextVector(value); memcpy(translation, value, sizeof(translation));
		}
		else if (key == "Scale")
		{
			double value[3] = {0.0, 0.0, 0.0};
			G3D_ReadTextVector(value); memcpy(scale, value, sizeof(scale));
		}
		else
		{
			if (key != "Rotation") token.ParseError();
			double value[4] = {0.0, 0.0, 0.0, 1.0};
			ReadTextRotation(value); memcpy(rotation, value, sizeof(rotation));
		}
	}
	double vector[3];
	memcpy(vector, translation, sizeof(vector));
	vector[0] *= G3D_GlobalScale; vector[1] *= G3D_GlobalScale; vector[2] *= G3D_GlobalScale;
	memcpy(translation, vector, sizeof(translation));
}

bool G3D_Transform::HasRotation() const //4C90C0, including its zero-vector special case.
{
	double inverse = G3D_InverseSqrt(1048576.0);
	double quaternion[4]; memcpy(quaternion, rotation, sizeof(quaternion));
	double axis[3];
	axis[0] = 1024.0 * inverse * 0.0;
	axis[1] = 0.0 * inverse * 0.0;
	axis[2] = axis[1];
	if (quaternion[2] * quaternion[2] + quaternion[1] * quaternion[1] + quaternion[0] * quaternion[0] != 0.0)
	{
		memcpy(quaternion, rotation, sizeof(quaternion));
		G3D_Quaternion identity(0.0, axis);
		return quaternion[0] != identity.value[0] || quaternion[1] != identity.value[1]
			|| quaternion[2] != identity.value[2] || quaternion[3] != identity.value[3];
	}
	return FALSE;
}

bool G3D_Transform::HasTranslation() const //4C91D0.
{
	double vector[3]; memcpy(vector, translation, sizeof(vector));
	return vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2] != 0.0;
}

bool G3D_Transform::HasScale() const //4C9220.
{
	double vector[3]; memcpy(vector, scale, sizeof(vector));
	return vector[0] != 1.0 || vector[1] != 1.0 || vector[2] != 1.0;
}

void G3D_Transform::Write(FILE* stream) //4CE9B0; predicates are deliberately evaluated twice.
{
	name.Write(stream);
	BYTE flags = 0;
	if (HasTranslation()) flags = 1;
	if (HasRotation()) flags |= 2;
	if (HasScale()) flags |= 4;
	fwrite(&flags, sizeof(flags), 1, stream);
	if (HasTranslation())
	{
		double value[3]; memcpy(value, translation, sizeof(value)); G3D_WriteFileVector(stream, value);
	}
	if (HasRotation())
	{
		double value[4]; memcpy(value, rotation, sizeof(value)); WriteFileRotation(stream, value);
	}
	if (HasScale())
	{
		double value[3]; memcpy(value, scale, sizeof(value)); G3D_WriteFileVector(stream, value);
	}
}

void G3D_Transform::WriteText(FILE* stream) //4CA9C0; no indentation or newline in this owner.
{
	fprintf(stream, "%s{ ", "Transform");
	name.WriteText(stream, 0);
	if (HasTranslation())
	{
		fprintf(stream, "Translation");
		double value[3]; memcpy(value, translation, sizeof(value)); G3D_WriteTextVector(stream, value);
	}
	if (HasRotation())
	{
		fprintf(stream, "Rotation");
		double value[4]; memcpy(value, rotation, sizeof(value)); WriteTextRotation(stream, value);
	}
	if (HasScale())
	{
		fprintf(stream, "Scale");
		double value[3]; memcpy(value, scale, sizeof(value)); G3D_WriteTextVector(stream, value);
	}
	fprintf(stream, "}");
}

G3D_Quaternion::G3D_Quaternion(double angle, const double* axis) //48FE30.
{
	SetAxisAngle(angle, axis);
}


BOOL G3D_Quaternion::operator!=(const G3D_Quaternion& other) const //48F7A0.
{
	return value[0] != other.value[0] || value[1] != other.value[1]
		|| value[2] != other.value[2] || value[3] != other.value[3];
}

G3D_AffineMatrix G3D_Transform::ToMatrix() const //4CA930.
{
	G3D_AffineMatrix value(1.0);
	double vector[4];
	memcpy(vector, translation, sizeof(translation)); value.Translate(vector);
	memcpy(vector, rotation, sizeof(rotation)); value.ApplyRotation(vector);
	memcpy(vector, scale, sizeof(scale)); value.ScaleColumns(vector);
	return value;
}
