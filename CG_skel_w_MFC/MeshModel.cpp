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

	// front face triangels
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
	bound_box_vertices = CalcBounds();
	_world_transform[2][3] = -5;
}

MeshModel::~MeshModel(void)
{
}

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
	vector<vec3> v_normals;
	vec3 curVertex;
	vec3 curCenter;
	vec3 curNormalEnd;


	Triangle curTriangle;
	Normal curVertexNormal;
	Normal curFaceNormal;

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
			v_normals.push_back(vec3fFromStream(issLine));
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
	
	triangles = new vector<Triangle>;
	facesNormals = new vector<Normal>;

	if (v_normals.size() != 0)
	{
		verticesNormals = new vector<Normal>;

	}
	else
	{
		verticesNormals = NULL;

	}

	// iterate through all stored faces and create triangles
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		//Create Trignagle
		curTriangle = Triangle(vertices.at(it->v[0] - 1), vertices.at(it->v[1] - 1), vertices.at(it->v[2] - 1));

		if (v_normals.size() != 0)
		{
			//Create Normals
			curNormalEnd = normalize(v_normals.at(it->vn[0] - 1) - curTriangle.p1_3d);
			curVertexNormal = Normal(curTriangle.p1_3d, curNormalEnd);
			verticesNormals->push_back(curVertexNormal);

			curNormalEnd = normalize(v_normals.at(it->vn[1] - 1) - curTriangle.p2_3d);
			curVertexNormal = Normal(curTriangle.p2_3d, curNormalEnd);
			verticesNormals->push_back(curVertexNormal);

			curNormalEnd = normalize(v_normals.at(it->vn[2] - 1) - curTriangle.p3_3d);
			curVertexNormal = Normal(curTriangle.p3_3d, curNormalEnd);
			verticesNormals->push_back(curVertexNormal);
		}

		curCenter = (curTriangle.p1_3d + curTriangle.p2_3d + curTriangle.p3_3d) / 3;
		curNormalEnd = normalize(cross(curTriangle.p2_3d - curTriangle.p1_3d, curTriangle.p3_3d - curTriangle.p1_3d));
		curFaceNormal = Normal(curCenter, curNormalEnd);
		facesNormals->push_back(curFaceNormal);
		triangles->push_back(curTriangle);

	}
}

