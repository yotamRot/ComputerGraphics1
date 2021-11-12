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

void Scene::lookAtModel(int modelId)
{
	activeModel = modelId;
	MeshModel* curModel = (MeshModel*)models.at(modelId);
	Camera* curCamera = cameras.at(activeCamera);
	vec4 modelCenter = vec4(curModel->getPosition());
	curCamera->cTransform = Translate(0, 0, 2) * curModel->_world_transform; //gets model location
	curCamera->LookAt(curCamera->cTransform * curCamera->eye, modelCenter, curCamera->up);
}


bool Scene::updateDrawBoundBox()
{
	if (draw_bound_box)		// hide boxes
	{
		draw_bound_box = false;
	}
	else					// show boxes
	{
		draw_bound_box = true;
	}
	return draw_bound_box;
}

void Scene::rotateAroundActiveModel(Axis direction)
{	
	MeshModel* curModel = (MeshModel*)models.at(activeModel);
	Camera* curCamera = cameras.at(activeCamera);
	mat4 rotationMatrix;
	curCamera->cTransform = Translate(curModel->getPosition()) * curCamera->cTransform;  // move to object center
	switch (direction)
	{
		case X:
			rotationMatrix = RotateX(3);
			break;
		case Y:
			rotationMatrix = RotateY(3);
			break;
		case Z:
		default:
			break;
	}
	curCamera->cTransform = rotationMatrix * curCamera->cTransform; // rotate 
	curCamera->cTransform = Translate((-1) * curModel->getPosition()) * curCamera->cTransform;  // return to orig
	// look at model for new location
	lookAtModel(activeModel);
}

void Scene::setActiveCameraProjection(Projection proj)
{
	Camera* curCamera = cameras.at(activeCamera);
	vec4 lbn = curCamera->cTransform * vec4(-3, -3, 3, 1); // left, bottom, near
	vec4 rtf = curCamera->cTransform * vec4(3, 3, 8, 1); // right, top, far

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

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves
	m_renderer->ClearColorBuffer();
	m_renderer->SetCameraTransform(cameras[activeCamera]->cTransform);
	m_renderer->SetProjection(cameras[activeCamera]->projection);

	for (vector<Model*>::iterator it = models.begin(); it != models.end(); ++it)
	{
		(*it)->draw(this->m_renderer, this->isShowVerticsNormals, draw_bound_box);

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
	Camera* initCamera = new Camera();

	// dont change in camera world of view!
	initCamera->eye = vec4(0, 0, 0, 1);
	initCamera->at = vec4(0, 0, -1, 1);
	initCamera->up = vec4(0, 1, 0, 1);
	//set camera world view aligned with world asix with offset in z
	initCamera->cTransform = mat4();
	initCamera->cTransform[2][3] = 0;
	//initCamera->LookAt(initCamera->cTransform * initCamera->eye, initCamera->cTransform * initCamera->at, initCamera->up);
	activeCamera = 0;
	cameras.push_back(initCamera);
	setActiveCameraProjection(ORTHOGRAPHIC);
}

void Scene::manipulateActiveModel(Transformation T, Axis axis)
{
	MeshModel* curModel = (MeshModel*)models.at(activeModel);
	curModel->manipulateModel(T, axis);
}

void Camera::setTransformation(const mat4& transform)
{
	cTransform = mat4(transform);
}

void Camera::LookAt(const vec4& eye , const vec4& at, const vec4& up)
{
	vec4 n = normalize(eye - at);
	vec4 u = normalize(cross(up, n));
	vec4 v = normalize(cross(n, u));
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	mat4 c = mat4(u, v, n, t);
	setTransformation(c * Translate(-eye));
}

mat4 Camera::Perspective(const float fovy, const float aspect,
	const float zNear, const float zFar)
{	
	float radian = M_PI * fovy / 180;
	float top = zNear * tan(radian / 2);
	float bottom = -top;
	float right = top * aspect;
	float left = -right;
	return mat4(
		(2 * zNear) / (right - left), 0, 0, 0,
		0, (2 * zNear) / (top - bottom), 0, 0,
		(right + left) / (right - left), (top + bottom) / (top - bottom), (-1) * ((zFar + zNear) / (zFar - zNear)), -1,
		0, 0, (-1) * (2 * zNear * zFar) / (zFar - zNear), 0);
}

void Camera::Frustum(const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar)
{
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
	projection = mat4(
		(2) / (right - left), 0, 0, 0,
		0, (2) / (top - bottom), 0, 0,
		0, 0, (-1) * ((2) / (zFar - zNear)), 0,
		(-1) * ((right + left) / (right - left)),
		(-1) * ((top + bottom) / (top - bottom)),
		(-1) * (zFar + zNear) / (zFar - zNear), 1);
}


