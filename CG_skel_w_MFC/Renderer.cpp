#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"
#include <limits>


#define INDEX(width,x,y,c) (x+y*width)*3+c
#define ZINDEX(width,x,y) (x+y*width)

color white{1, 1, 1}, red{1, 0, 0}, green{0, 1, 0}, blue{0, 0, 1};
color colors[] = { white, red ,green ,blue };
extern Renderer * renderer;


Triangle::Triangle(vec3& p1_3d, vec3& p2_3d, vec3& p3_3d, vec3 rgb)
	: p1_3d(p1_3d), p2_3d(p2_3d), p3_3d(p3_3d)
{
	should_draw = true;
	shape_color = rgb;
}

Normal::Normal(vec3& p1_3d, vec3& p2_3d) :p1_3d(p1_3d), p2_3d(p2_3d)
{

}

// gives the maximum in array
float maxi(float arr[], int n) {
	float m = 0;
	for (int i = 0; i < n; ++i)
		if (m < arr[i])
		{
			m = arr[i];
		}
	return m;
}

// gives the minimum in array
float mini(float arr[], int n) {
	float m = 1;
	for (int i = 0; i < n; ++i)
		if (m > arr[i])
		{
			m = arr[i];
		}
			
	return m;
}

bool LiangBarskyClipping(vec3 point1, vec3 point2, vec3 max, vec3 min)
{
	// defining variables
	float p[7];
	p[2] = -(point2.x - point1.x);
	p[4] = (point2.x - point1.x);
	p[1] = -(point2.y - point1.y);
	p[3] = (point2.y - point1.y);
	p[5] = -(point2.z - point1.z);
	p[6] = (point2.z - point1.z);

	float q[7];
	q[2] = point1.x - min.x;
	q[4] = max.x - point1.x;
	q[1] = point1.y - min.y;
	q[3] = max.y - point1.y;
	q[5] = point1.z - min.z;
	q[6] = max.z - point1.z;

	//check for parallel lines
	if ((p[1] == 0 && q[1] < 0) || (p[2] == 0 && q[2] < 0) ||
		(p[3] == 0 && q[3] < 0) || (p[4] == 0 && q[4] < 0) ||
		(p[5] == 0 && q[5] < 0) || (p[6] == 0 && q[6] < 0))
	{
		//lines are outside of the clipping box
		return false;
	}

	float positive[7], negative[7];
	int pos_i = 1, neg_i = 1;
	positive[0] = 1;
	negative[0] = 0;

	float r[7];
	if (p[2] != 0) {
		r[2] = q[2] / p[2];
		r[4] = q[4] / p[4];
		if (p[2] < 0) {
			negative[neg_i++] = r[2]; // for negative p2, add it to negative array
			positive[pos_i++] = r[4]; // and add p4 to positive array
		}
		else {
			negative[neg_i++] = r[4]; // for negative p4, add it to negative array
			positive[pos_i++] = r[2]; // and add p2 to positive array
		}
	}
	if (p[1] != 0) {
		r[1] = q[1] / p[1];
		r[3] = q[3] / p[3];
		if (p[1] < 0) {
			negative[neg_i++] = r[1]; // for negative p1, add it to negative array
			positive[pos_i++] = r[3]; // and add p3 to positive array
		}
		else {
			negative[neg_i++] = r[3]; // for negative p3, add it to negative array
			positive[pos_i++] = r[1]; // and add p1 to positive array
		}
	}
	if (p[5] != 0) {
		r[5] = q[5] / p[5];
		r[6] = q[6] / p[6];
		if (p[5] < 0) {
			negative[neg_i++] = r[5]; // for negative p1, add it to negative array
			positive[pos_i++] = r[6]; // and add p3 to positive array
		}
		else {
			negative[neg_i++] = r[6]; // for negative p3, add it to negative array
			positive[pos_i++] = r[5]; // and add p1 to positive array
		}
	}

	vec3 out1, out2;
	float u1, u2;
	u1 = maxi(negative, neg_i); // maximum of negative array
	u2 = mini(positive, pos_i); // minimum of positive array

	if (u1 > u2) { // reject
		// Line is outside the clipping window
		return false;
	}

	return true;

	//// computing new points
	//out1.x = point1.x + p[4] * u1;
	//out1.y = point1.y + p[3] * u1;

	//out2.x = point1.x + p[4] * u2;
	//out2.y = point1.y + p[3] * u2;

}

