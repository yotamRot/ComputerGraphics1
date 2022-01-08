#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
using namespace std;

#define ZOOM 2
#define ILLEGAL_ACTIVE_MOVEL -1

enum TransformationDirection
{
	X,
	Xn,
	Y,
	Yn,
	Z,
	Zn
};

enum Transformation
{
	SCALE,
	MOVE,
	ROTATE,
};

enum Projection
{
	ORTHOGRAPHIC,
	PERSPECTIVE,
	FRUSTUM
};

enum ZoomDirection
{
	ZOOM_IN,
	ZOOM_OUT
};








class Camera {
	vec3 lbn;  // left, bottom, near
	vec3 rtf;  // right, top, far
public:
	Camera(vec3& lbn, vec3& rtf, int modelId, Model* model);
	mat4 projection;
	int modelId;
	Model* model;
	vec4 eye;
	vec4 at;
	vec4 up;
	void setTransformation(const mat4& invTransform, const mat4& Transform);
	mat4 LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void Ortho( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void Frustum( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	mat4 Perspective( const float fovy, const float aspect,
		const float zNear, const float zFar);
	void MaintainRatio(float widthFactor, float heightFactor, Projection proj);
	vec3 Getlbn();
	vec3 Getrtf();
};

class Scene {

	vector<Camera*> cameras;
	vector<Light> lights;
	Projection proj;
	bool isShowVerticsNormals;
	bool isShowFacesNormals;
	bool isRenderCameras;
	bool isDrawBoundBox;
	bool isShowFog;
	bool isShowWireFrame;
	bool isSuperSample;
	TransAxis axis;
	Shadow current_shadow;
	void ResetZoom();
public:
	vector<int> modelToVectorId;
	vector<Model*> models;
	Scene();
	Scene(Renderer* renderer);
	void InitScene();
	int loadOBJModel(string fileName);
	int loadCubeModel();
	void lookAtModel(int modelId);
	int addCamera();
	void lookAtCamera(int cameraId);
	void lookAtLight(int lightId);
	void switchToCamera(int cameraId);
	void controlLight(int lightId);
	int addLight();
	void ClearScene();
	void ControlActiveCamera();
	void ControlActiveLight();
	void manipulateActiveModel(Transformation T,TransformationDirection direction,
								TransAxis axis, float power);
	void setActiveCameraProjection(Projection proj);
	const Projection GetProjection();
	bool toggleShowVerticesNormals();
	bool toggleShowFacesNormals();
	bool toggleRenderCameras();
	bool toggleDrawBoundBox();
	bool toggleShowFog();
	bool toggleShowWireFrame();
	bool toggleSuperSample();
	void draw();
	void drawDemo();
	int modelMenuIdToVectorId(int menuId);
	Camera* GetActiveCamera();
	Light& GetActiveLight();
	vec3 GetModelRGB();
	vec4 GetModelK();
	void ChangeActiveLightL(vec4& l_params);
	void ChangeAmbientRgbLa(vec4& rgbl);
	void Zoom(ZoomDirection direction);
	int activeModel;
	int activeLight;
	int activeCamera;
	void ChangeProjectionParameters(Projection proj, vec3& rtf, vec3& lbn, vec4& pers_param=vec4());
	void SetTrasformationAxis(TransAxis Axis);
	TransAxis GetTrasformationAxis();
	vec3 Getlbn();
	vec3 Getrtf();
	void MaintingCamerasRatios(int oldWidth, int oldHeight, int newWidth, int newHeight);
	void ChangeModelColorIndex(vec3& rgb);
	void ChangeModelIlluminationParams(vec4& k);
	void ApplyCrazyColors();
	void ApplyNonUniform();
	void ChangeShadow(Shadow s);
	vec4 GetAmbientRGB();
	GLuint program;
	GLuint light_program;
};