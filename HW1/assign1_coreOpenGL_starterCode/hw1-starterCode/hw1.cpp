/*
CSCI 420 Computer Graphics, USC
Assignment 1: Height Fields
C++ starter code

Student username: Nithin Chandrashekhar
*/

#include <iostream>
#include <cstring>
#include <vector>
#include "openGLHeader.h"
#include "glutHeader.h"

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#ifdef WIN32
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;

// Declare variables for handling the functionality.
// option for changing between solid, wireframe and points.
int option = 0;

// scaling factor for depth.
float hfactor = 0.03;

// vectors, buffers and vao
// points
std::vector<GLfloat> pointCoordinates;
std::vector<GLfloat> pointColor;
GLuint pointBuffer;
GLuint pointColorBuffer;
GLuint pVao;

// triangle
std::vector<GLfloat> triCoordinates;
std::vector<GLfloat> triColor;
GLuint triangleBuffer;
GLuint triangleColorBuffer;
GLuint tVao;

// wireframe
std::vector<GLfloat> wireCoordinates;
std::vector<GLfloat> wireColor;
GLuint wireBuffer;
GLuint wireColorBuffer;
GLuint wVao;

// overlap wireframe
std::vector<GLfloat> owireCoordinates;
std::vector<GLfloat> owireColor;
GLuint owireBuffer;
GLuint owireColorBuffer;
GLuint owVao;

// modelview and projection matrix
float m[16];
float p[16];

// animation toggle
bool aniCheck = false;

// OpenGLMatrix for handling the matrix operations
OpenGLMatrix * matrix;
// basic pipeline program
BasicPipelineProgram * pipelineProgram;
// for handling rotate
GLfloat theta[3] = { 0.0, 0.0, 0.0 };

GLuint program;

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
	unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;

	delete[] screenshotData;
}

// bind the co-ordinate and color buffer
void initVBO()
{
	// points
	glGenBuffers(1, &pointBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
	glBufferData(GL_ARRAY_BUFFER, pointCoordinates.size() * sizeof(GLfloat), &pointCoordinates[0], GL_STATIC_DRAW);
	glGenBuffers(1, &pointColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, pointColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, pointColor.size() * sizeof(GLfloat), &pointColor[0], GL_STATIC_DRAW);

	// wireframe
	glGenBuffers(1, &wireBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, wireBuffer);
	glBufferData(GL_ARRAY_BUFFER, wireCoordinates.size() * sizeof(GLfloat), &wireCoordinates[0], GL_STATIC_DRAW);
	glGenBuffers(1, &wireColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, wireColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, wireColor.size() * sizeof(GLfloat), &wireColor[0], GL_STATIC_DRAW);

	// overlap wireframe
	glGenBuffers(1, &owireBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, owireBuffer);
	glBufferData(GL_ARRAY_BUFFER, owireCoordinates.size() * sizeof(GLfloat), &owireCoordinates[0], GL_STATIC_DRAW);
	glGenBuffers(1, &owireColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, owireColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, owireColor.size() * sizeof(GLfloat), &owireColor[0], GL_STATIC_DRAW);

	// triangel
	glGenBuffers(1, &triangleBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangleBuffer);
	glBufferData(GL_ARRAY_BUFFER, triCoordinates.size() * sizeof(GLfloat), &triCoordinates[0], GL_STATIC_DRAW);
	glGenBuffers(1, &triangleColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangleColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, triColor.size() * sizeof(GLfloat), &triColor[0], GL_STATIC_DRAW);
}

// bind the array object
void initVAO()
{
	// points
	glGenVertexArrays(1, &pVao);
	glBindVertexArray(pVao);

	// wireframe
	glGenVertexArrays(1, &wVao);
	glBindVertexArray(wVao);

	// overlap wireframe
	glGenVertexArrays(1, &owVao);
	glBindVertexArray(owVao);

	// triangle
	glGenVertexArrays(1, &tVao);
	glBindVertexArray(tVao);
}

// pipeline program for init
void initPipelineProgram()
{
	pipelineProgram = new BasicPipelineProgram();
	pipelineProgram->Init("../openGLHelper-starterCode");
}

// render points
void renderPoints()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, pointColorBuffer);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_POINTS, 0, pointCoordinates.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// render wireframes
void renderWireframe()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, wireBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, wireColorBuffer);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_LINES, 0, wireCoordinates.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// render overlap wireframes
void renderoWireframe()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, owireBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, owireColorBuffer);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_LINES, 0, owireCoordinates.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// render Triangles
void renderTriangles()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, triangleBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, triangleColorBuffer);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, triCoordinates.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// display function
void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	matrix->SetMatrixMode(OpenGLMatrix::ModelView);
	matrix->LoadIdentity();
	// camera position
	matrix->LookAt(0, 256, 1, 0, 0, 0, 0, 1, 0);

	// transformations
	matrix->Scale(landScale[0], landScale[1], landScale[2]);
	matrix->Translate(landTranslate[0] * 10, landTranslate[1] * 10, landTranslate[2] * 10);
	matrix->Rotate(theta[0], 1.0, 0.0, 0.0);
	matrix->Rotate(theta[1], 0.0, 1.0, 0.0);
	matrix->Rotate(theta[2], 0.0, 0.0, 1.0);
	matrix->Rotate(landRotate[0], 1.0, 0.0, 0.0);
	matrix->Rotate(landRotate[1], 0.0, 1.0, 0.0);
	matrix->Rotate(landRotate[2], 0.0, 0.0, 1.0);

	pipelineProgram->Bind();
	program = pipelineProgram->GetProgramHandle();

	// get a handle to the modelViewMatrix shader variable
	GLint h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
	matrix->GetMatrix(m);
	glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);

	// get a handle to the projectionMatrix shader variable
	GLint h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
	matrix->SetMatrixMode(OpenGLMatrix::Projection);
	matrix->GetMatrix(p);
	glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);

	// render based on the options
	if (option == 0)
	{
		renderPoints();
	}
	else if (option == 1)
	{
		renderWireframe();
	}
	else if (option == 2)
	{
		renderTriangles();
	}
	else if (option == 3)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-100.0, -100.0);
		renderoWireframe();
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		renderTriangles();

	}
	glBindVertexArray(0);
	glutSwapBuffers();
}

