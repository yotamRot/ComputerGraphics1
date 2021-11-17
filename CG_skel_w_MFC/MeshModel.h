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
	mat4 moveModel(TransformationDirection direction, TransAxis axis);
	mat4 scaleModel(TransformationDirection direction, TransAxis axis);
	//add more attributes
	mat4 _world_normal_transform;
	mat4 _model_normal_transform;
	float x_bound_lenght;
	float y_bound_lenght;
	float z_bound_lenght;
	vec3 center;

public:	
	vector<vec3>* vertex_positions;
	vector<vec3>* vertix_normals;
	vector<vec3>* faces_normals;
	vector<vec3>* faces_centers;
	vector<vec3>* bound_box_vertices;
	mat4 _model_transform;
	mat4 _world_transform;

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(Renderer* renderer);
	vec3 GetPosition();
	vector<vec3>* CalcBounds();
	vec3 GetCenter();
	GLfloat GetXBoundLength();
	GLfloat GetYBoundLength();
	GLfloat GetZBoundLength();
	vec3 GetBoundsLength();
	mat4 manipulateModel(Transformation T, TransformationDirection direction,
						TransAxis axis, float power);
	mat4 rotateModel(TransformationDirection direction, int angle, TransAxis axis);
	mat4 GetObjectMatrix();
	mat4 GetNormalMatrix();
	GLfloat GetProportionalValue();
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


class CameraModel : public MeshModel
{
	public:
		int cameraIndex;
		mat4 _w_TransformInv;
		mat4 _m_TransformInv;
		CameraModel(int cameraIndex);
};

mat4 matrixInverse(mat4& mat, Transformation T);
