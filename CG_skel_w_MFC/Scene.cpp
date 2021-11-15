#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>
#include <math.h>

using namespace std;

void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	activeModel = models.size();
	models.push_back(model);
}

void Scene::loadCubeModel()
{
	PrimMeshModel* cube = new PrimMeshModel(0, 0, 0, 1,1,1);
	activeModel = models.size();
	models.push_back(cube);
}
Camera* Scene::GetActiveCamera() 
{
	return cameras.at(activeCamera);
}

void Scene::Zoom(Direction direction)
{
	if (activeModel == ILLEGAL_ACTIVE_MOVEL)
	{
		return;
	}
	vec3 lbn;
	vec3 rtf;
	GLfloat fovy;
	vec3 bounds;
	lbn = GetActiveCamera()->Getlbn();
	rtf = GetActiveCamera()->Getrtf();
	MeshModel* curModel = (MeshModel*)models.at(activeModel);
	bounds = curModel->GetBoundsLength();
	if (GetProjection() == ORTHOGRAPHIC)
	{
		if(direction == ZOOM_IN)
		{
			if ((lbn.x + bounds.x >= rtf.x - bounds.x) || (lbn.y + bounds.x >= rtf.y - bounds.x))
			{
				return;
			}
			GetActiveCamera()->Ortho(lbn.x + bounds.x, rtf.x - bounds.x, lbn.y + bounds.x, rtf.y - bounds.x, lbn.z, rtf.z);
		}
		else // WHEEL_SCROLL_DOWN
		{
			GetActiveCamera()->Ortho(lbn.x - bounds.x, rtf.x + bounds.x, lbn.y - bounds.x, rtf.y + bounds.x, lbn.z, rtf.z);
		}

	}
	if (GetProjection() == PRESPECTIVE)
	{

		if (direction == ZOOM_IN)
		{
			fovy = ((360 * atan(rtf.y / lbn.z)) / M_PI);
			if (fovy - ZOOM < 1)
			{
				return;
			}
			GetActiveCamera()->Perspective(fovy - ZOOM, rtf.x / rtf.y, lbn.z, rtf.z);
		}
		else // WHEEL_SCROLL_DOWN
		{
			fovy = ((360 * atan(rtf.y / lbn.z)) / M_PI);
			if (fovy + ZOOM > 179)
			{
				return;
			}
			GetActiveCamera()->Perspective(fovy + ZOOM, rtf.x / rtf.y, lbn.z, rtf.z);
		}

	}
}

void Scene::lookAtModel(int modelId)
{
	activeModel = modelId;
	MeshModel* curModel = (MeshModel*)models.at(modelId);
	Camera* curCamera = cameras.at(activeCamera);
	vec4 modelCenter = vec4(curModel->GetCenter());
	curCamera->cTransform = Translate(0, 0, 2*curModel->GetZBoundLength()) * Translate(modelCenter); //gets model location
	curCamera->LookAt(curCamera->cTransform * curCamera->eye, modelCenter, curCamera->up);
}

void Scene::ClearScene()
{
	models.clear();
	activeModel = ILLEGAL_ACTIVE_MOVEL;

}


bool Scene::updateDrawBoundBox()
{
	draw_bound_box = !draw_bound_box;
	return draw_bound_box;
}

void Scene::rotateAroundActiveModel(int dx, int dy)
{	
	if (activeModel == ILLEGAL_ACTIVE_MOVEL)
	{
		return;
	}

	MeshModel* curModel = (MeshModel*)models.at(activeModel);
	curModel->rotateModel(Y, dx / 10);
	curModel->rotateModel(X, dy / 10);

}

void Scene::setActiveCameraProjection(Projection proj)
{
	Camera* curCamera = cameras.at(activeCamera);
	const vec3 lbn = curCamera->Getlbn();
	const vec3 rtf = curCamera->Getrtf();
	this->proj = proj;

	if (proj == ORTHOGRAPHIC)
	{
		curCamera->Ortho(lbn.x, rtf.x, lbn.y, rtf.y, lbn.z, rtf.z);
	}
	else
	{
		curCamera->Frustum(lbn.x, rtf.x, lbn.y, rtf.y, lbn.z, rtf.z);
	}
}