// idle function
void idleFunc()
{
	// do some stuff...
	// updating theta for animation
	if (aniCheck)
	{
		theta[0] += 0.1;
		if (theta[0] > 360.0) theta[0] -= 360.0;
		theta[1] += 0.1;
		if (theta[1] > 360.0) theta[1] -= 360.0;
		theta[2] += 0.1;
		if (theta[2] > 360.0) theta[2] -= 360.0;
	}

	// for example, here, you can save the screenshots to disk (to make the animation)

	// make the screen update
	glutPostRedisplay();
}

// reshape function
void reshapeFunc(int w, int h)
{
	glViewport(0, 0, w, h);
	// setup perspective matrix...
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	matrix->SetMatrixMode(OpenGLMatrix::Projection);
	matrix->LoadIdentity();
	matrix->Perspective(60.0, 1.0 * aspect, 0.01, 1000.0);
	matrix->SetMatrixMode(OpenGLMatrix::ModelView);
}

void mouseMotionDragFunc(int x, int y)
{
	// mouse has moved and one of the mouse buttons is pressed (dragging)

	// the change in mouse position since the last invocation of this function
	int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

	switch (controlState)
	{
		// translate the landscape
	case TRANSLATE:
		if (leftMouseButton)
		{
			// control x,z translation via the left mouse button
			landTranslate[0] += mousePosDelta[0] * 0.01f;
			landTranslate[2] += mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton)
		{
			// control y translation via the middle mouse button
			landTranslate[1] += mousePosDelta[1] * 0.01f;
		}
		break;

		// rotate the landscape
	case ROTATE:
		if (leftMouseButton)
		{
			// control x,z rotation via the left mouse button
			landRotate[0] += mousePosDelta[1];
			landRotate[2] += mousePosDelta[0];
		}
		if (middleMouseButton)
		{
			// control y rotation via the middle mouse button
			landRotate[1] -= mousePosDelta[1];
		}
		break;

		// scale the landscape
	case SCALE:
		if (leftMouseButton)
		{
			// control x,z scaling via the left mouse button
			landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
			landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton)
		{
			// control y scaling via the middle mouse button
			landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
	// mouse has moved
	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
	// a mouse button has has been pressed or depressed

	// keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		leftMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_MIDDLE_BUTTON:
		middleMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_RIGHT_BUTTON:
		rightMouseButton = (state == GLUT_DOWN);
		break;
	}

	// keep track of whether CTRL and SHIFT keys are pressed
	switch (glutGetModifiers())
	{
	case GLUT_ACTIVE_CTRL:
		controlState = TRANSLATE;
		break;

	case GLUT_ACTIVE_SHIFT:
		controlState = SCALE;
		break;

		// if CTRL and SHIFT are not pressed, we are in rotate mode
	default:
		controlState = ROTATE;
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void calculatePoints(ImageIO* image)
{
	int halfWidth = image->getWidth() / 2;
	int halfHeight = image->getHeight() / 2;
	float heightFactor = 0.03f;
	heightFactor *= image->getHeight() / 100;

	// start calculating the co-ordinates by taking the xz plane and making y as the height
	// co-ordinates are symmetrically calculated over the xz axes
	for (int i = -halfHeight; i < halfHeight - 1; i++)
	{
		for (int j = -halfWidth; j < halfWidth - 1; j++)
		{
			GLfloat y = image->getPixel(i + halfHeight, j + halfWidth, 0) * heightFactor;
			GLfloat p1[3] = { i, y, -j };
			y = image->getPixel(i + halfHeight, j + halfWidth + 1, 0) * heightFactor;
			GLfloat p2[3] = { i, y, -(j + 1) };
			y = image->getPixel(i + halfHeight + 1, j + halfWidth + 1, 0) * heightFactor;
			GLfloat p3[3] = { i + 1, y, -(j + 1) };
			y = image->getPixel(i + halfHeight + 1, j + halfWidth, 0) * heightFactor;
			GLfloat p4[3] = { i + 1, y, -j };

			GLfloat color = (float)image->getPixel(i + halfHeight, j + halfWidth, 0) / (float)255.0f;
			GLfloat colorP1[4] = { color, color, color, 1.0f };
			color = (float)image->getPixel(i + halfHeight, j + halfWidth + 1, 0) / (float)255.0f;
			GLfloat colorP2[4] = { color, color, color, 1.0f };
			color = (float)image->getPixel(i + halfHeight + 1, j + halfWidth + 1, 0) / (float)255.0f;
			GLfloat colorP3[4] = { color, color, color, 1.0f };
			color = (float)image->getPixel(i + halfHeight + 1, j + halfWidth, 0) / (float)255.0f;
			GLfloat colorP4[4] = { color, color, color, 1.0f };

			// points
			color = (float)image->getPixel(i + halfHeight, j + halfWidth, 0) / (float)255.0f;
			GLfloat pColor[4] = { color, color, color, 1.0f };
			pointColor.insert(pointColor.end(), pColor, pColor + 4);
			pointCoordinates.insert(pointCoordinates.end(), p2, p2 + 3);
			pointColor.insert(pointColor.end(), pColor, pColor + 4);
			pointCoordinates.insert(pointCoordinates.end(), p3, p3 + 3);
			pointColor.insert(pointColor.end(), pColor, pColor + 4);
			pointCoordinates.insert(pointCoordinates.end(), p1, p1 + 3);
			pointColor.insert(pointColor.end(), pColor, pColor + 4);
			pointCoordinates.insert(pointCoordinates.end(), p4, p4 + 3);

			// triangle
			triCoordinates.insert(triCoordinates.end(), p2, p2 + 3);
			triColor.insert(triColor.end(), colorP2, colorP2 + 4);
			triCoordinates.insert(triCoordinates.end(), p3, p3 + 3);
			triColor.insert(triColor.end(), colorP3, colorP3 + 4);
			triCoordinates.insert(triCoordinates.end(), p1, p1 + 3);
			triColor.insert(triColor.end(), colorP1, colorP1 + 4);
			triCoordinates.insert(triCoordinates.end(), p1, p1 + 3);
			triColor.insert(triColor.end(), colorP1, colorP1 + 4);
			triCoordinates.insert(triCoordinates.end(), p3, p3 + 3);
			triColor.insert(triColor.end(), colorP3, colorP3 + 4);
			triCoordinates.insert(triCoordinates.end(), p4, p4 + 3);
			triColor.insert(triColor.end(), colorP4, colorP4 + 4);

			// wireframe
			wireCoordinates.insert(wireCoordinates.end(), p2, p2 + 3);
			wireColor.insert(wireColor.end(), colorP2, colorP2 + 4);
			wireCoordinates.insert(wireCoordinates.end(), p3, p3 + 3);
			wireColor.insert(wireColor.end(), colorP3, colorP3 + 4);
			wireCoordinates.insert(wireCoordinates.end(), p3, p3 + 3);
			wireColor.insert(wireColor.end(), colorP3, colorP3 + 4);
			wireCoordinates.insert(wireCoordinates.end(), p1, p1 + 3);
			wireColor.insert(wireColor.end(), colorP1, colorP1 + 4);
			wireCoordinates.insert(wireCoordinates.end(), p1, p1 + 3);
			wireColor.insert(wireColor.end(), colorP1, colorP1 + 4);
			wireCoordinates.insert(wireCoordinates.end(), p2, p2 + 3);
			wireColor.insert(wireColor.end(), colorP2, colorP2 + 4);
			wireCoordinates.insert(wireCoordinates.end(), p3, p3 + 3);
			wireColor.insert(wireColor.end(), colorP3, colorP3 + 4);
			wireCoordinates.insert(wireCoordinates.end(), p4, p4 + 3);
			wireColor.insert(wireColor.end(), colorP4, colorP4 + 4);
			wireCoordinates.insert(wireCoordinates.end(), p4, p4 + 3);
			wireColor.insert(wireColor.end(), colorP4, colorP4 + 4);
			wireCoordinates.insert(wireCoordinates.end(), p1, p1 + 3);
			wireColor.insert(wireColor.end(), colorP1, colorP1 + 4);

			// overlap wireframe
			GLfloat owColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
			owireCoordinates.insert(owireCoordinates.end(), p2, p2 + 3);
			owireColor.insert(owireColor.end(), owColor, owColor + 4);
			owireCoordinates.insert(owireCoordinates.end(), p3, p3 + 3);
			owireColor.insert(owireColor.end(), owColor, owColor + 4);
			owireCoordinates.insert(owireCoordinates.end(), p3, p3 + 3);
			owireColor.insert(owireColor.end(), owColor, owColor + 4);
			owireCoordinates.insert(owireCoordinates.end(), p1, p1 + 3);
			owireColor.insert(owireColor.end(), owColor, owColor + 4);
			owireCoordinates.insert(owireCoordinates.end(), p1, p1 + 3);
			owireColor.insert(owireColor.end(), owColor, owColor + 4);
			owireCoordinates.insert(owireCoordinates.end(), p2, p2 + 3);
			owireColor.insert(owireColor.end(), owColor, owColor + 4);
			owireCoordinates.insert(owireCoordinates.end(), p3, p3 + 3);
			owireColor.insert(owireColor.end(), owColor, owColor + 4);
			owireCoordinates.insert(owireCoordinates.end(), p4, p4 + 3);
			owireColor.insert(owireColor.end(), owColor, owColor + 4);
			owireCoordinates.insert(owireCoordinates.end(), p4, p4 + 3);
			owireColor.insert(owireColor.end(), owColor, owColor + 4);
			owireCoordinates.insert(owireCoordinates.end(), p1, p1 + 3);
			owireColor.insert(owireColor.end(), owColor, owColor + 4);
		}
	}
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: // ESC key
		exit(0); // exit the program
		break;

	case ' ':
		cout << "You pressed the spacebar." << endl;
		break;

	case 'x':
		// take a screenshot
		saveScreenshot("screenshot.jpg");
		break;

	case 'p':
		// points
		option = 0;
		break;

	case 'w':
		// wireframes
		option = 1;
		break;

	case 't':
		// triangles
		option = 2;
		break;

	case 'o':
		// wireframe over triangles
		option = 3;
		break;

	case 'a':
		// animation toggle
		if (aniCheck)
			aniCheck = false;
		else
			aniCheck = true;
		break;

	case 'r':
		// reset to the initial rendering
		landRotate[0] = 0.0f;
		landRotate[1] = 0.0f;
		landRotate[2] = 0.0f;
		landTranslate[0] = 0.0f;
		landTranslate[1] = 0.0f;
		landTranslate[2] = 0.0f;
		landScale[0] = 1.0f;
		landScale[1] = 1.0f;
		landScale[2] = 1.0f;
		theta[0] = 1.0f;
		theta[1] = 1.0f;
		theta[2] = 1.0f;
		break;
	}
}

void initScene(int argc, char *argv[])
{
	// load the image from a jpeg disk file to main memory
	heightmapImage = new ImageIO();
	if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
	{
		cout << "Error reading image " << argv[1] << "." << endl;
		exit(EXIT_FAILURE);
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// do additional initialization here...
	glEnable(GL_DEPTH_TEST);
	matrix = new OpenGLMatrix();
	calculatePoints(heightmapImage);
	initVBO();
	initPipelineProgram();
	initVAO();
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "The arguments are incorrect." << endl;
		cout << "usage: ./hw1 <heightmap file>" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Initializing GLUT..." << endl;
	glutInit(&argc, argv);

	cout << "Initializing OpenGL..." << endl;

#ifdef __APPLE__
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#endif

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowTitle);

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// tells glut to use a particular display function to redraw 
	glutDisplayFunc(displayFunc);
	// perform animation inside idleFunc
	glutIdleFunc(idleFunc);
	// callback for mouse drags
	glutMotionFunc(mouseMotionDragFunc);
	// callback for idle mouse movement
	glutPassiveMotionFunc(mouseMotionFunc);
	// callback for mouse button changes
	glutMouseFunc(mouseButtonFunc);
	// callback for resizing the window
	glutReshapeFunc(reshapeFunc);
	// callback for pressing the keys on the keyboard
	glutKeyboardFunc(keyboardFunc);

	// init glew
#ifdef __APPLE__
	// nothing is needed on Apple
#else
	// Windows, Linux
	GLint result = glewInit();
	if (result != GLEW_OK)
	{
		cout << "error: " << glewGetErrorString(result) << endl;
		exit(EXIT_FAILURE);
	}
#endif

	// do initialization
	initScene(argc, argv);

	// sink forever into the glut loop
	glutMainLoop();
}


