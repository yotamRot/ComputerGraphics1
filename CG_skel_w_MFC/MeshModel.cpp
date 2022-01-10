#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "InitShader.h"
#include <GL/freeglut_std.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



using namespace std;

struct FaceIdcs
{
	int v[4];
	int vn[4];
	int vt[4];
	

	FaceIdcs()
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;
	}

	FaceIdcs(std::istream & aStream)
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;

		char c;
		for(int i = 0; i < 3; i++)
		{
			aStream >> std::ws >> v[i] >> std::ws;
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> std::ws;
			if (aStream.peek() == '/')
			{
				aStream >> c >> std::ws >> vn[i];
				continue;
			}
			else
				aStream >> vt[i];
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> vn[i];
		}
	}
};

void  MeshModel::SetBoundingBoxVertices(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat lenX, GLfloat lenY, GLfloat lenZ)
{

	GLfloat halfX = lenX * 0.5f;
	GLfloat halfY = lenY * 0.5f;
	GLfloat halfZ = lenZ * 0.5f;

	// front face points
	vec3 frontTopLeft = vec3(posX - halfX, posY + halfY, posZ - halfZ);
	vec3 frontTopRight = vec3(posX + halfX, posY + halfY, posZ - halfZ);
	vec3 frontBottomLeft = vec3(posX - halfX, posY - halfY, posZ - halfZ);
	vec3 frontBottomRight = vec3(posX + halfX, posY - halfY, posZ - halfZ);

	// back face points
	vec3 backTopLeft = vec3(posX - halfX, posY + halfY, posZ + halfZ);
	vec3 backTopRight = vec3(posX + halfX, posY + halfY, posZ + halfZ);
	vec3 backBottomLeft = vec3(posX - halfX, posY - halfY, posZ + halfZ);
	vec3 backBottomRight = vec3(posX + halfX, posY - halfY, posZ + halfZ);

	//lines

	bound_box_vertices.push_back(frontTopLeft);
	bound_box_vertices.push_back(frontTopRight);
	bound_box_vertices.push_back(frontTopLeft);
	bound_box_vertices.push_back(frontBottomLeft);
	bound_box_vertices.push_back(frontTopLeft);
	bound_box_vertices.push_back(backTopLeft);

	bound_box_vertices.push_back(backTopRight);
	bound_box_vertices.push_back(backTopLeft);
	bound_box_vertices.push_back(backTopRight);
	bound_box_vertices.push_back(backBottomRight);
	bound_box_vertices.push_back(backTopRight);
	bound_box_vertices.push_back(frontTopRight);

	bound_box_vertices.push_back(backBottomLeft);
	bound_box_vertices.push_back(backTopLeft);
	bound_box_vertices.push_back(backBottomLeft);
	bound_box_vertices.push_back(backBottomRight);
	bound_box_vertices.push_back(backBottomLeft);
	bound_box_vertices.push_back(frontBottomLeft);

	bound_box_vertices.push_back(frontBottomRight);
	bound_box_vertices.push_back(frontBottomLeft);
	bound_box_vertices.push_back(frontBottomRight);
	bound_box_vertices.push_back(frontTopRight);
	bound_box_vertices.push_back(frontBottomRight);
	bound_box_vertices.push_back(backBottomRight);
	for (int i = 0; i < bound_box_vertices.size(); i++)
	{
		bound_box_indices.push_back(i);// = i;
	}
	//return lines;
}


vec3 vec3fFromStream(std::istream & aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}

vec2 vec2fFromStream(std::istream & aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return vec2(x, y);
}

mat4 CreateNormalTransform(mat4& matrix, Transformation T)
{
	float normalFactor = matrix[3][3];
	mat4 normalMatrix = matrix;
	normalMatrix = normalMatrix / normalFactor;
	if (T == SCALE) // need to inverse and transpose
	{
		normalMatrix[0][0] = 1 / matrix[0][0];
		normalMatrix[1][1] = 1 / matrix[1][1];
		normalMatrix[2][2] = 1 / matrix[2][2];
	}
	return normalMatrix;
}

MeshModel::MeshModel(string fileName, int modelId, GLuint program, GLuint simpleShader):mesh_color(BLUE), ka(0.5), kd(0.8), ks(1.0),ke(0), modelId(modelId), my_program(program), simple_shader(simpleShader)
{
	is_non_unfiorm = false;
	loadFile(fileName);
	CalcBounds();
	SetupMesh();
	_world_transform[2][3] = -5;
}

