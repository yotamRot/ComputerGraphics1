#pragma once
#include "vec.h"
#include "mat.h"
#include <string>
#include <set>
#include <tuple>
#include <vector>
#include "Renderer.h"

using namespace std;

enum TransformationDirection
{
	X_dir,
	Xn_dir,
	Y_dir,
	Yn_dir,
	Z_dir,
	Zn_dir
};


struct Texture {
	unsigned int id;
	string type;
};

struct Vertex {
	vec3 Position;
	vec3 V_Normal;
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


public:	
	float ka;// fraction of ambient light reflected from surface 
	float kd;
	float ks;
	float ke;
	bool is_non_unfiorm;
	GLuint my_program;
	LightType type;
	GLuint simple_shader;
	unsigned int VAO, VBO, EBO;
	unsigned int bounding_box_VAO, bounding_box_VBO, bounding_box_EBO;
	unsigned int vertex_normal_VAO, vertex_normal_VBO, vertex_normal_EBO;
	unsigned int face_normal_VAO, face_normal_VBO, face_normal_EBO;
	int modelId;

	//vector<Vertex>       vertices;
	vector<Vertex>       vertices;
	vector<unsigned int> indices;
	vector<Texture>      textures;

	vector<vec3> bound_box_vertices;
	vector<unsigned int> bound_box_indices;
	vector<vec3> vertices_normals;
	vector<unsigned int> vertices_normals_indices;
	vector<vec3> face_normals;
	vector<unsigned int> faces_normals_indices;

	vec3 mesh_color;
	vec3 matirials; // x - emissive, y - diffuse, z - specular

	MeshModel(string fileName, int modelId, GLuint program, GLuint simpleShader);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(bool draw_bounding_box, bool draw_vertix_normals, bool draw_faces_normals);
	void drawBoundingBox();
	void drawVerticesNormals();
	void drawFacesNormals();
	vec3 CenteringTranslation(TransAxis axis);
	void CalcBounds();
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
		float Ld;
		float Ls;
		float l_alpha;
		vec3 c_light_position;
		LightModel(int model_id, int lightIndex, GLuint program);
		vec4 GetL();
};

mat4 matrixInverse(mat4& mat, Transformation T);