Renderer::Renderer() :m_width(512), m_height(512)
{
	InitOpenGLRendering();
	CreateBuffers(512,512);
}
Renderer::Renderer(int width, int height) :m_width(width), m_height(height)
{
	InitOpenGLRendering();
	CreateBuffers(width,height);
}

Renderer::~Renderer(void)
{
}

void Renderer::transformToScreen(vec2& vec)
{
	vec.x = floorf((m_width / 2) * (vec.x + 1));
	vec.y = floorf((m_height / 2) * (vec.y + 1));
}


void Renderer::DrawPixel(int x, int y, vec3& rgb)
{
	if (x < 1 || x >= m_width || y < 1 || y >= m_height)
	{
		return;
	}

	m_outBuffer[INDEX(m_width, x, y, 0)] = rgb.x;//red
	m_outBuffer[INDEX(m_width, x, y, 1)] = rgb.y;//green
	m_outBuffer[INDEX(m_width, x, y, 2)] = rgb.z;//blue
}

void RasterizeArrangeVeritcs(vec2& ver1, vec2& ver2, bool byX = true)
{
	vec2 minVer, maxVer;
	if (byX) 
	{
		minVer = (ver1.x < ver2.x) ? ver1 : ver2;
		maxVer = (ver1.x < ver2.x) ? ver2 : ver1;
		ver1 = minVer;
		ver2 = maxVer;
	}
	else
	{
		minVer = (ver1.y < ver2.y) ? ver1 : ver2;
		maxVer = (ver1.y < ver2.y) ? ver2 : ver1;
		ver1 = minVer;
		ver2 = maxVer;
	}

}

void Normal::Rasterize()
{
	RasterizeLine(p1, p2);
}

void Triangle::Rasterize()
{
	RasterizeLine(p1, p2);
	RasterizeLine(p2, p3);
	RasterizeLine(p3, p1);
}

float Triangle::GetZ(int x, int y)
{
	float A1, A2, A3;
	vec2 cord = vec2(x, y);
	vec3 vec0 = vec3(p1 - cord, 0);
	vec3 vec1 = vec3(p2 - cord, 0);
	vec3 vec2 = vec3(p3 - cord, 0);
	A1 = length(cross(vec1, vec2));
	A2 = length(cross(vec2, vec0));
	A3 = length(cross(vec0, vec1));
	float normalFactor = A1 + A2 + A3;
	return (A1 * C_p1_3d.z + A2 * C_p2_3d.z + A3 * C_p3_3d.z) / normalFactor;
}

void Triangle::UpdateShape()
{
	Xranges.clear();
	C_p1_3d = renderer->Transform(p1_3d);
	C_p2_3d = renderer->Transform(p2_3d);
	C_p3_3d = renderer->Transform(p3_3d);
	p1 = renderer->vec3ToVec2(C_p1_3d);
	p2 = renderer->vec3ToVec2(C_p2_3d);
	p3 = renderer->vec3ToVec2(C_p3_3d);
	yMax = max(max(p1.y, p2.y), p3.y);
	yMin = min(min(p1.y, p2.y), p3.y);

	vec4 tmp[4];
	mat4 proj = renderer->GetProjection();
	tmp[1] = proj * vec4(C_p1_3d);
	tmp[2] = proj * vec4(C_p2_3d);
	tmp[3] = proj * vec4(C_p3_3d);

	if (!LiangBarskyClipping(vec3(tmp[1].x, tmp[1].y, tmp[1].z), vec3(tmp[2].x, tmp[2].y, tmp[2].z), vec3(tmp[1].w), vec3(-tmp[1].w)) ||
		!LiangBarskyClipping(vec3(tmp[1].x, tmp[1].y, tmp[1].z), vec3(tmp[3].x, tmp[3].y, tmp[3].z), vec3(tmp[1].w), vec3(-tmp[1].w)) ||
		!LiangBarskyClipping(vec3(tmp[3].x, tmp[3].y, tmp[3].z), vec3(tmp[2].x, tmp[2].y, tmp[2].z), vec3(tmp[2].w), vec3(-tmp[2].w)))
	{
		this->should_draw = false;
	}
	else
	{
		this->should_draw = true;
	}
}


float Normal::GetZ(int x, int y)
{
	const float t = length(vec2(x, y) - p1) / length(p2 - p1);
	return C_p1_3d.z * t + (1-t) * C_p2_3d.z;
}

