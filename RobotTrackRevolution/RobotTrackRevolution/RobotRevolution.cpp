



#include <cstdlib>
#include <GL/glut.h>
#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <iostream>

#include "inverse.h"

using namespace std;

// TO DO: 
// What is the problem with my enums? X
// How to move ball?
// Moving arm backward
// Condition for the arm? X
// How to animate arm? X
// How to make ball not jump into the arm?
// How to ride the sphere, robot, and track? X
// How to track the sphere, robot, and track? X
// How to aim at the sphere, robot, and track? X
// Implement forward and backwards direction X
// Submenus for everything / UI X
// Take out unnecssary code X

#define PI 3.14159
#define ORBIT_SLICES 360
#define TRACK_RING 3.5
#define BALL_RADIUS 0.1
#define ROBOT_ROTATION_STEP .01

#define RSTEP     5.00
#define WHEEL_RAD 0.35

//Constants defining the step increments for changing
//the location and the aim of the camera.
#define EYE_STEP 0.1
#define CEN_STEP 0.1
#define ZOOM_FACTOR 8.0
double WITHIN_RANGE = 40;

//Enumerated type and global variable for keeping track
//of the selected operation.
typedef enum {
	POSITION, AIM, ORIENTATION, ZOOM, HOME,
	ANIMATE, NOTHING
} operationType;
operationType operation = POSITION;


//Enumerated type and global variable for keeping track
//of the object the user has selected to track with the camera
//and the object the user has selected for the camera to ride.
typedef enum { ROBOT, TRACK, SPHERE,NONE } trackingType;
trackingType tracking = NONE;
trackingType riding = NONE;

//Enumerated type and global variable for talking about axies.
typedef enum { XCAM, YCAM, ZCAM } cameraAxisType;
cameraAxisType axisCam = ZCAM;

//Enumerated type and global variable for talking about
//direction of changes in camera position, and aim
//and the direction of time steps and animation.
typedef enum { DOWNCAM, UPCAM } cameraDirectionType;
cameraDirectionType directionCamera = UPCAM;

typedef enum { SHOULDER, ELBOW, WRIST } jointType;
typedef enum { X, Y, Z } robotAxisType;
typedef enum { DOWN, UP } robotDirectionType;
typedef enum { TRAVELING, REACHING, CARRYING, RETRACTING} robotState;
robotState currentState = TRAVELING;

robotAxisType axisRobot = Z;
robotDirectionType directionRobot = UP;
jointType joint = SHOULDER;

GLfloat shoulderX = 0, elbowX = 0, wristX = 0;
GLfloat shoulderY = 270, elbowY = 0, wristY = 0;
GLfloat shoulderZ = 0, elbowZ = 0, wristZ = 0;



double wheelRotation = 0;
double robotRevolution = 0;
double ballRevolution = 180;
bool alreadyPickedUpBall = false;
int certainStepsAway = 0;

GLUquadricObj *upperArm, *foreArm, *hand;

//Global variables for keeping track of the camera position.
GLdouble xEye = 0.0;
GLdouble yEye = 0.0;
GLdouble zEye = 5.0;

//Global variables for keeping track of the camera aim.
GLdouble xCen = 0.0;
GLdouble yCen = 0.0;
GLdouble zCen = 0.0;

//Global variables for keeping track of the camera orientation.
GLdouble xUp = 0.0;
GLdouble yUp = 1.0;
GLdouble zUp = 0.0;



int windowHeight, windowWidth;



//Initialize the display window.
void init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
}


void multiplyMatrixVector(GLdouble* m, GLdouble* v, GLdouble* w, int size)
//Multiplying a vector by a matrix.
{
	int i, j;
	for (i = 0; i<size; i++)
	{
		GLdouble temp = 0.0;
		for (j = 0; j<size; j++)
			temp += (*(m + i + j*size)) * (*(v + j));
		*(w + i) = temp;
	}
}

