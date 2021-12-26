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

vector<Line>* SetRectangleVertices(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat lenX, GLfloat lenY, GLfloat lenZ)
{
	vector<Line>* lines = new vector<Line>;
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
	lines->push_back(Line(frontTopLeft, frontTopRight, false));
	lines->push_back(Line(frontTopLeft, frontBottomLeft, false));
	lines->push_back(Line(frontTopLeft, backTopLeft, false));

	lines->push_back(Line(backTopRight, backTopLeft, false));
	lines->push_back(Line(backTopRight, backBottomRight, false));
	lines->push_back(Line(backTopRight, frontTopRight, false));

	lines->push_back(Line(backBottomLeft, backTopLeft, false));
	lines->push_back(Line(backBottomLeft, backBottomRight, false));
	lines->push_back(Line(backBottomLeft, frontBottomLeft, false));

	lines->push_back(Line(frontBottomRight, frontBottomLeft, false));
	lines->push_back(Line(frontBottomRight, frontTopRight, false));
	lines->push_back(Line(frontBottomRight, backBottomRight, false));

	return lines;
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

MeshModel::MeshModel(string fileName):mesh_color(BLUE), ka(0.5), kd(0.8), ks(1.0),ke(0), alpha(100.0)
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
	vec3 curCenter;
	vec3 curNormalEnd;

	vec3 p1, p2 , p3;


	Triangle curTriangle;
	Normal curVertex1Normal;
	Normal curVertex2Normal;
	Normal curVertex3Normal;
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

	// iterate through all stored faces and create triangles
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		//Create Trignagle
		p1 = vertices.at(it->v[0] - 1);
		p2 = vertices.at(it->v[1] - 1);
		p3 = vertices.at(it->v[2] - 1);
		curCenter = (p1 + p2 + p3) / 3;
		curNormalEnd = normalize(cross(p2 - p1,p3 - p1));
		curFaceNormal = Normal(curCenter, curNormalEnd, false, face_normal);

		if (v_normals.size() != 0)
		{
			//Create Normals
			curNormalEnd = normalize(v_normals.at(it->vn[0] - 1) - curTriangle.p1_3d);
			curVertex1Normal = Normal(curTriangle.p1_3d, curNormalEnd, false, vertix_normal);

			curNormalEnd = normalize(v_normals.at(it->vn[1] - 1) - curTriangle.p2_3d);
			curVertex2Normal = Normal(curTriangle.p2_3d, curNormalEnd, false, vertix_normal);

			curNormalEnd = normalize(v_normals.at(it->vn[2] - 1) - curTriangle.p3_3d);
			curVertex3Normal = Normal(curTriangle.p3_3d, curNormalEnd, false, vertix_normal);

			curTriangle = Triangle(p1, p2, p3, mesh_color, false ,curFaceNormal, curVertex1Normal, curVertex2Normal, curVertex3Normal);

		}
		else
		{
			curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
		}


		triangles->push_back(curTriangle);

	}
	UpdateTriangleIlluminationParams();
}

vector<Line>* MeshModel::CalcBounds()
{
	int size = triangles->size();
	vec3 min_bound, max_bound;
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


void MeshModel::draw(Renderer* renderer)
{	
	renderer->ClipModel(triangles, bound_box_vertices);
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
	ka = 0.5;
	kd = 0.8;
	ks = 1.0;
	ke = 0;
	triangles = new vector<Triangle>;
	vec3 p1, p2, p3;

	_world_transform[2][3] = -5;
	//faces_normal_end_positions = new vector<vec3>;

	Triangle curTriangle;
	Normal curFaceNormal;
	GLfloat halfX = lenX * 0.5f;
	GLfloat halfY = lenY * 0.5f;
	GLfloat halfZ = lenZ * 0.5f;
	mesh_color = RED;



	// front face triangels
	p1 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // bottom left
	p2 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top left
	p3 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, 1), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);

	p1 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // bottom left
	p2 = vec3(posX + halfX, posY + halfY, posZ + halfZ); // top left
	p3 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, 1), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);

	// back face triangels
	p1 = vec3(posX - halfX, posY - halfY, posZ - halfZ); // bottom left
	p2 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // b,ottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, -1), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);

	p1 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	p2 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // top right
	p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 0, -1), false, face_normal);
	curTriangle = Triangle(p1, p2, p3 , mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);

	// left face triangels
	p1 = vec3(posX - halfX, posY - halfY, posZ - halfZ); // bottom left
	p2 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	p3 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(-1, 0, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);

	p1 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // top left
	p2 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top right
	p3 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(-1, 0, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);

	// right face triangels
	//high triangle
	p1 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // bottom left
	p2 = vec3(posX + halfX, posY + halfY, posZ + halfZ); // top left
	p3 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // top right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(1, 0, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);


	//low triangle
	p1 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // bottom left
	p2 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // top right
	p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(1, 0, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);


	// top face triangels
	p1 = vec3(posX - halfX, posY + halfY, posZ - halfZ); // bottom left
	p2 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top left
	p3 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 1, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);

	p1 = vec3(posX - halfX, posY + halfY, posZ + halfZ); // top left
	p2 = vec3(posX + halfX, posY + halfY, posZ + halfZ); // top right
	p3 = vec3(posX + halfX, posY + halfY, posZ - halfZ); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 1, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);
	
	// down face triangels
	p1 = vec3(posX - halfX, posY - halfY, posZ - halfZ); // bottom left
	p2 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // top left
	p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, -1, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);

	p1 = vec3(posX - halfX, posY - halfY, posZ + halfZ); // top left
	p2 = vec3(posX + halfX, posY - halfY, posZ + halfZ); // top right
	p3 = vec3(posX + halfX, posY - halfY, posZ - halfZ); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, -1, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);
 
	bound_box_vertices = CalcBounds();
	UpdateTriangleIlluminationParams();
}

