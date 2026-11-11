#pragma once

#include <windows.h>
#include <d3dx8math.h>
#include <stdio.h>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <cstddef>
#include "Water.hpp"
#include "G3DName.hpp"

class G3D_Model;
class G3D_Node;
class G3D_WaterAttribute;

void G3D_ReadFileString(std::string& output, FILE* stream); //4D0450; real CRT FILE domain.
void G3D_ReadMemoryString(std::string& output, const BYTE* data, DWORD& offset); //4D0510; actual cdecl API.
void G3D_WriteFileString(std::string value, FILE* stream); //4CB420; original by-value string lifetime.
void G3D_WriteIndent(FILE* stream); //4C8B00; shared text-format indentation.
void G3D_OpenTextBlock(FILE* stream); //4C8B30.
void G3D_CloseTextBlock(FILE* stream); //4C8B70.
extern double G3D_GlobalScale; //6FB350; shared original text-coordinate scale.
extern double G3D_CoordinateScale; //6FB348; text writer rounding scale.
extern double G3D_CoordinateInverse; //708080; shared reciprocal of the text numeric scale.
extern double G3D_CoordinateScaleDouble; //6FB358; distinct original double-valued scale.
extern double G3D_TwoPi; //708048; original geometry angle constant.
double G3D_ReadScaledNumber(); //4C8720; original text numeric scale.
D3DXCOLOR G3D_ReadTextColor(); //4C8750; three scaled components, alpha1.
D3DXCOLOR G3D_ReadFileColor(FILE* stream); //4C88F0; RGB floats, alpha1.
D3DXCOLOR G3D_ReadMemoryColor(const BYTE* data, DWORD& offset); //4C8970.
void G3D_WriteFileColor(FILE* stream, const float* color); //4C8A80; RGB only.
void G3D_WriteTextColor(const float* color, FILE* stream); //4C8810; original internal register ABI remains separate.
double G3D_TextNumber(double value); //4C8730; floor(scale*value+0.5).
void G3D_ReadFileVector(FILE* stream, double* output); //4C9400; three float wire values.
void G3D_ReadMemoryVector(const BYTE* data, double* output, DWORD& offset); //4C9460.
void G3D_ReadTextVector(double* output); //4C92B0.
void G3D_WriteFileVector(FILE* stream, const double* value); //4C94F0.
void G3D_WriteFileTextureCoordinate(FILE* stream, const double* value); //4C94B0; two float-wire coordinates.
void G3D_WriteTextVector(FILE* stream, const double* value); //4C9330.

struct G3D_Vector3d
{
	double value[3];
	void CheckUnitMagnitude(); //48FC80; no default construction writes.
};

// Typed source for the game's B3D memory reader. No STL object is placed
// over original storage. Scalar readers have the original unchecked input domain.
class G3D_Input
{
public:
	G3D_Input(const BYTE* data, DWORD& offset) : data(data), offset(offset) {}
	template<class T> T Read()
	{
		T result;
		Copy(&result, sizeof(result));
		return result;
	}
	void Copy(void* output, DWORD count);
	void ReadString(std::string& output); //Adapter to the actual4D0510 cdecl API above.
	const BYTE* data;
	DWORD& offset;
};

double G3D_InverseSqrt(double value); //48F670; zero input returns zero.
double G3D_Sin(double value); //48F6A0; unresolved ownership, real CRT facade.
double G3D_Cos(double value); //48F6B0; unresolved ownership, real CRT facade.

// Actual double-precision value owner; its historical class spelling is unknown.
struct G3D_Quaternion
{
	G3D_Quaternion(double angle, const double* axis); //48FE30.
	void SetAxisAngle(double angle, const double* axis); //48F7F0.
	BOOL operator!=(const G3D_Quaternion& other) const; //48F7A0; full-EAX result.
	double value[4];
};

// The affine forwarding API owns this first72B matrix; historical spelling unknown.
struct G3D_LinearMatrix
{
	void SetZero(); //48F870; three independent24B row publications.
	void SetDiagonal(double diagonal); //48F940; scalar words copied to0/4/8.
	void SetIdentity(); //48FE50; zero then unit diagonal.
	void SetIdentity(double diagonal); //48FE70; zero then the supplied diagonal.
	void SetQuaternion(const double* quaternion); //48F960; three24B row publications.
	void ScaleColumns(const double* value); //48FA60.
	double value[9];
};