MeshModel::~MeshModel(void)
{
}

std::string dirnameOf(const std::string& fname)
{
	size_t pos = fname.find_last_of("\\/");
	return (std::string::npos == pos)
		? ""
		: fname.substr(0, pos);
}

int check_key(map<int, int> &m, int key, int val)
{
	// Key is not present
	if (m.find(key) == m.end())
	{
		m[key] = val;
		return val;
	}
		
	return m[key];
}
void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> l_vertices;
	vector<vec3> v_normals;
	vector<vec2> v_textures;
	vec3 curCenter;
	vec3 curNormalEnd;
	std::map<int, int> u;

	Vertex tempVertix;

	vec3 p1, p2 , p3;
	vec3 p1_nomral, p2_nomral, p3_nomral;
	vec2 p1_texture, p2_texture, p3_texture;

	// while not end of file
	while (!ifile.eof())
	{
		// get line
		string curLine;
		getline(ifile, curLine);

		// read type of the line
		istringstream issLine(curLine);
		string lineType;

		issLine >> std::ws >> lineType;

		// based on the type parse data
		if (lineType == "v") /*FIXED*/
			l_vertices.push_back(vec3fFromStream(issLine));
		if (lineType == "vn") /*FIXED*/
			v_normals.push_back(vec3fFromStream(issLine));
		if (lineType == "vt") /*FIXED*/
			v_textures.push_back(vec2fFromStream(issLine));
		else if (lineType == "f") /*FIXED*/
			faces.push_back(issLine);
		else if (lineType == "#" || lineType == "")
		{
			// comment / empty line
		}
		else
		{
			cout<< "Found unknown line Type \"" << lineType << "\"";
		}
	}
	//Vertex_positions is an array of vec3. Every three elements define a triangle in 3D.
	//If the face part of the obj is
	//f 1 2 3
	//f 1 3 4
	//Then vertex_positions should contain:
	//vertex_positions={v1,v2,v3,v1,v3,v4}
		// iterate through all stored faces and create triangles
	vertices = vector<Vertex>(l_vertices.size(), tempVertix);

	// iterate through all stored faces and create triangles
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		p1 = l_vertices[it->v[0] - 1];
		p2 = l_vertices[it->v[1] - 1];
		p3 = l_vertices[it->v[2] - 1];

		curCenter = (p1 + p2 + p3) / 3;
		curNormalEnd = normalize(cross(p2 - p1, p3 - p1));
		face_normals.push_back(curCenter);
		face_normals.push_back(curCenter + curNormalEnd);

		if (v_normals.size() != 0)
		{
			p1_nomral = (normalize(v_normals.at(check_key(u, it->v[0] - 1, it->vn[0] - 1))));
			
			p2_nomral = (normalize(v_normals.at(check_key(u, it->v[1] - 1, it->vn[1] - 1))));
			
			p3_nomral = (normalize(v_normals.at(check_key(u, it->v[2] - 1, it->vn[2] - 1))));
		
		}
		else
		{
			p1_nomral = p2_nomral = p3_nomral = curNormalEnd;
		}

		vertices_normals.push_back(p1);
		vertices_normals.push_back(p1 + p1_nomral);
		vertices_normals.push_back(p2);
		vertices_normals.push_back(p2 + p2_nomral);
		vertices_normals.push_back(p3);
		vertices_normals.push_back(p3 + p3_nomral);

		if (v_textures.size() != 0)
		{
			p1_texture = v_textures[it->vt[0] - 1];
			p2_texture = v_textures[it->vt[1] - 1];
			p3_texture = v_textures[it->vt[2] - 1];
			fileTexCord.push_back(p1_texture);
			fileTexCord.push_back(p2_texture);
			fileTexCord.push_back(p3_texture);
		}


		tempVertix.Position = p1;
		tempVertix.V_Normal = p1_nomral;
		tempVertix.TexCoords = p1_texture;
		this->vertices[it->v[0] - 1] = tempVertix;
		this->indices.push_back(it->v[0] - 1);

		tempVertix.Position = p2;
		tempVertix.V_Normal = p2_nomral;
		tempVertix.TexCoords = p2_texture;
		this->vertices[it->v[1] - 1] = tempVertix;
		this->indices.push_back(it->v[1] - 1);

		tempVertix.Position = p3;
		tempVertix.V_Normal = p3_nomral;
		tempVertix.TexCoords = p3_texture;
		this->vertices[it->v[2] - 1] = tempVertix;
		this->indices.push_back(it->v[2] - 1);
	}

	for (int i = 0; i < face_normals.size(); i++)
	{
		faces_normals_indices.push_back(i);
	}

	for (int i = 0; i < vertices_normals.size(); i++)
	{
		vertices_normals_indices.push_back(i);
	}
	
	if (v_textures.size() != 0)
	{
		texture.wrap = fromFile;
	}
	else
	{
		texture.wrap = Cylinder;
		CylinderMapping();
	}


	// load and set texture
	glGenTextures(1, &this->texture.id);
	glBindTexture(GL_TEXTURE_2D, this->texture.id);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(&(dirnameOf(fileName) + "\\texture.png")[0], &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);

	glUseProgram(my_program);
	glUniform1i(glGetUniformLocation(my_program, "texture1"), 0);
}