bool Scene::toggleShowVerticesNormals()
{
	isShowVerticsNormals = !isShowVerticsNormals;
	return isShowVerticsNormals;
}

bool Scene::toggleShowFacesNormals()
{
	isShowFacesNormals = !isShowFacesNormals;
	return isShowFacesNormals;
}

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves
	m_renderer->ClearColorBuffer();
	m_renderer->ConfigureRenderer(cameras[activeCamera]->projection, 
		cameras[activeCamera]->cTransform, isShowVerticsNormals, isShowFacesNormals);

	for (vector<Model*>::iterator it = models.begin(); it != models.end(); ++it)
	{
		(*it)->draw(m_renderer);
		if (draw_bound_box)
		{
			(*it)->drawBoundingBox(this->m_renderer);
		}

	}
	m_renderer->SwapBuffers();
}

void Scene::drawDemo()
{
	m_renderer->SetDemoBuffer();
	m_renderer->SwapBuffers();
}

Scene::Scene(Renderer *renderer) : m_renderer(renderer) 
{	
	Camera* initCamera = new Camera(vec3(-3, -3, 3), vec3(3, 3, 8));

	// dont change in camera world of view!
	initCamera->eye = vec4(0, 0, 0, 1);
	initCamera->at = vec4(0, 0, -1, 1);
	initCamera->up = vec4(0, 1, 0, 1);
	//set camera world view aligned with world asix with offset in z
	initCamera->cTransform = mat4();
	initCamera->cTransform[2][3] = 2;
	activeCamera = 0;
	activeModel = ILLEGAL_ACTIVE_MOVEL;
	cameras.push_back(initCamera);
	setActiveCameraProjection(PRESPECTIVE);
	isShowVerticsNormals = false;
	isShowFacesNormals = false;
}

void Scene::manipulateActiveModel(Transformation T, Axis axis)
{
	if (activeModel == ILLEGAL_ACTIVE_MOVEL)
	{
		return;
	}
	MeshModel* curModel = (MeshModel*)models.at(activeModel);
	curModel->manipulateModel(T, axis);
}

const Projection Scene::GetProjection()
{
	return proj;
}


void Camera::setTransformation(const mat4& transform)
{
	cTransform = mat4(transform);
}

void Camera::LookAt(const vec4& eye , const vec4& at, const vec4& up)
{
	vec4 n = normalize(eye - at);
	n.w = 0;
	vec4 u = normalize(cross(up, n));
	u.w = 0;
	vec4 v = normalize(cross(n, u));
	v.w = 0;
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	mat4 c = mat4(u, v, n, t);
	setTransformation(c * Translate(-eye));
}

mat4 Camera::Perspective(const float fovy, const float aspect,
	const float zNear, const float zFar)
{	
	const float radian = M_PI * fovy / 180;
	const float top = zNear * tan(radian / 2);
	const float bottom = -top;
	const float right = top * aspect;
	const float left = -right;
	Frustum(left, right, bottom, top, zNear, zFar);
	return projection;
}

void Camera::Frustum(const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar)
{
	lbn = vec3(left, bottom, zNear);
	rtf = vec3(right, top, zFar);
	projection = mat4(
		(2 * zNear) / (right - left), 0, 0, 0,
		0, (2 * zNear) / (top - bottom), 0, 0,
		(right + left) / (right - left), (top + bottom) / (top - bottom), (-1) * ((zFar + zNear) / (zFar - zNear)), -1,
		0, 0, (-1) * (2 * zNear * zFar) / (zFar - zNear), 0);
}

void Camera::Ortho(const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar)
{ 
	lbn = vec3(left, bottom, zNear);
	rtf = vec3(right, top, zFar);

	projection = mat4(
		(2) / (right - left), 0, 0, 0,
		0, (2) / (top - bottom), 0, 0,
		0, 0, (-1) * ((2) / (zFar - zNear)), 0,
		(-1) * ((right + left) / (right - left)),
		(-1) * ((top + bottom) / (top - bottom)),
		(-1) * (zFar + zNear) / (zFar - zNear), 1);
}

vec3 Camera::Getlbn() 
{
	return vec3(lbn);
}
vec3 Camera::Getrtf()
{
	return vec3(rtf);
}