struct G3D_AffineMatrix
{
	G3D_AffineMatrix(); //48FCE0; actual double-precision affine owner.
	explicit G3D_AffineMatrix(double diagonal); //490010.
	explicit G3D_AffineMatrix(const double* quaternion); //48FE90.
	void SetZero(); //48FB60; linear and translation reset, not a constructor alias.
	void SetDiagonal(double diagonal); //48FF70; full scalar reset, including translation.
	void SetQuaternion(const double* quaternion); //48FBB0; linear quaternion reset then zero translation.
	void SetIdentity(); //48FF20.
	void Multiply(const G3D_AffineMatrix& left, const G3D_AffineMatrix& right); //48FAC0.
	G3D_AffineMatrix operator*(const G3D_AffineMatrix& right) const; //48FEE0; native right reference, hidden value result.
	void ApplyRotation(const double* quaternion); //48FFD0.
	void Translate(const double* value); //48FC10; actual affine arithmetic, including identity terms.
	void ScaleColumns(const double* value); //48FC70; forwards to the linear owner.
	G3D_LinearMatrix linear;
	double translation[3];
};

class G3D_Transform
{
	public:
	G3D_Transform(); // 0x4CB190.
	G3D_Transform(const G3D_Transform& source); //4D3780; real name and raw numeric copies.
	virtual ~G3D_Transform() {} //4CB1F0;4CB220 is the delete wrapper.
	virtual void ReadText(); //4D4700.
	virtual void WriteText(FILE* stream); //4CA9C0.
	virtual void Read(FILE* stream); //4D0A00.
	virtual void Read(const BYTE* data, DWORD& offset); //4D0B40.
	virtual void Write(FILE* stream); //4CE9B0.
	void Read(G3D_Input& input); //Compatibility call to the original raw API.
	bool HasRotation() const; //4C90C0.
	bool HasTranslation() const; //4C91D0.
	bool HasScale() const; //4C9220.
	G3D_AffineMatrix ToMatrix() const; //4CA930; actual hidden96B value return.
	G3D_Name name;
	double rotation[4], translation[3], scale[3];
};

struct G3D_TexturePixel
{
	G3D_TexturePixel(); //502110; the four-byte color starts zero.
	BYTE red, green, blue, alpha;
};

struct G3D_Texture
{
	G3D_Texture() : width(0), height(0), pixels(NULL) {} //442200; flags/cube remain unwritten.
	G3D_Texture(const G3D_Texture& other); //4D0150; original shallow pixel-pointer copy.
	~G3D_Texture(); //442230; pixels before the two real string lifetimes.
	void Read(const BYTE* data, DWORD& offset); //4D1180; original two-argument raw API.
	void Read(G3D_Input& input); //Compatibility call to the actual raw API.
	void ReadText(); //4D4BA0; original shared token input.
	void Read(FILE* stream); //4D1050; distinct FILE reader.
	void ClearPixels(); //4CA250.
	LONG SetWidth(LONG value); //4CA270; clears pixels before writing width.
	LONG SetHeight(LONG value); //4CA290.
	void SetPixel(LONG x, LONG y, const G3D_TexturePixel& value); //4C8CD0.
	BYTE LoadImage(const BYTE* data, BYTE expand24); //4CA2B0; original unchecked TGA domain.
	std::string GetName() const; //4CE840; shared first-name value getter.
	std::string GetFilename() const; //4CE860; owned return.
	G3D_Name name;
	std::string file;
	BYTE flags[4], cube; //+38..3C.
	LONG width, height; //+40/+44.
	G3D_TexturePixel* pixels; //+48, owned raw array; copy deliberately borrows it.
};