void MeshModel::SetupMesh()
{
	glUseProgram(my_program);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
		&indices[0], GL_STATIC_DRAW);

	// vertex positions
	GLuint loc = glGetAttribLocation(my_program, "vPosition");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	loc = glGetAttribLocation(my_program, "vNormal");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, V_Normal));
	// vertex texture coords
	loc = glGetAttribLocation(my_program, "vTexture");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	if (bound_box_vertices.size()!=0)
	{

		glUseProgram(simple_shader);
		//bounding box
		glGenVertexArrays(1, &bounding_box_VAO);
		glGenBuffers(1, &bounding_box_VBO);
		glGenBuffers(1, &bounding_box_EBO);

		glBindVertexArray(bounding_box_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, bounding_box_VBO);

		glBufferData(GL_ARRAY_BUFFER, bound_box_vertices.size() * sizeof(vec3), &bound_box_vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bounding_box_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, bound_box_indices.size() * sizeof(unsigned int),
			&bound_box_indices[0], GL_STATIC_DRAW);

		// vertex positions
		loc = glGetAttribLocation(simple_shader, "vPosition");
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

		//vertex normals 

		glGenVertexArrays(1, &vertex_normal_VAO);
		glGenBuffers(1, &vertex_normal_VBO);
		glGenBuffers(1, &vertex_normal_EBO);

		glBindVertexArray(vertex_normal_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices_normals.size() * sizeof(vec3), &vertices_normals[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_normal_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices_normals_indices.size() * sizeof(unsigned int),
			&vertices_normals_indices[0], GL_STATIC_DRAW);

		// vertex positions
		loc = glGetAttribLocation(simple_shader, "vPosition");
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

		// faces normals

		glGenVertexArrays(1, &face_normal_VAO);
		glGenBuffers(1, &face_normal_VBO);
		glGenBuffers(1, &face_normal_EBO);

		glBindVertexArray(face_normal_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, face_normal_VBO);

		glBufferData(GL_ARRAY_BUFFER, face_normals.size() * sizeof(vec3), &face_normals[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_normal_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces_normals_indices.size() * sizeof(unsigned int),
			&faces_normals_indices[0], GL_STATIC_DRAW);
		// vertex positions
		loc = glGetAttribLocation(simple_shader, "vPosition");
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	}

}

void MeshModel::FileMapping()
{
	for (int i= 0; i < vertices.size(); i++)
	{
		vertices.at(i).TexCoords = fileTexCord.at(i);
	}
}

void MeshModel::CylinderMapping()
{
	for (auto it = vertices.begin(); it != vertices.end(); ++it)
	{
		it->TexCoords.x = (atan2(it->Position.z, it->Position.x)) + M_PI;
		it->TexCoords.y = it->Position.y;
	}
}

void MeshModel::CalcBounds()
{
	vec3 min_bound, max_bound;
	for (auto it = vertices.begin(); it != vertices.end(); ++ it)
	{
		// find max
		if (it->Position.x > max_bound.x)
		{
			max_bound.x = it->Position.x;
		}
		if (it->Position.y > max_bound.y)
		{
			max_bound.y = it->Position.y;
		}
		if (it->Position.z > max_bound.z)
		{
			max_bound.z = it->Position.z;
		}
		// find min
		if (it->Position.x < min_bound.x)
		{
			min_bound.x = it->Position.x;
		}
		if (it->Position.y < min_bound.y)
		{
			min_bound.y = it->Position.y;
		}
		if (it->Position.z < min_bound.z)
		{
			min_bound.z = it->Position.z;
		}
	}
	
	x_bound_lenght = fabs(max_bound.x - min_bound.x);
	y_bound_lenght = fabs(max_bound.y - min_bound.y);
	z_bound_lenght = fabs(max_bound.z - min_bound.z);
	center = (max_bound + min_bound) / 2;
	SetBoundingBoxVertices(center.x, center.y, center.z, x_bound_lenght, y_bound_lenght, z_bound_lenght);
}


vec3 MeshModel::GetCenter()
{
	return center;
}

GLfloat MeshModel::GetZBoundLength()
{
	return z_bound_lenght;
}
GLfloat MeshModel::GetXBoundLength()
{
	return x_bound_lenght;
}
GLfloat MeshModel::GetYBoundLength()
{
	return y_bound_lenght;
}
vec3 MeshModel::GetBoundsLength()
{
	return vec3(x_bound_lenght, y_bound_lenght, z_bound_lenght) ;
}


void MeshModel::draw(bool draw_bounding_box, bool draw_vertix_normals, bool draw_faces_normals)
{	
	glUseProgram(my_program);
	GLint my_color_location = glGetUniformLocation(my_program, "color");
	glUniform3f(my_color_location, mesh_color.x, mesh_color.y, mesh_color.z);

	GLint umM = glGetUniformLocation(my_program, "modelMatrix"); // Find the modelMatrix variable
	mat4 modelTrans = this->_world_transform * this->_model_transform;
	GLfloat modelMatrix[16];
	MattoArr(modelMatrix, modelTrans);
	glUniformMatrix4fv(umM, 1, GL_FALSE, modelMatrix);

	GLint umN = glGetUniformLocation(my_program, "normalMatrix"); // Find the normalMatrix variable
	mat4 normalTrans = this->_world_normal_transform * this->_model_normal_transform;
	GLfloat normalMatrix[16];
	MattoArr(normalMatrix, normalTrans);
	glUniformMatrix4fv(umN, 1, GL_FALSE, normalMatrix);

	GLint uKa = glGetUniformLocation(my_program, "Ka"); // Find the Ka variable
	glUniform1i(uKa, ka);
	GLint uKd = glGetUniformLocation(my_program, "Kd"); // Find the Kd variable
	glUniform1i(uKd, kd);
	GLint uKs = glGetUniformLocation(my_program, "Ks"); // Find the Ks variable
	glUniform1i(uKs, ks);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glFlush();
	if (draw_bounding_box)
	{
		drawBoundingBox();
	}

	if (draw_vertix_normals)
	{
		drawVerticesNormals();
	}

	if (draw_faces_normals)
	{
		drawFacesNormals();
	}


}

void MeshModel::drawBoundingBox()
{
	glUseProgram(simple_shader);
	GLint my_color_location = glGetUniformLocation(simple_shader, "color");
	glUniform3f(my_color_location, 1,1,0);

	GLint umM = glGetUniformLocation(simple_shader, "modelMatrix"); // Find the modelMatrix variable
	mat4 modelTrans = _world_transform * _model_transform;
	GLfloat modelMatrix[16];
	MattoArr(modelMatrix, modelTrans);
	glUniformMatrix4fv(umM, 1, GL_FALSE, modelMatrix);

	glBindVertexArray(bounding_box_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, bounding_box_VBO);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	glDrawElements(GL_LINES, bound_box_indices.size(), GL_UNSIGNED_INT, 0);
	glFlush();
}

void MeshModel::drawFacesNormals()
{
	glUseProgram(simple_shader);
	GLint my_color_location = glGetUniformLocation(simple_shader, "color");
	glUniform3f(my_color_location, 1, 0, 0);

	GLint umM = glGetUniformLocation(simple_shader, "modelMatrix"); // Find the modelMatrix variable
	mat4 modelTrans = _world_normal_transform * _model_normal_transform;
	GLfloat modelMatrix[16];
	MattoArr(modelMatrix, modelTrans);
	glUniformMatrix4fv(umM, 1, GL_FALSE, modelMatrix);

	glBindVertexArray(face_normal_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, face_normal_VBO);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	glDrawElements(GL_LINES, faces_normals_indices.size(), GL_UNSIGNED_INT, 0);
	glFlush();
}

void MeshModel::drawVerticesNormals()
{
	glUseProgram(simple_shader);
	GLint my_color_location = glGetUniformLocation(simple_shader, "color");
	glUniform3f(my_color_location, 1, 0, 0);

	GLint umM = glGetUniformLocation(simple_shader, "modelMatrix"); // Find the modelMatrix variable
	mat4 modelTrans = _world_normal_transform * _model_normal_transform;
	GLfloat modelMatrix[16];
	MattoArr(modelMatrix, modelTrans);
	glUniformMatrix4fv(umM, 1, GL_FALSE, modelMatrix);

	glBindVertexArray(vertex_normal_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_VBO);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	glDrawElements(GL_LINES, vertices_normals_indices.size(), GL_UNSIGNED_INT, 0);
	glFlush();
}

vec3 MeshModel::CenteringTranslation(TransAxis axis)
{
	vec3 worldCenterize = vec3(_world_transform[0][3] / _world_transform[3][3],
		_world_transform[1][3] / _world_transform[3][3],
		_world_transform[2][3] / _world_transform[3][3]);
	vec3 modelCenterize = vec3(_model_transform[0][3] / _model_transform[3][3],
		_model_transform[1][3] / _model_transform[3][3],
		_model_transform[2][3] / _model_transform[3][3]);
	if (axis == MODEL)
	{
		return modelCenterize;
	}
	else
	{
		return worldCenterize;
	}
}


PrimMeshModel::PrimMeshModel(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat lenX, GLfloat lenY, GLfloat lenZ) : posX(posX), posY(posY), posZ(posZ), lenX(lenX), lenY(lenY), lenZ(lenZ)
{
	//ka = 0.5;
	//kd = 0.8;
	//ks = 1.0;
	//ke = 0;
	//is_non_unfiorm = false;
	//triangles = new vector<Triangle>;
	//vec3 p1, p2, p3;

	//_world_transform[2][3] = -5;
	////faces_normal_end_positions = new vector<vec3>;

	//Triangle curTriangle;
	//Normal curFaceNormal;
	//GLfloat halfX = lenX * 0.5f;
	//GLfloat halfY = lenY * 0.5f;
	//GLfloat halfZ = lenZ * 0.5f;
	//mesh_color = RED;



	//// front face triangels
	//p1 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // bottom left
	//p2 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top left
	//p3 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, 1), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);

	//p1 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // bottom left
	//p2 = vec3(posX + halfX, posY + halfY, posZ + halfZ); // top left
	//p3 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, 1), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);

	//// back face triangels
	//p1 = vec3(posX - halfX, posY - halfY, posZ - halfZ); // bottom left
	//p2 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	//p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // b,ottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, -1), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);

	//p1 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	//p2 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // top right
	//p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, -1), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3 , mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);

	//// left face triangels
	//p1 = vec3(posX - halfX, posY - halfY, posZ - halfZ); // bottom left
	//p2 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	//p3 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(-1, 0, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);

	//p1 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	//p2 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top right
	//p3 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(-1, 0, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);

	//// right face triangels
	////high triangle
	//p1 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // bottom left
	//p2 = vec3(posX + halfX, posY + halfY, posZ + halfZ); // top left
	//p3 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // top right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(1, 0, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);


	////low triangle
	//p1 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // bottom left
	//p2 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // top right
	//p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(1, 0, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);


	//// top face triangels
	//p1 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // bottom left
	//p2 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top left
	//p3 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 1, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);

	//p1 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top left
	//p2 = vec3(posX + halfX, posY + halfY, posZ + halfZ); // top right
	//p3 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 1, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);
	//
	//// down face triangels
	//p1 = vec3(posX - halfX, posY - halfY, posZ - halfZ); // bottom left
	//p2 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // top left
	//p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, -1, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);

	//p1 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // top left
	//p2 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // top right
	//p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, -1, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);
 //
	//bound_box_vertices = CalcBounds();
	//UpdateTriangleIlluminationParams();
}

CameraModel::CameraModel(int cameraIndex, GLuint program) : cameraIndex(cameraIndex)
{
	ka = 0.5;
	kd = 0.8;
	ks = 1.0;
	//ke = 1.0;
	//is_non_unfiorm = false;
	//triangles = new vector<Triangle>;
	//Normal curFaceNormal;
	//vec3 p1, p2, p3;
	//mesh_color = GREEN;
	////faces_normal_end_positions = new vector<vec3>;

	//Triangle curTriangle;
	//_world_transform[2][3] = 2; //initialized same as camera regarding location


	//// first triangle
	//p1 = vec3(0.1, 0, 0); // bottom left
	//p2 = vec3(-0.1, 0, 0); // top left
	//p3 = vec3(0, 0, -0.5); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 1, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);

	//// second triangle
	//p1 = vec3(0, 0.1, 0); // bottom left
	//p2 = vec3(0, -0.1, 0); // top left
	//p3 = vec3(0, 0, -0.5); // bottom right
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(1, 0, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	//triangles->push_back(curTriangle);
	//
	//bound_box_vertices = CalcBounds();
	//UpdateTriangleIlluminationParams();
}

LightModel::LightModel(int model_id, int lightIndex, GLuint program) : lightIndex(lightIndex), light_color(vec3(1,1,1)),La(0.5),Ld(0.8),Ls(1.0)
{
	ka = 1;
	kd = 1;
	ks = 1;
	//ke = 1;
	//is_non_unfiorm = false;
	modelId = model_id;
	my_program = program;
	mesh_color = RED;
	//triangles = new vector<Triangle>;
	//Normal curFaceNormal;
	vec3 p1, p2, p3;
	////faces_normal_end_positions = new vector<vec3>;

	//Triangle curTriangle;
	//_world_transform[2][3] = -2; //initialized same as camera regarding location

	Vertex temp;
	// first triangle
	p1 = vec3(0.1, 0, 0); // bottom left
	temp.Position = p1;

	vertices.push_back(temp);

	p2 = vec3(-0.1, 0, 0); // top left
	temp.Position = p2;
	vertices.push_back(temp);
	p3 = vec3(0, 0, -0.1); // bottom right
	temp.Position = p3;
	vertices.push_back(temp);
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 1, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, true, curFaceNormal);
	//triangles->push_back(curTriangle);
	// first triangle other side
	p1 = vec3(0.1, 0, 0); // bottom left
	temp.Position = p1;
	vertices.push_back(temp);
	p2 = vec3(-0.1, 0, 0); // top left
	temp.Position = p2;
	vertices.push_back(temp);
	p3 = vec3(0, 0, -0.1); // bottom right
	temp.Position = p3;
	vertices.push_back(temp);
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, -1, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, true, curFaceNormal);
	//triangles->push_back(curTriangle);

	// second triangle
	p1 = vec3(0, 0.1, 0); // bottom left
	temp.Position = p1;
	vertices.push_back(temp);
	p2 = vec3(0, -0.1, 0); // top left
	temp.Position = p2;
	vertices.push_back(temp);
	p3 = vec3(0, 0, -0.1); // bottom right
	temp.Position = p3;
	vertices.push_back(temp);
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(1, 0, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, true, curFaceNormal);
	//triangles->push_back(curTriangle);
	// second triangle other side
	p1 = vec3(0, 0.1, 0); // bottom left
	temp.Position = p1;
	vertices.push_back(temp);
	p2 = vec3(0, -0.1, 0); // top left
	temp.Position = p2;
	vertices.push_back(temp);
	p3 = vec3(0, 0, -0.1); // bottom right
	temp.Position = p3;
	vertices.push_back(temp);
	//curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(-1, 0, 0), false, face_normal);
	//curTriangle = Triangle(p1, p2, p3, mesh_color, true, curFaceNormal);
	//triangles->push_back(curTriangle);

	for (int i = 0; i < vertices.size(); i++)
	{
		indices.push_back(i);
	}

	//bound_box_vertices = CalcBounds();
	//UpdateTriangleIlluminationParams();
	SetupMesh();

}