void getCurrentLocation(GLdouble* x, GLdouble* y, GLdouble* z)
//Finding the location in world coordinates to which the current
//modelview matrix maps the origin.
{
	GLdouble modelviewMatrix[16];
	GLdouble v[4] = { 0.0,0.0,0.0,1.0 };
	GLdouble w[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
	multiplyMatrixVector(modelviewMatrix, v, w, 4);
	*x = w[0] / w[3];
	*y = w[1] / w[3];
	*z = w[2] / w[3];
}

void drawCircle(GLdouble radius)
{
	GLdouble theta, delta;
	delta = 2 * PI / ORBIT_SLICES;
	glBegin(GL_LINE_LOOP);
	for (theta = 0; theta < 2 * PI; theta += delta)
		glVertex3f(radius*cos(theta), radius*sin(theta), 0.0);
	glEnd();
}

void drawWheel(GLdouble radius) {
	GLdouble theta, delta;
	delta = 2 * PI / ORBIT_SLICES;
	glBegin(GL_LINE_LOOP);
	for (theta = 0; theta < 2 * PI; theta += delta)
		glVertex3f(radius*sin(theta), radius*cos(theta), 0.0);
	glEnd();
}

// -- DRAWING TRACK --

void drawTrack() { // Not showing up
	glColor3f(0.0, 0.0, 1.0);
	drawCircle(TRACK_RING);
}

void drawRightWheel() {
	glPushMatrix();
	glTranslatef(-.4, 0.1, -1.0);
	// Putting it into the right plane
	glRotatef(90, 0, 1, 0);
	// Moving the wheel with the robot
	glRotatef(wheelRotation, 0, 0, 1);
	glColor3f(1.0, 0.0, 0.0);
	drawWheel(WHEEL_RAD);
	glPopMatrix();
}

void drawLeftWheel() {
	glPushMatrix();
	glTranslatef(0.4, 0.1, -1.0);
	// Putting it into the right plane
	glRotatef(90, 0, 1, 0);
	// Moving the wheel with the robot
	glRotatef(wheelRotation, 0, 0, 1);
	glColor3f(1.0, 0.0, 0.0);
	drawWheel(WHEEL_RAD);
	glPopMatrix();
}

// -- DRAWING ROBOT ARM --

void gotoShoulderCoordinates()
{
	glRotatef((GLfloat)shoulderX, 1.0, 0.0, 0.0);
	glRotatef((GLfloat)shoulderY, 0.0, 1.0, 0.0);
	glRotatef((GLfloat)shoulderZ, 0.0, 0.0, 1.0);
}

void drawUpperArm()
{
	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0);
	//glRotatef((GLfloat)90, 0.0, 1.0, 0.0);
	gluCylinder(upperArm, 0.1, 0.1, 1.0, 8, 1);
	glPopMatrix();
}

void gotoElbowCoordinates()
{
	glTranslatef(0.0, 0.0, 1.0);
	glRotatef((GLfloat)elbowX, 1.0, 0.0, 0.0);
	glRotatef((GLfloat)elbowY, 0.0, 1.0, 0.0);
	glRotatef((GLfloat)elbowZ, 0.0, 0.0, 1.0);
}

void drawForeArm()
{
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	//glRotatef((GLfloat)90, 0.0, 1.0, 0.0);
	gluCylinder(foreArm, 0.1, 0.1, 1.0, 8, 1);
	glPopMatrix();
}

void gotoWristCoordinates()
{
	glTranslatef(0.0, 0.0, 1.0);
	glRotatef((GLfloat)wristX, 1.0, 0.0, 0.0);
	glRotatef((GLfloat)wristY, 0.0, 1.0, 0.0);
	glRotatef((GLfloat)wristZ, 0.0, 0.0, 1.0);
}

void drawHand()
{
	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0);
	//glRotatef((GLfloat)90, 0.0, 1.0, 0.0);
	gluCylinder(hand, 0.1, 0.1, 1.0, 8, 1);
	glPopMatrix();
}

