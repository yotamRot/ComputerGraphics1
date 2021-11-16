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
	PERSPECTIVE
};

enum ZoomDirection
{
	ZOOM_IN,
	ZOOM_OUT
};

enum TransAxis
{
	MODEL,
	WORLD
};


class Model { 
public:
	void virtual draw(Renderer* renderer) = 0;
	vec3 virtual GetPosition() = 0;
	void virtual drawBoundingBox(Renderer* renderer) = 0;
protected:
	virtual ~Model() {}
};


class Light {
};

class Camera {
	vec3 lbn;  // left, bottom, near
	vec3 rtf;  // right, top, far
public:
	Camera(vec3 lbn, vec3 rtf, int modelId);
	mat4 cTransform;
	mat4 projection;
	int modelId;
	vec4 eye;
	vec4 at;
	vec4 up;
	void setTransformation(const mat4& transform);
	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void Ortho( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void Frustum( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	mat4 Perspective( const float fovy, const float aspect,
		const float zNear, const float zFar);
	vec3 Getlbn();
	vec3 Getrtf();

};

class Scene {

	vector<Camera*> cameras;
	vector<Light*> lights;
	Projection proj;
	bool isShowVerticsNormals;
	bool isShowFacesNormals;
	bool isRenderCameras;
	bool isDrawBoundBox;
	Renderer* m_renderer;
	vector<int> modelToVectorId;
	TransAxis axis;
public:
	
	vector<Model*> models;
	Scene() {};
	Scene(Renderer* renderer);
	int loadOBJModel(string fileName);
	int loadCubeModel();
	void lookAtModel(int modelId);
	int addCamera();
	void lookAtCamera(int cameraId);
	void switchToCamera(int cameraId);
	void ClearScene();
	void rotateAroundActiveModel(int dx, int dy);
	void manipulateActiveModel(Transformation T,TransformationDirection direction, TransAxis axis);
	void setActiveCameraProjection(Projection proj);
	const Projection GetProjection();
	bool toggleShowVerticesNormals();
	bool toggleShowFacesNormals();
	bool toggleRenderCameras();
	bool toggleDrawBoundBox();
	void draw();
	void drawDemo();
	int modelMenuIdToVectorId(int menuId);
	Camera* GetActiveCamera();
	void Zoom(ZoomDirection direction);
	int activeModel;
	int activeLight;
	int activeCamera;
	void ChangeProjectionParameters(Projection proj, vec3 rtf, vec3 lbn);
	void SetTrasformationAxis(TransAxis Axis);
	TransAxis GetTrasformationAxis();
};