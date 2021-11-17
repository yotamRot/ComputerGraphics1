// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CG_skel_w_MFC.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object
#include "InputDialog.h"
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"
#include <string>

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

// file menu defines
#define FILE_OPEN 1
#define ADD_CUBE 2


#define MAIN_DEMO 1
#define MAIN_ABOUT 2
#define OBJECTS 4
#define FEATURES 5
#define CLEAR 6
#define CAMERAS 7

// features menu
#define SHOW_VERTICES_NORMAL 1
#define SHOW_FACES_NORMAL 2
#define SHOW_BOUNDING_BOX 3

#define ORTHOGRPHIC_PARAMETERS 1
#define PRESPECTIVE_PARAMETERS 2

//camers menu
#define ADD_CAMERA 1
#define RENDER_CAMERAS 2
#define CONTROL_ACTIVE_CAMERA 3

#define WHEEL_SCROLL_UP 3
#define WHEEL_SCROLL_DOWN 4

#define LEFT 97
#define RIGHT 100
#define UP 119
#define DOWN 115
#define IN 113
#define OUT 101


Scene *scene;
Renderer *renderer;

int last_x,last_y;
bool newRotate = false;

std::string cameraPrefix = "Camera ";
 
int mainMenuId;
int menuObjectsId;
int menuSwitchToCameraId;
int menuLookAtCameraId;
Transformation curTramsformation = MOVE;
Projection curProjection = PERSPECTIVE;
bool lb_down,rb_down,mb_down;
bool prv_lb_down = false;


//----------------------------------------------------------------------------
// Callbacks

void display( void )
{
	scene->draw();
}

void reshape( int width, int height )
{
//update the renderer's buffers
	renderer->ResizeBuffers(width, height);
	glViewport(0, 0, width, height);
}

void keyboard( unsigned char key, int x, int y )
{
	TransformationDirection axis;
	switch ( key ) {
	case 033:			// escape
		exit( EXIT_SUCCESS );
		break;
	case LEFT:			//a
		axis = Xn;
		break;
	case RIGHT:			//d 
		axis = X;
		break;
	case DOWN:			//s
		axis = Yn;
		break;
	case UP:			//w
		axis = Y;
		break;
	case IN:			//q
		axis = Zn;
		break;
	case OUT:			//e
		axis = Z;
		break;
	default:
		AfxMessageBox(_T("Check Language is English and Caps Lock is disable"));
		return;
	}
	scene->manipulateActiveModel(curTramsformation, axis,scene->GetTrasformationAxis());
	scene->draw();
}

void mouse(int button, int state, int x, int y)
{
	//set down flags

	switch(button) {
		case GLUT_LEFT_BUTTON:
			lb_down = (state==GLUT_UP)?0:1;
			if (!lb_down)
			{
				newRotate = false;
			}
			break;
		case GLUT_RIGHT_BUTTON:
			rb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_MIDDLE_BUTTON:
			mb_down = (state==GLUT_UP)?0:1;	
			break;
		case WHEEL_SCROLL_UP:
			scene->Zoom(ZOOM_IN);
			break;
		case WHEEL_SCROLL_DOWN:
			scene->Zoom(ZOOM_OUT);
			break;
	}
	// add your code
	scene->draw();
}

void motion(int x, int y)
{
	// calc difference in mouse movement
	int dx=x-last_x;
	int dy=y-last_y;
	// update last x,y
	last_x=x;
	last_y=y;
	
	if (newRotate)
	{
		scene->rotateAroundActiveModel(dx, dy);
		scene->draw();
	}
	else
	{
		newRotate = true;
	}

}


void objectsMenu(int id)
{
	scene->lookAtModel(scene->modelMenuIdToVectorId(id));
	scene->draw();
}

void camersMenu(int id)
{
	int newCameraId;
	switch (id)
	{
		case ADD_CAMERA:
			newCameraId = scene->addCamera();
			glutSetMenu(menuLookAtCameraId);
			glutAddMenuEntry((cameraPrefix + std::to_string(newCameraId)).c_str(), newCameraId);
			glutSetMenu(menuSwitchToCameraId);
			glutAddMenuEntry((cameraPrefix + std::to_string(newCameraId)).c_str(), newCameraId);
			break;
		case RENDER_CAMERAS:
			scene->toggleRenderCameras();
			break;
		case CONTROL_ACTIVE_CAMERA:
			scene->ControlActiveCamera();
			break;
	}
	scene->draw();
}

void lookAtCameraMenu(int id)
{
	scene->lookAtCamera(id);
	scene->draw();
}

void switchToCameraMenu(int id)
{
	scene->switchToCamera(id);
	scene->draw();
}

void fileMenu(int id)
{
	int newModelId;
	CFileDialog dlg(TRUE, _T(".obj"), NULL, NULL, _T("*.obj|*.*"));
	
	switch (id)
	{
		case FILE_OPEN:

			if(dlg.DoModal()==IDOK)
			{
				glutSetMenu(menuObjectsId);
				std::string s((LPCTSTR)dlg.GetPathName());
				newModelId =scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
				glutAddMenuEntry((LPCTSTR)dlg.GetFileName(), newModelId);
			}
			else
			{
				return;
			}
			break;
		case ADD_CUBE:
			glutSetMenu(menuObjectsId);
			newModelId = scene->loadCubeModel();
			glutAddMenuEntry("Cube",newModelId);
			scene->draw();
			break;
	}
	scene->lookAtModel(scene->activeModel);
	scene->draw();
}


void transformationMenu(int id)
{
	curTramsformation = (Transformation)id;
}

void transAxiesMenu(int id)
{
	scene->SetTrasformationAxis((TransAxis)id);
}

