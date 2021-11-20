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

vector<vec3>* SetRectangleVertices(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat lenX, GLfloat lenY, GLfloat lenZ)
{
	vector<vec3>* vertices = new vector<vec3>;
	GLfloat halfX = lenX * 0.5f;
	GLfloat halfY = lenY * 0.5f;
	GLfloat halfZ = lenZ * 0.5f;

	// front face
	vertices->push_back(vec3(posX - halfX, posY + halfY, posZ + halfZ)); // top left
	vertices->push_back(vec3(posX + halfX, posY + halfY, posZ + halfZ)); // top right
	vertices->push_back(vec3(posX + halfX, posY - halfY, posZ + halfZ)); // bottom right
	vertices->push_back(vec3(posX - halfX, posY - halfY, posZ + halfZ)); // bottom left

	// back face
	vertices->push_back(vec3(posX - halfX, posY + halfY, posZ - halfZ)); // top left
	vertices->push_back(vec3(posX + halfX, posY + halfY, posZ - halfZ)); // top right
	vertices->push_back(vec3(posX + halfX, posY - halfY, posZ - halfZ)); // bottom right
	vertices->push_back(vec3(posX - halfX, posY - halfY, posZ - halfZ)); // bottom left

	// left face
	vertices->push_back(vec3(posX - halfX, posY + halfY, posZ + halfZ)); // top left
	vertices->push_back(vec3(posX - halfX, posY + halfY, posZ - halfZ)); // top right
	vertices->push_back(vec3(posX - halfX, posY - halfY, posZ - halfZ)); // bottom right
	vertices->push_back(vec3(posX - halfX, posY - halfY, posZ + halfZ)); // bottom left

	// right face
	vertices->push_back(vec3(posX + halfX, posY + halfY, posZ + halfZ)); // top left
	vertices->push_back(vec3(posX + halfX, posY + halfY, posZ - halfZ)); // top right
	vertices->push_back(vec3(posX + halfX, posY - halfY, posZ - halfZ)); // bottom right
	vertices->push_back(vec3(posX + halfX, posY - halfY, posZ + halfZ)); // bottom left

	// top face
	vertices->push_back(vec3(posX - halfX, posY + halfY, posZ + halfZ)); // top left
	vertices->push_back(vec3(posX - halfX, posY + halfY, posZ - halfZ)); // top right
	vertices->push_back(vec3(posX + halfX, posY + halfY, posZ - halfZ)); // bottom right
	vertices->push_back(vec3(posX + halfX, posY + halfY, posZ + halfZ)); // bottom left

	// down face
	vertices->push_back(vec3(posX - halfX, posY - halfY, posZ + halfZ)); // top left
	vertices->push_back(vec3(posX - halfX, posY - halfY, posZ - halfZ)); // top right
	vertices->push_back(vec3(posX + halfX, posY - halfY, posZ - halfZ)); // bottom right 
	vertices->push_back(vec3(posX + halfX, posY - halfY, posZ + halfZ)); // bottom left

	return vertices;
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

MeshModel::MeshModel(string fileName)
{
	loadFile(fileName);
	this->_world_transform[2][3] = -5;
	bound_box_vertices = CalcBounds();
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
	faces_normals = new vector<vec3>;
	faces_centers = new vector<vec3>;

	if (verticesNormals.size() != 0)
	{
		vertix_normals = new vector<vec3>;

	}
	else
	{
		vertix_normals = NULL;

	}

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
			}
		}

		curCenter = curCenter / 3;
		faces_centers->push_back(curCenter);
		curFaceNormal = normalize(cross(triangle[1] - triangle[0], triangle[2] - triangle[0]));
		faces_normals->push_back(curFaceNormal);
	}
}

vector<vec3>* MeshModel::CalcBounds()
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
	return SetRectangleVertices(center.x, center.y, center.z, x_bound_lenght, y_bound_lenght, z_bound_lenght);
}

vec3 MeshModel::GetCenter()
{
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
	GLfloat proportionalValue = GetProportionalValue();
	renderer->DrawTriangles(vertex_positions, vertix_normals, faces_centers, faces_normals, bound_box_vertices,proportionalValue);
}

vec3 MeshModel::GetPosition(TransAxis axis)
{
	if (axis == MODEL)
	{
		return vec3(_model_transform[0][3] / _model_transform[3][3],
			_model_transform[1][3] / _model_transform[3][3],
			_model_transform[2][3] / _model_transform[3][3]);
	}
	else
	{
		return vec3(_world_transform[0][3] / _world_transform[3][3],
			_world_transform[1][3] / _world_transform[3][3],
			_world_transform[2][3] / _world_transform[3][3]);
	}
}


