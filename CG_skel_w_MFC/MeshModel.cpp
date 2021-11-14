#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

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

MeshModel::MeshModel(string fileName)
{
	_world_transform = Translate(0,0,-5);
	loadFile(fileName);
	CalcBounds();
	this->preformTransform(_world_transform, MOVE);
}

MeshModel::~MeshModel(void)
{
}

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
	vector<vec3> verticesNormals;
	vec3 curVertex;
	vec3 curCenter;
	vec3 curVertexNormal;
	vec3 curFaceNormal;
	vec3 triangle[3];

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
			vertices.push_back(vec3fFromStream(issLine));
		if (lineType == "vn") /*FIXED*/
			verticesNormals.push_back(vec3fFromStream(issLine));
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

	vertex_positions = new vector<vec3>;
	vertices_normal_end_positions = new vector<vec3>;
	vertix_normals = new vector<vec3>;
	faces_normals = new vector<vec3>;
	faces_centers = new vector<vec3>;
	faces_normal_end_positions = new vector<vec3>;
	// iterate through all stored faces and create triangles
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		curCenter = vec3(0, 0, 0);
		for (int i = 0; i < 3; i++)
		{	
			curVertex = vertices.at(it->v[i] - 1);
			triangle[i] = curVertex;
			curCenter += curVertex;
			vertex_positions->push_back(curVertex); /*FIXED*/
			if (verticesNormals.size() != 0)
			{
				curVertexNormal = normalize(verticesNormals.at(it->vn[i] - 1) - vertices.at(it->v[i] - 1));
				vertix_normals->push_back(curVertexNormal);
				vertices_normal_end_positions->push_back(curVertex + 0.1 * curVertexNormal);
			}
		}

		curCenter = curCenter / 3;
		faces_centers->push_back(curCenter);
		curFaceNormal = normalize(cross(triangle[1] - triangle[0], triangle[2] - triangle[0]));
		faces_normals->push_back(curFaceNormal);
		faces_normal_end_positions->push_back(curCenter + 0.1 * curFaceNormal);
	}
}

void MeshModel::CalcBounds()
{
	int size = vertex_positions->size();
	vec3 min_bound, max_bound;
	max_bound = min_bound = vertex_positions->at(0);
	for (int i = 1; i < size; i++)
	{
		// find max
		if (vertex_positions->at(i).x > max_bound.x)
		{
			max_bound.x = vertex_positions->at(i).x;
		}
		if (vertex_positions->at(i).y > max_bound.y)
		{
			max_bound.y = vertex_positions->at(i).y;
		}
		if (vertex_positions->at(i).z > max_bound.z)
		{
			max_bound.z = vertex_positions->at(i).z;
		}
		// find min
		if (vertex_positions->at(i).x < min_bound.x)
		{
			min_bound.x = vertex_positions->at(i).x;
		}
		if (vertex_positions->at(i).y < min_bound.y)
		{
			min_bound.y = vertex_positions->at(i).y;
		}
		if (vertex_positions->at(i).z < min_bound.z)
		{
			min_bound.z = vertex_positions->at(i).z;
		}
	}
	x_bound_lenght = abs(max_bound.x - min_bound.x);
	y_bound_lenght = abs(max_bound.y - min_bound.y);
	z_bound_lenght = abs(max_bound.z - min_bound.z);
	center = (max_bound + min_bound) / 2;
}

