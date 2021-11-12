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
	_world_transform = mat4();
	_world_transform[2][3] = -2; // move center to (0,0,-2)
	loadFile(fileName);
}

MeshModel::~MeshModel(void)
{
}

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
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

	vertex_positions = new vector<vec3>; /*FIXED*/
	world_vertex_positions = new vector<vec3>; /*FIXED*/
	// iterate through all stored faces and create triangles
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		for (int i = 0; i < 3; i++)
		{
			vertex_positions->push_back(vec3(vertices[it->v[i] - 1])); /*FIXED*/
		}
	}
}

void MeshModel::CalcBounds()
{
	int size = world_vertex_positions->size();
	vec3 min_bound, max_bound;
	max_bound = min_bound = world_vertex_positions->at(0);
	for (int i = 1; i < size; i++)
	{
		// find max
		if (world_vertex_positions->at(i).x > max_bound.x)
		{
			max_bound.x = world_vertex_positions->at(i).x;
		}
		if (world_vertex_positions->at(i).y > max_bound.y)
		{
			max_bound.y = world_vertex_positions->at(i).y;
		}
		if (world_vertex_positions->at(i).z > max_bound.z)
		{
			max_bound.z = world_vertex_positions->at(i).z;
		}
		// find min
		if (world_vertex_positions->at(i).x < min_bound.x)
		{
			min_bound.x = world_vertex_positions->at(i).x;
		}
		if (world_vertex_positions->at(i).y < min_bound.y)
		{
			min_bound.y = world_vertex_positions->at(i).y;
		}
		if (world_vertex_positions->at(i).z < min_bound.z)
		{
			min_bound.z = world_vertex_positions->at(i).z;
		}
	}
	x_bound_lenght = abs(max_bound.x - min_bound.x);
	y_bound_lenght = abs(max_bound.y - min_bound.y);
	z_bound_lenght = abs(max_bound.z - min_bound.z);
	center = (max_bound + min_bound) / 2;
}

void MeshModel::draw(Renderer* renderer, bool draw_bound_box)
{
	int size = vertex_positions->size();
	world_vertex_positions->clear();
	vec4 tempVec;
	for (int i = 0; i < size; i++)
	{
		tempVec = _world_transform * vec4(vertex_positions->at(i));
		world_vertex_positions->push_back(vec3(tempVec.x / tempVec.w, tempVec.y / tempVec.w, tempVec.z / tempVec.w));
	}
	renderer->DrawTriangles(world_vertex_positions);
	
	if (draw_bound_box)
	{
		CalcBounds();
		PrimMeshModel* bound_box = new PrimMeshModel(center.x, center.y, center.z, x_bound_lenght, y_bound_lenght, z_bound_lenght);
		bound_box->draw(renderer,true);
	}
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
	world_vertex_positions = new vector<vec3>; /*FIXED*/
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

void PrimMeshModel::draw(Renderer* renderer, bool draw_bound_box)
{
	int size = this->vertex_positions->size();
	this->world_vertex_positions->clear();
	vec4 tempVec;
	for (int i = 0; i < size; i++)
	{
		tempVec = this->_world_transform * vec4(vertex_positions->at(i));
		this->world_vertex_positions->push_back(vec3(tempVec.x / tempVec.w, tempVec.y / tempVec.w, tempVec.z / tempVec.w));
	}

	// draw the faces
	renderer->DrawRectangles(this->world_vertex_positions);

}