vector<vec3>* MeshModel::CalcBounds()
{
	int size = triangles->size();
	vec3 min_bound, max_bound;
	Triangle curTriangle;
	max_bound = min_bound = triangles->at(0).p1_3d;
	float curMaxX, curMinX, curMaxY, curMinY, curMaxZ, curMinZ;
	for (auto it = triangles->begin(); it != triangles->end(); ++ it)
	{
		curMaxX = max(max(it->p1_3d.x, it->p2_3d.x),it->p3_3d.x);
		curMinX = min(min(it->p1_3d.x, it->p2_3d.x),it->p3_3d.x);
		curMaxY = max(max(it->p1_3d.y, it->p2_3d.y), it->p3_3d.y);
		curMinY = min(min(it->p1_3d.y, it->p2_3d.y), it->p3_3d.y);
		curMaxZ = max(max(it->p1_3d.z, it->p2_3d.z), it->p3_3d.z);
		curMinZ = min(min(it->p1_3d.z, it->p2_3d.z), it->p3_3d.z);
		// find max
		if (curMaxX > max_bound.x)
		{
			max_bound.x = curMaxX;
		}
		if (curMaxY > max_bound.y)
		{
			max_bound.y = curMaxY;
		}
		if (curMaxZ > max_bound.z)
		{
			max_bound.z = curMaxZ;
		}
		// find min
		if (curMinX < min_bound.x)
		{
			min_bound.x = curMinX;
		}
		if (curMinY < min_bound.y)
		{
			min_bound.y = curMinY;
		}
		if (curMinZ < min_bound.z)
		{
			min_bound.z = curMinZ;
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
	renderer->DrawTriangles(triangles, verticesNormals, facesNormals, bound_box_vertices, GetProportionalValue());
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


PrimMeshModel::PrimMeshModel(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat lenX, GLfloat lenY, GLfloat lenZ) :posX(posX), posY(posY), posZ(posZ), lenX(lenX), lenY(lenY), lenZ(lenZ)
{
	triangles = new vector<Triangle>;
	facesNormals = new vector<Normal>;
	verticesNormals = new vector<Normal>;
	vec3 p1, p2, p3;
	//faces_normal_end_positions = new vector<vec3>;

	Triangle curTriangle;
	Normal curVertexNormal;
	Normal curFaceNormal;
	GLfloat halfX = lenX * 0.5f;
	GLfloat halfY = lenY * 0.5f;
	GLfloat halfZ = lenZ * 0.5f;



	// front face triangels
	p1 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // bottom left
	p2 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top left
	p3 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, 1));
	facesNormals->push_back(curFaceNormal);

	p1 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // bottom left
	p2 = vec3(posX + halfX, posY + halfY, posZ + halfZ); // top left
	p3 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, 1));
	facesNormals->push_back(curFaceNormal);

	// back face triangels
	p1 = vec3(posX - halfX, posY - halfY, posZ - halfZ); // bottom left
	p2 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, -1));
	facesNormals->push_back(curFaceNormal);

	p1 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	p2 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // top right
	p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, -1));
	facesNormals->push_back(curFaceNormal);

	// left face triangels
	p1 = vec3(posX - halfX, posY - halfY, posZ - halfZ); // bottom left
	p2 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	p3 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(-1, 0, 0));
	facesNormals->push_back(curFaceNormal);

	p1 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	p2 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top right
	p3 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(-1, 0, 0));
	facesNormals->push_back(curFaceNormal);

	// right face triangels
	p1 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom left
	p2 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // top left
	p3 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(1, 0, 0));
	facesNormals->push_back(curFaceNormal);

	p1 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // top left
	p2 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // top right
	p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(1, 0, 0));
	facesNormals->push_back(curFaceNormal);


	// top face triangels
	p1 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // bottom left
	p2 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top left
	p3 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 1, 0));
	facesNormals->push_back(curFaceNormal);

	p1 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top left
	p2 = vec3(posX + halfX, posY + halfY, posZ + halfZ); // top right
	p3 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 1, 0));
	facesNormals->push_back(curFaceNormal);
	
	// down face triangels
	p1 = vec3(posX - halfX, posY - halfY, posZ - halfZ); // bottom left
	p2 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // top left
	p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, -1, 0));
	facesNormals->push_back(curFaceNormal);

	p1 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // top left
	p2 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // top right
	p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, -1, 0));
	facesNormals->push_back(curFaceNormal);
 
	bound_box_vertices = CalcBounds();
}

CameraModel::CameraModel(int cameraIndex) : cameraIndex(cameraIndex)
{
	triangles = new vector<Triangle>;
	verticesNormals = NULL; //unused
	facesNormals = NULL;  //unused
	vec3 p1, p2, p3;
	//faces_normal_end_positions = new vector<vec3>;

	Triangle curTriangle;
	_world_transform[2][3] = 2; //initialized same as camera regarding location


	// first triangle
	p1 = vec3(0.1, 0, 0); // bottom left
	p2 = vec3(-0.1, 0, 0); // top left
	p3 = vec3(0, 0, -0.5); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);

	// second triangle
	p1 = vec3(0, 0.1, 0); // bottom left
	p2 = vec3(0, -0.1, 0); // top left
	p3 = vec3(0, 0, -0.5); // bottom right
	curTriangle = Triangle(p1, p2, p3);
	triangles->push_back(curTriangle);
	
	bound_box_vertices = CalcBounds();
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
	vec3 centeringVec;
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