vec4 LightModel::GetL()
{
	return vec4();
}

mat4 matrixInverse(mat4& mat , Transformation T)
{
	mat4 InverseMat;
	switch (T)
	{
		case MOVE:
			InverseMat = mat;
			InverseMat[0][3] = (-1) * mat[0][3];
			InverseMat[1][3] = (-1) * mat[1][3];;
			InverseMat[2][3] = (-1) * mat[2][3];
			break;
		case ROTATE:
			InverseMat = transpose(mat);
			break;
		case SCALE:
			InverseMat = mat;
			InverseMat[0][0] = 1 / mat[0][0];
			InverseMat[1][1] = 1 / mat[1][1];
			InverseMat[2][2] = 1 / mat[2][2];
			break;
	}
	return InverseMat;
}

mat4 MeshModel::moveModel(TransformationDirection direction, TransAxis axis)
{
	mat4 tranlateMatrix;
	GLfloat move = 0.1; //(x_bound_lenght + y_bound_lenght + z_bound_lenght) / 3;
	if (dynamic_cast<LightModel*>(this))
	{
		move = move * 10;
	}
	switch (direction)
	{
		case X_dir:
			tranlateMatrix = Translate(move*0.1, 0, 0);
			break;
		case Xn_dir:
			tranlateMatrix = Translate(-move * 0.1, 0, 0);
			break;
		case Y_dir:
			tranlateMatrix = Translate(0, move *0.1, 0);
			break;
		case Yn_dir:
			tranlateMatrix = Translate(0, -move * 0.1, 0);
			break;
		case Z_dir:
			tranlateMatrix = Translate(0, 0, move * 0.1);
			break;
		case Zn_dir:
			tranlateMatrix = Translate(0, 0, -move * 0.1);
			break;
		default:
			break;
	}

	if (axis == WORLD)
	{
		_world_transform = tranlateMatrix * _world_transform;						// translate in world axis
	}
	else		// axis == MODEL	
	{
		_model_transform = tranlateMatrix * _model_transform;						// translate in model axis
	}
	return matrixInverse(tranlateMatrix, MOVE);
}

