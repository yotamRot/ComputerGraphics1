#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>
#include <set>
#include <tuple>

using namespace std;

struct Texture {
	unsigned int id;
	string type;
};

struct Vertex {
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
};

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
	int modelId;

public:	
	float ka;// fraction of ambient light reflected from surface 
	float kd;
	float ks;
	float ke;
	bool is_non_unfiorm;
	GLuint my_program;
	unsigned int VAO, VBO, EBO;


	vector<Vertex>       vertices;
	vector<vec4>       tempo;
	vector<unsigned int> indices;
	vector<Texture>      textures;

	vector<Line>* bound_box_vertices;

	vec3 mesh_color;
	vec3 matirials; // x - emissive, y - diffuse, z - specular

	MeshModel(string fileName, int modelId, GLuint program);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw();
	vec3 CenteringTranslation(TransAxis axis);
	vector<Line>* CalcBounds();
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
	void UpdateTriangleIlluminationParams();
	void RandomizePolygons();
	void  SetBoundingBoxVertices(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat lenX, GLfloat lenY, GLfloat lenZ);
	void SetupMesh();

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
		CameraModel(int cameraIndex, GLuint program);
};

class LightModel : public MeshModel
{
	public:
		int lightIndex;
		vec3 light_color;
		float La;
		//float Ld;
		//float Ls;
		LightModel(int model_id, int lightIndex, GLuint program);
};

mat4 matrixInverse(mat4& mat, Transformation T);