void drawRobotArm()
{
	glPushMatrix();
	gotoShoulderCoordinates();
	drawUpperArm();
	gotoElbowCoordinates();
	drawForeArm();
	gotoWristCoordinates();
	drawHand();
	glPopMatrix();
}

void drawPlatform() {
	glColor3f(0.0, 1.0, 1.0);
	glPushMatrix();
	glTranslatef(-0.1, 0.1, 0);
	glutWireCube(0.8);
	glPopMatrix();
}

void goToRobot() {
	// rotate to where the robot is on the track
		glRotatef((GLdouble)robotRevolution, 0.0, 0.0, 1.0);
		// translate out to it
		glTranslatef(TRACK_RING, 0.0, 0.0);
}

void drawRobot() { // Causes the program to crash
	glPushMatrix();
	goToRobot();
	drawPlatform();
	drawRightWheel();
	drawLeftWheel();
	drawRobotArm();
	glPopMatrix();
}

// -- DRAWING BALL --

void goToBall() {
	// rotate to where the robot is on the track
	glRotatef((GLdouble)ballRevolution, 0.0, 0.0, 1.0);
	// translate out to it
	glTranslatef(TRACK_RING, 0.0, 0.0);
}

GLdouble TOH[16];
GLdouble TOB[16];
GLdouble THB[16];

void computeTOH() {
	glLoadIdentity();
	glPushMatrix();
	goToRobot();
	gotoShoulderCoordinates();
	gotoElbowCoordinates();
	gotoWristCoordinates();
	glGetDoublev(GL_MODELVIEW_MATRIX, TOH);
	glPopMatrix();
}

void computeTOB() {
	glLoadIdentity();
	glPushMatrix();
	goToBall();
	glGetDoublev(GL_MODELVIEW_MATRIX, TOB);
	glPopMatrix();
}

void computeTHB() {
	glLoadIdentity();
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	GLdouble THO[16];
	invertColumnMajor(TOH, THO);
	glMultMatrixd(THO);
	glMultMatrixd(TOB);
	glGetDoublev(GL_MODELVIEW_MATRIX, THB);
	glPopMatrix();
}

void updateTOB() {
	glLoadIdentity();
	glPushMatrix();
	glMultMatrixd(TOH);
	glMultMatrixd(THB);
	glGetDoublev(GL_MODELVIEW_MATRIX, TOB);
	glPopMatrix();
}

void drawBall() {
	glPushMatrix();
	//glMultMatrixd(TOB);
	if (currentState == CARRYING) {
		goToRobot();
		gotoShoulderCoordinates();
		gotoElbowCoordinates();
		gotoWristCoordinates();
		glTranslated(0, 0, 1);
		glutSolidSphere(BALL_RADIUS, 10, 8);
	}//else// if (currentState == REACHING) {
		//ballRevolution = 360 - robotRevolution 
	//}
	else {
		goToBall();
		glutSolidSphere(BALL_RADIUS, 10, 8);
		glPopMatrix();
	}
	glPopMatrix();
	
}

// -- DRAWING EVERYTHING --

void drawRobotOnTrack()
{
	drawTrack();
	drawRobot();
	drawBall();
}


void reshape(int w, int h)
//Callback for responding to reshaping of the display window.
{
	windowWidth = w;
	windowHeight = h;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / (GLdouble)h, 1.0, 200.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void display()
//Callback for redisplaying the image.
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Set the camera position, aim and orientation.
	glLoadIdentity();
	gluLookAt(xEye, yEye, zEye, xCen, yCen, zCen, xUp, yUp, zUp);

	//Set the projection type and clipping planes.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)windowWidth / (GLdouble)windowHeight,
		0.1, 200.0);
	glMatrixMode(GL_MODELVIEW);


	//Draw the picture
	drawRobotOnTrack();

	glutSwapBuffers();

	glFlush();
}