mat4 MeshModel::rotateModel(TransformationDirection direction, int angle, TransAxis axis)
{
	mat4 rotateMatrix;
	mat4 cameraInverseMatrix;
	vec3 centeringVec;
	switch (direction)
	{
	case X_dir:
		rotateMatrix = RotateX(angle);
		break;
	case Xn_dir:
		rotateMatrix = RotateX(-angle);
		break;
	case Y_dir:
		rotateMatrix = RotateY(angle);
		break;
	case Yn_dir:
		rotateMatrix = RotateY(-angle);
		break;
	case Z_dir:
		rotateMatrix = RotateZ(angle);
		break;
	case Zn_dir:
		rotateMatrix = RotateZ(-angle);
		break;
	default:
		break;
	}

	centeringVec = CenteringTranslation(axis);
	if (axis == WORLD)
	{
		cameraInverseMatrix = matrixInverse(Translate((-1) * centeringVec),MOVE);
		_world_transform = Translate((-1) * centeringVec) * _world_transform;	// move to the origin

		_world_transform = rotateMatrix * _world_transform;	// rotate in world axis
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(rotateMatrix,ROTATE);

		_world_transform = Translate(centeringVec) * _world_transform;		 	// move back to old location
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(Translate(centeringVec),MOVE);

		_world_normal_transform = CreateNormalTransform(rotateMatrix, ROTATE) * _world_normal_transform;
	}
	else		// axis == MODEL	
	{
		cameraInverseMatrix = matrixInverse(Translate((-1) * centeringVec), MOVE);
		_model_transform = Translate((-1) * centeringVec) * _model_transform;	// move to the origin

		_model_transform = rotateMatrix * _model_transform;							// rotate in world axis
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(rotateMatrix, ROTATE);

		_model_transform = Translate(centeringVec) * _model_transform;			// move back to old location
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(Translate(centeringVec), MOVE);

		_model_normal_transform = CreateNormalTransform(rotateMatrix, ROTATE) * _model_normal_transform;
	}
	return cameraInverseMatrix;
}

