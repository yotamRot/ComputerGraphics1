#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>
#include <math.h>

using namespace std;

int Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	activeModel = models.size();
	modelToVectorId.push_back(activeModel);
	models.push_back(model);
	return modelToVectorId.size() - 1; // new model index
}

int Scene::loadCubeModel()
{
	PrimMeshModel* cube = new PrimMeshModel(0, 0, 0, 1 , 1, 1);
	activeModel = models.size();
	modelToVectorId.push_back(activeModel);
	models.push_back(cube);
	return modelToVectorId.size() - 1; // new model index
}

int Scene::modelMenuIdToVectorId(int menuId)
{
	return modelToVectorId.at(menuId);
}

Camera* Scene::GetActiveCamera() 
{
	return cameras.at(activeCamera);
}

void Scene::Zoom(ZoomDirection direction)
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
	if (GetProjection() == PERSPECTIVE)
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
	MeshModel* curModel = (MeshModel*)models.at(activeModel);
	Camera* curCamera = cameras.at(activeCamera);
	mat4 modelCenterTranslation = curModel->_world_transform * curModel->_model_transform * Translate(curModel->GetCenter());
	vec4 modelCenter = modelCenterTranslation * vec4(vec3(0)); // (0,0,0,1)
	mat4 eyeTranslation = Translate(0, 0, 2 * curModel->GetZBoundLength()) * modelCenterTranslation;
	curCamera->cTransform = eyeTranslation; //gets model location
	curCamera->LookAt(eyeTranslation * curCamera->eye, modelCenter, curCamera->up);
	CameraModel* cameraModel = (CameraModel*)models.at(curCamera->modelId);
	cameraModel->_world_transform = curCamera->cTransform;
}

void Scene::ClearScene()
{
	models.clear();
	activeModel = ILLEGAL_ACTIVE_MOVEL;
	addCamera();
	activeCamera = 0;
	modelToVectorId.clear();
	setActiveCameraProjection(PERSPECTIVE);
	isShowVerticsNormals = false;
	isShowFacesNormals = false;
	isRenderCameras = false;
	isDrawBoundBox = false;
	axis = WORLD;
}



void Scene::rotateAroundActiveModel(int dx, int dy)
{	
	if (activeModel == ILLEGAL_ACTIVE_MOVEL)
	{
		return;
	}

	MeshModel* curModel = (MeshModel*)models.at(activeModel);
	
	curModel->rotateModel(Y, dx / 10, axis);
	curModel->rotateModel(X, dy / 10, axis);

}

void Scene::ControlActiveCamera()
{
	activeModel = cameras.at(activeCamera)->modelId;
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

void Scene::ChangeProjectionParameters(Projection proj, vec3 rtf, vec3 lbn)
{
	Camera* curCamera = cameras.at(activeCamera);
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

void Scene::SetTrasformationAxis(TransAxis Axis)
{
	axis = Axis;
}

TransAxis Scene::GetTrasformationAxis()
{
	return axis;
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

bool Scene::toggleRenderCameras()
{
	isRenderCameras = !isRenderCameras;
	return isRenderCameras;
}

bool Scene::toggleDrawBoundBox()
{
	isDrawBoundBox = !isDrawBoundBox;
	return isDrawBoundBox;
}
void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves
	int cameraIndex = 0;
	m_renderer->ClearColorBuffer();
	m_renderer->ConfigureRenderer(cameras[activeCamera]->projection, 
		cameras[activeCamera]->cTransform, isShowVerticsNormals, isShowFacesNormals, isDrawBoundBox);
	MeshModel* curModel;

	for (vector<Model*>::iterator it = models.begin(); it != models.end(); ++it)
	{
		curModel = (MeshModel*)(*it);
		m_renderer->SetObjectMatrices(curModel->GetObjectMatrix(), curModel->GetNormalMatrix());
		if (dynamic_cast<CameraModel*>(*it))
		{
			if (isRenderCameras && cameraIndex!= activeCamera) //dont want to draw active camera
			{
				(*it)->draw(m_renderer); // draw camera
			}
			cameraIndex++;
		}
		else
		{
			(*it)->draw(m_renderer);
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
	addCamera();
	activeCamera = 0;
	activeModel = ILLEGAL_ACTIVE_MOVEL;
	setActiveCameraProjection(PERSPECTIVE);
	isShowVerticsNormals = false;
	isShowFacesNormals = false;
	isRenderCameras = false;
	isDrawBoundBox = false;
	axis = WORLD;
}

void Scene::manipulateActiveModel(Transformation T, TransformationDirection direction, TransAxis axis)
{
	if (activeModel == ILLEGAL_ACTIVE_MOVEL)
	{
		return;
	}
	MeshModel* curModel = (MeshModel*)models.at(activeModel);
	curModel->manipulateModel(T, direction, axis);
	
	if (CameraModel* cameraModel = dynamic_cast<CameraModel*>(curModel))
	{
		cameras.at(cameraModel->cameraIndex)->cTransform = cameraModel->_world_transform * cameraModel->_model_transform;
	}
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

Camera::Camera(vec3 lbn, vec3 rtf, int modelId) :lbn(lbn), rtf(rtf), modelId(modelId)
{
	eye = vec4(0, 0, 0, 1);
	at = vec4(0, 0, -1, 1);
	up = vec4(0, 1, 0, 1);
	//set camera world view aligned with world asix with offset in z
	cTransform[2][3] = 2;
}


vec3 Camera::Getlbn() 
{
	return vec3(lbn);
}
vec3 Camera::Getrtf()
{
	return vec3(rtf);
}


int Scene::addCamera()
{
	int newCameraIndex = cameras.size();
	CameraModel* cameraModel = new CameraModel(newCameraIndex);
	Camera* newCamera = new Camera(vec3(-3, -3, 3), vec3(3, 3, 8), models.size());
	cameras.push_back(newCamera);
	models.push_back(cameraModel);
	return newCameraIndex;
}
void Scene::lookAtCamera(int cameraId)
{
	if (activeCamera == cameraId)
	{
		return;
	}
	
	Camera* cameraToLookAt = cameras.at(cameraId);
	lookAtModel(cameraToLookAt->modelId);
}

void Scene::switchToCamera(int cameraId)
{
	activeCamera = cameraId;
}