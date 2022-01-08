#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>
#include <math.h>
#include <chrono>
#include "InitShader.h"
#include "GL\freeglut.h"
using namespace std;



mat4 LookAt(const vec4& eye, const vec4& at, const vec4& up)
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

mat4 Frustum(const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar)
{
	vec3 lbn = vec3(left, bottom, zNear);
	vec3 rtf = vec3(right, top, zFar);
	return  mat4(
		(2 * zNear) / (right - left), 0, 0, 0,
		0, (2 * zNear) / (top - bottom), 0, 0,
		(right + left) / (right - left), (top + bottom) / (top - bottom), (-1) * ((zFar + zNear) / (zFar - zNear)), -1,
		0, 0, (-1) * (2 * zNear * zFar) / (zFar - zNear), 0);
}

vec3 LightPosition(mat4& c_transform, mat4& w_transform, mat4& m_transform)
{
	vec4 tempVec = c_transform * w_transform * m_transform * vec4(vec3(0));
	return vec3(tempVec.x / tempVec.w, tempVec.y / tempVec.w, tempVec.z / tempVec.w);
}

int Scene::loadOBJModel(string fileName)
{
	activeModel = models.size();

	MeshModel *model = new MeshModel(fileName, activeModel, program);
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

Light& Scene::GetActiveLight()
{
	return lights.at(activeLight);
}

vec3 Scene::GetModelRGB()
{
	MeshModel* curModel = (MeshModel*)models.at(activeModel);
	return curModel->mesh_color;
}

vec4 Scene::GetModelK()
{
	MeshModel* curModel = (MeshModel*)models.at(activeModel);
	return vec4(curModel->ka, curModel->kd, curModel->ks, curModel->ke) ;
}

void Scene::ChangeActiveLightL(vec4& l_params)
{
	lights.at(activeLight).La = l_params.x;
	lights.at(activeLight).Ld = l_params.y;
	lights.at(activeLight).Ls = l_params.z;
	lights.at(activeLight).l_alpha = l_params.w;
}

void Scene::ChangeAmbientRgbLa(vec4 & rgbl)
{
	LightModel* light = (LightModel*)lights.at(0).model;
	
	light->light_color.x = rgbl.x;
	light->light_color.y = rgbl.y;
	light->light_color.z = rgbl.z;
	light->La = rgbl.w;
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
	if (proj == ORTHOGRAPHIC)
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
	if (proj == FRUSTUM)
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
	//reset Camera
	CameraModel* cameraModel =(CameraModel *) curCamera->model;
	cameraModel->_m_TransformInv = mat4();
	cameraModel->_w_TransformInv = mat4();
	cameraModel->_model_transform = mat4();
	cameraModel->_world_transform = mat4();

	float maxModelAxisSize = max(max(max(curModel->GetZBoundLength(), curModel->GetXBoundLength()), curModel->GetYBoundLength()), 0.2);
	// model in camera cord
	mat4 toModelTransform = curModel->_world_transform * curModel->_model_transform * Translate(curModel->GetCenter());
	mat4 fromModelTransform = Translate(0, 0, 10 * maxModelAxisSize) * curModel->_world_transform * curModel->_model_transform * Translate(curModel->GetCenter());
	vec4 c_atCenter = toModelTransform * vec4(vec3());
	vec4 c_from = fromModelTransform * vec4(vec3());
	vec4 c_atDirection = normalize(c_atCenter/ c_atCenter.w - c_from / c_from.w);

	cameraModel->_m_TransformInv = curCamera->LookAt(c_from, c_from + c_atDirection,curCamera->up);
	cameraModel->_world_transform = fromModelTransform;
	ResetZoom();
}

void Scene::ResetZoom()
{
	Camera* curCamera = cameras.at(activeCamera);
	vec3 lbn;
	lbn.x = lbn.y = -0.5;
	lbn.z = 0.5;
	vec3 rtf;
	rtf.x = rtf.y = 0.5;
	rtf.z = 50;
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
	lights.clear();
	InitScene();
	ResetZoom();
}




void Scene::ControlActiveCamera()
{
	activeModel = cameras.at(activeCamera)->modelId;
}

void Scene::ControlActiveLight()
{
	activeModel = lights.at(activeLight).modelId;
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

void Scene::ChangeProjectionParameters(Projection proj, vec3& rtf, vec3& lbn,vec4& pers_param)
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

bool Scene::toggleShowFog()
{
	isShowFog = !isShowFog;
	return isShowFog;
}

bool Scene::toggleShowWireFrame()
{
	isShowWireFrame = !isShowWireFrame;
	return isShowWireFrame;
}

bool Scene::toggleSuperSample()
{
	isSuperSample = !isSuperSample;
	return isSuperSample;
}

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves

	//this->drawDemo();
	//return;
	GLfloat curProjection[16];
	MattoArr(curProjection, cameras[activeCamera]->projection);
	CameraModel* curCameraModel = (CameraModel*)(cameras[activeCamera]->model);
	GLfloat curCameraInv[16];
	MattoArr(curCameraInv, curCameraModel->_w_TransformInv * curCameraModel->_m_TransformInv);
	int cameraIndex = 0;
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
	GLint umV = glGetUniformLocation(program, "modelViewMatrix"); // Find the modelViewMatrix variable
	glUniformMatrix4fv(umV, 1, GL_FALSE, curCameraInv);
	GLint umP = glGetUniformLocation(program, "projectionMatrix"); // Find the projectionMatrix variable
	glUniformMatrix4fv(umP, 1, GL_FALSE, curProjection);
	glUseProgram(light_program);
	GLint viewLightLoc = glGetUniformLocation(light_program, "modelViewMatrix"); // Find the modelViewMatrix variable
	glUniformMatrix4fv(viewLightLoc, 1, GL_FALSE, curCameraInv);
	GLint projectionLoc = glGetUniformLocation(light_program, "projectionMatrix"); // Find the projectionMatrix variable
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, curProjection);

	MeshModel* curModel;
	
	/*
	for (auto it = lights.begin(); it != lights.end(); ++it)
	{
		(it)->c_light_position = LightPosition(curCameraInv, (it)->model->_world_transform, (it)->model->_model_transform);
	}*/
	for (vector<Model*>::iterator it = models.begin(); it != models.end(); ++it)
	{
		curModel = (MeshModel*)(*it);
		if (dynamic_cast<CameraModel*>(*it))
		{
			if (isRenderCameras && cameraIndex!= activeCamera) //dont want to draw active camera
			{
				(*it)->draw(); // draw camera
			}
			cameraIndex++;
		}
		else if(dynamic_cast<LightModel*>(*it))
		{
			LightModel* light_model = (LightModel*)(*it);
			glUseProgram(program);
			GLint my_light_location = glGetUniformLocation(program, "lightColor");
			glUniform3f(my_light_location, light_model->light_color.x, light_model->light_color.y, light_model->light_color.z);
			GLint l_a_location = glGetUniformLocation(program, "La");
			glUniform1f(l_a_location, light_model->La);
			//if (light_model->lightIndex == 0) //dont want to draw ambient light
			//{
			//	continue;
			//}
			//else
			//{
				(*it)->draw();// draw models
			//}
		}
		else
		{
			(*it)->draw();// draw models
		}
	}

	
	glutSwapBuffers();

}