void featuresMenu(int id)
{
	switch (id)
	{
		case SHOW_VERTICES_NORMAL:
			scene->toggleShowVerticesNormals();
			break;
		case SHOW_FACES_NORMAL:
			scene->toggleShowFacesNormals();
			break;
		case SHOW_BOUNDING_BOX:
			scene->toggleDrawBoundBox();
			break;
	}
	scene->draw();
}

void projectionMenu(int id)
{
	scene->setActiveCameraProjection((Projection)id);
	scene->draw();
}

void ProjParameresMenu(int id)
{
	setLbnRtf(scene->Getlbn(), scene->Getrtf());
	CRltbnfDialog dlg;
	if (dlg.DoModal() == IDOK) {
		vec3 rtf = dlg.GetRTF();
		vec3 lbn = dlg.GetLBN();
		if (id == ORTHOGRPHIC_PARAMETERS)
		{
			scene->ChangeProjectionParameters(ORTHOGRAPHIC, rtf, lbn);
		}
		else	// PERSPECTIVE
		{
			scene->ChangeProjectionParameters(PERSPECTIVE, rtf, lbn);
		}
	}
	scene->draw();
}

void mainMenu(int id)
{
	switch (id)
	{
	case MAIN_ABOUT:
		AfxMessageBox(_T("Computer Graphics"));
		break;
	case CLEAR:
		glutSetMenu(menuObjectsId);
		for (int i = 0; i < scene->models.size(); i++)
		{
			glutRemoveMenuItem(i);
		}
		scene->ClearScene();
		//glutDestroyMenu(menuObjectsId);
		//glutAddSubMenu("Objects", menuObjectsId);
		scene->draw();
		break;
	}
}

void initMenu()
{
	int menuFile = glutCreateMenu(fileMenu);
	glutAddMenuEntry("Open..",FILE_OPEN);
	glutAddMenuEntry("Add Cube", ADD_CUBE);
	int menuTramsformation = glutCreateMenu(transformationMenu);
	glutAddMenuEntry("Move", MOVE);
	glutAddMenuEntry("Rotate", ROTATE);
	glutAddMenuEntry("Scale", SCALE);

	int menuTransformationAxies = glutCreateMenu(transAxiesMenu);
	glutAddMenuEntry("Model Axies", MODEL);
	glutAddMenuEntry("World Axies", WORLD);
	
	menuLookAtCameraId = glutCreateMenu(lookAtCameraMenu);
	glutAddMenuEntry((cameraPrefix + "0").c_str(), 0);
	menuSwitchToCameraId = glutCreateMenu(switchToCameraMenu);
	glutAddMenuEntry((cameraPrefix + "0").c_str(), 0);
	int menuCameras = glutCreateMenu(camersMenu);
	glutAddMenuEntry("Add Camera", ADD_CAMERA);
	glutAddMenuEntry("Render Camers", RENDER_CAMERAS);
	glutAddMenuEntry("Control Active Camera", CONTROL_ACTIVE_CAMERA);
	glutAddSubMenu("Look At Camera", menuLookAtCameraId);
	glutAddSubMenu("Select Camera", menuSwitchToCameraId);
	
	int menuFeatures = glutCreateMenu(featuresMenu);
	glutAddMenuEntry("Show Vertices Normal", SHOW_VERTICES_NORMAL);
	glutAddMenuEntry("Show Faces Normal", SHOW_FACES_NORMAL);
	glutAddMenuEntry("Draw Bound Box", SHOW_BOUNDING_BOX);

	int menuProjections = glutCreateMenu(projectionMenu);
	glutAddMenuEntry("Orthographic", ORTHOGRAPHIC);
	glutAddMenuEntry("Prespective", PERSPECTIVE);
	

	int menuProjectionParameters = glutCreateMenu(ProjParameresMenu);
	glutAddMenuEntry("Orthographic Parameters", ORTHOGRPHIC_PARAMETERS);
	glutAddMenuEntry("Prespective Parameters", PRESPECTIVE_PARAMETERS);
	
	menuObjectsId = glutCreateMenu(objectsMenu);

	//glutSetMenu(mainMenuId);
	mainMenuId = glutCreateMenu(mainMenu);

	glutAddSubMenu("Load", menuFile);
	glutAddSubMenu("Transformations", menuTramsformation);
	glutAddSubMenu("Transformations axis", menuTransformationAxies);
	glutAddSubMenu("Projection", menuProjections);
	glutAddSubMenu("Projection Parameters", menuProjectionParameters);
	glutAddSubMenu("Features", menuFeatures);
	glutAddSubMenu("Objects", menuObjectsId);
	glutAddMenuEntry("Clear Screen", CLEAR);
	glutAddMenuEntry("About", MAIN_ABOUT);
	glutAddSubMenu("Cameras", menuCameras);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//----------------------------------------------------------------------------



int my_main( int argc, char **argv )
{
	//----------------------------------------------------------------------------
	// Initialize window
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA| GLUT_DOUBLE);
	glutInitWindowSize( 512, 512 );
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutCreateWindow( "CG" );
	glewExperimental = GL_TRUE;
	glewInit();
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		/*		...*/
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	
	
	renderer = new Renderer(512,512);
	scene = new Scene(renderer);
	//----------------------------------------------------------------------------
	// Initialize Callbacks

	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc ( motion );
	glutReshapeFunc( reshape );
	initMenu();
	

	glutMainLoop();
	delete scene;
	delete renderer;
	return 0;
}

CWinApp theApp;

using namespace std;

int main( int argc, char **argv )
{
	int nRetCode = 0;
	
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		my_main(argc, argv );
	}
	
	return nRetCode;
}
