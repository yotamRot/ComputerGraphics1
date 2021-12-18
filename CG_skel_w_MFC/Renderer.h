#pragma once
#include <vector>
#include <set>
#include <tuple>
#include <array>
#include <map>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"



using namespace std;
class Renderer;
#define DEFAULT_NORMAL_SIZE			0.1
#define WHITE						vec3(1,1,1)
#define RED							vec3(1,0,0)
#define GREEN						vec3(0,1,0)
#define BLUE						vec3(0,0,1)

struct color {
	int red, green, blue;
};

enum NormalKind
{
	face_normal,
	vertix_normal
};

enum TransAxis
{
	MODEL,
	WORLD
};

enum LightType
{
	POINT_SOURCE,
	PARALLEL_SOURCE
};

enum Shadow
{
	FLAT,
	GOURAUD,
	PHONG
};




class Model {
public:
	mat4 _model_transform;
	mat4 _world_transform;
	void virtual draw(Renderer* renderer) = 0;
	vec3 virtual CenteringTranslation(TransAxis axis) = 0;
protected:
	virtual ~Model() {}
};

class Light {
public:
	Light(int modelId, Model* model);
	int modelId;
	Model* model;
	LightType type;
	vec3 c_light_position;
	float La;
	float Ld;
	float Ls;
	vec3 GetL();
};


class Shape
{
public:
	Shape() = default;

	void RasterizeLine(vec2 verMin, vec2 verMax);
	void RasterizeBigNegetive(vec2& ver1, vec2& ver2);
	void RasterizeRegularNegetive(vec2& ver1, vec2& ver2);
	void RasterizeBig(vec2& ver1, vec2& ver2);
	void RasterizeRegular(vec2& ver1, vec2& ver2);	
	virtual float  GetZ(int x, int y) =0;
	virtual void  Rasterize() =0;
	virtual void  UpdateShape() = 0;
	virtual bool  ShouldDrawShape() = 0;
	virtual float GetColor(int x, int y, int z, vector<Light*> lights, Shadow shadow) = 0;

	bool is_light;
	int yMin;
	int yMax;
	vec3 shape_color;
	bool should_draw;
	int* x_min;
	int* x_max;
	void UpdateLimits(int x, int y);

};


class Line : public virtual Shape
{
public:
	Line() = default;

	 ~Line()=default;
	
	vec3 p1_3d;
	vec3 p2_3d;

	vec2 p1;
	vec2 p2;
	vec3 C_p1_3d;
	vec3 C_p2_3d;

	void  Rasterize() override;
	float GetZ(int x, int y) override;
	bool  ShouldDrawShape() override;
	void UpdateShape() override;
	float GetColor(int x, int y, int z, vector<Light*> lights, Shadow shadow) override;



	Line(vec3& p1_3d, vec3& p2_3d, bool is_light);
};

class Normal : public Line
{
public:
	Normal() = default;

	~Normal() = default;

	vec3 normal_direction;
	float normal_size;
	bool is_valid;
	NormalKind normal_kind;
	void  UpdateShape() override;

	Normal(vec3& p1_3d, vec3& p2_3d, bool is_light,  NormalKind normal_kind, float normal_size=DEFAULT_NORMAL_SIZE, bool is_valid =true);
};

extern Normal invalid_normal;

class Triangle : public virtual Shape
{
	
public:
	Triangle() = default;
	
	vec3 p1_3d;
	vec3 p2_3d;
	vec3 p3_3d;

	vec3 C_p1_3d;
	vec3 C_p2_3d;
	vec3 C_p3_3d;
	
	vec2 p1;
	vec2 p2;
	vec2 p3;
	
	Normal p1_normal;
	Normal p2_normal;
	Normal p3_normal;

	Normal normal;

	float ka;
	float kd;
	float ks;

	float p1_illumination;
	float p2_illumination;
	float p3_illumination;

	void Rasterize() override;
	float GetZ(int x, int y) override ;
	bool  ShouldDrawShape() override;
	void  UpdateShape() override;
	float GetColor(int x, int y, int z, vector<Light*> lights, Shadow shadow) override;
	float GetGouruad(int x, int y);
	vec3 GetPhong(int x, int y);

	Triangle(vec3& p1_3d, vec3& p2_3d, vec3& p3_3d, vec3 rgb, bool is_light, Normal& normal,
		Normal& p1_normal=invalid_normal, Normal& p2_normal=invalid_normal,
		Normal& p3_normal=invalid_normal);
};

struct CustomCompareYmin
{
	bool operator()(Shape* shape1, Shape* shape2) const;
};

struct CustomCompareYMax
{
	bool operator()(Shape* shape1, Shape* shape2) const;
};

class Renderer
{
	float *m_outBuffer; // 3*width*height
	float *m_zbuffer; // width*height
	int m_width, m_height;
	mat4 cTransform;
	mat4 cProjection;
	mat4 nTransform;
	mat4 oTransform;

	bool isShowVerticsNormals;
	bool isShowFacesNormals;
	bool isShowBoundBox;

	vector<Shape*> shapes;

	Shadow shadow;

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();

	void transformToScreen(vec2& vec);
	bool shouldDrawModel(const vector<vec3>* boundingBox);



	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
public:
	Renderer();
	Renderer(int width, int height);
	~Renderer(void);
	void Init();
	void Renderer::DrawTriangles(vector<Triangle>* triangles,
		vector<Line>* boundBoxLines, GLfloat proportionalValue);
	void DrawBoundingBox(vector<Line>* lines);
	void DrawPixel(int x, int y, vec3& rgb);
	void ConfigureRenderer(const mat4& projection, const mat4& cTransform ,
		bool isDrawVertexNormal, bool isDrawFaceNormal, bool isDrawBoundBox,
		vector<Light*> scene_lights, Shadow scene_shadow);
	void SetObjectMatrices(const mat4& oTransform, const mat4& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
	void ResizeBuffers(int width, int height);
	void ZBufferScanConvert();
	vec2 vec3ToVec2(const vec3& ver);
	vec3 Transform(const vec3& ver);
	vec3 NormTransform(const vec3& ver);
	mat4 GetProjection();
	int yMin;
	int yMax;
	vector<Light*> lights;
	vector<Light*> GetLights();
};