void homePosition() {
	// Setting up view
	xEye = 0.0;
	yEye = 0.0;
	zEye = 10.0;
	xCen = 0.0;
	yCen = 0.0;
	zCen = 0.0;
	xUp = 0.0;
	yUp = 1.0;
	zUp = 0.0;

	// Setting up the camera
	axisCam = ZCAM;
	directionCamera = UPCAM;
	operation = POSITION;
	tracking = NONE;
	riding = NONE;
	robotRevolution = 0;
	ballRevolution = 180;

	// Setting up Robot
	axisRobot = Z;
	directionRobot = UP;
	WITHIN_RANGE = 40;
	joint = SHOULDER;
	
	shoulderX = 0; elbowX = 0; wristX = 0;
	shoulderY = 0; elbowY = 0; wristY = 0;
	shoulderZ = 0; elbowZ = 0; wristZ = 0;

	currentState = TRAVELING;
	certainStepsAway = 0;

	computeTOB();

}

void getRobotPosition(GLdouble* x, GLdouble* y, GLdouble* z)
//Finding the current position of the earth.
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	goToRobot();
	getCurrentLocation(x, y, z);
	glPopMatrix();
}

void getTrackPosition(GLdouble* x, GLdouble* y, GLdouble* z) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	getCurrentLocation(x, y, z);
	glPopMatrix();
}

void getBallPosition(GLdouble* x, GLdouble* y, GLdouble* z) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	goToBall();
	getCurrentLocation(x, y, z);
	glPopMatrix();
}

void ride()
//Setting global variables that define the location of the camera.
{
	switch (riding)
	{
	case ROBOT: getRobotPosition(&xEye, &yEye, &zEye); break;
	case TRACK: getTrackPosition(&xEye, &yEye, &zEye); break;
	case SPHERE: getBallPosition(&xEye, &yEye, &zEye); break;
	default: break;
	}
}

void track()
//Setting global variables that define an point at which the
//camera is aimed.
{
	switch (tracking)
	{
	case ROBOT: getRobotPosition(&xCen, &yCen, &zCen); break;
	case TRACK: getTrackPosition(&xCen, &yCen, &zCen); break;
	case SPHERE: getBallPosition(&xCen, &yCen, &zCen); break;
	default: break;
	}
}

double readyToPickUp = -1000000000;

