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
	CameraModel* cameraModel =(CameraModel *) curCamera->model;
	float maxModelAxisSize = max(max(curModel->GetZBoundLength(), curModel->GetXBoundLength()), curModel->GetYBoundLength());
	// model in camera cord
	vec3 m_modelCenter = curModel->GetCenter();
	vec4 c_atCenter = cameraModel->_m_TransformInv * cameraModel->_w_TransformInv * curModel->_world_transform * curModel->_model_transform * m_modelCenter;
	vec4 c_from = Translate(0, 0, 3 * maxModelAxisSize) * Translate(c_atCenter) * vec4(vec3());
	vec4 c_atDirection = c_atCenter - c_from;
	c_atDirection.w = 0;

	mat4 c_w_InvMatrix = curCamera->LookAt(c_from, c_from + c_atDirection, curCamera->up);
	mat4 c_w_Matrix = Translate(0, 0, 3 * curModel->GetZBoundLength()) * curModel->_world_transform * curModel->_model_transform;
	curCamera->setTransformation(c_w_InvMatrix, c_w_Matrix);
	ResetZoom();
}

void Scene::ResetZoom()
{
	Camera* curCamera = cameras.at(activeCamera);
	vec3 lbn = vec3(-0.5, -0.5, 0.5);
	vec3 rtf = vec3(0.5, 0.5, 5);
	if (proj == ORTHOGRAPHIC)
	{
		curCamera->Ortho(lbn.x, rtf.x, lbn.y, rtf.y, lbn.z, rtf.z);
	}
	else
	{
		curCamera->Frustum(lbn.x, rtf.x, lbn.y, rtf.y, lbn.z, rtf.z);
	}
}

void Scene::ClearScene()
{
	models.clear();
	cameras.clear();
	activeModel = ILLEGAL_ACTIVE_MOVEL;
	activeCamera = addCamera();
	modelToVectorId.clear();
	setActiveCameraProjection(PERSPECTIVE);
	isShowVerticsNormals = false;
	isShowFacesNormals = false;
	isRenderCameras = false;
	isDrawBoundBox = false;
	axis = MODEL;
	ResetZoom();
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

void Scene::ChangeProjectionParameters(Projection proj, vec3 rtf, vec3 lbn,vec4 pers_param)
{
	Camera* curCamera = cameras.at(activeCamera);
	this->proj = proj;
	if (proj == ORTHOGRAPHIC)
	{
		curCamera->Ortho(lbn.x, rtf.x, lbn.y, rtf.y, lbn.z, rtf.z);
	}
	else if(proj == FRUSTUM)
	{
		curCamera->Frustum(lbn.x, rtf.x, lbn.y, rtf.y, lbn.z, rtf.z);
	}
	else if (proj == PERSPECTIVE)
	{
		curCamera->Perspective(pers_param.x, pers_param.y, pers_param.z, pers_param.w);
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
	mat4 curProjection = cameras[activeCamera]->projection;
	CameraModel* curCameraModel = (CameraModel*)(cameras[activeCamera]->model);
	mat4 curCameraInv = curCameraModel->_w_TransformInv * curCameraModel->_m_TransformInv;
	m_renderer->ClearColorBuffer();
	m_renderer->ConfigureRenderer(curProjection, curCameraInv, isShowVerticsNormals, isShowFacesNormals, isDrawBoundBox);
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
	activeCamera = addCamera();
	activeModel = ILLEGAL_ACTIVE_MOVEL;
	setActiveCameraProjection(PERSPECTIVE);
	isShowVerticsNormals = false;
	isShowFacesNormals = false;
	isRenderCameras = false;
	isDrawBoundBox = false;
	axis = MODEL;
}

void Scene::manipulateActiveModel(Transformation T, TransformationDirection direction
									, TransAxis axis, float power)
{
	if (activeModel == ILLEGAL_ACTIVE_MOVEL)
	{
		return;
	}

	MeshModel* curModel = (MeshModel*)models.at(activeModel);
	mat4 cameraInverseMat = curModel->manipulateModel(T, direction, axis,power);
	
	if (CameraModel* cameraModel = dynamic_cast<CameraModel*>(curModel))
	{
		if (axis == WORLD)
		{
			cameraModel->_w_TransformInv = cameraModel->_w_TransformInv * cameraInverseMat;
		}
		else
		{
			cameraModel->_m_TransformInv = cameraModel->_m_TransformInv * cameraInverseMat;

		}

	}
}

const Projection Scene::GetProjection()
{
	return proj;
}


void Camera::setTransformation(const mat4& invTransform, const mat4& Transform)
{
	CameraModel* cameraModel =(CameraModel *) this->model;
	cameraModel->_w_TransformInv = cameraModel->_w_TransformInv * invTransform;
	cameraModel->_world_transform = Transform * cameraModel->_world_transform;
}

mat4 Camera::LookAt(const vec4& eye , const vec4& at, const vec4& up)
{
	vec4 n = normalize(eye - at);
	n.w = 0;
	vec4 u = normalize(cross(up, n));
	u.w = 0;
	vec4 v = normalize(cross(n, u));
	v.w = 0;
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	mat4 c = mat4(u, v, n, t);
	return c * Translate(-eye);
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

Camera::Camera(vec3 lbn, vec3 rtf, int modelId, Model* model) :lbn(lbn), rtf(rtf), modelId(modelId) ,model(model)
{
	eye = vec4(0, 0, 0, 1);
	at = vec4(0, 0, -1, 1);
	up = vec4(0, 1, 0, 1);
	//set camera world view aligned with world asix with offset in z
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
	Camera* newCamera = new Camera(vec3(-0.5, -0.5, 0.5), vec3(0.5, 0.5, 5), models.size(), cameraModel);
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

vec3 Scene::Getlbn()
{
	Camera* activeCamera = GetActiveCamera();
	return activeCamera->Getlbn();
}
vec3 Scene::Getrtf()
{
	Camera* activeCamera = GetActiveCamera();
	return activeCamera->Getrtf();
}