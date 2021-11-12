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

#define FILE_OPEN 1
#define MAIN_DEMO 1
#define MAIN_ABOUT 2
#define ADD_CUBE 3
#define OBJECTS 4
#define LEFT 97
#define RIGHT 100
#define UP 119
#define DOWN 115
#define IN 113
#define OUT 101

Scene *scene;
Renderer *renderer;

int last_x,last_y;
int mainMenuId;
int menuObjectsId;
Transformation curTramsformation = MOVE;
Projection curProjection = PRESPECTIVE;
bool lb_down,rb_down,mb_down;
uint32_t mouse_status = 0;

//----------------------------------------------------------------------------
// Callbacks

void display( void )
{
	scene->draw();
}

void reshape( int width, int height )
{
//update the renderer's buffers
}

void keyboard( unsigned char key, int x, int y )
{
	Axis axis;
	switch ( key ) {
	case 033:			// escape
		exit( EXIT_SUCCESS );
		break;
	case LEFT:  //a
		axis = Xn;
		break;
	case RIGHT: //d 
		axis = X;
		break;
	case DOWN: //s
		axis = Yn;
		break;
	case UP:   //w
		axis = Y;
		break;
	case IN:  //q
		axis = Zn;
		break;
	case OUT: //e
		axis = Z;
		break;
	default:
		return;
	}
	scene->manipulateActiveModel(curTramsformation, axis);
	scene->draw();
}

void mouse(int button, int state, int x, int y)
{
	//set down flags
	switch(button) {
		case GLUT_LEFT_BUTTON:
			lb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_RIGHT_BUTTON:
			rb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_MIDDLE_BUTTON:
			mb_down = (state==GLUT_UP)?0:1;	
			break;
	}

	// add your code
}

void motion(int x, int y)
{
	// calc difference in mouse movement
	int dx=x-last_x;
	int dy=y-last_y;
	// update last x,y
	last_x=x;
	last_y=y;
}

void objectsMenu(int id)
{
	scene->lookAtModel(id);
	scene->draw();
}

void fileMenu(int id)
{
	switch (id)
	{
		case FILE_OPEN:
			CFileDialog dlg(TRUE,_T(".obj"),NULL,NULL,_T("*.obj|*.*"));
			if(dlg.DoModal()==IDOK)
			{
				std::string s((LPCTSTR)dlg.GetPathName());
				scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
				if (scene->models.size() == 1)
				{
					menuObjectsId = glutCreateMenu(objectsMenu);
					glutSetMenu(mainMenuId);
					glutAddSubMenu("objects", menuObjectsId);
				}
				glutSetMenu(menuObjectsId);
				glutAddMenuEntry((LPCTSTR)dlg.GetFileName(), scene->models.size() - 1);

			}
			break;
	}
	scene->draw();
}


void transformationMenu(int id)
{
	curTramsformation = (Transformation)id;
}

void projectionMenu(int id)
{
	scene->setActiveCameraProjection((Projection)id);
	scene->draw();
}

void mainMenu(int id)
{
	switch (id)
	{
	case MAIN_DEMO:
		scene->drawDemo();
		break;
	case MAIN_ABOUT:
		AfxMessageBox(_T("Computer Graphics"));
		break;
	case ADD_CUBE:
		scene->loadCubeModel();
		if (scene->models.size() == 1)
		{
			menuObjectsId = glutCreateMenu(objectsMenu);
			glutSetMenu(mainMenuId);
			glutAddSubMenu("objects", menuObjectsId);
		}
		glutSetMenu(menuObjectsId);
		glutAddMenuEntry("Cube", scene->models.size() - 1);
		scene->draw();
		break;
	}
}

void initMenu()
{
	int menuFile = glutCreateMenu(fileMenu);
	glutAddMenuEntry("Open..",FILE_OPEN);
	int menuTramsformation = glutCreateMenu(transformationMenu);
	glutAddMenuEntry("Move", MOVE);
	glutAddMenuEntry("Rotate", ROTATE);
	glutAddMenuEntry("Scale", SCALE);
	int menuProjections = glutCreateMenu(projectionMenu);
	glutAddMenuEntry("Orthographic", ORTHOGRAPHIC);
	glutAddMenuEntry("Prespective", PRESPECTIVE);
	mainMenuId = glutCreateMenu(mainMenu);
	glutAddSubMenu("File",menuFile);
	glutAddMenuEntry("Demo",MAIN_DEMO);
	glutAddMenuEntry("About",MAIN_ABOUT);
	glutAddMenuEntry("Add cube", ADD_CUBE);
	glutAddSubMenu("Transformations", menuTramsformation);
	glutAddSubMenu("Projection", menuProjections);

	


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