void timeStep() {
	switch (currentState) {
	case TRAVELING:
		wheelRotation++;
		if (directionRobot == UP) {
			if (abs(fmod(ballRevolution, 360.0) - fmod(robotRevolution,360.0)) <= WITHIN_RANGE && !alreadyPickedUpBall) {
				currentState = REACHING;
				certainStepsAway = 0;
			}
			else {
				if (certainStepsAway >= 2000) {
					alreadyPickedUpBall = false;
				}
				robotRevolution += ROBOT_ROTATION_STEP;
				certainStepsAway++;
			}
		}
		else {
			//cout << "In range?" << readyToPickUp << endl;
			if (((abs(fmod(ballRevolution, 360.0) + fmod(robotRevolution, 360.0)) <= WITHIN_RANGE) || readyToPickUp >= 32750 - (WITHIN_RANGE *100) + (5 * 100)) && !alreadyPickedUpBall ) {
				currentState = REACHING;
				certainStepsAway = 0;
				readyToPickUp = -robotRevolution;
			}
			else {
				if (certainStepsAway >= 20000) {
					alreadyPickedUpBall = false;
				}
					robotRevolution -= ROBOT_ROTATION_STEP;
					readyToPickUp++;
				certainStepsAway++;
			}
		}
		break;
	case REACHING:
		wheelRotation = 0;
		if (directionRobot == UP) {
			bool changeDirection = true;
			if (shoulderX >= -45) {
				shoulderX -= .01;
				changeDirection = false;
			}
			if (elbowX >= -45) {
				elbowX -= .01;
				changeDirection = false;
			}
			if (wristX >= -45) {
				wristX -= .01;
				changeDirection = false;
			}
			if (shoulderY >= -27) {
				shoulderY -= .01;
				changeDirection = false;
			}
			if (changeDirection) {
			//	computeTHB();
				currentState = CARRYING;
			}
			
		}
		else {
			bool changeDirection = true;
			if (shoulderX <= 60) {
				shoulderX += .01;
				changeDirection = false;
			}
			if (elbowX <= 60) {
				elbowX += .01;
				changeDirection = false;
			}
			if (wristX <= 60) {
				wristX += .01;
				changeDirection = false;
			}
			if (shoulderY <= -120) {
				shoulderY -= .01;
				changeDirection = false;
			}
			if (changeDirection) {
			//	computeTHB();
				currentState = CARRYING;
			}
		}
		break;
	case CARRYING:
		if (directionRobot == UP) {
			bool changeDirection = true;
			if (shoulderX <= 60) {
				shoulderX += .01;
				changeDirection = false;
			}
			if (elbowX <= 60) {
				elbowX += .01;
				changeDirection = false;
			}
			if (wristX <= 60) {
				wristX += .01;
				changeDirection = false;
			}
			if (shoulderY <= 0) {
				shoulderY += .01;
				changeDirection = false;
			}
			//computeTOH();
			//updateTOB();
			if (changeDirection) {
				ballRevolution = robotRevolution - (WITHIN_RANGE) + 15;
				currentState = RETRACTING;
			}
		}
		else {
			bool changeDirection = true;
			if (shoulderX >= -61) {
				shoulderX -= .01;
				changeDirection = false;
			}
			if (elbowX >= -61) {
				elbowX -= .01;
				changeDirection = false;
			}
			if (wristX >= -61) {
				wristX -= .01;
				changeDirection = false;
			}
			if (shoulderY <= 0) {
				shoulderY += .01;
				changeDirection = false;
			}
		//	computeTOH();
		//	updateTOB();
			if (changeDirection) {
				ballRevolution = robotRevolution + WITHIN_RANGE;
			//	cout << "Ball Rev" << ballRevolution << endl;
				currentState = RETRACTING;
			}
		}
		break;
	case RETRACTING:
		alreadyPickedUpBall = true;
		if (directionRobot == UP) {
			bool changeDirection = true;
			if (shoulderX >= 0) {
				shoulderX -= .01;
				changeDirection = false;
			}
			if (elbowX >= 0) {
				elbowX -= .01;
				changeDirection = false;
			}
			if (wristX >= 0) {
				wristX -= .01;
				changeDirection = false;
			}
			if (shoulderY >= 0) {
				shoulderY -= .01;
				changeDirection = false;
			}
			if (changeDirection) {
				currentState = TRAVELING;
			}
		}
		else {
			bool changeDirection = true;
			if (shoulderX <= 0) {
				shoulderX += .01;
				changeDirection = false;
			}
			if (elbowX <= 0) {
				elbowX += .01;
				changeDirection = false;
			}
			if (wristX <= 0) {
				wristX += .01;
				changeDirection = false;
			}
			if (shoulderY <= 0) {
				shoulderY += .01;
				changeDirection = false;
			}
			if (changeDirection) {
				currentState = TRAVELING;
			}
		}
		break;
	}

	

	track();
	ride();
	glutPostRedisplay();
}

void operate()
//Process the operation that the user has selected.
{
	if (operation == POSITION)
		switch (axisCam)
		{
		case X:
			if (directionCamera == UPCAM) xEye += EYE_STEP;
			else xEye -= EYE_STEP;
			break;
		case Y:
			if (directionCamera == UPCAM) yEye += EYE_STEP;
			else yEye -= EYE_STEP;
			break;
		case Z:
			if (directionCamera == UPCAM) zEye += EYE_STEP;
			else zEye -= EYE_STEP;
			break;
		}
	else if (operation == AIM)
		switch (axisCam)
		{
		case X:
			if (directionCamera == UPCAM) xCen += CEN_STEP;
			else xCen -= CEN_STEP;
			break;
		case Y:
			if (directionCamera == UPCAM) yCen += CEN_STEP;
			else yCen -= CEN_STEP;
			break;
		case Z:
			if (directionCamera == UPCAM) zCen += CEN_STEP;
			else zCen -= CEN_STEP;
			break;
		}
		glutPostRedisplay();
}