struct G3D_Bone
{
	G3D_Bone(); //4CB2E0.
	~G3D_Bone(); //4CB300.
	std::string GetName() const; //4CE840, owned name value.
	void Read(const BYTE* data, DWORD& offset); //4D09D0; own name, then embedded Transform.
	void Read(G3D_Input& input); //Compatibility forwarder to the raw reader.
	void ReadText(); //4D07F0.
	void Read(FILE* stream); //4D09B0.
	G3D_Name name;
	G3D_Transform transform;
};

struct G3D_Material
{
	G3D_Material(); // 0x4D02A0, ordinary real string members.
	void Read(const BYTE* data, DWORD& offset); //4D16A0; temporary technique through return.
	void Read(G3D_Input& input); //Compatibility forwarder to the raw reader.
	void ReadText(); //4D5100; original shared token input.
	void Read(FILE* stream); //4D1290.
	void SetSinU(BYTE enabled, double first, double second); //4C8610.
	void SetSinV(BYTE enabled, double first, double second); //4C8650.
	void SetRotation(BYTE enabled, double first, double second, double third); //4C8690.
	void SetAnimation(BYTE enabled, double rate, LONG first, LONG second, LONG third); //4C86E0.
	std::string GetName() const; //4CE8C0; owned value of name at20.
	std::string GetBlend() const; //4CE8E0; owned return.
	std::string technique; //+00.
	LONG index; //+1C.
	G3D_Name name; //+20.
	D3DXCOLOR ambient, diffuse, emissive, specular; //+3C/+4C/+5C/+6C.
	double power, alpha; //+80/+88; natural alignment, no stored field at7C.
	BYTE twoSided, flat, isFloor, noShadow, bottomStory, bottomStoryInterior, topStory; //+90..96.
	DWORD buildingNumber; //+98; actual BLD_NUM01..15 destination.
	std::string blend, materialType; //+9C/+B8.
	BYTE scrollUEnabled; double scrollU; //+D4/+D8.
	BYTE scrollVEnabled; double scrollV; //+E0/+E8.
	BYTE sinUEnabled; double sinUFirst, sinUSecond; //+F0/+F8.
	BYTE sinVEnabled; double sinVFirst, sinVSecond; //+108/+110.
	BYTE rotationEnabled; double rotationFirst, rotationSecond, rotationThird; //+120/+128.
	BYTE animationEnabled; double animationRate; //+140/+148.
	LONG animationFirst, animationSecond, animationThird; //+150..158; complete natural size160h.
};

//4CC100 constructs the six strings only; scalar fields and the borrowed/raw
//name-array owner are unwritten until supplied by a reader or copy operation.
struct G3D_MeshMaterial
{
	void Read(G3D_Input& input); // Converted branch within 0x4DC330.
	std::string name, technique;
	float colors[16], power;
	std::string blend, materialType;
	BYTE flags180[3];
	DWORD value184;
	BYTE flag188;
	DWORD value192;
	BYTE flag196;
	float parametersA[2];
	BYTE flag208;
	float parametersB[2];
	BYTE flag220;
	float parametersC[3];
	BYTE flag236;
	float parametersD[4];
	DWORD textureCount;
	std::string textures[2];
	DWORD nameCount;
	std::string* names; //Mesh reset owns deletion, not the material destructor.
};

class G3D_BaseAttribute
{
public:
	G3D_BaseAttribute() : owner(NULL) {}
	virtual ~G3D_BaseAttribute() {} //4CB670; normal inline cleanup, separate unwind entry.
	virtual void ReadText() = 0;
	virtual void WriteText(FILE* stream) = 0;
	virtual void Read(FILE* stream) = 0;
	virtual void Read(const BYTE* data, DWORD& offset) = 0;
	virtual void Write(FILE* stream) = 0;
	void ReadName(FILE* stream); //4D06B0; ordinary original FILE name reader.
	void ReadName(const BYTE* data, DWORD& offset); //4D0750; temporary before assigning the actual name.
	void WriteName(FILE* stream); //4CE980.
	std::string GetName() const; //4CE730; same shared getter used by derived attributes.
	G3D_Name name;
	G3D_Node* owner; // Original +20; borrowed node, published before the reader.
};

// Data read from a model's Water attribute. The two source dimensions are
// doubles, not integer grid storage; their natural alignment explains +28.
struct G3D_WaterDimensions
{
	G3D_WaterDimensions() { columns = rows = 0.0; }
	double columns, rows;
};

