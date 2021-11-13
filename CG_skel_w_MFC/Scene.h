#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
using namespace std;

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



class Model { 
public:
	void virtual draw(Renderer* renderer, bool isShowVerticsNormals, bool draw_bound_box) = 0;
	vec3 virtual getPosition() = 0;
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
	void draw();
	void drawDemo();
	bool updateDrawBoundBox();
	Camera* GetActiveCamera();
	int activeModel;
	int activeLight;
	int activeCamera;
};