void Scene::drawDemo()
{
	const int pnum = 3;
	static const GLfloat points[pnum][4] = {
		{-1.1, -1.1f, 0.0f,1.0f},
		{1.1f, -1.1f, 0.0f,1.0f},
		{0.0f,  1.1f, 0.0f,1.0f}
	};
	GLuint program = InitShader("minimal_vshader.glsl",
		"minimal_fshader.glsl");

	glUseProgram(program);


	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points),
		points, GL_STATIC_DRAW);



	GLuint loc = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	GLint umM = glGetUniformLocation(program, "modelMatrix"); // Find the mM variable
	mat4 modelTrans = Translate(5, 0, -1);
	GLfloat ModelMatrix[16];
	MattoArr(ModelMatrix, modelTrans);
	glUniformMatrix4fv(umM, 1, GL_FALSE, ModelMatrix);

	GLint umC = glGetUniformLocation(program, "modelViewMatrix"); // Find the mM variable
	mat4 camera = LookAt(vec4(0,0,0,1) ,vec4(0,0,-1,1), normalize(vec4(0, 1, 0, 1)));
	GLfloat CameraMatrix[16];
	MattoArr(CameraMatrix, camera);
	glUniformMatrix4fv(umC, 1, GL_FALSE, CameraMatrix);

	GLint umP = glGetUniformLocation(program, "projectionMatrix"); // Find the mM variable
	mat4 projection = Frustum(-2, 2, -2, 2, 0.1, 20);
	GLfloat ProjMatrix[16];
	MattoArr(ProjMatrix, projection);
	glUniformMatrix4fv(umP, 1, GL_FALSE, ProjMatrix);

	vec3 check = getXYZ(projection * camera * modelTrans * vec4(-1.1, -1.1f, 0.0f, 1.0f));
	vec3 check1 = getXYZ(projection * camera * modelTrans * vec4(1.1, -1.1f, 0.0f, 1.0f));
	vec3 check2 = getXYZ(projection * camera * modelTrans * vec4(0, 1.1f, 0.0f, 1.0f));

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, pnum);
	glFlush();
	glutSwapBuffers();


}