void rideSubMenu(int item)
// Callback for processing camera set ride submenu.
{
	switch (item)
	{
	case 1: riding = ROBOT; ride(); break;
	case 2: riding = TRACK; ride(); break;
	case 3: riding = SPHERE; ride(); break;
	}
	glutPostRedisplay();
}




void trackSubMenu(int item)
// Callback for processing camera set aim submenu.
{
	switch (item)
	{
	case 1: tracking = ROBOT; track(); break;
	case 2: tracking = TRACK; track(); break;
	case 3: tracking = SPHERE; track(); break;
	}
	glutPostRedisplay();
}

void aimSubMenu(int item)
// Callback for processing camera change aim submenu.
{
	operation = AIM;
	tracking = NONE;
	switch (item)
	{
	case 1: axisCam = XCAM; break;
	case 2: axisCam = YCAM; break;
	case 3: axisCam = ZCAM; break;
	}
}

void positionSubMenu(int item)
// Callback for processing camera position submenu.
{
	operation = POSITION;
	riding = NONE;
	switch (item)
	{
	case 1: axisCam = XCAM; break;
	case 2: axisCam = YCAM; break;
	case 3: axisCam = ZCAM; break;
	}
}


void orientationSubMenu(int item)
// Callback for processing camera orientation submenu.
{
	switch (item)
	{
	case 1: {xUp = 1.0; yUp = 0.0; zUp = 0.0; break; }
	case 2: {xUp = 0.0; yUp = 1.0; zUp = 0.0; break; }
	case 3: {xUp = 0.0; yUp = 0.0; zUp = 1.0; break; }
	}
	glutPostRedisplay();
}


void animateSubMenu(int item)
// Callback for processing animate submenu.
{
	operation = ANIMATE;
	glutIdleFunc(timeStep);
	switch (item)
	{
	case 1: directionRobot = UP; WITHIN_RANGE = 40; break;
	case 2: directionRobot = DOWN;  WITHIN_RANGE = 27; readyToPickUp = -1000000000; break;
	}
}

void axisSubMenu(int item)
// Callback for processing axis submenu.
{
	switch (item)
	{
	case 1: axisCam = XCAM; break;
	case 2: axisCam = YCAM; break;
	case 3: axisCam = ZCAM; break;
	}
}

void keyboard(unsigned char key, int, int)
//Function to support keyboard control of some operations.
{
	switch (key) {
	case 't': directionRobot = DOWN; timeStep(); break;
	case 'T': directionRobot = UP; timeStep(); break;
	case 'x':
		xEye -= EYE_STEP;
		glutPostRedisplay();
		break;
	case 'X':
		xEye += EYE_STEP;
		glutPostRedisplay();
		break;
	case 'y':
		yEye -= EYE_STEP;
		glutPostRedisplay();
		break;
	case 'Y':
		yEye += EYE_STEP;
		glutPostRedisplay();
		break;
	case 'z':
		zEye -= EYE_STEP;
		glutPostRedisplay();
		break;
	case 'Z':
		zEye += EYE_STEP;
		glutPostRedisplay();
		break;
	case 'a':
		xCen -= CEN_STEP;
		glutPostRedisplay();
		break;
	case 'A':
		xCen += CEN_STEP;
		glutPostRedisplay();
		break;
	case 'b':
		yCen -= CEN_STEP;
		glutPostRedisplay();
		break;
	case 'B':
		yCen += CEN_STEP;
		glutPostRedisplay();
		break;
	case 'c':
		zCen -= CEN_STEP;
		glutPostRedisplay();
		break;
	case 'C':
		zCen += CEN_STEP;
		glutPostRedisplay();
		break;
	case 27:
		std::exit(0);
		break;
	default:
		break;
	}
}