PrimMeshModel::PrimMeshModel(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat lenX, GLfloat lenY, GLfloat lenZ) :posX(posX), posY(posY), posZ(posZ), lenX(lenX), lenY(lenY), lenZ(lenZ)
{
	vertex_positions = SetRectangleVertices(posX, posY, posZ, lenX, lenY, lenZ);
	faces_centers = new vector<vec3>;
	faces_normals = new vector<vec3>;
	//faces_normal_end_positions = new vector<vec3>;
	GLfloat halfX = lenX * 0.5f;
	GLfloat halfY = lenY * 0.5f;
	GLfloat halfZ = lenZ * 0.5f;

	// front face
	faces_centers->push_back(vec3(posX, posY, posZ + halfZ));
	faces_normals->push_back(vec3(0, 0, 1));

	// back face

	faces_centers->push_back(vec3(posX, posY, posZ - halfZ));
	faces_normals->push_back(vec3(0, 0, -1));

	// left face

	faces_centers->push_back(vec3(posX - halfX, posY, posZ));
	faces_normals->push_back(vec3(-1, 0, 0));

	// right face
	faces_centers->push_back(vec3(posX + halfX, posY, posZ));
	faces_normals->push_back(vec3(1, 0, 0));

	// top face
	faces_centers->push_back(vec3(posX, posY + halfY, posZ));
	faces_normals->push_back(vec3(0, 1, 0));

	// down face
	faces_centers->push_back(vec3(posX, posY - halfY, posZ));
	faces_normals->push_back(vec3(0, -1, 0));

	bound_box_vertices = CalcBounds();
}

CameraModel::CameraModel(int cameraIndex) : cameraIndex(cameraIndex)
{
	vertex_positions = new vector<vec3>;
	vertix_normals = NULL; //unused
	faces_centers = NULL;  //unused
	faces_normals = NULL; //unused
	_world_transform[2][3] = 2; //initialized same as camera regarding location


	// first triangle
	vertex_positions->push_back(vec3(0.1, 0, 0));
	vertex_positions->push_back(vec3(-0.1, 0, 0));
	vertex_positions->push_back(vec3(0, 0, -0.5));

	// second triangle
	vertex_positions->push_back(vec3(0, 0.1, 0));
	vertex_positions->push_back(vec3(0, -0.1, 0));
	vertex_positions->push_back(vec3(0, 0, -0.5));
	
	bound_box_vertices = CalcBounds();
}


void PrimMeshModel::draw(Renderer* renderer)
{
	renderer->DrawRectangles(vertex_positions, faces_centers, faces_normals);
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
	const GLfloat move = (x_bound_lenght + y_bound_lenght + z_bound_lenght) / 3;
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
	vec3 translationVector;
	switch (direction)
	{
	case X:
		rotateMatrix = RotateX(angle);
		break;
	case Xn:
		rotateMatrix = RotateX(-angle);
		break;
	case Y:
		rotateMatrix = RotateY(angle);
		break;
	case Yn:
		rotateMatrix = RotateY(-angle);
		break;
	case Z:
		rotateMatrix = RotateZ(angle);
		break;
	case Zn:
		rotateMatrix = RotateZ(-angle);
		break;
	default:
		break;
	}

	translationVector = GetPosition(axis);
	if (axis == WORLD)
	{
		cameraInverseMatrix = matrixInverse(Translate((-1) * translationVector),MOVE);
		_world_transform = Translate((-1) * translationVector) * _world_transform;	// move to the origin

		_world_transform = rotateMatrix * _world_transform;	// rotate in world axis
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(rotateMatrix,ROTATE);

		_world_transform = Translate(translationVector) * _world_transform;		 	// move back to old location
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(Translate(translationVector),MOVE);

		_world_normal_transform = CreateNormalTransform(rotateMatrix, ROTATE) * _world_normal_transform;
	}
	else		// axis == MODEL	
	{
		cameraInverseMatrix = matrixInverse(Translate((-1) * translationVector), MOVE);
		_model_transform = Translate((-1) * translationVector) * _model_transform;	// move to the origin

		_model_transform = rotateMatrix * _model_transform;							// rotate in world axis
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(rotateMatrix, ROTATE);

		_model_transform = Translate(translationVector) * _model_transform;			// move back to old location
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(Translate(translationVector), MOVE);

		_model_normal_transform = CreateNormalTransform(rotateMatrix, ROTATE) * _model_normal_transform;
	}
	return cameraInverseMatrix;
}

mat4 MeshModel::scaleModel(TransformationDirection direction, TransAxis axis)
{
	mat4 scaleMatrix;
	mat4 scaleInverse;
	mat4 cameraInverseMatrix;
	vec3 translationVector;
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

	translationVector = GetPosition(axis);
	if (axis == WORLD)
	{
		_world_transform = Translate((-1) * translationVector) * _world_transform;	// move to the origin
		cameraInverseMatrix = matrixInverse(Translate((-1) * translationVector), MOVE);

		_world_transform = scaleMatrix * _world_transform;							// scale in world axis
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(scaleMatrix, SCALE);

		_world_transform = Translate(translationVector) * _world_transform;			// move back to old location
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(Translate(translationVector), MOVE);

		_world_normal_transform = CreateNormalTransform(scaleMatrix, SCALE) * _world_normal_transform;
	}
	else		// axis == MODEL	
	{

		_model_transform = Translate((-1) * translationVector) * _model_transform;	// move to the origin
		cameraInverseMatrix = matrixInverse(Translate((-1) * translationVector),MOVE);

		_model_transform = scaleMatrix * _model_transform;							// scale in world axis
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(scaleMatrix, SCALE);

		_model_transform = Translate(translationVector) * _model_transform;			// move back to old location
		cameraInverseMatrix = cameraInverseMatrix * matrixInverse(Translate(translationVector), MOVE);

		_model_normal_transform = CreateNormalTransform(scaleMatrix, SCALE) * _model_normal_transform;
	}
	return cameraInverseMatrix;
}

GLfloat MeshModel::GetProportionalValue()
{
	return ((x_bound_lenght + y_bound_lenght + z_bound_lenght) / 3) * 0.1;
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