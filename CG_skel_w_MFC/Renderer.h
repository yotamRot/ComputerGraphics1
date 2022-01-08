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

// fog params
#define FOG_COLOR				vec3(0.5,0.5,0.5)
#define FOG_MIN					2
#define FOG_MAX					45

enum Face
{
	Up,
	Down,
	Left,
	Right,
	Near,
	Far
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

enum RendererActions
{
	NotDraw,
	Draw,
	Clip
};

enum Axis
{
	X,
	Y,
	Z
};

enum Transformation
{
	SCALE,
	MOVE,
	ROTATE,
};

enum Projection
{
	ORTHOGRAPHIC,
	PERSPECTIVE,
	FRUSTUM
};

enum ZoomDirection
{
	ZOOM_IN,
	ZOOM_OUT
};


class Model {
public:
	mat4 _model_transform;
	mat4 _world_transform;
	void virtual draw(bool draw_bounding_box, bool draw_vertix_normals, bool draw_faces_normals) = 0;
	vec3 virtual CenteringTranslation(TransAxis axis) = 0;
protected:
	virtual ~Model() {}
};

//class Light {
//public:
//	float La;
//	float Ld;
//	float Ls;
//	float l_alpha;
//	int modelId;
//	vec3 c_light_position;
//	Model* model;
//	LightType type;
//
//	Light(int modelId, Model* model);
//	vec4 GetL(); // return L and alpha
//};


class Shape
{
public:
	Shape();
	Shape(const Shape& a);
	virtual ~Shape() = 0;
	void RasterizeLine(vec2 verMin, vec2 verMax);
	void RasterizeBigNegetive(vec2& ver1, vec2& ver2);
	void RasterizeRegularNegetive(vec2& ver1, vec2& ver2);
	void RasterizeBig(vec2& ver1, vec2& ver2);
	void RasterizeRegular(vec2& ver1, vec2& ver2);	

	virtual vec3  GetCoordinates(int x, int y) =0;
	virtual void  Rasterize() =0;
	virtual void  UpdateShape() = 0;
	//virtual vec3 GetColor(vec3& C_cords, vector<Light>& lights, Shadow shadow) = 0;
	virtual void Clip() = 0;

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
	Line();

	// ~Line();
	
	vec3 p1_3d;
	vec3 p2_3d;

	vec2 p1_2d;
	vec2 p2_2d;

	vec3 C_p1_3d;
	vec3 C_p2_3d;

	vec4 P_p1_4d;
	vec4 P_p2_4d;

	void  Rasterize() override;
	vec3 GetCoordinates(int x, int y) override;
	void UpdateShape() override;
	void Clip() override;
	void ClipFace(Face face);

	//vec3 GetColor(vec3& C_cords, vector<Light>& lights, Shadow shadow) override;




	Line(vec3& p1_3d, vec3& p2_3d, bool is_light);
};

class Normal : public Line
{
public:
	Normal();

	//~Normal();

	vec3 normal_direction;
	float normal_size;
	bool is_valid;
	NormalKind normal_kind;
	void UpdateShape() override;

	Normal(vec3& p1_3d, vec3& p2_3d, bool is_light,  NormalKind normal_kind, float normal_size=DEFAULT_NORMAL_SIZE, bool is_valid =true);
};

extern Normal invalid_normal;

//class Triangle : public virtual Shape
//{
//	
//public:
//	Triangle();
//	//~Triangle();
//	vec3 p1_3d;
//	vec3 p2_3d;
//	vec3 p3_3d;
//
//	vec3 C_p1_3d;
//	vec3 C_p2_3d;
//	vec3 C_p3_3d;
//
//	vec4 P_p1_4d;
//	vec4 P_p2_4d;
//	vec4 P_p3_4d;
//	
//	vec2 p1_2d;
//	vec2 p2_2d;
//	vec2 p3_2d;
//	
//	Normal p1_normal;
//	Normal p2_normal;
//	Normal p3_normal;
//
//	Normal normal;
//
//	float ka;
//	float kd;
//	float ks;
//	float ke;
//
//	float p1_illumination;
//	float p2_illumination;
//	float p3_illumination;
//
//	bool is_non_uniform;
//
//	void Rasterize() override;
//	vec3 GetCoordinates(int x, int y) override ;
//	void UpdateShape() override;
//	void Clip() override;
//	bool ShouldDraw();
//	//vec3 GetColor(vec3& C_cords, vector<Light>& lights, Shadow shadow) override;
//	float GetGouruad(vec3& C_cords);
//	vec3 GetPhong(vec3& C_cords);
//	int ClipFace(Triangle& triangle1, Triangle& triangle2, Face face);
//
//
//	Triangle(vec3& p1_3d, vec3& p2_3d, vec3& p3_3d, vec3 rgb, bool is_light, Normal& normal,
//		Normal& p1_normal=invalid_normal, Normal& p2_normal=invalid_normal,
//		Normal& p3_normal=invalid_normal);
//};

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
	float* m_zbuffer; // width*height
	float *m_super_sample_out_Buffer; // 3*width*height
	float *m_super_sample_z_Buffer; // 3*width*height
	float *m_local_color_buffer; // 3*width*height
	float *m_local_z_buffer; // 3*width*height
	int m_width, m_height;
	int extended_m_width, extended_m_height;
	int cur_width, cur_height;
	mat4 cTransform;

	mat4 nTransform;
	mat4 oTransform;
	bool isShowBoundBox;
	bool is_wire_frame;
	bool isSuperSample;

	vector<Shape*> shapes;

	void CreateBuffers(int width, int height);

	void transformToScreen(vec2& vec);
	RendererActions Renderer::shouldDrawModel(const vector<Line>* boundBoxLines);

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
	void DrawTriangles();
	//void ClipModel(vector<Triangle>* triangles, vector<Line>* boundBoxLines);
	void DrawBoundingBox(vector<Line>* lines);
	void DrawPixel(int x, int y, vec3& rgb);
	vec3 GetPixel(int x, int y);
	//void Renderer::ConfigureRenderer(const mat4& projection, const mat4& transform,
	//							bool isDrawVertexNormal, bool isDrawFaceNormal, bool isWireFrame, bool isFog,bool isSuperSample,
	//							bool isDrawBoundBox, vector<Light> scene_lights,
	//							Shadow scene_shadow);
	void SetObjectMatrices(const mat4& oTransform, const mat4& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
	void ResizeBuffers(int width, int height);
	void ZBufferScanConvert();
	vec2 vec4ToVec2(const vec4& ver);
	void Transform(const vec3& ver, vec3& C_ver, vec4& P_ver);
	void NormTransform(const vec3& ver, vec3& direction);
	int yMin;
	int yMax;
	bool isShowVerticsNormals;
	bool isShowFacesNormals;
	bool isFog;
	mat4 cProjection;
	Shadow shadow;
	//vector<Light> lights;
	//vector<Light> GetLights();
	//vector<Triangle> triangulation_triangles;
	void addFog(vec3& color, float z);
	void superSampling();
	vector<tuple<int,int>> activePixels;
};
