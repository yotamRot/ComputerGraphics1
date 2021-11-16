#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c

struct color{
	int red, green, blue;
} white{ 1,1,1 }, red{ 1,0,0 }, green{ 0, 1, 0 }, blue{ 0,0,1 };

color curColor = red;

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
	vec.x = (m_width / 2) * (vec.x + 1);
	vec.y = ((m_height / 2) * (vec.y + 1) / 2);
}



void Renderer::DrawPixel(int x, int y)
{
	if (x < 1 || x >= m_width || y < 1 || y >= m_height)
		{
			return;
		}
	m_outBuffer[INDEX(m_width, x, y, 0)] = curColor.red;	m_outBuffer[INDEX(m_width, x, y, 1)] = curColor.green;	m_outBuffer[INDEX(m_width, x, y, 2)] = curColor.blue;
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

void Renderer::RasterizeLine(vec2 ver1, vec2 ver2)
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

void Renderer::RasterizeRegular(vec2& ver1, vec2& ver2)
{
	RasterizeArrangeVeritcs(ver1, ver2);
	int x = ver1.x;
	int y = ver1.y;
	int dX = ver2.x - ver1.x;
	int dY = ver2.y - ver1.y;
	int d = 2*dY - dX;
	int dE = 2*dY;
	int dNE = 2 * dY - 2 * dX;
	DrawPixel(x, y);
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
		DrawPixel(i, y);
	}
}

void Renderer::RasterizeBig(vec2& ver1, vec2& ver2)
{
	RasterizeArrangeVeritcs(ver1, ver2,false);
	int x = ver1.y;
	int y = ver1.x;
	int dX = ver2.y - ver1.y;
	int dY = ver2.x - ver1.x;
	int d = 2 * dY - dX;
	int dE = 2 * dY;
	int dNE = 2 * dY - 2 * dX;
	DrawPixel(y, x);
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
		DrawPixel(y, i);
	}
}


void Renderer::RasterizeRegularNegetive(vec2& ver1, vec2& ver2)
{
	RasterizeArrangeVeritcs(ver1, ver2);
	int x = ver1.x;
	int y = -ver1.y;
	int dX = ver2.x - ver1.x;
	int dY = -ver2.y - (-ver1.y);
	int d = 2 * dY - dX;
	int dE = 2 * dY;
	int dNE = 2 * dY - 2 * dX;
	DrawPixel(x, -y);
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
		DrawPixel(i, -y);
	}
}


void Renderer::RasterizeBigNegetive(vec2& ver1, vec2& ver2)
{
	RasterizeArrangeVeritcs(ver1, ver2,false);
	int x = ver1.y;
	int y = -ver1.x;
	int dX = ver2.y - ver1.y;
	int dY = -ver2.x - (-ver1.x);
	int d = 2 * dY - dX;
	int dE = 2 * dY;
	int dNE = 2 * dY - 2 * dX;
	DrawPixel(-y, x);
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
		DrawPixel(-y, i);
	}
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width = width;
	m_height = height;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3*m_width*m_height];
	m_zbuffer = new float[m_width * m_height];
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

