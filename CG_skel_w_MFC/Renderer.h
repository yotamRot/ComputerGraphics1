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

class Range
{
public:
	int minX;
	int maxX;

	Range() = default;
	Range(int  minX, int maxX) :minX(minX), maxX(maxX) {};
	~Range() = default;

private:

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

	int yMin;
	int yMax;
	int shapeColorIndex;
	bool should_draw;
	map<int, Range> Xranges;
	void UpdateLimits(int x, int y);

};


class Normal : public virtual Shape
{
public:
	Normal() = default;

	 ~Normal()=default;
	
	vec3 p1_3d;
	vec3 p2_3d;

	vec2 p1;
	vec2 p2;
	vec3 C_p1_3d;
	vec3 C_p2_3d;

	void Rasterize() override;
	float GetZ(int x, int y) override;
	void  UpdateShape() override;


	Normal(vec3& p1_3d, vec3& p2_3d);
};

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

	

	void Rasterize() override;
	float GetZ(int x, int y) override ;
	void  UpdateShape() override;

	Triangle(vec3& p1_3d, vec3& p2_3d, vec3& p3_3d);
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

	multiset<Shape*, CustomCompareYmin> shapesSet;


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
	void Renderer::DrawTriangles(vector<Triangle>* triangles, vector<Normal>* verticesNormals,
		vector<Normal>* facesNormals,
		vector<vec3>* boundBoxVertices, GLfloat proportionalValue);
	void DrawBoundingBox(const vector<vec3>* vertices);
	void DrawPixel(int x, int y);
	void ConfigureRenderer(const mat4& projection, const mat4& cTransform ,
		bool isDrawVertexNormal, bool isDrawFaceNormal, bool isDrawBoundBox);
	void SetObjectMatrices(const mat4& oTransform, const mat4& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
	void ResizeBuffers(int width, int height);
	void ScanConvert();
	void ZBufferScanConvert();
	vec2 vec3ToVec2(const vec3& ver);
	vec3 Transform(const vec3& ver);
	vec3 NormTransform(const vec3& ver);
	mat4 GetProjection();
	int yMin;
	int yMax;

};