CameraModel::CameraModel(int cameraIndex) : cameraIndex(cameraIndex)
{
	ka = 0.5;
	kd = 0.8;
	ks = 1.0;
	triangles = new vector<Triangle>;
	Normal curFaceNormal;
	vec3 p1, p2, p3;
	mesh_color = GREEN;
	//faces_normal_end_positions = new vector<vec3>;

	Triangle curTriangle;
	_world_transform[2][3] = 2; //initialized same as camera regarding location


	// first triangle
	p1 = vec3(0.1, 0, 0); // bottom left
	p2 = vec3(-0.1, 0, 0); // top left
	p3 = vec3(0, 0, -0.5); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 1, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);

	// second triangle
	p1 = vec3(0, 0.1, 0); // bottom left
	p2 = vec3(0, -0.1, 0); // top left
	p3 = vec3(0, 0, -0.5); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(1, 0, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, false, curFaceNormal);
	triangles->push_back(curTriangle);
	
	bound_box_vertices = CalcBounds();
	UpdateTriangleIlluminationParams();
}

LightModel::LightModel(int lightIndex) : lightIndex(lightIndex)
{
	ka = 1;
	kd = 1;
	ks = 1;
	mesh_color = WHITE;
	triangles = new vector<Triangle>;
	Normal curFaceNormal;
	vec3 p1, p2, p3;
	//faces_normal_end_positions = new vector<vec3>;

	Triangle curTriangle;
	_world_transform[2][3] = -2; //initialized same as camera regarding location


	// first triangle
	p1 = vec3(0.1, 0, 0); // bottom left
	p2 = vec3(-0.1, 0, 0); // top left
	p3 = vec3(0, 0, -0.1); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(0, 1, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, true, curFaceNormal);
	triangles->push_back(curTriangle);

	// second triangle
	p1 = vec3(0, 0.1, 0); // bottom left
	p2 = vec3(0, -0.1, 0); // top left
	p3 = vec3(0, 0, -0.1); // bottom right
	curFaceNormal = Normal((p1 + p2 + p3) / 3, vec3(1, 0, 0), false, face_normal);
	curTriangle = Triangle(p1, p2, p3, mesh_color, true, curFaceNormal);
	triangles->push_back(curTriangle);

	bound_box_vertices = CalcBounds();
	UpdateTriangleIlluminationParams();
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
	GLfloat move = (x_bound_lenght + y_bound_lenght + z_bound_lenght) / 3;
	if (dynamic_cast<LightModel*>(this))
	{
		move = move * 10;
	}
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

void MeshModel::UpdateTriangleColor()
{
	for (auto it = triangles->begin(); it != triangles->end(); ++it)
	{
		(*it).shape_color = mesh_color;
	}
}

void MeshModel::UpdateTriangleIlluminationParams()
{
	for (auto it = triangles->begin(); it != triangles->end(); ++it)
	{
		(*it).ka = ka;
		(*it).kd = kd;
		(*it).ks = ks;
		(*it).ke = ke;
	}
}

void MeshModel::RandomizePolygons()
{
	int third = triangles->size() / 3;
	float step = (float) 3 / triangles->size();
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

	for (int i = third; i < 2* third; i++)
	{
		triangles->at(i).shape_color.y = step * i;
	}

	for (int i = 2* third; i < triangles->size(); i++)
	{
		triangles->at(i).shape_color.z = step * i;
	}

	
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