void Normal::UpdateShape()
{
	Xranges.clear();
	C_p1_3d = renderer->NormTransform(p1_3d);
	C_p2_3d = renderer->NormTransform(p2_3d);
	p1 = renderer->vec3ToVec2(C_p1_3d);
	p2 = renderer->vec3ToVec2(C_p2_3d);
	yMax = max(p1.y, p2.y);
	yMin = min(p1.y, p2.y);
}

void Shape::RasterizeLine(vec2 ver1, vec2 ver2)
{
	float dX = ver2.x - ver1.x;
	float dY = ver2.y - ver1.y;
	if (abs(dY) < abs(dX) )
	{
		if (dX * dY > 0)
		{
			RasterizeRegular(ver1, ver2);
		}
		else
		{
			RasterizeRegularNegetive(ver1, ver2);
		}
	}
	else
	{
		if (dX * dY >= 0)
		{
			RasterizeBig(ver1, ver2);
		}
		else
		{
			RasterizeBigNegetive( ver2,ver1);
		}
	}


}

void Shape::RasterizeRegular(vec2& ver1, vec2& ver2)
{
	RasterizeArrangeVeritcs(ver1, ver2);
	int x = ver1.x;
	int y = ver1.y;
	int dX = ver2.x - ver1.x;
	int dY = ver2.y - ver1.y;
	int d = 2*dY - dX;
	int dE = 2*dY;
	int dNE = 2 * dY - 2 * dX;
	UpdateLimits(x, y);
	for (int i = x; i < (int)ver2.x; i++)
	{
		if (d < 0)
		{
			d += dE;
		}
		else
		{
			y++;
			d += dNE;
		}
		UpdateLimits(i, y);
	}
}

void Shape::RasterizeBig(vec2& ver1, vec2& ver2)
{
	RasterizeArrangeVeritcs(ver1, ver2,false);
	int x = ver1.y;
	int y = ver1.x;
	int dX = ver2.y - ver1.y;
	int dY = ver2.x - ver1.x;
	int d = 2 * dY - dX;
	int dE = 2 * dY;
	int dNE = 2 * dY - 2 * dX;
	UpdateLimits(y, x);
	for (int i = x; i < (int)ver2.y; i++)
	{
		if (d < 0)
		{
			d += dE;
		}
		else
		{
			y++;
			d += dNE;
		}
		UpdateLimits(y, i);
	}
}


void Shape::RasterizeRegularNegetive(vec2& ver1, vec2& ver2)
{
	RasterizeArrangeVeritcs(ver1, ver2);
	int x = ver1.x;
	int y = -ver1.y;
	int dX = ver2.x - ver1.x;
	int dY = -ver2.y - (-ver1.y);
	int d = 2 * dY - dX;
	int dE = 2 * dY;
	int dNE = 2 * dY - 2 * dX;
	UpdateLimits(x, -y);
	for (int i = x; i < (int)ver2.x; i++)
	{
		if (d < 0)
		{
			d += dE;
		}
		else
		{
			y++;
			d += dNE;
		}
		UpdateLimits(i, -y);
	}
}


void Shape::RasterizeBigNegetive(vec2& ver1, vec2& ver2)
{
	RasterizeArrangeVeritcs(ver1, ver2,false);
	int x = ver1.y;
	int y = -ver1.x;
	int dX = ver2.y - ver1.y;
	int dY = -ver2.x - (-ver1.x);
	int d = 2 * dY - dX;
	int dE = 2 * dY;
	int dNE = 2 * dY - 2 * dX;
	UpdateLimits(-y, x);
	for (int i = x; i < (int)ver2.y; i++)
	{
		if (d < 0)
		{
			d += dE;
		}
		else
		{
			y++;
			d += dNE;
		}
		UpdateLimits(-y, i);
	}
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width = width;
	m_height = height;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3 * m_width * m_height];
	m_zbuffer = new float[m_width * m_height];
	//ClearColorBuffer();
	ClearDepthBuffer();
}

void Renderer::SetDemoBuffer()
{
	//vertical line
	for(int i=0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width,256,i,0)]=1;	m_outBuffer[INDEX(m_width,256,i,1)]=0;	m_outBuffer[INDEX(m_width,256,i,2)]=0;

	}
	//horizontal line
	for(int i=0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width,i,256,0)]=1;	m_outBuffer[INDEX(m_width,i,256,1)]=0;	m_outBuffer[INDEX(m_width,i,256,2)]=1;

	}
}