void Renderer::DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* verticesNormals,
			const vector<vec3>* facesCenters, const vector<vec3>* facesNormals, const vector<vec3>* boundBoxVertices)
{
	vec2 triangle [3];
	vec2 normal [2];
	vec3 normalStart;
	vec3 normalEnd;
	// iterate over all vertices to draw triangles
	for (int i = 0; i < vertices->size(); i++)
	{
		curColor = white;
		triangle[0] = vec3ToVec2(Transform(vertices->at(i)));
		i++;
		triangle[1] = vec3ToVec2(Transform(vertices->at(i)));
		i++;
		triangle[2] = vec3ToVec2(Transform(vertices->at(i)));

		RasterizeLine(triangle[0], triangle[1]);
		RasterizeLine(triangle[1], triangle[2]);
		RasterizeLine(triangle[2], triangle[0]);
	}

	if ((verticesNormals != NULL) && (isShowVerticsNormals))
	{
		curColor = red;
		// iterate over all vertices to draw vertices normals
		for (int i = 0; i < vertices->size(); i++)
		{
				normalStart = Transform(vertices->at(i));
				normalEnd = normalStart + 0.1 * NormTransform(verticesNormals->at(i));
				RasterizeLine(vec3ToVec2(normalStart), vec3ToVec2(normalEnd));
		}	
	}

	if ((facesCenters != NULL) && (facesCenters != NULL) && (isShowFacesNormals))
	{
		curColor = red;
		// iterate over all faces to draw faces normals
		for (int i = 0; i < facesCenters->size(); i++)
		{
			normalStart = Transform(facesCenters->at(i));
			normalEnd = normalStart + 0.1 * NormTransform(facesNormals->at(i));
			RasterizeLine(vec3ToVec2(normalStart), vec3ToVec2(normalEnd));
		}
		curColor = white;
	}
	if (isShowBoundBox)
	{
		DrawRectangles(boundBoxVertices);
	}
}

void Renderer::DrawRectangles(const vector<vec3>* vertices, const vector<vec3>* facesCenters, const vector<vec3>* facesNormals)
{
	vec2 rectangle[4];
	vec2 normal[2];
	vec3 start;
	vec3 end;

	// iterate over all vertices to draw rectangles
	for (auto it = vertices->begin(); it != vertices->end(); ++it)
	{
		rectangle[0] = vec3ToVec2(Transform(*it));
		it++;
		rectangle[1] = vec3ToVec2(Transform(*it));
		it++;
		rectangle[2] = vec3ToVec2(Transform(*it));
		it++;
		rectangle[3] = vec3ToVec2(Transform(*it));

		RasterizeLine(rectangle[0], rectangle[1]);
		RasterizeLine(rectangle[1], rectangle[2]);
		RasterizeLine(rectangle[2], rectangle[3]);
		RasterizeLine(rectangle[3], rectangle[0]);

	}

	if ((facesCenters != NULL) && (facesCenters != NULL) && (isShowFacesNormals))
	{
		curColor = red;
		// iterate over all faces to draw faces normals
		for (int i = 0; i < facesCenters->size(); i++)
		{
			start = Transform(facesCenters->at(i));
			end = Transform(facesCenters->at(i)) + 0.1 * NormTransform(facesNormals->at(i));
			RasterizeLine(vec3ToVec2(start), vec3ToVec2(end));
		}
		curColor = white;
	}


}

vec2 Renderer::vec3ToVec2(const vec3& ver)
{
	vec4 tempVec = vec4(ver);
	tempVec = this->cProjection * this->cTransform *  tempVec;

	vec2 point = vec2(tempVec.x / tempVec.w, tempVec.y / tempVec.w);
	transformToScreen(point);
	return point;
}

vec3 Renderer::Transform(const vec3& ver)
{
	vec4 tempVec = oTransform * vec4(ver);
	return vec3(tempVec.x / tempVec.w, tempVec.y / tempVec.w, tempVec.z / tempVec.w);
}

vec3 Renderer::NormTransform(const vec3& ver)
{
	vec4 tempVec = vec4(ver);
	tempVec.w = 0;
	tempVec = nTransform * tempVec;
	return normalize(vec3(tempVec.x, tempVec.y, tempVec.z));
}

void Renderer::ConfigureRenderer(const mat4& projection, const mat4& transform, bool isDrawVertexNormal, bool isDrawFaceNormal, bool isDrawBoundBox)
{
	cTransform = mat4(transform);
	cProjection = mat4(projection);
	isShowVerticsNormals = isDrawVertexNormal;
	isShowFacesNormals = isDrawFaceNormal;
	isShowBoundBox = isDrawBoundBox;
}



/////////////////////////////////////////////////////
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
	memset(m_zbuffer, 0, (sizeof(float) * m_width * m_height));
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