class G3D_WaterAttribute : public G3D_BaseAttribute
{
public:
	G3D_WaterAttribute(); //4D8120; original RTTI name G3D_Water.
	virtual ~G3D_WaterAttribute() {} //4D81C0; normal list/string/base lifetime.
	virtual void ReadText(); //4D24A0.
	virtual void WriteText(FILE* stream); //4CEF90.
	virtual void Read(FILE* stream); //4D2620.
	virtual void Read(const BYTE* data, DWORD& offset); //4D26D0.
	virtual void Write(FILE* stream); //4CF090; original omits the attribute token.
	std::string GetName() const; //4CE730, original by-value name accessor.
	G3D_WaterDimensions dimensions; //+28, two double values.
	std::string textureName; //+38.
	float cellSize; //+54.
	std::string secondaryTextureName; //+58.
	LONG tilesX, tilesZ; //+74/+78; text reader evaluates Z before X.
	std::list<Gfx_WaterTexturePatch> texturePatches; //+7C, actual shared64B record policy.
};

struct G3D_BoneWeight
{
	G3D_BoneWeight() : bone(-1), weight(0.0) {} // 0x4C8BA0; alignment padding unwritten.
	void Read(const BYTE* data, DWORD& offset); //4C8C60; signed index, then float weight.
	size_t Read(FILE* stream); //4C8C20; preserves final fread result and reused argument bytes.
	void Write(FILE* stream); //4C8C90; signed index, then float weight.
	void ReadText(); //4C8BC0; unscaled parenthesized index/weight.
	void WriteText(FILE* stream); //4C8BF0; exact Bone(index weight) text.
	LONG bone;
	double weight;
};

struct G3D_Vertex
{
	G3D_Vertex(); // 0x4C9B30.
	void SetTextureCoordinate(LONG index, const double* value); //4C85B0; unchecked full16-byte slot copy.
	void SetBoneWeight(LONG index, const G3D_BoneWeight& value); //4C85E0; includes original record padding.
	void Read(const BYTE* data, DWORD& offset); //4C9FB0; staged float-wire values.
	void Read(G3D_Input& input); //Compatibility forwarder to the raw reader.
	void Read(FILE* stream); //4C9E00, actual binary FILE domain.
	void ReadText(); //4CC620, shared original text-token input.
	void Write(FILE* stream); //4CA150.
	void WriteText(FILE* stream); //4C9CE0.
	double position[3], normal[3];
	float color[4];
	double textureCoordinates[2][2];
	LONG textureCount;
	G3D_BoneWeight boneWeights[2]; // Signed index+00, unwritten pad+04, double weight+08.
	LONG boneCount;
};

struct G3D_Polygon
{
	G3D_Polygon() : value00(DWORD(-1)), textureCount(0) {} //Primitive remains unwritten.
	void ReadMemoryProperties(const BYTE* data, DWORD& offset); //4D9A80; one reusable temporary string.
	void Read(const BYTE* data, DWORD& offset); //4D9C90; properties, then indices/edge flags.
	void Read(G3D_Input& input); //Compatibility call to the actual raw API.
	void Read(FILE* stream); //4D9BB0, properties then indices/edge flags.
	void ReadFileProperties(FILE* stream); //4D9930.
	void ReadTextIndices(); //4D8780; property state belongs to the enclosing mesh reader.
	void ClearNames(); //4D46C0; releases only the zone-name vector.
	void Write(FILE* stream); //4D2DA0.
	void WriteText(FILE* stream); //4D2C70.
	void WriteFileProperties(FILE* stream); //4D29F0.
	void WriteTextProperties(FILE* stream); //4D2770.
	std::string TextureValue(LONG index) const; //4CE750; unchecked indexed value.
	std::string MaterialValue() const; //4CE790.
	std::string GetTextureName(LONG index) const; //4CE800 ->4CE750.
	std::string GetMaterial() const; //4CE820 ->4CE790.
	std::string GetName(LONG index) const; //4CE7C0; unchecked indexed value.
	DWORD value00;
	DWORD textureCount;
	std::string textures[2]; //Actual fixed strings; serialized count remains unchecked.
	std::string material;
	std::vector<std::string> names;
	LONG primitive;
	std::vector<LONG> indices;
	std::vector<BYTE> edgeFlags;
};

