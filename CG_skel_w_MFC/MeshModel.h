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
	MeshModel() = default;
	mat4 moveModel(TransformationDirection direction, TransAxis axis);
	mat4 scaleModel(TransformationDirection direction, TransAxis axis);
	mat4 rotateModel(TransformationDirection direction, int angle, TransAxis axis);
	//add more attributes
	mat4 _world_normal_transform;
	mat4 _model_normal_transform;
	float x_bound_lenght;
	float y_bound_lenght;
	float z_bound_lenght;
	vec3 center;

public:	

	vector<Triangle>* triangles;
	vector<Normal>* facesNormals;
	vector<Normal>* verticesNormals;

	int mesh_color_index;
	vector<vec3>* bound_box_vertices;
	mat4 _model_transform;
	mat4 _world_transform;

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(Renderer* renderer);
	vec3 CenteringTranslation(TransAxis axis);
	vector<vec3>* CalcBounds();
	vec3 GetCenter();
	GLfloat GetXBoundLength();
	GLfloat GetYBoundLength();
	GLfloat GetZBoundLength();
	vec3 GetBoundsLength();
	mat4 manipulateModel(Transformation T, TransformationDirection direction,
						TransAxis axis, float power);
	mat4 GetObjectMatrix();
	mat4 GetNormalMatrix();
	GLfloat GetProportionalValue();
	void UpdateTriangleColor();
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
};

class CameraModel : public MeshModel
{
	public:
		int cameraIndex;
		mat4 _w_TransformInv;
		mat4 _m_TransformInv;
		CameraModel(int cameraIndex);
};

class LightModel : public MeshModel
{
	public:
		int lightIndex;
		mat4 _w_TransformInv;
		mat4 _m_TransformInv;
		LightModel(int lightIndex);
};

mat4 matrixInverse(mat4& mat, Transformation T);
