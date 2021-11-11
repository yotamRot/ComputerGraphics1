#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
using namespace std;

class Model { 
public:
	void virtual draw(Renderer* renderer) = 0;
protected:
	virtual ~Model() {}
};


class Light {

};

class Camera {

public:
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

};

class Scene {

	
	vector<Light*> lights;
	
	Renderer *m_renderer;

public:
	vector<Model*> models;
	vector<Camera*> cameras;
	Scene() {};
	Scene(Renderer* renderer);
	void loadOBJModel(string fileName);
	void draw();
	void DrawRec();
	void drawDemo();
	
	int activeModel;
	int activeLight;
	int activeCamera;
};