mat4 MeshModel::scaleModel(TransformationDirection direction, TransAxis axis)
{
	mat4 scaleMatrix;
	mat4 scaleInverse;
	mat4 cameraInverseMatrix;
	vec3 centeringVec;
	switch (direction)
	{
	case X_dir:
		scaleMatrix = Scale(2, 1, 1);
		break;
	case Xn_dir:
		scaleMatrix = Scale(0.5, 1, 1);
		break;
	case Y_dir:
		scaleMatrix = Scale(1, 2, 1);
		break;
	case Yn_dir:
		scaleMatrix = Scale(1, 0.5, 1);
		break;
	case Z_dir:
		scaleMatrix = Scale(1, 1, 2);
		break;
	case Zn_dir:
		scaleMatrix = Scale(1, 1, 0.5);
		break;
	default:
		break;
	}

	centeringVec = CenteringTranslation(axis);
	if (axis == WORLD)
	{
		_world_transform = Translate((-1) * centeringVec) * _world_transform;	// move to the origin
		cameraInverseMatrix = matrixInverse(Translate((-1) * centeringVec), MOVE);

		_world_transform = scaleMatrix * _world_transform;							// scale in world axis
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(scaleMatrix, SCALE);

		_world_transform = Translate(centeringVec) * _world_transform;			// move back to old location
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(Translate(centeringVec), MOVE);

		_world_normal_transform = CreateNormalTransform(scaleMatrix, SCALE) * _world_normal_transform;
	}
	else		// axis == MODEL	
	{

		_model_transform = Translate((-1) * centeringVec) * _model_transform;	// move to the origin
		cameraInverseMatrix = matrixInverse(Translate((-1) * centeringVec),MOVE);

		_model_transform = scaleMatrix * _model_transform;							// scale in world axis
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(scaleMatrix, SCALE);

		_model_transform = Translate(centeringVec) * _model_transform;			// move back to old location
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(Translate(centeringVec), MOVE);

		_model_normal_transform = CreateNormalTransform(scaleMatrix, SCALE) * _model_normal_transform;
	}
	return cameraInverseMatrix;
}