bool CustomCompareYmin::operator()( Shape* shape1,  Shape* shape2) const
{
	return shape1->yMin < shape2->yMin;
}

bool CustomCompareYMax::operator()( Shape* shape1,  Shape* shape2) const
{
	return shape1->yMax < shape2->yMax;
}

void Renderer::DrawTriangles(vector<Triangle>* triangles,vector<Normal>* verticesNormals,
			vector<Normal>* facesNormals,
			 vector<vec3>* boundBoxVertices, GLfloat proportionalValue)
{
	Triangle* triangle;
	Normal* normal;
	vec4 tmp[4];
	
	for (int i = 0; i < triangles->size(); i++)
	{

		//curColor = white;

		triangle = &(triangles->at(i));
		triangle->UpdateShape();
		yMax = max(yMax, triangle->yMax);
		yMin = min(yMin, triangle->yMin);
		triangle->Rasterize();
		shapesSet.insert(triangle);
	}

	if ((verticesNormals != NULL) && (isShowVerticsNormals))
	{
		//curColor = red;
		// iterate over all vertices to draw vertices normals
		for (int i = 0; i < verticesNormals->size(); i++)
		{
				normal = &(verticesNormals->at(i));
				normal->UpdateShape();
				yMax = max(yMax, normal->yMax);
				yMin = min(yMin, normal->yMin);
				normal->Rasterize();
				shapesSet.insert(normal);
		}	
	}

	if ((facesNormals != NULL)  && (isShowFacesNormals))
	{
		//curColor = red;
		// iterate over all faces to draw faces normals
		for (int i = 0; i < facesNormals->size(); i++)
		{
			normal = &(facesNormals->at(i));
			normal->UpdateShape();
			yMax = max(yMax, normal->yMax);
			yMin = min(yMin, normal->yMin);
			normal->Rasterize();
			shapesSet.insert(normal);
		}
	}

	if (isShowBoundBox)
	{
		DrawBoundingBox(boundBoxVertices);
	}
}

void Renderer::DrawBoundingBox(const vector<vec3>* vertices)
{
	//vec2 rectangle[4];
	//if (!shouldDrawModel(vertices))
	//{
	//	return;
	//}
	//// iterate over all vertices to draw rectangles
	//for (auto it = vertices->begin(); it != vertices->end(); ++it)
	//{
	//	curColor = white;
	//	rectangle[0] = vec3ToVec2(Transform(*it));
	//	it++;
	//	rectangle[1] = vec3ToVec2(Transform(*it));
	//	it++;
	//	rectangle[2] = vec3ToVec2(Transform(*it));
	//	it++;
	//	rectangle[3] = vec3ToVec2(Transform(*it));

	//	RasterizeLine(rectangle[0], rectangle[1]);
	//	RasterizeLine(rectangle[1], rectangle[2]);
	//	RasterizeLine(rectangle[2], rectangle[3]);
	//	RasterizeLine(rectangle[3], rectangle[0]);

	//}
}

//check if part of the object is inside the camera view frame	
bool Renderer::shouldDrawModel(const vector<vec3>* boundingBox)
{
	vec4 maxBounds = cProjection * cTransform * vec4(Transform(vec3(boundingBox->at(1).x, boundingBox->at(0).y, boundingBox->at(0).z)));
	vec4 minBounds = cProjection * cTransform * vec4(Transform(vec3(boundingBox->at(0).x, boundingBox->at(2).y, boundingBox->at(4).z)));
	//check max
	
	if (minBounds.x > minBounds.w || minBounds.y > minBounds.w || minBounds.z > minBounds.w)
	{
		return false;
	}

	if (maxBounds.x < -maxBounds.w || maxBounds.y < -maxBounds.w || maxBounds.z < -maxBounds.w)
	{
		return false;
	}

	return true;
}
vec2 Renderer::vec3ToVec2(const vec3& ver)
{
	vec4 tempVec = vec4(ver);
	tempVec = this->cProjection *  tempVec;

	vec2 point = vec2(tempVec.x / tempVec.w, tempVec.y / tempVec.w);
	transformToScreen(point);
	return point;
}

mat4 Renderer::GetProjection()
{
	return cProjection;
}

