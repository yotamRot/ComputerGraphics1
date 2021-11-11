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
#define DOWN 120
#define IN 113
#define OUT 101

Scene *scene;
Renderer *renderer;

int last_x,last_y;
int mainMenuId;
int menuObjectsId;
Tramsformation curTramsformation = MOVE;
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
	Axis axis = X;
	switch ( key ) {
	case 033:			// escape
		exit( EXIT_SUCCESS );
		break;
	case LEFT:
		axis = Xn;
		break;
	case RIGHT:
		axis = X;
	case DOWN:
		axis = Yn;
		break;
	case UP:
		axis = Y;
		break;
	case IN:
		axis = Zn;
		break;
	case OUT:
		axis = Z;
		break;
	case ROTATE_Z_RIGHT:		// e
		rotation = RotateZ(10);
		new_eye = rotation * scene->cameras.at(scene->activeCamera)->eye;
		at = rotation * scene->cameras.at(scene->activeCamera)->at;
		up = rotation * scene->cameras.at(scene->activeCamera)->up;
		scene->cameras.at(scene->activeCamera)->LookAt(new_eye, at, up);
		break;
	case ROTATE_Z_LEFT:			// q
		rotation = RotateZ(350);
		new_eye = rotation * scene->cameras.at(scene->activeCamera)->eye;
		at = rotation * scene->cameras.at(scene->activeCamera)->at;
		up = rotation * scene->cameras.at(scene->activeCamera)->up;
		scene->cameras.at(scene->activeCamera)->LookAt(new_eye, at, up);
		break;
	case RECTANGLE:				// r
		break;
	}
	scene->manipulateActiveModel(curTramsformation, axis);
	scene->draw();
	

}

void mouse(int button, int state, int x, int y)
{
	//button = {GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON}
	//state = {GLUT_DOWN,GLUT_UP}
	vec4 eye = vec4(0, -1, 0, 1);
	vec4 at = vec4(0, 0, -1, 1);
	vec4 up = vec4(0, 1, 0, 1);*/
	//set down flags
	switch(button) {
		case GLUT_LEFT_BUTTON:
			lb_down = (state==GLUT_UP)?0:1;
			//scene->cameras.at(scene->activeCamera)->LookAt(eye, at, up);
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
}


void tramsformationMenu(int id)
{
	curTramsformation = (Tramsformation)id;
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
	}
}

void initMenu()
{
	int menuFile = glutCreateMenu(fileMenu);
	glutAddMenuEntry("Open..",FILE_OPEN);
	int menuTramsformation = glutCreateMenu(tramsformationMenu);
	glutAddMenuEntry("Move", MOVE);
	glutAddMenuEntry("Rotate", ROTATE);
	glutAddMenuEntry("Scale", SCALE);
	mainMenuId = glutCreateMenu(mainMenu);
	glutAddSubMenu("File",menuFile);
	glutAddMenuEntry("Demo",MAIN_DEMO);
	glutAddMenuEntry("About",MAIN_ABOUT);
	glutAddMenuEntry("Add cube", ADD_CUBE);
	glutAddSubMenu("Transformations", menuTramsformation);

	


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