struct Gfx_G3D_BoneWeight
{
	LONG bone;
	float weight;
};

struct G3D_ConvertedMeshData
{
	DWORD vertexCount, vertexStride;
	BYTE* vertices;
	float* minimum;
	float* maximum;
	BYTE hasBones;
	Gfx_G3D_BoneWeight** boneWeights;
	DWORD groupCount;
	G3D_MeshMaterial* materials;
	std::string* materialNames;
	DWORD* indexCounts;
	DWORD* primitiveCounts;
	void** indices;
};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char GfxG3DBoneWeightSize[(sizeof(Gfx_G3D_BoneWeight) == 8) ? 1 : -1];
typedef char GfxG3DBoneWeightValueOffset[(offsetof(Gfx_G3D_BoneWeight, weight) == 4) ? 1 : -1];
typedef char G3DConvertedMeshDataSize[(sizeof(G3D_ConvertedMeshData) == 52) ? 1 : -1];
#define G3D_CONVERTED_OFFSET(field, offset) \
	typedef char G3DConvertedMeshData_##field[(offsetof(G3D_ConvertedMeshData, field) == offset) ? 1 : -1]
G3D_CONVERTED_OFFSET(vertexCount, 0x00);
G3D_CONVERTED_OFFSET(vertexStride, 0x04);
G3D_CONVERTED_OFFSET(vertices, 0x08);
G3D_CONVERTED_OFFSET(minimum, 0x0C);
G3D_CONVERTED_OFFSET(maximum, 0x10);
G3D_CONVERTED_OFFSET(hasBones, 0x14);
G3D_CONVERTED_OFFSET(boneWeights, 0x18);
G3D_CONVERTED_OFFSET(groupCount, 0x1C);
G3D_CONVERTED_OFFSET(materials, 0x20);
G3D_CONVERTED_OFFSET(materialNames, 0x24);
G3D_CONVERTED_OFFSET(indexCounts, 0x28);
G3D_CONVERTED_OFFSET(primitiveCounts, 0x2C);
G3D_CONVERTED_OFFSET(indices, 0x30);
#undef G3D_CONVERTED_OFFSET
#endif

class G3D_Mesh : public G3D_BaseAttribute
{
public:
	G3D_Mesh(); // 0x4D7A20.
	virtual ~G3D_Mesh(); //4D9130.
	virtual void ReadText(); //4DB040; shared original lexer, no FILE argument.
	virtual void WriteText(FILE* stream); //4D2E80.
	virtual void Read(FILE* stream); //4DB810; distinct original FILE reader.
	virtual void Read(const BYTE* data, DWORD& offset); //4DC330; VC7 reverses same-name virtual overload declarations.
	virtual void Write(FILE* stream); //4D2F40.
	void Clear(); //4D8C10; actual mesh payload reset.
	void ResetConverted(); //4CCC10; releases every converted index group.
	BYTE GetConvertedData(G3D_ConvertedMeshData* output); //4C8D70; borrowed52B snapshot.
	void AddPolygon(const G3D_Polygon& polygon); //4D9D50; allocates/deep-copies an owned polygon.
	void CopyFrom(const G3D_Mesh& source); //4D9DD0; retains actual derived kind and original shallow name-array copy.
	BYTE kind; //+24;15 for Mesh,26 for WaterTile. BaseAttribute has no kind member.
	std::vector<G3D_Vertex> legacyVertices; //+28; actual real VC7 vector.
	std::vector<G3D_Polygon*> polygons; //+38; owned children.
	BYTE converted;
	DWORD vertexCount, vertexStride;
	BYTE* vertices;
	float minimum[3], maximum[3];
	BYTE hasBones;
	Gfx_G3D_BoneWeight* boneWeights[4]; //Four raw arrays of8B records.
	DWORD groupCount;
	G3D_MeshMaterial* materials;
	std::string* materialNames;
	DWORD* indexCounts;
	DWORD* vertexCounts;
	WORD** indices;
};

