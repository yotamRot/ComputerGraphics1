#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"
#include <limits>
#include <chrono>


#define INDEX(width,x,y,c) (x+y*width)*3+c
#define ZINDEX(width,x,y) (x+y*width)
#define DRAW_OPEN_MODELS 1
#define ALPHA 2


#define SUPER_SAMPLE_FACTOR 2
#define SUPER_SAMPLE_AVREGE 0.25

#define EPSILON 0.01

enum Axis
{
	X,
	Y,
	Z
};

extern Renderer * renderer;

Normal invalid_normal = Normal(vec3(0, 0), vec3(0, 0), false, vertix_normal, 0, false);

Triangle::Triangle(vec3& p1_3d, vec3& p2_3d, vec3& p3_3d, vec3 rgb, bool is_light,
	Normal& normal, Normal& p1_normal, Normal& p2_normal, Normal& p3_normal)
	: p1_3d(p1_3d), p2_3d(p2_3d), p3_3d(p3_3d) ,normal(normal),
	p1_normal(p1_normal), p2_normal(p2_normal) , p3_normal(p3_normal)
{
	should_draw = true;
	shape_color = rgb;
	this->is_light = is_light;
	x_max = NULL;
	x_min = NULL;
}

Line::Line(vec3& p1_3d, vec3& p2_3d, bool is_light) :p1_3d(p1_3d), p2_3d(p2_3d)
{
	shape_color = BLUE;
	should_draw = true;
	this->is_light = is_light;
	x_max = NULL;
	x_min = NULL;
}

Normal::Normal(vec3& p1_3d, vec3& p2_3d, bool is_light, NormalKind normal_kind, float normal_size, bool is_valid)
	: Line(p1_3d, p2_3d, is_light),normal_kind(normal_kind), normal_size(normal_size), is_valid(is_valid)
{
	shape_color = RED;
	x_max = NULL;
	x_min = NULL;
}

Light::Light(int modelId, Model* model) : modelId(modelId), model(model), La(0.3), Ld(0.3), Ls(0.3), type(POINT_SOURCE), light_color(vec3(1,1,1))
{
}

