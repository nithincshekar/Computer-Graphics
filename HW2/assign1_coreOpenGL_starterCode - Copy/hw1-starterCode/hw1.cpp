/*
CSCI 420 Computer Graphics, USC
Assignment 1: Height Fields
C++ starter code

Student username: Nithin Chandrashekhar
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <cstring>
#include <vector>
#include "openGLHeader.h"
#include "glutHeader.h"

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"
#include "texturePipelineProgram.h"

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

// gap between the track
float gapScale = 0.2f;

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
char windowTitle[512] = "CSCI 420 homework II";

ImageIO * heightmapImage;

// Declare variables for handling the functionality.
// option for changing between solid, wireframe and points.
int option = 0;

// represents one control point along the spline 
struct Point
{
	double x;
	double y;
	double z;
};

// spline related coordinates
std::vector<GLfloat> splineCoordinates;
std::vector<GLfloat> newsplineCoordinates;
std::vector<Point> tangentCoordinates;
std::vector<Point> normalCoordinates;
std::vector<Point> binormalCoordinates;

// ground
std::vector<GLfloat> groundCoordinates;
std::vector<GLfloat> groundUVs;
GLuint groundVAO;
GLuint groundVerticesVBO;
GLuint groundUVsVBO;
GLuint groundTexture;

// sky
std::vector<GLfloat> skyCoordinates;
std::vector<GLfloat> skyUVs;
GLuint skyVAO;
GLuint skyVerticesVBO;
GLuint skyUVsVBO;
GLuint skyTexture;

// track0
std::vector<GLfloat> trackCoordinates;
std::vector<GLfloat> trackboxUVs;
GLuint trackVAO;
GLuint trackVerticesVBO;
GLuint trackUVsVBO;
GLuint trackTexture;

// track1
std::vector<GLfloat> trackCoordinates1;
std::vector<GLfloat> trackboxUVs1;
GLuint trackVAO1;
GLuint trackVerticesVBO1;
GLuint trackUVsVBO1;
GLuint trackTexture1;

// track2
std::vector<GLfloat> trackCoordinates2;
std::vector<GLfloat> trackboxUVs2;
GLuint trackVAO2;
GLuint trackVerticesVBO2;
GLuint trackUVsVBO2;
GLuint trackTexture2;

// track3
std::vector<GLfloat> trackCoordinates3;
std::vector<GLfloat> trackboxUVs3;
GLuint trackVAO3;
GLuint trackVerticesVBO3;
GLuint trackUVsVBO3;
GLuint trackTexture3;

// track0
std::vector<GLfloat> newtrackCoordinates;
std::vector<GLfloat> newtrackboxUVs;
GLuint newtrackVAO;
GLuint newtrackVerticesVBO;
GLuint newtrackUVsVBO;
GLuint newtrackTexture;

// track1
std::vector<GLfloat> newtrackCoordinates1;
std::vector<GLfloat> newtrackboxUVs1;
GLuint newtrackVAO1;
GLuint newtrackVerticesVBO1;
GLuint newtrackUVsVBO1;
GLuint newtrackTexture1;

// track2
std::vector<GLfloat> newtrackCoordinates2;
std::vector<GLfloat> newtrackboxUVs2;
GLuint newtrackVAO2;
GLuint newtrackVerticesVBO2;
GLuint newtrackUVsVBO2;
GLuint newtrackTexture2;

// track3
std::vector<GLfloat> newtrackCoordinates3;
std::vector<GLfloat> newtrackboxUVs3;
GLuint newtrackVAO3;
GLuint newtrackVerticesVBO3;
GLuint newtrackUVsVBO3;
GLuint newtrackTexture3;

// plank coordinates
std::vector<GLfloat> leftplankCoordinates;
std::vector<GLfloat> rightplankCoordinates;

// plank 
std::vector<GLfloat> plankCoordinates;
std::vector<GLfloat> plankboxUVs;
GLuint plankVAO;
GLuint plankVerticesVBO;
GLuint plankUVsVBO;
GLuint plankTexture;

// modelview and projection matrix
float m[16];
float p[16];

// animation toggle
bool aniCheck = false;

// OpenGLMatrix for handling the matrix operations
OpenGLMatrix * matrix;
// basic pipeline program
BasicPipelineProgram * pipelineProgram;
// texture pipeline program
TexturePipelineProgram * texturePipelineProgram;
// for handling rotate
GLfloat theta[3] = { 0.0, 0.0, 0.0 };

GLuint bprogram;
GLuint tprogram;
int counter = 0;
int initTexture(const char * imageFilename, GLuint textureHandle);

// spline struct 
// contains how many control points the spline has, and an array of control points 
struct Spline
{
	int numControlPoints;
	Point * points;
};

// the spline array 
Spline * splines;
// total number of splines 
int numSplines;

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
	// ground
	glGenTextures(1, &groundTexture);
	// initialise the texture for ground
	initTexture("./ground.jpg", groundTexture);
	glGenBuffers(1, &groundVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, groundVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, groundCoordinates.size() * sizeof(GLfloat), &groundCoordinates[0], GL_STATIC_DRAW);
	glGenBuffers(1, &groundUVsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, groundUVsVBO);
	glBufferData(GL_ARRAY_BUFFER, groundUVs.size() * sizeof(GLfloat), &groundUVs[0], GL_STATIC_DRAW);

	// sky
	glGenTextures(1, &skyTexture);
	// initialise the texture for sky
	initTexture("./sky.jpg", skyTexture);
	glGenBuffers(1, &skyVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, skyCoordinates.size() * sizeof(GLfloat), &skyCoordinates[0], GL_STATIC_DRAW);
	glGenBuffers(1, &skyUVsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyUVsVBO);
	glBufferData(GL_ARRAY_BUFFER, skyUVs.size() * sizeof(GLfloat), &skyUVs[0], GL_STATIC_DRAW);

	// track0
	// initialise the texture for the faces of the track
	// top and bottom faces of the track has one jpg image 
	// and the side faces of the track has one jpg image
	glGenTextures(1, &trackTexture);
	initTexture("./track.jpg", trackTexture);
	glGenBuffers(1, &trackVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, trackVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, trackCoordinates.size() * sizeof(GLfloat), &trackCoordinates[0], GL_STATIC_DRAW);
	glGenBuffers(1, &trackUVsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, trackUVsVBO);
	glBufferData(GL_ARRAY_BUFFER, trackboxUVs.size() * sizeof(GLfloat), &trackboxUVs[0], GL_STATIC_DRAW);

	//track3
	glGenTextures(1, &trackTexture3);
	initTexture("./track.jpg", trackTexture3);
	glGenBuffers(1, &trackVerticesVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, trackVerticesVBO3);
	glBufferData(GL_ARRAY_BUFFER, trackCoordinates3.size() * sizeof(GLfloat), &trackCoordinates3[0], GL_STATIC_DRAW);
	glGenBuffers(1, &trackUVsVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, trackUVsVBO3);
	glBufferData(GL_ARRAY_BUFFER, trackboxUVs3.size() * sizeof(GLfloat), &trackboxUVs3[0], GL_STATIC_DRAW);

	// track2
	glGenTextures(1, &trackTexture2);
	initTexture("./track1.jpg", trackTexture2);
	glGenBuffers(1, &trackVerticesVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, trackVerticesVBO2);
	glBufferData(GL_ARRAY_BUFFER, trackCoordinates2.size() * sizeof(GLfloat), &trackCoordinates2[0], GL_STATIC_DRAW);
	glGenBuffers(1, &trackUVsVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, trackUVsVBO2);
	glBufferData(GL_ARRAY_BUFFER, trackboxUVs2.size() * sizeof(GLfloat), &trackboxUVs2[0], GL_STATIC_DRAW);

	//track1
	glGenTextures(1, &trackTexture1);
	initTexture("./track1.jpg", trackTexture1);
	glGenBuffers(1, &trackVerticesVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, trackVerticesVBO1);
	glBufferData(GL_ARRAY_BUFFER, trackCoordinates1.size() * sizeof(GLfloat), &trackCoordinates1[0], GL_STATIC_DRAW);
	glGenBuffers(1, &trackUVsVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, trackUVsVBO1);
	glBufferData(GL_ARRAY_BUFFER, trackboxUVs1.size() * sizeof(GLfloat), &trackboxUVs1[0], GL_STATIC_DRAW);

	// track0
	glGenTextures(1, &newtrackTexture);
	initTexture("./track.jpg", newtrackTexture);
	glGenBuffers(1, &newtrackVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, newtrackCoordinates.size() * sizeof(GLfloat), &newtrackCoordinates[0], GL_STATIC_DRAW);
	glGenBuffers(1, &newtrackUVsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackUVsVBO);
	glBufferData(GL_ARRAY_BUFFER, newtrackboxUVs.size() * sizeof(GLfloat), &newtrackboxUVs[0], GL_STATIC_DRAW);

	//track3
	glGenTextures(1, &newtrackTexture3);
	initTexture("./track.jpg", newtrackTexture3);
	glGenBuffers(1, &newtrackVerticesVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackVerticesVBO3);
	glBufferData(GL_ARRAY_BUFFER, newtrackCoordinates3.size() * sizeof(GLfloat), &newtrackCoordinates3[0], GL_STATIC_DRAW);
	glGenBuffers(1, &newtrackUVsVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackUVsVBO3);
	glBufferData(GL_ARRAY_BUFFER, newtrackboxUVs3.size() * sizeof(GLfloat), &newtrackboxUVs3[0], GL_STATIC_DRAW);

	// track2
	glGenTextures(1, &newtrackTexture2);
	initTexture("./track1.jpg", newtrackTexture2);
	glGenBuffers(1, &newtrackVerticesVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackVerticesVBO2);
	glBufferData(GL_ARRAY_BUFFER, newtrackCoordinates2.size() * sizeof(GLfloat), &newtrackCoordinates2[0], GL_STATIC_DRAW);
	glGenBuffers(1, &newtrackUVsVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackUVsVBO2);
	glBufferData(GL_ARRAY_BUFFER, newtrackboxUVs2.size() * sizeof(GLfloat), &newtrackboxUVs2[0], GL_STATIC_DRAW);

	//track1
	glGenTextures(1, &newtrackTexture1);
	initTexture("./track1.jpg", newtrackTexture1);
	glGenBuffers(1, &newtrackVerticesVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackVerticesVBO1);
	glBufferData(GL_ARRAY_BUFFER, newtrackCoordinates1.size() * sizeof(GLfloat), &newtrackCoordinates1[0], GL_STATIC_DRAW);
	glGenBuffers(1, &newtrackUVsVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackUVsVBO1);
	glBufferData(GL_ARRAY_BUFFER, newtrackboxUVs1.size() * sizeof(GLfloat), &newtrackboxUVs1[0], GL_STATIC_DRAW);

	//plank
	glGenTextures(1, &plankTexture);
	initTexture("./rail.jpg", plankTexture);
	glGenBuffers(1, &plankVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, plankVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, plankCoordinates.size() * sizeof(GLfloat), &plankCoordinates[0], GL_STATIC_DRAW);
	glGenBuffers(1, &plankUVsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, plankUVsVBO);
	glBufferData(GL_ARRAY_BUFFER, plankboxUVs.size() * sizeof(GLfloat), &plankboxUVs[0], GL_STATIC_DRAW);
}

// bind the array object
void initVAO()
{
	// ground
	glGenVertexArrays(1, &groundVAO);
	glBindVertexArray(groundVAO);

	// sky
	glGenVertexArrays(1, &skyVAO);
	glBindVertexArray(skyVAO);

	// track0
	glGenVertexArrays(1, &trackVAO);
	glBindVertexArray(trackVAO);

	// track1
	glGenVertexArrays(1, &trackVAO1);
	glBindVertexArray(trackVAO1);

	// track2
	glGenVertexArrays(1, &trackVAO2);
	glBindVertexArray(trackVAO2);

	//// track3
	glGenVertexArrays(1, &trackVAO3);
	glBindVertexArray(trackVAO3);

	// track0
	glGenVertexArrays(1, &newtrackVAO);
	glBindVertexArray(newtrackVAO);

	// track1
	glGenVertexArrays(1, &newtrackVAO1);
	glBindVertexArray(newtrackVAO1);

	// track2
	glGenVertexArrays(1, &newtrackVAO2);
	glBindVertexArray(newtrackVAO2);

	//// track3
	glGenVertexArrays(1, &newtrackVAO3);
	glBindVertexArray(newtrackVAO3);

	//// plank
	glGenVertexArrays(1, &plankVAO);
	glBindVertexArray(plankVAO);
}

// pipeline program for init
void initPipelineProgram()
{
	// for normal drawing
	pipelineProgram = new BasicPipelineProgram();
	// for texture drawing
	texturePipelineProgram = new TexturePipelineProgram();
	pipelineProgram->Init("../openGLHelper-starterCode");
	texturePipelineProgram->Init("../openGLHelper-starterCode");
}

void renderTrack()
{
	// render all faces of the track.
	// each track has 4 faces which have to be rendered.
	// in total for 2 tracks, there are eight faces which are
	// being rendered here.
	glBindTexture(GL_TEXTURE_2D, trackTexture);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, trackVerticesVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, trackUVsVBO);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, trackCoordinates.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	////////////////////////////////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D, trackTexture1);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, trackVerticesVBO1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, trackUVsVBO1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, trackCoordinates1.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	////////////////////////////////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D, trackTexture2);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, trackVerticesVBO2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, trackUVsVBO2);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, trackCoordinates2.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	////////////////////////////////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D, trackTexture3);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, trackVerticesVBO3);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, trackUVsVBO3);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, trackCoordinates3.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	///////////////////////////////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D, newtrackTexture);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackVerticesVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackUVsVBO);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, newtrackCoordinates.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	////////////////////////////////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D, newtrackTexture1);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackVerticesVBO1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackUVsVBO1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, newtrackCoordinates1.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	////////////////////////////////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D, newtrackTexture2);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackVerticesVBO2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackUVsVBO2);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, newtrackCoordinates2.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	////////////////////////////////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D, newtrackTexture3);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackVerticesVBO3);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, newtrackUVsVBO3);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, newtrackCoordinates3.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	////////////////////////////////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D, plankTexture);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, plankVerticesVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, plankUVsVBO);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, plankCoordinates.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// render ground
void renderGround()
{
	// render the ground
	glBindTexture(GL_TEXTURE_2D, groundTexture);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, groundVerticesVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, groundUVsVBO);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, groundCoordinates.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// render sky
void renderSky()
{
	// render the sky.
	// there are six faces that have to be rendered.
	// all the faces are being rendered here.
	glBindTexture(GL_TEXTURE_2D, skyTexture);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, skyVerticesVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, skyUVsVBO);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, skyCoordinates.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void setTextureUnit(GLint unit, GLuint program)
{
	glActiveTexture(unit);
	GLint h_textureImage = glGetUniformLocation(program, "textureImage");
	glUniform1i(h_textureImage, unit - GL_TEXTURE0);
}

int i = 0;
// display function
void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	matrix->SetMatrixMode(OpenGLMatrix::ModelView);
	matrix->LoadIdentity();
	//// camera position
	//matrix->LookAt(0, 1, 128, 0, 0, 0, 0, 1, 0);

	Point eye, center;
	// set the eye and center
	eye.x = splineCoordinates[i * 3] + binormalCoordinates[i].x + (gapScale * normalCoordinates[i].x);
	eye.y = splineCoordinates[(i * 3) + 1] + binormalCoordinates[i].y + (gapScale * normalCoordinates[i].y);
	eye.z = splineCoordinates[(i * 3) + 2] + binormalCoordinates[i].z + (gapScale * normalCoordinates[i].z);

	center.x = splineCoordinates[i * 3] + tangentCoordinates[i].x + (gapScale * normalCoordinates[i].x);
	center.y = splineCoordinates[(i * 3) + 1] + tangentCoordinates[i].y + (gapScale * normalCoordinates[i].y);
	center.z = splineCoordinates[(i * 3) + 2] + tangentCoordinates[i].z + (gapScale * normalCoordinates[i].z);

	matrix->LookAt(eye.x, eye.y, eye.z,
		center.x, center.y, center.z,
		binormalCoordinates[i].x, binormalCoordinates[i].y, binormalCoordinates[i].z);
	if ((i + 6) >= binormalCoordinates.size())
		exit(0);
	else
		i += 6;

	// transformations
	matrix->Scale(landScale[0], landScale[1], landScale[2]);
	matrix->Translate(landTranslate[0] * 10, landTranslate[1] * 10, landTranslate[2] * 10);
	matrix->Rotate(theta[0], 1.0, 0.0, 0.0);
	matrix->Rotate(theta[1], 0.0, 1.0, 0.0);
	matrix->Rotate(theta[2], 0.0, 0.0, 1.0);
	matrix->Rotate(landRotate[0], 1.0, 0.0, 0.0);
	matrix->Rotate(landRotate[1], 0.0, 1.0, 0.0);
	matrix->Rotate(landRotate[2], 0.0, 0.0, 1.0);

	texturePipelineProgram->Bind();
	tprogram = texturePipelineProgram->GetProgramHandle();

	// get a handle to the modelViewMatrix shader variable
	GLint modelViewMatrix = glGetUniformLocation(tprogram, "modelViewMatrix");
	matrix->SetMatrixMode(OpenGLMatrix::ModelView);
	matrix->GetMatrix(m);
	glUniformMatrix4fv(modelViewMatrix, 1, GL_FALSE, m);

	// get a handle to the projectionMatrix shader variable
	GLint projectionMatrix = glGetUniformLocation(tprogram, "projectionMatrix");
	matrix->SetMatrixMode(OpenGLMatrix::Projection);
	matrix->GetMatrix(p);
	glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, p);
	setTextureUnit(GL_TEXTURE0, tprogram);

	renderGround();
	renderSky();
	renderTrack();

	glBindVertexArray(0);
	glutSwapBuffers();
}

int j = 0;
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

	string name = "roller_coaster" + std::to_string(j) + ".jpg";
	//saveScreenshot(name.c_str());

	j += 1;

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

bool fl = false;


Point tangentCalc(Point p0, Point p1, Point p2, Point p3, float u)
{
	// calculating tangent for all the points
	// by substituting the values in the spline equation which has been 
	// differentiated.
	float s = 0.5F;
	Point tangent;
	tangent.x = 3 * pow(u, 2) * (((-s) * p0.x) + ((2 - s) * p1.x) + ((s - 2) * p2.x) + (s * p3.x)) +
		2 * pow(u, 1) * (((2 * s) * p0.x) + ((s - 3) * p1.x) + ((3 - (2 * s)) * p2.x) + ((-s) * p3.x)) +
		1 * (((-s) * p0.x) + (s * p2.x));

	tangent.y = 3 * pow(u, 2) * (((-s) * p0.y) + ((2 - s) * p1.y) + ((s - 2) * p2.y) + (s * p3.y)) +
		2 * pow(u, 1) * (((2 * s) * p0.y) + ((s - 3) * p1.y) + ((3 - (2 * s)) * p2.y) + ((-s) * p3.y)) +
		1 * (((-s) * p0.y) + (s * p2.y));

	tangent.z = 3 * pow(u, 2) * (((-s) * p0.z) + ((2 - s) * p1.z) + ((s - 2) * p2.z) + (s * p3.z)) +
		2 * pow(u, 1) * (((2 * s) * p0.z) + ((s - 3) * p1.z) + ((3 - (2 * s)) * p2.z) + ((-s) * p3.z)) +
		1 * (((-s) * p0.z) + (s * p2.z));

	return tangent;
}

Point unitize(Point p)
{
	// calculating the unit vecctor by finding the magnitude and 
	// divide the coordinates by the magnitude.
	float magnitude = std::sqrt(std::pow(p.x, 2) + std::pow(p.y, 2) + std::pow(p.z, 2));
	Point res;
	res.x = p.x / magnitude;
	res.y = p.y / magnitude;
	res.z = p.z / magnitude;
	return res;
}

Point crossProduct(Point a, Point b)
{
	// calculate the cross  product of 2 vectors
	Point c;
	c.x = a.y * b.z - a.z * b.y;
	c.y = a.z * b.x - a.x * b.z;
	c.z = a.x * b.y - a.y * b.x;
	return c;
}

Point tempNorm = { 0.0, 0.0, -1.0 };

float alpha = 0.05;
Point* calFour(Point P, int i)
{
	// calculating four corner points based on the spline points to 
	// render the track
	Point *res = new Point[4];
	res[0].x = P.x + alpha * (normalCoordinates[i].x - binormalCoordinates[i].x);
	res[0].y = P.y + alpha * (normalCoordinates[i].y - binormalCoordinates[i].y);
	res[0].z = P.z + alpha * (normalCoordinates[i].z - binormalCoordinates[i].z);

	res[1].x = P.x + alpha * (normalCoordinates[i].x + binormalCoordinates[i].x);
	res[1].y = P.y + alpha * (normalCoordinates[i].y + binormalCoordinates[i].y);
	res[1].z = P.z + alpha * (normalCoordinates[i].z + binormalCoordinates[i].z);

	res[2].x = P.x + alpha * (binormalCoordinates[i].x - normalCoordinates[i].x);
	res[2].y = P.y + alpha * (binormalCoordinates[i].y - normalCoordinates[i].y);
	res[2].z = P.z + alpha * (binormalCoordinates[i].z - normalCoordinates[i].z);

	res[3].x = P.x + alpha * (-1 * (normalCoordinates[i].x + binormalCoordinates[i].x));
	res[3].y = P.y + alpha * (-1 * (normalCoordinates[i].y + binormalCoordinates[i].y));
	res[3].z = P.z + alpha * (-1 * (normalCoordinates[i].z + binormalCoordinates[i].z));

	return res;
}

// calculate the track points
void calculateTrack(int trackCheck)
{
	// calculating the triangles to render the texture for the track.
	if (trackCheck == 1)
	{
		Point P0 = { splineCoordinates[0], splineCoordinates[1], splineCoordinates[2] };
		Point* pRes0 = calFour(P0, 0);
		int i = 120;
		while (i < splineCoordinates.size() / 3)
		{
			Point p1 = { splineCoordinates[i * 3], splineCoordinates[i * 3 + 1], splineCoordinates[i * 3 + 2] };
			Point* pRes1 = calFour(p1, i);
			{
				GLfloat V1[3] = { pRes0[1].x, pRes0[1].y, pRes0[1].z };
				GLfloat V2[3] = { pRes0[2].x, pRes0[2].y, pRes0[2].z };
				GLfloat S1[3] = { pRes1[1].x, pRes1[1].y, pRes1[1].z };
				GLfloat S2[3] = { pRes1[2].x, pRes1[2].y, pRes1[2].z };

				trackCoordinates.insert(trackCoordinates.end(), V1, V1 + 3);
				trackCoordinates.insert(trackCoordinates.end(), V2, V2 + 3);
				trackCoordinates.insert(trackCoordinates.end(), S1, S1 + 3);
				trackCoordinates.insert(trackCoordinates.end(), S1, S1 + 3);
				trackCoordinates.insert(trackCoordinates.end(), S2, S2 + 3);
				trackCoordinates.insert(trackCoordinates.end(), V2, V2 + 3);


				GLfloat bl_uv[3] = { 0, 0 };
				GLfloat tl_uv[3] = { 0, 1 };
				GLfloat tr_uv[3] = { 1, 1 };
				GLfloat br_uv[3] = { 1, 0 };

				trackboxUVs.insert(trackboxUVs.end(), tl_uv, tl_uv + 2);
				trackboxUVs.insert(trackboxUVs.end(), tr_uv, tr_uv + 2);
				trackboxUVs.insert(trackboxUVs.end(), bl_uv, bl_uv + 2);
				trackboxUVs.insert(trackboxUVs.end(), bl_uv, bl_uv + 2);
				trackboxUVs.insert(trackboxUVs.end(), tr_uv, tr_uv + 2);
				trackboxUVs.insert(trackboxUVs.end(), br_uv, br_uv + 2);
			}

			///////////////////////////////////////////////////////////////////////
			{
				GLfloat V1[3] = { pRes0[1].x, pRes0[1].y, pRes0[1].z };
				GLfloat V0[3] = { pRes0[0].x, pRes0[0].y, pRes0[0].z };
				GLfloat S1[3] = { pRes1[1].x, pRes1[1].y, pRes1[1].z };
				GLfloat S0[3] = { pRes1[0].x, pRes1[0].y, pRes1[0].z };

				leftplankCoordinates.insert(leftplankCoordinates.end(), V1, V1 + 3);
				leftplankCoordinates.insert(leftplankCoordinates.end(), S1, S1 + 3);

				trackCoordinates1.insert(trackCoordinates1.end(), V1, V1 + 3);
				trackCoordinates1.insert(trackCoordinates1.end(), V0, V0 + 3);
				trackCoordinates1.insert(trackCoordinates1.end(), S1, S1 + 3);
				trackCoordinates1.insert(trackCoordinates1.end(), S1, S1 + 3);
				trackCoordinates1.insert(trackCoordinates1.end(), S0, S0 + 3);
				trackCoordinates1.insert(trackCoordinates1.end(), V1, V1 + 3);

				GLfloat bl_uv[3] = { 0, 0 };
				GLfloat tl_uv[3] = { 0, 1 };
				GLfloat tr_uv[3] = { 1, 1 };
				GLfloat br_uv[3] = { 1, 0 };

				trackboxUVs1.insert(trackboxUVs1.end(), tl_uv, tl_uv + 2);
				trackboxUVs1.insert(trackboxUVs1.end(), tr_uv, tr_uv + 2);
				trackboxUVs1.insert(trackboxUVs1.end(), bl_uv, bl_uv + 2);
				trackboxUVs1.insert(trackboxUVs1.end(), bl_uv, bl_uv + 2);
				trackboxUVs1.insert(trackboxUVs1.end(), tr_uv, tr_uv + 2);
				trackboxUVs1.insert(trackboxUVs1.end(), br_uv, br_uv + 2);
			}

			{
				GLfloat V3[3] = { pRes0[3].x, pRes0[3].y, pRes0[3].z };
				GLfloat V2[3] = { pRes0[2].x, pRes0[2].y, pRes0[2].z };
				GLfloat S3[3] = { pRes1[3].x, pRes1[3].y, pRes1[3].z };
				GLfloat S2[3] = { pRes1[2].x, pRes1[2].y, pRes1[2].z };

				trackCoordinates2.insert(trackCoordinates2.end(), V3, V3 + 3);
				trackCoordinates2.insert(trackCoordinates2.end(), V2, V2 + 3);
				trackCoordinates2.insert(trackCoordinates2.end(), S3, S3 + 3);
				trackCoordinates2.insert(trackCoordinates2.end(), S3, S3 + 3);
				trackCoordinates2.insert(trackCoordinates2.end(), S2, S2 + 3);
				trackCoordinates2.insert(trackCoordinates2.end(), V2, V2 + 3);

				GLfloat bl_uv[3] = { 0, 0 };
				GLfloat tl_uv[3] = { 0, 1 };
				GLfloat tr_uv[3] = { 1, 1 };
				GLfloat br_uv[3] = { 1, 0 };

				trackboxUVs2.insert(trackboxUVs2.end(), tl_uv, tl_uv + 2);
				trackboxUVs2.insert(trackboxUVs2.end(), tr_uv, tr_uv + 2);
				trackboxUVs2.insert(trackboxUVs2.end(), bl_uv, bl_uv + 2);
				trackboxUVs2.insert(trackboxUVs2.end(), bl_uv, bl_uv + 2);
				trackboxUVs2.insert(trackboxUVs2.end(), tr_uv, tr_uv + 2);
				trackboxUVs2.insert(trackboxUVs2.end(), br_uv, br_uv + 2);
			}

			{
				GLfloat V0[3] = { pRes0[0].x, pRes0[0].y, pRes0[0].z };
				GLfloat V3[3] = { pRes0[3].x, pRes0[3].y, pRes0[3].z };
				GLfloat S0[3] = { pRes1[0].x, pRes1[0].y, pRes1[0].z };
				GLfloat S3[3] = { pRes1[3].x, pRes1[3].y, pRes1[3].z };

				trackCoordinates3.insert(trackCoordinates3.end(), V0, V0 + 3);
				trackCoordinates3.insert(trackCoordinates3.end(), V3, V3 + 3);
				trackCoordinates3.insert(trackCoordinates3.end(), S0, S0 + 3);
				trackCoordinates3.insert(trackCoordinates3.end(), S0, S0 + 3);
				trackCoordinates3.insert(trackCoordinates3.end(), S3, S3 + 3);
				trackCoordinates3.insert(trackCoordinates3.end(), V3, V3 + 3);

				GLfloat bl_uv[3] = { 0, 0 };
				GLfloat tl_uv[3] = { 0, 1 };
				GLfloat tr_uv[3] = { 1, 1 };
				GLfloat br_uv[3] = { 1, 0 };

				trackboxUVs3.insert(trackboxUVs3.end(), tl_uv, tl_uv + 2);
				trackboxUVs3.insert(trackboxUVs3.end(), tr_uv, tr_uv + 2);
				trackboxUVs3.insert(trackboxUVs3.end(), bl_uv, bl_uv + 2);
				trackboxUVs3.insert(trackboxUVs3.end(), bl_uv, bl_uv + 2);
				trackboxUVs3.insert(trackboxUVs3.end(), tr_uv, tr_uv + 2);
				trackboxUVs3.insert(trackboxUVs3.end(), br_uv, br_uv + 2);
			}

			pRes0 = pRes1;
			i += 120;
		}
	}
	else
	{
		Point P0 = { newsplineCoordinates[0], newsplineCoordinates[1], newsplineCoordinates[2] };
		Point* pRes0 = calFour(P0, 0);
		int i = 120;
		while (i < newsplineCoordinates.size() / 3)
		{
			Point p1 = { newsplineCoordinates[i * 3], newsplineCoordinates[i * 3 + 1], newsplineCoordinates[i * 3 + 2] };
			Point* pRes1 = calFour(p1, i);
			{
				GLfloat V1[3] = { pRes0[1].x, pRes0[1].y, pRes0[1].z };
				GLfloat V2[3] = { pRes0[2].x, pRes0[2].y, pRes0[2].z };
				GLfloat S1[3] = { pRes1[1].x, pRes1[1].y, pRes1[1].z };
				GLfloat S2[3] = { pRes1[2].x, pRes1[2].y, pRes1[2].z };

				newtrackCoordinates.insert(newtrackCoordinates.end(), V1, V1 + 3);
				newtrackCoordinates.insert(newtrackCoordinates.end(), V2, V2 + 3);
				newtrackCoordinates.insert(newtrackCoordinates.end(), S1, S1 + 3);
				newtrackCoordinates.insert(newtrackCoordinates.end(), S1, S1 + 3);
				newtrackCoordinates.insert(newtrackCoordinates.end(), S2, S2 + 3);
				newtrackCoordinates.insert(newtrackCoordinates.end(), V2, V2 + 3);

				GLfloat bl_uv[3] = { 0, 0 };
				GLfloat tl_uv[3] = { 0, 1 };
				GLfloat tr_uv[3] = { 1, 1 };
				GLfloat br_uv[3] = { 1, 0 };

				newtrackboxUVs.insert(newtrackboxUVs.end(), tl_uv, tl_uv + 2);
				newtrackboxUVs.insert(newtrackboxUVs.end(), tr_uv, tr_uv + 2);
				newtrackboxUVs.insert(newtrackboxUVs.end(), bl_uv, bl_uv + 2);
				newtrackboxUVs.insert(newtrackboxUVs.end(), bl_uv, bl_uv + 2);
				newtrackboxUVs.insert(newtrackboxUVs.end(), tr_uv, tr_uv + 2);
				newtrackboxUVs.insert(newtrackboxUVs.end(), br_uv, br_uv + 2);
			}

			///////////////////////////////////////////////////////////////////////
			{
				GLfloat V1[3] = { pRes0[1].x, pRes0[1].y, pRes0[1].z };
				GLfloat V0[3] = { pRes0[0].x, pRes0[0].y, pRes0[0].z };
				GLfloat S1[3] = { pRes1[1].x, pRes1[1].y, pRes1[1].z };
				GLfloat S0[3] = { pRes1[0].x, pRes1[0].y, pRes1[0].z };

				newtrackCoordinates1.insert(newtrackCoordinates1.end(), V1, V1 + 3);
				newtrackCoordinates1.insert(newtrackCoordinates1.end(), V0, V0 + 3);
				newtrackCoordinates1.insert(newtrackCoordinates1.end(), S1, S1 + 3);
				newtrackCoordinates1.insert(newtrackCoordinates1.end(), S1, S1 + 3);
				newtrackCoordinates1.insert(newtrackCoordinates1.end(), S0, S0 + 3);
				newtrackCoordinates1.insert(newtrackCoordinates1.end(), V1, V1 + 3);

				GLfloat bl_uv[3] = { 0, 0 };
				GLfloat tl_uv[3] = { 0, 1 };
				GLfloat tr_uv[3] = { 1, 1 };
				GLfloat br_uv[3] = { 1, 0 };

				newtrackboxUVs1.insert(newtrackboxUVs1.end(), tl_uv, tl_uv + 2);
				newtrackboxUVs1.insert(newtrackboxUVs1.end(), tr_uv, tr_uv + 2);
				newtrackboxUVs1.insert(newtrackboxUVs1.end(), bl_uv, bl_uv + 2);
				newtrackboxUVs1.insert(newtrackboxUVs1.end(), bl_uv, bl_uv + 2);
				newtrackboxUVs1.insert(newtrackboxUVs1.end(), tr_uv, tr_uv + 2);
				newtrackboxUVs1.insert(newtrackboxUVs1.end(), br_uv, br_uv + 2);
			}

			{
				GLfloat V3[3] = { pRes0[3].x, pRes0[3].y, pRes0[3].z };
				GLfloat V2[3] = { pRes0[2].x, pRes0[2].y, pRes0[2].z };
				GLfloat S3[3] = { pRes1[3].x, pRes1[3].y, pRes1[3].z };
				GLfloat S2[3] = { pRes1[2].x, pRes1[2].y, pRes1[2].z };

				rightplankCoordinates.insert(rightplankCoordinates.end(), V2, V2 + 3);
				rightplankCoordinates.insert(rightplankCoordinates.end(), S2, S2 + 3);

				newtrackCoordinates2.insert(newtrackCoordinates2.end(), V3, V3 + 3);
				newtrackCoordinates2.insert(newtrackCoordinates2.end(), V2, V2 + 3);
				newtrackCoordinates2.insert(newtrackCoordinates2.end(), S3, S3 + 3);
				newtrackCoordinates2.insert(newtrackCoordinates2.end(), S3, S3 + 3);
				newtrackCoordinates2.insert(newtrackCoordinates2.end(), S2, S2 + 3);
				newtrackCoordinates2.insert(newtrackCoordinates2.end(), V2, V2 + 3);

				GLfloat bl_uv[3] = { 0, 0 };
				GLfloat tl_uv[3] = { 0, 1 };
				GLfloat tr_uv[3] = { 1, 1 };
				GLfloat br_uv[3] = { 1, 0 };

				newtrackboxUVs2.insert(newtrackboxUVs2.end(), tl_uv, tl_uv + 2);
				newtrackboxUVs2.insert(newtrackboxUVs2.end(), tr_uv, tr_uv + 2);
				newtrackboxUVs2.insert(newtrackboxUVs2.end(), bl_uv, bl_uv + 2);
				newtrackboxUVs2.insert(newtrackboxUVs2.end(), bl_uv, bl_uv + 2);
				newtrackboxUVs2.insert(newtrackboxUVs2.end(), tr_uv, tr_uv + 2);
				newtrackboxUVs2.insert(newtrackboxUVs2.end(), br_uv, br_uv + 2);
			}

			{
				GLfloat V0[3] = { pRes0[0].x, pRes0[0].y, pRes0[0].z };
				GLfloat V3[3] = { pRes0[3].x, pRes0[3].y, pRes0[3].z };
				GLfloat S0[3] = { pRes1[0].x, pRes1[0].y, pRes1[0].z };
				GLfloat S3[3] = { pRes1[3].x, pRes1[3].y, pRes1[3].z };

				newtrackCoordinates3.insert(newtrackCoordinates3.end(), V0, V0 + 3);
				newtrackCoordinates3.insert(newtrackCoordinates3.end(), V3, V3 + 3);
				newtrackCoordinates3.insert(newtrackCoordinates3.end(), S0, S0 + 3);
				newtrackCoordinates3.insert(newtrackCoordinates3.end(), S0, S0 + 3);
				newtrackCoordinates3.insert(newtrackCoordinates3.end(), S3, S3 + 3);
				newtrackCoordinates3.insert(newtrackCoordinates3.end(), V3, V3 + 3);

				GLfloat bl_uv[3] = { 0, 0 };
				GLfloat tl_uv[3] = { 0, 1 };
				GLfloat tr_uv[3] = { 1, 1 };
				GLfloat br_uv[3] = { 1, 0 };

				newtrackboxUVs3.insert(newtrackboxUVs3.end(), tl_uv, tl_uv + 2);
				newtrackboxUVs3.insert(newtrackboxUVs3.end(), tr_uv, tr_uv + 2);
				newtrackboxUVs3.insert(newtrackboxUVs3.end(), bl_uv, bl_uv + 2);
				newtrackboxUVs3.insert(newtrackboxUVs3.end(), bl_uv, bl_uv + 2);
				newtrackboxUVs3.insert(newtrackboxUVs3.end(), tr_uv, tr_uv + 2);
				newtrackboxUVs3.insert(newtrackboxUVs3.end(), br_uv, br_uv + 2);
			}

			pRes0 = pRes1;
			i += 120;
		}
	}
}

// calculate the plank points
void calculatePlank()
{
	// calculating the triangles to render the plank using the texture.
	int i = 0;
	while (i < leftplankCoordinates.size())
	{
		GLfloat V1[3] = { leftplankCoordinates[i], leftplankCoordinates[i + 1], leftplankCoordinates[i + 2] };
		GLfloat V2[3] = { leftplankCoordinates[i + 3], leftplankCoordinates[i + 4], leftplankCoordinates[i + 5] };
		GLfloat S1[3] = { rightplankCoordinates[i], rightplankCoordinates[i + 1], rightplankCoordinates[i + 2] };
		GLfloat S2[3] = { rightplankCoordinates[i + 3], rightplankCoordinates[i + 4], rightplankCoordinates[i + 5] };

		plankCoordinates.insert(plankCoordinates.end(), V1, V1 + 3);
		plankCoordinates.insert(plankCoordinates.end(), V2, V2 + 3);
		plankCoordinates.insert(plankCoordinates.end(), S1, S1 + 3);
		plankCoordinates.insert(plankCoordinates.end(), S1, S1 + 3);
		plankCoordinates.insert(plankCoordinates.end(), S2, S2 + 3);
		plankCoordinates.insert(plankCoordinates.end(), V2, V2 + 3);

		GLfloat bl_uv[3] = { 0, 0 };
		GLfloat tl_uv[3] = { 0, 1 };
		GLfloat tr_uv[3] = { 1, 1 };
		GLfloat br_uv[3] = { 1, 0 };

		plankboxUVs.insert(plankboxUVs.end(), tl_uv, tl_uv + 2);
		plankboxUVs.insert(plankboxUVs.end(), tr_uv, tr_uv + 2);
		plankboxUVs.insert(plankboxUVs.end(), bl_uv, bl_uv + 2);
		plankboxUVs.insert(plankboxUVs.end(), bl_uv, bl_uv + 2);
		plankboxUVs.insert(plankboxUVs.end(), tr_uv, tr_uv + 2);
		plankboxUVs.insert(plankboxUVs.end(), br_uv, br_uv + 2);

		i += 30;
	}
}

// calculate the spline points
void calculatePoints()
{
	// calculate the spline points using the equation given in the slides.
	float s = 0.5;
	for (int i = 0; i < numSplines; i++)
	{
		for (int j = 0; j < splines[i].numControlPoints - 3; j++)
		{
			Point p0 = splines[i].points[j];
			Point p1 = splines[i].points[j+1];
			Point p2 = splines[i].points[j+2];
			Point p3 = splines[i].points[j+3];

			for (float u = 0; u <= 1; u += 0.0005)
			{
				GLfloat x = pow(u, 3) * (((-s) * p0.x) + ((2 - s) * p1.x) + ((s - 2) * p2.x) + (s * p3.x)) +
					pow(u, 2) * (((2 * s) * p0.x) + ((s - 3) * p1.x) + ((3 - (2 * s)) * p2.x) + ((-s) * p3.x)) +
					u * (((-s) * p0.x) + (s * p2.x)) +
					p1.x;

				GLfloat y = pow(u, 3) * (((-s) * p0.y) + ((2 - s) * p1.y) + ((s - 2) * p2.y) + (s * p3.y)) +
					pow(u, 2) * (((2 * s) * p0.y) + ((s - 3) * p1.y) + ((3 - (2 * s)) * p2.y) + ((-s) * p3.y)) +
					u * (((-s) * p0.y) + (s * p2.y)) +
					p1.y;

				GLfloat z = pow(u, 3) * (((-s) * p0.z) + ((2 - s) * p1.z) + ((s - 2) * p2.z) + (s * p3.z)) +
					pow(u, 2) * (((2 * s) * p0.z) + ((s - 3) * p1.z) + ((3 - (2 * s)) * p2.z) + ((-s) * p3.z)) +
					u * (((-s) * p0.z) + (s * p2.z)) +
					p1.z;

				Point tangent = tangentCalc(p0, p1, p2, p3, u);
				tangentCoordinates.push_back(unitize(tangent));
				Point splineNormal, splineBinormal;
				if (!fl)
				{
					tempNorm.x = 0.0;
					tempNorm.y = 0.0;
					tempNorm.z = -1.0;

					Point T0 = tangentCoordinates[0];
					splineNormal = unitize(crossProduct(T0, tempNorm));
					splineBinormal = unitize(crossProduct(T0, splineNormal));
					fl = true;
				}
				else
				{
					Point B0 = binormalCoordinates[binormalCoordinates.size() - 1];
					Point T1 = unitize(tangent);
					
					splineNormal = unitize(crossProduct(B0, T1));
					splineBinormal = unitize(crossProduct(T1, splineNormal));
				}
				normalCoordinates.push_back(splineNormal);
				binormalCoordinates.push_back(splineBinormal);
				
				GLfloat p[3] = { x - (gapScale * splineNormal.x), y - (gapScale * splineNormal.y), z - (gapScale * splineNormal.z) };
				GLfloat p1[3] = { x + (gapScale * splineNormal.x), y + (gapScale * splineNormal.y), z + (gapScale * splineNormal.z) };
				splineCoordinates.insert(splineCoordinates.end(), p, p + 3);
				newsplineCoordinates.insert(newsplineCoordinates.end(), p1, p1 + 3);
			}
		}
	}
	calculateTrack(1);
	calculateTrack(2);
	calculatePlank();
}

// calculate the ground points
void calculateGround()
{
	// calculate the triangles to render the ground using the texture
	GLfloat bl[3] = { -128, -128, -50};
	GLfloat tl[3] = { -128, 128, -50 };
	GLfloat tr[3] = { 128, 128, -50};
	GLfloat br[3] = { 128, -128, -50};

	groundCoordinates.insert(groundCoordinates.end(), tl, tl + 3);
	groundCoordinates.insert(groundCoordinates.end(), tr, tr + 3);
	groundCoordinates.insert(groundCoordinates.end(), bl, bl + 3);
	groundCoordinates.insert(groundCoordinates.end(), bl, bl + 3);
	groundCoordinates.insert(groundCoordinates.end(), tr, tr + 3);
	groundCoordinates.insert(groundCoordinates.end(), br, br + 3);

	GLfloat bl_uv[3] = { 0, 0 };
	GLfloat tl_uv[3] = { 0, 1 };
	GLfloat tr_uv[3] = { 1, 1 };
	GLfloat br_uv[3] = { 1, 0 };

	groundUVs.insert(groundUVs.end(), tl_uv, tl_uv + 2);
	groundUVs.insert(groundUVs.end(), tr_uv, tr_uv + 2);
	groundUVs.insert(groundUVs.end(), bl_uv, bl_uv + 2);
	groundUVs.insert(groundUVs.end(), bl_uv, bl_uv + 2);
	groundUVs.insert(groundUVs.end(), tr_uv, tr_uv + 2);
	groundUVs.insert(groundUVs.end(), br_uv, br_uv + 2);
}

// calculate the sky points
void calculateSky() 
{
	// calculate the triangles to render the sky using texture
	{
		GLfloat bl[3] = { -128, -128, -128 };
		GLfloat tl[3] = { -128, 128, -128 };
		GLfloat tr[3] = { -128, 128, 128 };
		GLfloat br[3] = { -128, -128, 128 };

		skyCoordinates.insert(skyCoordinates.end(), tl, tl + 3);
		skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
		skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
		skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
		skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
		skyCoordinates.insert(skyCoordinates.end(), br, br + 3);
	}

	{
	GLfloat bl[3] = { -128, -128, 128 };
	GLfloat tl[3] = { -128, 128, 128 };
	GLfloat tr[3] = { 128, 128, 128 };
	GLfloat br[3] = { 128, -128, 128 };

	skyCoordinates.insert(skyCoordinates.end(), tl, tl + 3);
	skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
	skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
	skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
	skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
	skyCoordinates.insert(skyCoordinates.end(), br, br + 3);
}

	{
		GLfloat bl[3] = { 128, -128, 128 };
		GLfloat tl[3] = { 128, 128, 128 };
		GLfloat tr[3] = { 128, 128, -128 };
		GLfloat br[3] = { 128, -128, -128 };

		skyCoordinates.insert(skyCoordinates.end(), tl, tl + 3);
		skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
		skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
		skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
		skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
		skyCoordinates.insert(skyCoordinates.end(), br, br + 3);
	}

	{
		GLfloat bl[3] = { -128, -128, -128 };
		GLfloat tl[3] = { -128, 128, -128 };
		GLfloat tr[3] = { 128, 128, -128 };
		GLfloat br[3] = { 128, -128, -128 };

		skyCoordinates.insert(skyCoordinates.end(), tl, tl + 3);
		skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
		skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
		skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
		skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
		skyCoordinates.insert(skyCoordinates.end(), br, br + 3);
	}

	{
		GLfloat bl[3] = { -128, 128, -128 };
		GLfloat tl[3] = { 128, 128, -128 };
		GLfloat tr[3] = { 128, 128, 128 };
		GLfloat br[3] = { -128, 128, 128 };

		skyCoordinates.insert(skyCoordinates.end(), tl, tl + 3);
		skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
		skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
		skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
		skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
		skyCoordinates.insert(skyCoordinates.end(), br, br + 3);
	}

	{
		GLfloat bl[3] = { -128, -128, -128 };
		GLfloat tl[3] = { 128, -128, -128 };
		GLfloat tr[3] = { 128, -128, 128 };
		GLfloat br[3] = { -128, -128, 128 };

		skyCoordinates.insert(skyCoordinates.end(), tl, tl + 3);
		skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
		skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
		skyCoordinates.insert(skyCoordinates.end(), bl, bl + 3);
		skyCoordinates.insert(skyCoordinates.end(), tr, tr + 3);
		skyCoordinates.insert(skyCoordinates.end(), br, br + 3);
	}

	for (int i = 0; i < 6; i++) {
		GLfloat bl[4] = { 0, 0 };
		GLfloat tl[4] = { 0, 1 };
		GLfloat tr[4] = { 1, 1 };
		GLfloat br[4] = { 1, 0 };

		skyUVs.insert(skyUVs.end(), tl, tl + 2);
		skyUVs.insert(skyUVs.end(), tr, tr + 2);
		skyUVs.insert(skyUVs.end(), bl, bl + 2);
		skyUVs.insert(skyUVs.end(), bl, bl + 2);
		skyUVs.insert(skyUVs.end(), tr, tr + 2);
		skyUVs.insert(skyUVs.end(), br, br + 2);
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// do additional initialization here...
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_LINE_SMOOTH);

	matrix = new OpenGLMatrix();
	calculatePoints();
	calculateGround();
	calculateSky();
	initVBO();
	initVAO();
	initPipelineProgram();
	
}

int loadSplines(char * argv)
{
	char * cName = (char *)malloc(128 * sizeof(char));
	FILE * fileList;
	FILE * fileSpline;
	int iType, i = 0, j, iLength;

	// load the track file 
	fileList = fopen(argv, "r");
	if (fileList == NULL)
	{
		printf("can't open file\n");
		exit(1);
	}

	// stores the number of splines in a global variable 
	fscanf(fileList, "%d", &numSplines);

	splines = (Spline*)malloc(numSplines * sizeof(Spline));

	// reads through the spline files 
	for (j = 0; j < numSplines; j++)
	{
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL)
		{
			printf("can't open file\n");
			exit(1);
		}

		// gets length for spline file
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		// allocate memory for all the points
		splines[j].points = (Point *)malloc(iLength * sizeof(Point));
		splines[j].numControlPoints = iLength;

		// saves the data to the struct
		while (fscanf(fileSpline, "%lf %lf %lf",
			&splines[j].points[i].x,
			&splines[j].points[i].y,
			&splines[j].points[i].z) != EOF)
		{
			i++;
		}
	}

	free(cName);

	return 0;
}

int initTexture(const char * imageFilename, GLuint textureHandle)
{
	// read the texture image
	ImageIO img;
	ImageIO::fileFormatType imgFormat;
	ImageIO::errorType err = img.load(imageFilename, &imgFormat);

	if (err != ImageIO::OK)
	{
		printf("Loading texture from %s failed.\n", imageFilename);
		return -1;
	}

	// check that the number of bytes is a multiple of 4
	if (img.getWidth() * img.getBytesPerPixel() % 4)
	{
		printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
		return -1;
	}

	// allocate space for an array of pixels
	int width = img.getWidth();
	int height = img.getHeight();
	unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

	// fill the pixelsRGBA array with the image pixels
	memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
	for (int h = 0; h < height; h++)
	for (int w = 0; w < width; w++)
	{
		// assign some default byte values (for the case where img.getBytesPerPixel() < 4)
		pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
		pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
		pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
		pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

		// set the RGBA channels, based on the loaded image
		int numChannels = img.getBytesPerPixel();
		for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
			pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
	}

	// bind the texture
	glBindTexture(GL_TEXTURE_2D, textureHandle);

	// initialize the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

	// generate the mipmaps for this texture
	glGenerateMipmap(GL_TEXTURE_2D);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// query support for anisotropic texture filtering
	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	printf("Max available anisotropic samples: %f\n", fLargest);
	// set anisotropic texture filtering
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

	// query for any errors
	GLenum errCode = glGetError();
	if (errCode != 0)
	{
		printf("Texture initialization error. Error code: %d.\n", errCode);
		return -1;
	}

	// de-allocate the pixel array -- it is no longer needed
	delete[] pixelsRGBA;

	return 0;
}

// Note: You should combine this file
// with the solution of homework 1.

// Note for Windows/MS Visual Studio:
// You should set argv[1] to track.txt.
// To do this, on the "Solution Explorer",
// right click your project, choose "Properties",
// go to "Configuration Properties", click "Debug",
// then type your track file name for the "Command Arguments".
// You can also repeat this process for the "Release" configuration.

int main(int argc, char *argv[])
{
	if (argc<2)
	{
		printf("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	// load the splines from the provided filename
	loadSplines(argv[1]);

	printf("Loaded %d spline(s).\n", numSplines);
	for (int i = 0; i<numSplines; i++)
		printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);

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