class G3D_OcclusionZones : public G3D_BaseAttribute
{
public:
	G3D_OcclusionZones() { names.clear(); } //4D7460; original explicit empty-vector cleanup.
	virtual ~G3D_OcclusionZones() {} //4D7500; original derived destruction phase.
	virtual void ReadText(); //4D9590.
	virtual void WriteText(FILE* stream); //4CB480.
	virtual void Read(FILE* stream); //4D9770.
	virtual void Read(const BYTE* data, DWORD& offset); //4D9830.
	virtual void Write(FILE* stream); //4D0F60.
	std::string GetName(DWORD index) const; //5246C0, unchecked indexed value.
	std::vector<std::string> names;
};

class G3D_Node
{
public:
	G3D_Node(); //4D7600; owner publication belongs to its actual callers.
	virtual ~G3D_Node(); //4CCEF0;4CEA90 is the delete wrapper.
	virtual void ReadText(); //4DA230.
	virtual void WriteText(FILE* stream); //4CAB30.
	virtual void Read(FILE* stream); //4DA760.
	virtual void Read(const BYTE* data, DWORD& offset); //4DABD0.
	virtual void Write(FILE* stream); //4CEAB0.
	void Read(G3D_Input& input); //Compatibility call to the actual raw API.
	void CacheTransform(); //4CAAA0.
	std::string GetName() const; //4CE920; owned name value.
	void AddChild(G3D_Node* child); // 0x4D80F0, before virtual child Read.
	void AddAttribute(G3D_BaseAttribute* attribute); // 0x4D9400, before virtual attribute Read.
	G3D_Name name;
	G3D_Transform transform;
	G3D_AffineMatrix cachedTransform;
	G3D_Node* parent;
	G3D_Model* model;
	std::vector<G3D_BaseAttribute*> attributes;
	std::vector<G3D_Node*> children;
	BYTE written;

private:
	G3D_Node(const G3D_Node&);
	G3D_Node& operator=(const G3D_Node&);
};

class G3D_Model
{
public:
	explicit G3D_Model(const char* filename = NULL); //4DED60, optional actual file dispatch.
	~G3D_Model(); // 0x4F3DE0.
	void Read(const BYTE* data); // 0x4DE680; original unbounded memory input.
	void ReadText(FILE* stream); //4DDAF0.
	BYTE Read(FILE* stream); //4DE120.
	LONG ReadFile(const char* filename); //4DEB90; returns1text/2binary.
	void AddMaterial(const G3D_Material& material); // 0x4D8E30; first nonempty name wins.
	void AddTexture(const G3D_Texture& texture); // 0x4D8F10; first nonempty name wins.
	void AddBone(const G3D_Bone& bone); //4D9430; first nonempty name wins.
	void AddWater(const G3D_WaterAttribute& water); //4DDA10; original by-value map.
	void RegisterNode(G3D_Node* node); // 0x4D8FF0; later same-name node replaces lookup only.
	void RegisterAttribute(G3D_BaseAttribute* attribute); // 0x4D9080.
	G3D_Material* FindMaterial(const std::string& name); //4CD010; NULL on absence.
	G3D_Texture* FindTexture(const std::string& name); //4CD040; NULL on absence.

	BYTE version;
	std::map<std::string, G3D_Material*> materialsByName;
	std::map<std::string, G3D_Texture*> texturesByName;
	std::map<std::string, G3D_Bone*> bonesByName;
	std::map<std::string, G3D_Node*> nodesByName;
	std::map<std::string, G3D_BaseAttribute*> attributesByName;
	std::list<G3D_Material> materials;
	std::list<G3D_Texture> textures;
	std::list<G3D_Bone> bones;
	std::map<std::string, G3D_WaterAttribute> waterByName;
	G3D_Node* root;
	double globalScale, coordinateScale; // Unwritten until corresponding tag, as in original.
	BYTE hasColor;
	float color[4];
	std::string coordinateSystem;
	LONG valueC0;

private:
	G3D_Model(const G3D_Model&);
	G3D_Model& operator=(const G3D_Model&);
};
