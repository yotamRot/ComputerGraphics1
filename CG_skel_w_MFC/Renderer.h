#pragma once
#include <vector>
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
	mat4 projection;

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
	void DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* normals=NULL);
	void SetCameraTransform(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
};