void mainMenu(int item)
// Callback for processing main menu.
{
	switch (item)
	{
	case 3: homePosition();glutPostRedisplay(); break;
	case 4: std::exit(0);
	}
}

void setMenus()
// Routine for creating menus.
{
	int trackSubMenuCode, rideSubMenuCode;
	int aimSubMenuCode, positionSubMenuCode, orientationSubMenuCode;
	int animateSubMenuCode;

	trackSubMenuCode = glutCreateMenu(trackSubMenu);
	glutAddMenuEntry("Robot", 1);
	glutAddMenuEntry("Track", 2);
	glutAddMenuEntry("Ball", 3);

	rideSubMenuCode = glutCreateMenu(rideSubMenu);
	glutAddMenuEntry("Robot", 1);
	glutAddMenuEntry("Track", 2);
	glutAddMenuEntry("Ball", 3);

	aimSubMenuCode = glutCreateMenu(aimSubMenu);
	glutAddMenuEntry("X Axis", 1);
	glutAddMenuEntry("Y Axis", 2);
	glutAddMenuEntry("Z Axis", 3);

	positionSubMenuCode = glutCreateMenu(positionSubMenu);
	glutAddMenuEntry("X Axis", 1);
	glutAddMenuEntry("Y Axis", 2);
	glutAddMenuEntry("Z Axis", 3);

	orientationSubMenuCode = glutCreateMenu(orientationSubMenu);
	glutAddMenuEntry("X Axis", 1);
	glutAddMenuEntry("Y Axis", 2);
	glutAddMenuEntry("Z Axis", 3);

	animateSubMenuCode = glutCreateMenu(animateSubMenu);
	glutAddMenuEntry("Forward", 1);
	glutAddMenuEntry("Backward", 2);

	glutCreateMenu(mainMenu);
	glutAddSubMenu("Set Body Tracking  ...", trackSubMenuCode);
	glutAddSubMenu("Set Body Riding  ...", rideSubMenuCode);
	glutAddSubMenu("Change Camera Aim ...", aimSubMenuCode);
	glutAddSubMenu("Change Camera Position ...", positionSubMenuCode);
	glutAddSubMenu("Change Camera Orientation ...", orientationSubMenuCode);
	glutAddSubMenu("Animate ...", animateSubMenuCode);
	glutAddMenuEntry("Home Position", 3);
	glutAddMenuEntry("Exit", 4);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}


void mouse(int button, int state, int, int)
// Routine for processing mouse events.
{
	if (operation == ANIMATE)
	{
		glutIdleFunc(NULL); operation = NOTHING; return;
	}
	if (button == GLUT_LEFT_BUTTON)
		switch (state)
		{
		case GLUT_DOWN: directionCamera = DOWNCAM; operate(); break;
		case GLUT_UP: break;
		}
	else if (button == GLUT_RIGHT_BUTTON)
		switch (state)
		{
		case GLUT_DOWN: directionCamera = UPCAM; operate(); break;
		case GLUT_UP: break;
		}
}

int main(int argc, char** argv)
{
	// Mask floating point exceptions.
	_control87(MCW_EM, MCW_EM);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Robot on a Track");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glEnable(GL_DEPTH_TEST);
	glutIdleFunc(timeStep);

	setMenus();
	homePosition();

	upperArm = gluNewQuadric();
	gluQuadricDrawStyle(upperArm, (GLenum)GLU_LINE);
	foreArm = gluNewQuadric();
	gluQuadricDrawStyle(foreArm, (GLenum)GLU_LINE);
	hand = gluNewQuadric();
	gluQuadricDrawStyle(hand, (GLenum)GLU_LINE);


	glutMainLoop();
	return 0;
}