vec3 Renderer::Transform(const vec3& ver)
{
	vec4 tempVec = cTransform * oTransform * vec4(ver);
	return vec3(tempVec.x / tempVec.w, tempVec.y / tempVec.w, tempVec.z / tempVec.w);
}

vec3 Renderer::NormTransform(const vec3& ver)
{
	vec4 tempVec = vec4(ver);
	tempVec.w = 0;
	tempVec = cTransform * nTransform * tempVec;
	return normalize(vec3(tempVec.x, tempVec.y, tempVec.z));
}

void Renderer::ConfigureRenderer(const mat4& projection, const mat4& transform, bool isDrawVertexNormal, bool isDrawFaceNormal, bool isDrawBoundBox)
{
	cTransform = mat4(transform);
	cProjection = mat4(projection);
	isShowVerticsNormals = isDrawVertexNormal;
	isShowFacesNormals = isDrawFaceNormal;
	isShowBoundBox = isDrawBoundBox;
	renderer = this;
	yMin = m_height;
	yMax = 0;
	shapesSet.clear();

}	



/////////////////////////////////////////////c////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	a = glGetError();
	glGenTextures(1, &gScreenTex);
	a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[]={
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1
	};
	const GLfloat tex[]={
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1};
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc)+sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
	glUseProgram( program );
	GLint  vPosition = glGetAttribLocation( program, "vPosition" );

	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0,
		0 );

	GLint  vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		(GLvoid *) sizeof(vtc) );
	glProgramUniform1i( program, glGetUniformLocation(program, "texture"), 0 );
	a = glGetError();
}

void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_width, m_height);
}

void Renderer::SwapBuffers()
{

	int a = glGetError();
	glActiveTexture(GL_TEXTURE0);
	a = glGetError();
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	a = glGetError();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	glutSwapBuffers();
	a = glGetError();
}

void Renderer::ClearColorBuffer()
{
	memset(m_outBuffer, 0, (sizeof(float) * 3 * m_width * m_height));
}

void Renderer::ClearDepthBuffer()
{
	//memset(m_zbuffer, numeric_limits<float>::infinity(), (sizeof(float) * m_width ));
}


void Renderer::ResizeBuffers(int width, int height)
{
	delete m_outBuffer;
	delete m_zbuffer;
	m_width = width;
	m_height = height;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3 * m_width * m_height];
	m_zbuffer = new float[m_width * m_height];
}


void Renderer::SetObjectMatrices(const mat4& oTransform, const mat4& nTransform)
{
	this->oTransform = oTransform;
	this->nTransform = nTransform;
}


void Renderer::ZBufferScanConvert()
{

	multiset<Shape*, CustomCompareYMax> A;
	Normal* dummy = new Normal();
	float z;
	int minX, maxX;
	float i;

	//A.insert(shapesSet.begin(), shapesSet.end());

	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			m_zbuffer[ZINDEX(m_width, x, y)] = numeric_limits<float>::infinity();
		}

	}

	for (auto it = shapesSet.begin(); it != shapesSet.end(); ++it)
	{
		if ((*it)->should_draw == false)
		{
			continue;
		}
		yMin = max(0, (*it)->yMin);
		yMax = min((*it)->yMax, m_height - 1);
		for (int y = yMin; y < yMax; y++)
		{
			
	/*		dummy->yMin = y;
			dummy->yMax = y;*/
			//A.insert(shapesSet.begin(), shapesSet.upper_bound(dummy));
			//A.erase(A.begin(), A.upper_bound(dummy));


			
			minX = max((*it)->Xranges[y].minX,0);
			maxX = min((*it)->Xranges[y].maxX, m_width -1);
			for (int i = minX; i <= maxX; i++)
			{
				z = abs((*it)->GetZ(i, y));
				if (z < m_zbuffer[ZINDEX(m_width, i, y)])
				{
					m_zbuffer[ZINDEX(m_width, i, y)] = z;
					DrawPixel(i, y,(*it)->shape_color);

				}
			}


		}
	}

	//curColor = red;

}

void Shape::UpdateLimits(int x, int y)
{
	if ((Xranges.find(y)) == Xranges.end()) // x values starts from 1
	{
		Xranges[y] = Range(x, x);
	}
	else if (x < Xranges[y].minX)
	{
		Xranges[y].minX = x;
	}
	else if (x > Xranges[y].maxX)
	{
		Xranges[y].maxX = x;
	}
}