vec3 MeshModel::GetCenter()
{
	CalcBounds();
	return vec3(center);
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


void MeshModel::draw(Renderer* renderer)
{	
	renderer->DrawTriangles(vertex_positions, vertices_normal_end_positions, faces_centers, faces_normal_end_positions);
}

vec3 MeshModel::getPosition()
{
	return vec3(this->_world_transform[0][3]/ this->_world_transform[3][3],
		this->_world_transform[1][3]/ this->_world_transform[3][3],
		this->_world_transform[2][3]/ this->_world_transform[3][3]);
}


PrimMeshModel::PrimMeshModel(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat lenX, GLfloat lenY, GLfloat lenZ) :posX(posX), posY(posY), posZ(posZ), lenX(lenX), lenY(lenY), lenZ(lenZ)
{
	vertex_positions = new vector<vec3>;
	//_world_transform = mat4();
	//_world_transform[2][3] = -2; // move center to (0,0,-2)
	GLfloat halfX = lenX * 0.5f;
	GLfloat halfY = lenY * 0.5f;
	GLfloat halfZ = lenZ * 0.5f;

	// front face
	vertex_positions->push_back(vec3(posX - halfX, posY + halfY, posZ + halfZ)); // top left
	vertex_positions->push_back(vec3(posX + halfX, posY + halfY, posZ + halfZ)); // top right
	vertex_positions->push_back(vec3(posX + halfX, posY - halfY, posZ + halfZ)); // bottom right
	vertex_positions->push_back(vec3(posX - halfX, posY - halfY, posZ + halfZ)); // bottom left

	// back face
	vertex_positions->push_back(vec3(posX - halfX, posY + halfY, posZ - halfZ)); // top left
	vertex_positions->push_back(vec3(posX + halfX, posY + halfY, posZ - halfZ)); // top right
	vertex_positions->push_back(vec3(posX + halfX, posY - halfY, posZ - halfZ)); // bottom right
	vertex_positions->push_back(vec3(posX - halfX, posY - halfY, posZ - halfZ)); // bottom left

	// left face
	vertex_positions->push_back(vec3(posX - halfX, posY + halfY, posZ + halfZ)); // top left
	vertex_positions->push_back(vec3(posX - halfX, posY + halfY, posZ - halfZ)); // top right
	vertex_positions->push_back(vec3(posX - halfX, posY - halfY, posZ - halfZ)); // bottom right
	vertex_positions->push_back(vec3(posX - halfX, posY - halfY, posZ + halfZ)); // bottom left

	// right face
	vertex_positions->push_back(vec3(posX + halfX, posY + halfY, posZ + halfZ)); // top left
	vertex_positions->push_back(vec3(posX + halfX, posY + halfY, posZ - halfZ)); // top right
	vertex_positions->push_back(vec3(posX + halfX, posY - halfY, posZ - halfZ)); // bottom right
	vertex_positions->push_back(vec3(posX + halfX, posY - halfY, posZ + halfZ)); // bottom left

	// top face
	vertex_positions->push_back(vec3(posX - halfX, posY + halfY, posZ + halfZ)); // top left
	vertex_positions->push_back(vec3(posX - halfX, posY + halfY, posZ - halfZ)); // top right
	vertex_positions->push_back(vec3(posX + halfX, posY + halfY, posZ - halfZ)); // bottom right
	vertex_positions->push_back(vec3(posX + halfX, posY + halfY, posZ + halfZ)); // bottom left

	// down face
	vertex_positions->push_back(vec3(posX - halfX, posY - halfY, posZ + halfZ)); // top left
	vertex_positions->push_back(vec3(posX - halfX, posY - halfY, posZ - halfZ)); // top right
	vertex_positions->push_back(vec3(posX + halfX, posY - halfY, posZ - halfZ)); // bottom right 
	vertex_positions->push_back(vec3(posX + halfX, posY - halfY, posZ + halfZ)); // bottom left
}

void PrimMeshModel::draw(Renderer* renderer)
{
	renderer->DrawRectangles(this->vertex_positions);
}


void MeshModel::moveModel(Axis direction)
{
	mat4 tranlateMatrix;
	GLfloat move = (x_bound_lenght + y_bound_lenght + z_bound_lenght) / 3;
	switch (direction)
	{
		case X:
			tranlateMatrix = Translate(move*0.1, 0, 0);
			break;
		case Xn:
			tranlateMatrix = Translate(-move * 0.1, 0, 0);
			break;
		case Y:
			tranlateMatrix = Translate(0, move *0.1, 0);
			break;
		case Yn:
			tranlateMatrix = Translate(0, -move * 0.1, 0);
			break;
		case Z:
			tranlateMatrix = Translate(0, 0, move * 0.1);
			break;
		case Zn:
			tranlateMatrix = Translate(0, 0, -move * 0.1);
			break;
		default:
			break;
	}
	preformTransform(tranlateMatrix, MOVE);
	_world_transform = tranlateMatrix * _world_transform; // translate 

}

void MeshModel::rotateModel(Axis direction)
{
	mat4 rotateMatrix;
	vec3 modelCenter = GetCenter();
	preformTransform(Translate((-1) * modelCenter), MOVE); // move to center
	switch (direction)
	{
	case X:
		rotateMatrix = RotateX(10);
		break;
	case Xn:
		rotateMatrix = RotateX(-10);
		break;
	case Y:
		rotateMatrix = RotateY(10);
		break;
	case Yn:
		rotateMatrix = RotateY(-10);
		break;
	case Z:
		rotateMatrix = RotateZ(10);
		break;
	case Zn:
		rotateMatrix = RotateZ(-10);
		break;
	default:
		break;
	}
	preformTransform(rotateMatrix, ROTATE);
	preformTransform(Translate(modelCenter), MOVE); //move back to orig loctaion
	_world_transform = _world_transform * rotateMatrix; // rotate around center so first rotate then move!

}

void MeshModel::scaleModel(Axis direction)
{
	mat4 scaleMatrix;
	vec3 modelCenter = GetCenter();
	preformTransform(Translate((-1) * modelCenter), MOVE); // move to center
	switch (direction)
	{
	case X:
		scaleMatrix = Scale(2, 1, 1);
		break;
	case Xn:
		scaleMatrix = Scale(0.5, 1, 1);
		break;
	case Y:
		scaleMatrix = Scale(1, 2, 1);
		break;
	case Yn:
		scaleMatrix = Scale(1, 0.5, 1);
		break;
	case Z:
		scaleMatrix = Scale(1, 1, 2);
		break;
	case Zn:
		scaleMatrix = Scale(1, 1, 0.5);
		break;
	default:
		break;
	}
	_world_transform = scaleMatrix * _world_transform; // translate 
	preformTransform(scaleMatrix, SCALE);
	preformTransform(Translate(modelCenter), MOVE); //move back to orig loctaion
}

void MeshModel::preformTransform(mat4& matrix, Transformation T)
{
	vec4 tempVec;
	mat4 noramlMatrix;
	for (auto it = vertex_positions->begin(); it != vertex_positions->end(); ++it)
	{
		tempVec = matrix * vec4(*it);
		*it = vec3(tempVec.x / tempVec.w, tempVec.y / tempVec.w, tempVec.z / tempVec.w);
	}

	this->CalcBounds();
	float proportionFactorNormals = ((x_bound_lenght + y_bound_lenght + z_bound_lenght) / 3) * 0.2;

	noramlMatrix = matrix;
	if (T == SCALE) // need to inverse and transpose
	{
		noramlMatrix[0][0] = 1 / matrix[0][0];
		noramlMatrix[1][1] = 1 / matrix[1][1];
		noramlMatrix[2][2] = 1 / matrix[2][2];
	}

	for (int i = 0; i < vertex_positions->size(); i++)
	{
		tempVec = vec4(vertix_normals->at(i));
		tempVec.w = 0; // we dont want to translate matrix!
		tempVec = noramlMatrix * tempVec;
		vertix_normals->at(i) = normalize(vec3(tempVec.x, tempVec.y, tempVec.z));
		vertices_normal_end_positions->at(i)= vertex_positions->at(i) + proportionFactorNormals * vertix_normals->at(i);
	}

	for (int i = 0; i < faces_centers->size(); i++)
	{
		//update Center
		tempVec = matrix * faces_centers->at(i);
		faces_centers->at(i) = vec3(tempVec.x / tempVec.w, tempVec.y / tempVec.w, tempVec.z / tempVec.w);
		
		//update Normal Vector
		tempVec = vec4(faces_normals->at(i));
		tempVec.w = 0; // we dont want to translate matrix!
		tempVec = noramlMatrix * tempVec;
		faces_normals->at(i) = normalize(vec3(tempVec.x, tempVec.y, tempVec.z));

		//Updagte normal ending point
		faces_normal_end_positions->at(i) = faces_centers->at(i) + proportionFactorNormals * vertix_normals->at(i);
	}
}

void MeshModel::manipulateModel(Transformation T, Axis axis)
{
	switch (T)
	{
		case ROTATE:
			rotateModel(axis);
			break;
		case MOVE:
			moveModel(axis);
			break;
		case SCALE:
			scaleModel(axis);
			break;
	}
}

void MeshModel::drawBoundingBox(Renderer* renderer)
{
	CalcBounds();
	PrimMeshModel* bound_box = new PrimMeshModel(center.x, center.y, center.z, x_bound_lenght, y_bound_lenght, z_bound_lenght);
	bound_box->draw(renderer);
}
