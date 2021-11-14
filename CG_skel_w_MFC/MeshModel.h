#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>
#include <set>
#include <tuple>

using namespace std;

class MeshModel : public Model
{
protected :
	MeshModel() {}
	void moveModel(Axis direction);
	void rotateModel(Axis direction);
	void scaleModel(Axis direction);
	void preformTransform(mat4& matrix, Transformation T);
	//add more attributes
	mat3 _normal_transform;
	float x_bound_lenght;
	float y_bound_lenght;
	float z_bound_lenght;
	vec3 center;

public:	
	vector<vec3>* vertex_positions;
	vector<vec3>* vertix_normals;
	vector<vec3>* vertices_normal_end_positions; // used for renderer
	vector<vec3>* faces_normals;
	vector<vec3>* faces_centers;
	vector<vec3>* faces_normal_end_positions; // used for renderer
	mat4 _world_transform;
	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(Renderer* renderer);
	void drawBoundingBox(Renderer* renderer);
	vec3 getPosition();
	void CalcBounds();
	vec3 GetCenter();
	GLfloat GetXBoundLength();
	GLfloat GetYBoundLength();
	GLfloat GetZBoundLength();
	vec3 GetBoundsLength();
	void manipulateModel(Transformation T, Axis axis);
};

class PrimMeshModel : public MeshModel
{
	GLfloat posX;
	GLfloat posY;
	GLfloat posZ;
	GLfloat lenX;
	GLfloat lenY;
	GLfloat lenZ;
public:
	PrimMeshModel();
	PrimMeshModel(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat lenX, GLfloat lenY, GLfloat lenZ);
	void draw(Renderer* renderer);
};


