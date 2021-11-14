#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
using namespace std;

#define ZOOM 2
#define ILLEGAL_ACTIVE_MOVEL -1

enum Axis
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
	PRESPECTIVE
};

enum Direction
{
	ZOOM_IN,
	ZOOM_OUT
};



class Model { 
public:
	void virtual draw(Renderer* renderer) = 0;
	vec3 virtual getPosition() = 0;
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
	Camera(vec3 lbn, vec3 rtf) :lbn(lbn), rtf(rtf) {};
	mat4 cTransform;
	mat4 projection;
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
	bool draw_bound_box = false;
	Renderer* m_renderer;

public:
	
	vector<Model*> models;
	
	Scene() {};
	Scene(Renderer* renderer);
	void loadOBJModel(string fileName);
	void loadCubeModel();
	void lookAtModel(int modelId);
	void ClearScene();
	void rotateAroundActiveModel(Axis direction);
	void manipulateActiveModel(Transformation T,Axis axis);
	void setActiveCameraProjection(Projection proj);
	const Projection GetProjection();
	bool toggleShowVerticesNormals();
	bool toggleShowFacesNormals();
	void draw();
	void drawDemo();
	bool updateDrawBoundBox();
	Camera* GetActiveCamera();
	void Zoom(Direction direction);
	int activeModel;
	int activeLight;
	int activeCamera;
};