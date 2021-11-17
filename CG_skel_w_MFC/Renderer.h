#pragma once
#include <vector>
#include <set>
#include <tuple>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"

using namespace std;
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

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();
	void RasterizeLine(vec2 verMin, vec2 verMax);
	void RasterizeBigNegetive(vec2& ver1, vec2& ver2);
	void RasterizeRegularNegetive(vec2& ver1, vec2& ver2);
	void RasterizeBig(vec2& ver1, vec2& ver2);
	void RasterizeRegular(vec2& ver1, vec2& ver2);
	void DrawPixel(int x, int y);
	void transformToScreen(vec2& vec);
	vec2 vec3ToVec2(const vec3& ver);
	vec3 NormTransform(const vec3& ver);
	vec3 Transform(const vec3& ver);



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
	void DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* VerticesNormals=NULL,
		const vector<vec3>*facesCenters = NULL,const vector<vec3>* facesNormals = NULL,
		const vector<vec3>* boundBoxVertices = NULL, GLfloat proportionalValue = 0);
	void DrawRectangles(const vector<vec3>* vertices, const vector<vec3>* facesCenters = NULL,
		const vector<vec3>* facesNormals = NULL);
	void ConfigureRenderer(const mat4& projection, const mat4& cTransform ,
		bool isDrawVertexNormal, bool isDrawFaceNormal, bool isDrawBoundBox);
	void SetObjectMatrices(const mat4& oTransform, const mat4& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
	void ResizeBuffers(int width, int height);
};