Scene::Scene() : current_shadow(FLAT)
{
	InitScene();
	activeModel = ILLEGAL_ACTIVE_MOVEL;
	proj = FRUSTUM;
	setActiveCameraProjection(proj);
	isShowVerticsNormals = false;
	isShowFacesNormals = false;
	isRenderCameras = false;
	isDrawBoundBox = false;
	isShowFog = false;
	isShowWireFrame = false;
	isSuperSample = false;
	axis = MODEL;
}

Scene::Scene(Renderer *renderer=NULL) : current_shadow(FLAT)
{	
	program = InitShader("minimal_vshader.glsl", "minimal_fshader.glsl");
	light_program = InitShader("light_vshader.glsl", "light_fshader.glsl");
	InitScene();
	activeModel = ILLEGAL_ACTIVE_MOVEL;
	proj = FRUSTUM;
	setActiveCameraProjection(proj);
	isShowVerticsNormals = false;
	isShowFacesNormals = false;
	isRenderCameras = false;
	isDrawBoundBox = false;
	isShowFog = false;
	isShowWireFrame = false;
	isSuperSample = false;
	axis = MODEL;
}

void Scene::InitScene()
{
	activeCamera = addCamera();
	// add ambient Light with id 0 and set Ld & Ls to 0
	//activeLight = addLight();
	//lights.at(0).Ld = 0;
	//lights.at(0).Ls = 0;
	//lights.at(0).La = 0.4;

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
	cameraModel->_m_TransformInv = cameraModel->_m_TransformInv * invTransform;
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



mat4 Perspective(const float fovy, const float aspect,
	const float zNear, const float zFar)
{
	const float radian = M_PI * fovy / 180;
	const float top = zNear * tan(radian / 2);
	const float bottom = -top;
	const float right = top * aspect;
	const float left = -right;
	return	Frustum(left, right, bottom, top, zNear, zFar);
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
	lbn.x = left;
	lbn.y = bottom;
	lbn.z = zNear;
	rtf.x = right;
	rtf.y = top;
	rtf.z = zFar;

	projection = mat4(
		(2) / (right - left), 0, 0, 0,
		0, (2) / (top - bottom), 0, 0,
		0, 0, (-1) * ((2) / (zFar - zNear)), 0,
		(-1) * ((right + left) / (right - left)),
		(-1) * ((top + bottom) / (top - bottom)),
		(-1) * (zFar + zNear) / (zFar - zNear), 1);
}

Camera::Camera(vec3& lbn, vec3& rtf, int modelId, Model* model) :lbn(lbn), rtf(rtf), modelId(modelId) ,model(model)
{
	eye.x = eye.y = eye.z = 0;
	eye.w = 1;
	at.x = at.y = 0;
	at.z = -1;
	at.w = 1;
	up.x = up.z = 0;
	up.y = 1;
	up.w = 1;
	//set camera world view aligned with world asix with offset in z
}



void Scene::MaintingCamerasRatios(int oldWidth, int oldHeight, int newWidth, int newHeight)
{
	float widthRatio = (float)newWidth / (float)oldWidth;
	float heightRatio = (float)newHeight / (float)oldHeight;
	for (auto it = cameras.begin(); it != cameras.end(); ++it)
	{
		(*it)->MaintainRatio(widthRatio, heightRatio, proj);
	}
}

void Scene::ChangeModelColorIndex(vec3& rgb)
{
	MeshModel* cur_model = (MeshModel*)models.at(activeModel);
	cur_model->mesh_color = rgb;
	cur_model->UpdateTriangleColor();
}


void Scene::ChangeModelIlluminationParams(vec4& k)
{
	MeshModel* cur_model = (MeshModel*)models.at(activeModel);
	cur_model->ka = k.x;
	cur_model->kd = k.y;
	cur_model->ks = k.z;
	cur_model->ke = k.w;
	cur_model->UpdateTriangleIlluminationParams();
}

void Scene::ApplyCrazyColors()
{
	MeshModel* cur_model = (MeshModel*)models.at(activeModel);
	cur_model->RandomizePolygons();
}


void Scene::ApplyNonUniform()
{
	MeshModel* cur_model = (MeshModel*)models.at(activeModel);
	cur_model->is_non_unfiorm = !cur_model->is_non_unfiorm;
	cur_model->UpdateTriangleIlluminationParams();
}



void Scene::ChangeShadow(Shadow s)
{
	current_shadow = s;
}

vec4 Scene::GetAmbientRGB()
{
	LightModel* light = (LightModel*)lights.at(0).model;
	return vec4(light->light_color, light->La); 
}

void Camera::MaintainRatio(float widthRatio, float heightRatio, Projection proj)
{
	lbn.x = lbn.x * widthRatio;
	rtf.x = rtf.x * widthRatio;
	lbn.y = lbn.y * heightRatio;
	rtf.y = rtf.y * heightRatio;
	if (proj == ORTHOGRAPHIC)
	{
		Ortho(lbn.x, rtf.x, lbn.y, rtf.y, lbn.z, rtf.z);
	}
	else if (proj == FRUSTUM)
	{
		Frustum(lbn.x, rtf.x, lbn.y, rtf.y, lbn.z, rtf.z);
	}
}

vec3 Camera::Getlbn() 
{
	return lbn;
}
vec3 Camera::Getrtf()
{
	return rtf;
}


int Scene::addCamera()
{
	int newCameraIndex = cameras.size();
	CameraModel* cameraModel = new CameraModel(newCameraIndex, program);
	Camera* newCamera = new Camera(vec3(-2, -2, 0.5), vec3(2, 2, 50), models.size(), cameraModel);
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
	Camera* activeCamera = cameras.at(cameraId);
	ChangeProjectionParameters(proj, activeCamera->Getrtf(), activeCamera->Getlbn());
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


int Scene::addLight()
{
	int newLightIndex = lights.size();
	activeLight = newLightIndex;
	activeModel = models.size();
	LightModel* lightModel = new LightModel(activeModel, newLightIndex, light_program);
	Light newLight = Light(models.size(), lightModel);
	lights.push_back(newLight);
	models.push_back(lightModel);
	return newLightIndex;
}


void Scene::controlLight(int lightId)
{
	activeLight = lightId;
	Light activeLight = lights.at(lightId);
	ControlActiveLight();
}

void Scene::lookAtLight(int lightId)
{
	Light lightToLookAt = lights.at(lightId);
	lookAtModel(lightToLookAt.modelId);
}