vec3 Light::GetL()
{
	return vec3(La, Ld, Ls);
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



bool LiangBarskyClipping(vec3& point1, vec3& point2, vec3& max, vec3& min)
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

	//check if line outside	
	if ((p[1] == 0 && q[1] < 0) || (p[2] == 0 && q[2] < 0) ||
		(p[3] == 0 && q[3] < 0) || (p[4] == 0 && q[4] < 0) ||
		(p[5] == 0 && q[5] < 0) || (p[6] == 0 && q[6] < 0))
	{
		//line is outside of the clipping box
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


}



Renderer::Renderer() :m_width(512), m_height(512), extended_m_height(512 * SUPER_SAMPLE_FACTOR), extended_m_width(512 * SUPER_SAMPLE_FACTOR), is_wire_frame(false)
{
	InitOpenGLRendering();
	CreateBuffers(512,512);
}
Renderer::Renderer(int width, int height) :m_width(width), m_height(height) , extended_m_height(512 * SUPER_SAMPLE_FACTOR), extended_m_width(512 * SUPER_SAMPLE_FACTOR), is_wire_frame(false)
{
	InitOpenGLRendering();
	CreateBuffers(width,height);
}

Renderer::~Renderer(void)
{
}

void Renderer::transformToScreen(vec2& vec)
{
	vec.x = floorf((cur_width / 2) * (vec.x + 1));
	vec.y = floorf((cur_height / 2) * (vec.y + 1));
}


void Renderer::DrawPixel(int x, int y, vec3& rgb)
{
	if (x < 1 || x >= cur_width || y < 1 || y >= cur_height)
	{
		return;
	}

	m_local_color_buffer[INDEX(cur_width, x, y, 0)] = rgb.x;//red
	m_local_color_buffer[INDEX(cur_width, x, y, 1)] = rgb.y;//green
	m_local_color_buffer[INDEX(cur_width, x, y, 2)] = rgb.z;//blue
}

vec3 Renderer::GetPixel(int x, int y)
{
	return vec3(m_local_color_buffer[INDEX(cur_width, x, y, 0)],
		m_local_color_buffer[INDEX(cur_width, x, y, 1)],
		m_local_color_buffer[INDEX(cur_width, x, y, 2)]);
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

void Line::Rasterize()
{
	p1_2d = renderer->vec4ToVec2(P_p1_4d);
	p2_2d = renderer->vec4ToVec2(P_p2_4d);

	yMax = max(p1_2d.y, p2_2d.y);
	yMin = min(p1_2d.y, p2_2d.y);

	renderer->yMin = min(yMin, renderer->yMin);
	renderer->yMax = max(yMin, renderer->yMax);

	if (should_draw)
	{
		if (x_min != NULL)
		{
			delete x_min;
		}
		if (x_max != NULL)
		{
			delete x_max;
		}

		x_max = new int[yMax - yMin + 1]();
		x_min = new int[yMax - yMin + 1]();
		std::fill_n(x_max, yMax - yMin + 1, -1);
		std::fill_n(x_min, yMax - yMin + 1, -1);
		RasterizeLine(p1_2d, p2_2d);
	}
	
}

Shape::~Shape()
{
	if (this->x_max != NULL)
	{
		delete[] x_max;
	}

	if (this->x_min != NULL)
	{
		delete[] x_min;
	}

}

Shape::Shape()
{
	x_max = x_min = NULL;
}

Shape::Shape(const Shape& a)
{
	x_max = x_min = NULL;
	yMin = a.yMin;
	yMax = a.yMin;
	shape_color = a.shape_color;
	should_draw = a.should_draw;
	is_light = a.is_light;
}



void Triangle::Rasterize()
{
	p1_2d = renderer->vec4ToVec2(P_p1_4d);
	p2_2d = renderer->vec4ToVec2(P_p2_4d);
	p3_2d = renderer->vec4ToVec2(P_p3_4d);
	yMax = max(max(p1_2d.y, p2_2d.y), p3_2d.y);
	yMin = min(min(p1_2d.y, p2_2d.y), p3_2d.y);
	renderer->yMin = min(yMin, renderer->yMin);
	renderer->yMax = max(yMax, renderer->yMax);
	x_max = new int[yMax - yMin + 1]();
	x_min = new int[yMax - yMin + 1]();
	std::fill_n(x_max, yMax - yMin + 1, -1);
	std::fill_n(x_min, yMax - yMin + 1, -1);

	RasterizeLine(p1_2d, p2_2d);
	RasterizeLine(p2_2d, p3_2d);
	RasterizeLine(p3_2d, p1_2d);
}

vec3 Triangle::GetCoordinates(int x, int y)
{
	float A1, A2, A3;
	vec2 cord = vec2(x, y);
	vec3 vec0 = vec3(p1_2d - cord, 0);
	vec3 vec1 = vec3(p2_2d - cord, 0);
	vec3 vec2 = vec3(p3_2d - cord, 0);
	A1 = fabs(cross(vec1, vec2).z);
	A2 = fabs(cross(vec2, vec0).z);
	A3 = fabs(cross(vec0, vec1).z);
	float normalFactor = A1 + A2 + A3;
	if (normalFactor == 0)
	{
		float line_len = length(p2_2d - p1_2d);
		if (line_len == 0)
		{
			return C_p1_3d;
		}
		const float t = length(cord - p1_2d) / line_len;
		return C_p1_3d * t + (1 - t) * C_p2_3d;
	}
	return (A1 * C_p1_3d + A2 * C_p2_3d + A3 * C_p3_3d) / normalFactor;
}

float Triangle::GetGouruad(vec3& C_cord)
{
	float A1, A2, A3;
	vec3 vec_0 = C_p1_3d - C_cord;
	vec3 vec_1 = C_p2_3d - C_cord;
	vec3 vec_2 = C_p3_3d - C_cord;
	A1 = length(cross(vec_1, vec_2));
	A2 = length(cross(vec_2, vec_0));
	A3 = length(cross(vec_0, vec_1));
	float normalFactor = A1 + A2 + A3;
	if (normalFactor == 0)
	{
		if (length(p2_2d - p1_2d) == 0)
		{
			return p1_illumination;
		}
		const float t = length(vec2(C_cord.x,C_cord.y) - p1_2d) / length(p2_2d - p1_2d);
		return p1_illumination * t + (1 - t) * p2_illumination;
	}
	return ((A1 * p1_illumination + A2 * p2_illumination + A3 * p3_illumination) / normalFactor);
}

vec3 Triangle::GetPhong(vec3& C_cord)
{
	float A1, A2, A3;
	vec3 vec_0 = C_p1_3d - C_cord;
	vec3 vec_1 = C_p2_3d - C_cord;
	vec3 vec_2 = C_p3_3d - C_cord;
	A1 = length(cross(vec_1, vec_2));
	A2 = length(cross(vec_2, vec_0));
	A3 = length(cross(vec_0, vec_1));
	float normalFactor = A1 + A2 + A3;
	if (normalFactor == 0)
	{
		if (length(p2_2d - p1_2d) == 0)
		{
			return p1_normal.normal_direction;
		}
		const float t = length(vec2(C_cord.x, C_cord.y) - p1_2d) / length(p2_2d - p1_2d);
		return p1_normal.normal_direction * t + (1 - t) * p2_normal.normal_direction;
	}
	return ((A1 * p1_normal.normal_direction + A2 * p2_normal.normal_direction + A3 * p3_normal.normal_direction) / normalFactor);
}


void Triangle::UpdateShape()
{
	renderer->Transform(p1_3d, C_p1_3d, P_p1_4d);
	renderer->Transform(p2_3d, C_p2_3d, P_p2_4d);
	renderer->Transform(p3_3d, C_p3_3d, P_p3_4d);

	if (normal.is_valid)
	{
		normal.UpdateShape();
	}

	if (!ShouldDraw())
	{
		return;
	}
	vec3 p1_light_direction, p2_light_direction, p3_light_direction;
	vec3 p1_camera_direction, p2_camera_direction, p3_camera_direction;
	vec3 p1_reflect_direction, p2_reflect_direction, p3_reflect_direction;
	float i, ia, id, is;
	p1_illumination = p2_illumination = p3_illumination = 0;
	if (renderer->shadow == GOURAUD)
	{
		for (auto it = renderer->lights.begin(); it != renderer->lights.end(); ++it)
		{
			
			if ((it)->type == PARALLEL_SOURCE)
			{
				normalize((it)->c_light_position);
				p1_light_direction = (it)->c_light_position;
				p2_light_direction = (it)->c_light_position;
				p3_light_direction = (it)->c_light_position;
			}
			else // Point source
			{
				p1_light_direction = normalize((it)->c_light_position - C_p1_3d);
				p2_light_direction = normalize((it)->c_light_position - C_p2_3d);
				p3_light_direction = normalize((it)->c_light_position - C_p3_3d);
			}
			p1_camera_direction = normalize(-C_p1_3d);
			p2_camera_direction = normalize(-C_p2_3d);
			p3_camera_direction = normalize(-C_p3_3d);
			p1_reflect_direction = normalize(-p1_light_direction - 2 * (max(dot(-p1_light_direction, p1_normal.normal_direction), 0)) * p1_normal.normal_direction);
			p2_reflect_direction = normalize(-p2_light_direction - 2 * (max(dot(-p2_light_direction, p2_normal.normal_direction), 0)) * p2_normal.normal_direction);
			p3_reflect_direction = normalize(-p3_light_direction - 2 * (max(dot(-p3_light_direction, p3_normal.normal_direction), 0)) * p3_normal.normal_direction);
			p1_illumination += /*ia*/ka * (it)->La +
				/*id*/kd * max(dot(p1_light_direction, p1_normal.normal_direction), 0) * (it)->Ld +
				/*is*/ks * pow(max(dot(p1_reflect_direction, p1_camera_direction), 0), ALPHA) * (it)->Ls;
			p2_illumination += /*ia*/ka * (it)->La +
				/*id*/kd * max(dot(p2_light_direction, p2_normal.normal_direction), 0) * (it)->Ld +
				/*is*/ks * pow(max(dot(p2_reflect_direction, p2_camera_direction), 0), ALPHA) * (it)->Ls;
			p3_illumination += /*ia*/ka * (it)->La +
				/*id*/kd * max(dot(p3_light_direction, p3_normal.normal_direction), 0) * (it)->Ld +
				/*is*/ks * pow(max(dot(p3_reflect_direction, p3_camera_direction), 0), ALPHA) * (it)->Ls;
		}
	}
	
	if (p1_normal.is_valid)
	{
		p1_normal.UpdateShape();
		p2_normal.UpdateShape();
		p3_normal.UpdateShape();
	}
}



void ClipLinePoints(vec3& P_p1, vec3& P_p2, vec3& C_p1, vec3& C_p2, vec4& P_to_clip , vec3& C_to_clip, Face face)
{
	vec3 p0;
	vec3 n;
	float alpha;
	switch (face)
	{
	case Down:
		n = p0 = vec3(0, -1, 0);
		break;
	case Up:
		n = p0 = vec3(0, 1, 0);
		break;
	case Left:
		n = p0 = vec3(-1, 0, 0);
		break;
	case Right:
		n = p0 = vec3(1, 0, 0);
		break;
	case Near:
		n = p0 = vec3(0, 0, -1);
		break;
	case Far:
		n = p0 = vec3(0, 0, 1);
		break;
	default:
		break;
	}

	alpha =  dot((n + p0), (p0 - P_p2)) / dot((n + p0), (P_p1 - P_p2));
	P_to_clip = (1 - alpha) * P_p2 + alpha * P_p1;
	C_to_clip = (1 - alpha) * C_p2 + alpha * C_p1;
}


int trueCounter(bool first, bool second, bool third)
{
	int counter = 0;
	if (first) { counter++; }
	if (second) { counter++; }
	if (third) { counter++; }
	return counter;
}


// num of triangles after clip 0/1/2
int Triangle::ClipFace(Triangle& triangle1, Triangle& triangle2, Face face)
{
	bool p1Cond, p2Cond, p3Cond;
	P_p1_4d = P_p1_4d;
	P_p2_4d = P_p2_4d;
	P_p3_4d = P_p3_4d;
	switch (face)
	{
	case Down:
		p1Cond = P_p1_4d.y > -P_p1_4d.w;
		p2Cond = P_p2_4d.y > -P_p2_4d.w;
		p3Cond = P_p3_4d.y > -P_p3_4d.w;
		break;
	case Up:
		p1Cond = P_p1_4d.y < P_p1_4d.w;
		p2Cond = P_p2_4d.y < P_p2_4d.w;
		p3Cond = P_p3_4d.y < P_p3_4d.w;
		break;
	case Left:
		p1Cond = P_p1_4d.x > -P_p1_4d.w;
		p2Cond = P_p2_4d.x > -P_p2_4d.w;
		p3Cond = P_p3_4d.x > -P_p3_4d.w;
		break;
	case Right:
		p1Cond = P_p1_4d.x < P_p1_4d.w;
		p2Cond = P_p2_4d.x < P_p2_4d.w;
		p3Cond = P_p3_4d.x < P_p3_4d.w;
		break;
	case Far:
		p1Cond = P_p1_4d.z < P_p1_4d.w;
		p2Cond = P_p2_4d.z < P_p2_4d.w;
		p3Cond = P_p3_4d.z < P_p3_4d.w;
		break;
	case Near:
		p1Cond = P_p1_4d.z > -P_p1_4d.w ;
		p2Cond = P_p2_4d.z > -P_p2_4d.w ;
		p3Cond = P_p3_4d.z > -P_p3_4d.w ;
		break;
	default:
		break;
	}
	int insideCounter = trueCounter(p1Cond, p2Cond, p3Cond);
	if (insideCounter == 3) // all inside no need clip
	{
		triangle1 = *this;
		return 1;
	}

	if (insideCounter == 2) // one outside need to clip and tirangulate ):
	{
		triangle1 = *this;
		triangle2 = *this;
		if (!p1Cond) // p1 is out
		{
			ClipLinePoints(getXYZ(P_p2_4d), getXYZ(P_p1_4d), C_p2_3d, C_p1_3d, triangle1.P_p1_4d, triangle1.C_p1_3d, face);
			ClipLinePoints(getXYZ(P_p3_4d), getXYZ(P_p1_4d),C_p3_3d, C_p1_3d, triangle2.P_p1_4d, triangle2.C_p1_3d, face);
			triangle2.C_p2_3d = triangle1.C_p1_3d;
			triangle2.P_p2_4d = triangle1.P_p1_4d;
		}
		else if (!p2Cond) // p2 is out
		{
			ClipLinePoints(getXYZ(P_p1_4d), getXYZ(P_p2_4d), C_p1_3d, C_p2_3d, triangle1.P_p2_4d, triangle1.C_p2_3d, face);
			ClipLinePoints(getXYZ(P_p3_4d), getXYZ(P_p2_4d), C_p3_3d, C_p2_3d, triangle2.P_p2_4d, triangle2.C_p2_3d, face);
			triangle2.C_p1_3d = triangle1.C_p2_3d;
			triangle2.P_p1_4d = triangle1.P_p2_4d;
		}
		else // p3 is out
		{
			ClipLinePoints(getXYZ(P_p2_4d), getXYZ(P_p3_4d), C_p2_3d, C_p3_3d, triangle1.P_p3_4d , triangle1.C_p3_3d , face);
			ClipLinePoints(getXYZ(P_p1_4d), getXYZ(P_p3_4d), C_p1_3d, C_p3_3d, triangle2.P_p3_4d, triangle2.C_p3_3d, face);
			triangle2.C_p2_3d = triangle1.C_p3_3d;
			triangle2.P_p2_4d = triangle1.P_p3_4d;
		}
		return 2;
	}

	if (insideCounter == 1) // one inside need to clip ):
	{
		triangle1 = *this;
		if (p1Cond) // p1 is in
		{
			ClipLinePoints(getXYZ(P_p1_4d), getXYZ(P_p2_4d), C_p1_3d, C_p2_3d, triangle1.P_p2_4d, triangle1.C_p2_3d, face);
			ClipLinePoints(getXYZ(P_p1_4d), getXYZ(P_p3_4d), C_p1_3d, C_p3_3d, triangle1.P_p3_4d, triangle1.C_p3_3d, face);
		}
		else if (p2Cond) // p2 is in
		{
			ClipLinePoints(getXYZ(P_p2_4d), getXYZ(P_p1_4d), C_p2_3d, C_p1_3d, triangle1.P_p1_4d, triangle1.C_p1_3d, face);
			ClipLinePoints(getXYZ(P_p2_4d), getXYZ(P_p3_4d), C_p2_3d, C_p3_3d, triangle1.P_p3_4d, triangle1.C_p3_3d, face);
		}
		else // (this->C_p3_3d.y < 1)
		{
			ClipLinePoints(getXYZ(P_p3_4d), getXYZ(P_p1_4d), C_p3_3d, C_p1_3d, triangle1.P_p1_4d, triangle1.C_p1_3d, face);
			ClipLinePoints(getXYZ(P_p3_4d), getXYZ(P_p2_4d), C_p3_3d, C_p2_3d, triangle1.P_p2_4d, triangle1.C_p2_3d, face);
		}
		
		return 1;
	}
	// trianle is out
	triangle1.should_draw = false;
	return 0;
}

vec3 Triangle::GetColor(vec3& C_cords, vector<Light>& lights, Shadow shadow, vec3& shape_color)
{
	float ia, id, is;
	vec3 light_direction;
	vec3 camera_direction;
	vec3 reflect_direction;
	vec3 normal;
	vec3 color;
	color = vec3(0);
	int printCounter = 0;
	switch (shadow)
	{
	case FLAT:
		normal = this->normal.normal_direction;
		break;
	case GOURAUD:		
		if (this->p1_normal.is_valid)
		{
			return (GetGouruad(C_cords) * shape_color);
		}
		else // there isn't normal for the vertex
		{
			// lets do flat shading
			normal = this->normal.normal_direction;
			break;
		}
	case PHONG:
		if (this->p1_normal.is_valid)
		{
			normal = GetPhong(C_cords);
			break;
		}
		else // there isn't normal for the vertex
		{
			// lets do flat shading
			normal = this->normal.normal_direction;
			break;
		}
	}
	for (auto it = lights.begin(); it != lights.end(); ++it)
	{
		if ((it)->type == PARALLEL_SOURCE)
		{
			light_direction = normalize((it)->c_light_position);
		}
		else // Point source
		{
			light_direction = normalize((it)->c_light_position - C_cords);
		}
		camera_direction = normalize(vec3(0) - C_cords);
		reflect_direction = normalize(-light_direction - 2 * (dot(-light_direction, normal)) * normal);
		ia = ka * (it)->La;
		id = kd * max(dot(light_direction, normal),0) * (it)->Ld;
		is = ks * pow(max(dot(reflect_direction, camera_direction),0), ALPHA) * (it)->Ls;
		color += (it)->light_color * (ia + id + is);
	}
	color = color * shape_color + shape_color * ke;

	//apply fog
	if (renderer->isFog)
	{
		renderer->addFog(color, C_cords.z);
	}
	return color;
}

void Line::Clip()
{
	should_draw = true;

	for (int face = Up; face <= Far && should_draw; face++)
	{
		ClipFace(Face(face));
	}
}

void Line::ClipFace(Face face)
{
	bool p1Cond, p2Cond, p3Cond;
	P_p1_4d = P_p1_4d;
	P_p2_4d = P_p2_4d;
	switch (face)
	{
	case Down:
		p1Cond = P_p1_4d.y > -P_p1_4d.w;
		p2Cond = P_p2_4d.y > -P_p2_4d.w;
		break;
	case Up:
		p1Cond = P_p1_4d.y < P_p1_4d.w;
		p2Cond = P_p2_4d.y < P_p2_4d.w;
		break;
	case Left:
		p1Cond = P_p1_4d.x > -P_p1_4d.w;
		p2Cond = P_p2_4d.x > -P_p2_4d.w;
		break;
	case Right:
		p1Cond = P_p1_4d.x < P_p1_4d.w;
		p2Cond = P_p2_4d.x < P_p2_4d.w;
		break;
	case Far:
		p1Cond = P_p1_4d.z < P_p1_4d.w;
		p2Cond = P_p2_4d.z < P_p2_4d.w;
		break;
	case Near:
		p1Cond = P_p1_4d.z > -P_p1_4d.w;
		p2Cond = P_p2_4d.z > -P_p2_4d.w;
		break;
	default:
		break;
	}

	int insideCounter = trueCounter(p1Cond, p2Cond, false);

	if (insideCounter == 2) // all inside no need clip
	{
		return;
	}
	else if (insideCounter == 1) // one outside need to clip 
	{
		if (!p1Cond) // p1 is out
		{
			ClipLinePoints(getXYZ(P_p1_4d), getXYZ(P_p2_4d), C_p1_3d, C_p2_3d, P_p1_4d, C_p1_3d, face);
		}
		else // p2 is out
		{
			ClipLinePoints(getXYZ(P_p2_4d), getXYZ(P_p1_4d), C_p2_3d, C_p1_3d, P_p2_4d, C_p1_3d, face);
		}
	}
	else // outside
	{
		should_draw = false;
	}
}


void Triangle::Clip()
{
	//first clip normals because we might need them clipped in new triangle
	if (normal.is_valid && renderer->isShowFacesNormals)
	{
		normal.Clip();
	}

	if (p1_normal.is_valid && renderer->isShowVerticsNormals)
	{
		p1_normal.Clip();
		p2_normal.Clip();
		p3_normal.Clip();
	}

	vector<Triangle> newTriangles;
	Triangle Newtriangle1;
	Triangle Newtriangle2;
	int curNumOfTriangles;
	int numOfTriangles;
	newTriangles.push_back(*this);
	for (int face = Up; face <= Far; face++)
	{
		curNumOfTriangles = newTriangles.size();
		for (int i = 0; i < curNumOfTriangles; ++i)
		{
			numOfTriangles = newTriangles.at(0).ClipFace(Newtriangle1, Newtriangle2, (Face)face);
			newTriangles.erase(newTriangles.begin());
			if (numOfTriangles == 1)
			{
				newTriangles.push_back(Newtriangle1);
			
			}
			else if (numOfTriangles ==2)
			{
				newTriangles.push_back(Newtriangle1);
				newTriangles.push_back(Newtriangle2);
			}
		}
	}
	renderer->triangulation_triangles.insert(renderer->triangulation_triangles.end() ,newTriangles.begin(), newTriangles.end());

}

bool Triangle::ShouldDraw()
{
#if DRAW_OPEN_MODELS
	if (dot(normal.C_p2_3d - normal.C_p1_3d, -(normal.C_p1_3d)) < 0)
	{
		should_draw = false;
		return false;
	}
#endif
	should_draw = true;
	return true;
}

vec3 Line::GetCoordinates(int x, int y)
{
	float dist = length(p2_2d - p1_2d);
	if (dist == 0)
	{
		return C_p1_3d;
	}
	const float t = length(vec2(x, y) - p1_2d) / dist;
	return C_p1_3d * t + (1-t) * C_p2_3d;
}


void Line::UpdateShape()
{
	renderer->Transform(p1_3d, C_p1_3d, P_p1_4d);
	renderer->Transform(p2_3d, C_p2_3d, P_p2_4d);
}
vec3 Line::GetColor(vec3& C_cords, vector<Light>& lights, Shadow shadow, vec3& shape_color)
{
	return 1;
}

void Normal::UpdateShape()
{
	renderer->Transform(p1_3d, C_p1_3d, P_p1_4d);
	renderer->NormTransform(p2_3d, normal_direction);
	C_p2_3d = C_p1_3d + normal_size * normal_direction;
	P_p2_4d = renderer->cProjection * vec4(C_p2_3d);
}

void Shape::RasterizeLine(vec2 ver1, vec2 ver2)
{
	float dX = ver2.x - ver1.x;
	float dY = ver2.y - ver1.y;
	if (fabs(dY) < fabs(dX) )
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

Triangle::Triangle()
{
	this->x_max = NULL;
	this->x_min = NULL;
}

Normal::Normal()
{
	this->x_max = NULL;
	this->x_min = NULL;
}

Line::Line()
{
	this->x_max = NULL;
	this->x_min = NULL;
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
	cur_width = width;
	cur_height = height;
	extended_m_height = m_height * SUPER_SAMPLE_FACTOR;
	extended_m_width = m_width * SUPER_SAMPLE_FACTOR;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3 * m_width * m_height];
	m_zbuffer = new float[m_width * m_height];
	m_super_sample_out_Buffer = new float[3 * extended_m_width * extended_m_height];
	m_super_sample_z_Buffer = new float[extended_m_width * extended_m_height];
	m_local_color_buffer = m_outBuffer;
	m_local_z_buffer = m_zbuffer;
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

void Renderer::ClipModel(vector<Triangle>* triangles, vector<Line>* boundBoxLines)
{
	RendererActions action = shouldDrawModel(boundBoxLines);
	if (action != NotDraw)
	{
		for (auto it = triangles->begin(); it != triangles->end(); ++it)
		{
			it->UpdateShape();
			if (it->should_draw)
			{
				if (action == Clip)
				{
					it->Clip();
				}
				else
				{
					triangulation_triangles.push_back(*it);
				}
			}
			
		}
	}

	if (isShowBoundBox)
	{
		DrawBoundingBox(boundBoxLines);
	}
}

void Renderer::DrawTriangles()
{	
	for (auto it = triangulation_triangles.begin(); it != triangulation_triangles.end(); ++it)
	{
		it->Rasterize();
		yMax = max(yMax, it->yMax);
		yMin = min(yMin, it->yMin);
		shapes.push_back(&(*it));

		if (isShowFacesNormals)
		{
			if (it->normal.should_draw)
			{
				yMax = max(yMax, it->normal.yMax);
				yMin = min(yMin, it->normal.yMin);
				it->normal.Rasterize();
				shapes.push_back(&(it->normal));
			}
		}

		if (isShowFacesNormals)
		{
			if (it->normal.should_draw)
			{
				yMax = max(yMax, it->normal.yMax);
				yMin = min(yMin, it->normal.yMin);
				it->normal.Rasterize();
				shapes.push_back(&(it->normal));
			}
		}

		if (isShowVerticsNormals)
		{
			if (it->p1_normal.should_draw)
			{
				yMax = max(yMax, it->p1_normal.yMax);
				yMin = min(yMin, it->p1_normal.yMin);
				it->p1_normal.Rasterize();
				shapes.push_back(&(it->p1_normal));
			}
			if (it->p2_normal.should_draw)
			{
				yMax = max(yMax, it->p2_normal.yMax);
				yMin = min(yMin, it->p2_normal.yMin);
				it->p2_normal.Rasterize();
				shapes.push_back(&(it->p2_normal));
			}
			if (it->p3_normal.should_draw)
			{
				yMax = max(yMax, it->p3_normal.yMax);
				yMin = min(yMin, it->p3_normal.yMin);
				it->p3_normal.Rasterize();
				shapes.push_back(&(it->p3_normal));
			}
		}
	}
}
		

void Renderer::DrawBoundingBox(vector<Line>* boundBoxLines)
{
	Line *line;
	// iterate over all vertices to draw rectangles
	for (int i = 0; i < boundBoxLines->size(); i++)
	{
		line = &(boundBoxLines->at(i));
		line->UpdateShape();
		line->Clip();
		if (line->should_draw)
		{
			shapes.push_back(line);
			line->Rasterize();
			yMax = max(yMax, line->yMax);
			yMin = min(yMin, line->yMin);

		}

	}
}

//check if part of the object is inside the camera view frame	
RendererActions Renderer::shouldDrawModel(const vector<Line>* boundBoxLines)
{
	//original bounds
	vec3 tranformed_orig_back_top_right = getXYZ(cProjection * cTransform * oTransform * vec4(boundBoxLines->at(4).p1_3d));
	vec3 tranformed_orig_front_bottom_left = getXYZ(cProjection * cTransform * oTransform * vec4(boundBoxLines->at(9).p2_3d));

	vec3 maxBounds = vec3(max(tranformed_orig_back_top_right.x, tranformed_orig_front_bottom_left.x), max(tranformed_orig_back_top_right.y, tranformed_orig_front_bottom_left.y), max(tranformed_orig_back_top_right.z, tranformed_orig_front_bottom_left.z));
	vec3 minBounds = vec3(min(tranformed_orig_back_top_right.x, tranformed_orig_front_bottom_left.x), min(tranformed_orig_back_top_right.y, tranformed_orig_front_bottom_left.y), min(tranformed_orig_back_top_right.z, tranformed_orig_front_bottom_left.z));

	//check max
	
	// no need to draw
	if (minBounds.x > 1 || minBounds.y > 1 || minBounds.z >1)
	{
		return NotDraw;
	}

	if (maxBounds.x < -1 || maxBounds.y < -1 || maxBounds.z < -1)
	{
		return NotDraw;
	}

	//no need to clip all inside
	if (minBounds.x > -1 && minBounds.y > -1 && minBounds.z > -1)
	{
		if (maxBounds.x < 1 && maxBounds.y < 1 && maxBounds.z < 1)
		{
			return Draw;
		}
	}

	return Clip;
}
vec2 Renderer::vec4ToVec2(const vec4& ver)
{
	vec2 point = getXY(ver);
	transformToScreen(point);
	return point;
}

vector<Light> Renderer::GetLights()
{
	return lights;
}

void Renderer::addFog(vec3& color, float z)
{
	float clamped_z = max(min(fabs(z), FOG_MAX), FOG_MIN);
	float fogFactor = (FOG_MAX - clamped_z) /(FOG_MAX - FOG_MIN);
	color = color * fogFactor + (1 - fogFactor) * FOG_COLOR;
}

void Renderer::superSampling()
{	
	vec3 avarege;
	int i, j;
	for (auto it = activePixels.begin(); it != activePixels.end(); ++it)
	{
		i = get<0>(*it);
		j = get<1>(*it);
		if (!(m_outBuffer[INDEX(m_width, i, j, 0)] || m_outBuffer[INDEX(m_width, i, j, 1)] || m_outBuffer[INDEX(m_width, i, j, 2)]))
		{
			avarege = (GetPixel(2 * i, 2 * j) + GetPixel(2 * i, 2 * j + 1)
				+ GetPixel(2 * i + 1, 2 * j) + GetPixel(2 * i + 1, 2 * j + 1)) * SUPER_SAMPLE_AVREGE;
				m_outBuffer[INDEX(m_width, i, j, 0)] = avarege.x;//red
				m_outBuffer[INDEX(m_width, i, j, 1)] = avarege.y;//green
				m_outBuffer[INDEX(m_width, i, j, 2)] = avarege.z;//blue
		}
	}
}

void Renderer::Transform(const vec3& ver ,vec3& C_ver ,vec4& P_ver)
{
	const vec4 tempVec = cTransform * oTransform * vec4(ver);
	C_ver = getXYZ(tempVec);
	P_ver = cProjection * tempVec;
}

void Renderer::NormTransform(const vec3& ver, vec3& direction)
{
	vec4 tempVec = vec4(ver);
	tempVec.w = 0;
	tempVec = cTransform * nTransform * tempVec;
	direction =  normalize(vec3(tempVec.x, tempVec.y, tempVec.z));
}

void Renderer::ConfigureRenderer(const mat4& projection, const mat4& transform,
								bool isDrawVertexNormal, bool isDrawFaceNormal, bool isWireFrame, bool isFog, bool isSuperSample,
								bool isDrawBoundBox, vector<Light> scene_lights,
								Shadow scene_shadow)
{
	cTransform = transform;
	cProjection = projection;

	isShowVerticsNormals = isDrawVertexNormal;
	isShowFacesNormals = isDrawFaceNormal;
	isShowBoundBox = isDrawBoundBox;
	this->isSuperSample = isSuperSample;
	this->isFog = isFog;
	is_wire_frame = isWireFrame;

	renderer = this;
	yMin = m_height;
	yMax = 0;
	lights = scene_lights;
	shadow = scene_shadow;
	if (isSuperSample)
	{
		m_local_color_buffer = m_super_sample_out_Buffer;
		m_local_z_buffer = m_super_sample_z_Buffer;
		cur_width = extended_m_width;
		cur_height = extended_m_height;
	}
	else
	{
		m_local_color_buffer = m_outBuffer;
		m_local_z_buffer = m_zbuffer;
		cur_width = m_width;
		cur_height = m_height;
	}
	ClearColorBuffer();
	ClearDepthBuffer();

	triangulation_triangles.clear();
	shapes.clear();
	activePixels.clear();
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
	memset(m_outBuffer, 0, (sizeof(float) * 3 * m_width * m_height));// always need to clean
	if (isSuperSample)
	{
		memset(m_local_color_buffer, 0, (sizeof(float) * 3 * cur_width * cur_height));
	}
}

void Renderer::ClearDepthBuffer()
{
	std::fill(m_local_z_buffer, m_local_z_buffer + cur_width * cur_height, std::numeric_limits<float>::infinity());
}


void Renderer::ResizeBuffers(int width, int height)
{
	delete m_outBuffer;
	delete m_zbuffer;
	delete m_super_sample_out_Buffer;
	delete m_super_sample_z_Buffer;
	CreateBuffers(width, height);
	
}


void Renderer::SetObjectMatrices(const mat4& oTransform, const mat4& nTransform)
{
	this->oTransform = oTransform;
	this->nTransform = nTransform;
}


void Renderer::ZBufferScanConvert()
{
	vec3 C_cords;
	int minX, maxX;
	int fixed_y;
	vec3 color;
	for (auto it = shapes.begin(); it != shapes.end(); ++it)
	{
		yMin = max(0, (*it)->yMin);
		yMax = min((*it)->yMax, cur_height - 1);
		for (int y = yMin; y <= yMax; y++)
		{
			fixed_y = y - yMin;
			minX = max((*it)->x_min[fixed_y],0);
			maxX = min((*it)->x_max[fixed_y], cur_width -1);
			for (int i = minX; i <= maxX; i++)
			{
				C_cords =(*it)->GetCoordinates(i, y);
				if (fabs(C_cords.z) <= m_local_z_buffer[ZINDEX(cur_width, i, y)])
				{
					activePixels.push_back(tuple<int,int>(i / 2,y / 2));
					m_local_z_buffer[ZINDEX(cur_width, i, y)] = fabs(C_cords.z);
					if (is_wire_frame && (i == maxX || i == minX))
					{
						DrawPixel(i, y, WHITE);
					}
					else if ((lights.size() > 0) && ((*it)->is_light == false))
					{
						color = (*it)->GetColor(C_cords, lights, shadow, (*it)->shape_color);
						DrawPixel(i, y, color);
					}
					else
					{
						DrawPixel(i, y, (*it)->shape_color);
					}
				}
			}
		}
	}
	if (isSuperSample)
	{
		superSampling();
	}
}

void Shape::UpdateLimits(int x, int y)
{
	int fixed_y = y - yMin;
	if (x_min[fixed_y] == -1) // x values starts from 1
	{
		x_min[fixed_y] = x;
		x_max[fixed_y] = x;
	}
	else if (x < x_min[fixed_y])
	{
		x_min[fixed_y] = x;
	}
	if (x > x_max[fixed_y])
	{
		x_max[fixed_y] = x;
	}
}
