#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

using namespace std;

class MeshModel : public Model
{
protected :
	MeshModel() {}

	//add more attributes
	mat3 _normal_transform;

public:
	vector<vec3>* vertex_positions;
	vector<vec3>* world_vertex_positions;
	mat4 _world_transform;
	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(Renderer* renderer);
	vec3 getPosition();
};

class PrimMeshModel : public MeshModel
{
	GLfloat posX;
	GLfloat posY;
	GLfloat posZ;
	GLfloat length;
public:
	PrimMeshModel(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat length);
	void draw(Renderer* renderer);
	//vec3 getPosition();
};