GLfloat MeshModel::GetProportionalValue()
{
	return ((x_bound_lenght + y_bound_lenght + z_bound_lenght) / 3) * 0.1;
}

void MeshModel::UpdateTriangleColor()
{
	/*for (auto it = triangles->begin(); it != triangles->end(); ++it)
	{
		(*it).shape_color = mesh_color;
	}*/
}

void MeshModel::RandomizePolygons()
{
	/*int third = triangles->size() / 3;
	float step = (float)3 / triangles->size();
	vec3 black = vec3(0);
	for (auto it = triangles->begin(); it != triangles->end(); ++it)
	{
		(*it).ka = (float)rand() / RAND_MAX;
		(*it).kd = (float)rand() / RAND_MAX;
		(*it).ks = (float)rand() / RAND_MAX;
		(*it).shape_color = black;
	}
	
	for (int i = 0; i < third; i++)
	{
		triangles->at(i).shape_color.x = step * i;
	}
	
	for (int i = third; i < 2 * third; i++)
		{
		triangles->at(i).shape_color.y = step * i;
	}
	
	for (int i = 2 * third; i < triangles->size(); i++)
	{
		triangles->at(i).shape_color.z = step * i;
	}
	*/
		
}


void MeshModel::UpdateTriangleIlluminationParams()
{
	/*for (auto it = triangles->begin(); it != triangles->end(); ++it)
	{
		(*it).ka = ka;
		(*it).kd = kd;
		(*it).ks = ks;
		(*it).ke = ke;
		(*it).is_non_uniform = this->is_non_unfiorm;
	}*/
}

mat4 MeshModel::manipulateModel(Transformation T, TransformationDirection direction,
								TransAxis axis, float power)
{
	mat4 cameraInverseMatrix;
	switch (T)
	{
		case ROTATE:
			cameraInverseMatrix = rotateModel(direction, power, axis);
			break;
		case MOVE:
			cameraInverseMatrix = moveModel(direction, axis);
			break;
		case SCALE:
			cameraInverseMatrix = scaleModel(direction, axis);
			break;
	}
	return cameraInverseMatrix;
}

mat4 MeshModel::GetObjectMatrix()
{
	return _world_transform * _model_transform;
}
mat4 MeshModel::GetNormalMatrix()
{
	return _world_normal_transform * _model_normal_transform;
}