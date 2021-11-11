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
	vector<vec3>* vertex_positions;
	//add more attributes
	mat3 _normal_transform;

public:
	mat4 _world_transform;
	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(Renderer* renderer);
	
};

class PrimMeshModel : public MeshModel
{
	GLfloat posX;
	GLfloat posY;
	GLfloat posZ;
	GLfloat length;
public:
	PrimMeshModel(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat length) :posX(posX), posY(posY), posZ(posZ), length(length) {};
	void draw(Renderer* renderer);
};