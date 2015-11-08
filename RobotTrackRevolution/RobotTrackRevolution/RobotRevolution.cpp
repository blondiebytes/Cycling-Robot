



#include <cstdlib>
#include <GL/glut.h>
#include <cstdlib>
#include <cmath>
#include <cfloat>

using namespace std;

#define PI 3.14159
#define ORBIT_SLICES 360
#define TRACK_RING 10
#define BALL_RADIUS 0.1

#define RSTEP     5.00

typedef enum { SHOULDER, ELBOW, WRIST } jointType;
typedef enum { X, Y, Z } axisType;
typedef enum { DOWN, UP } directionType;

axisType axis = Z;
directionType direction = UP;
jointType joint = SHOULDER;

GLfloat shoulderX = 0, elbowX = 0, wristX = 0;
GLfloat shoulderY = 0, elbowY = 0, wristY = 0;
GLfloat shoulderZ = 0, elbowZ = 0, wristZ = 0;

int robotRevolution = 0;

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

void drawCircle(GLdouble radius)
{
	GLdouble theta, delta;
	delta = 2 * PI / ORBIT_SLICES;
	glBegin(GL_LINE_LOOP);
	for (theta = 0; theta < 2 * PI; theta += delta)
		glVertex3f(radius*cos(theta), radius*sin(theta), 0.0);
	glEnd();
}

// -- DRAWING TRACK --

void drawTrack() { // Not showing up
	glColor3f(0.0, 0.0, 1.0);
	drawCircle(TRACK_RING);
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
	glRotatef((GLfloat)90, 0.0, 1.0, 0.0);
	gluCylinder(upperArm, 0.1, 0.1, 1.0, 8, 1);
	glPopMatrix();
}

void gotoElbowCoordinates()
{
	glTranslatef(1.0, 0.0, 0.0);
	glRotatef((GLfloat)elbowX, 1.0, 0.0, 0.0);
	glRotatef((GLfloat)elbowY, 0.0, 1.0, 0.0);
	glRotatef((GLfloat)elbowZ, 0.0, 0.0, 1.0);
}

void drawForeArm()
{
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	glRotatef((GLfloat)90, 0.0, 1.0, 0.0);
	gluCylinder(foreArm, 0.1, 0.1, 1.0, 8, 1);
	glPopMatrix();
}

void gotoWristCoordinates()
{
	glTranslatef(1.0, 0.0, 0.0);
	glRotatef((GLfloat)wristX, 1.0, 0.0, 0.0);
	glRotatef((GLfloat)wristY, 0.0, 1.0, 0.0);
	glRotatef((GLfloat)wristZ, 0.0, 0.0, 1.0);
}

void drawHand()
{
	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0);
	glRotatef((GLfloat)90, 0.0, 1.0, 0.0);
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
	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < 50; j++) {
		  //  glutSolidCube();

		}
	}
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
	drawRobotArm();
	glPopMatrix();
}

// -- DRAWING BALL --

void drawBall() {
	glutSolidSphere(BALL_RADIUS, 10, 8);
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
	zEye = 5.0;
	xCen = 0.0;
	yCen = 0.0;
	zCen = 0.0;
	xUp = 0.0;
	yUp = 1.0;
	zUp = 0.0;

	// Setting up Robot
	axis = Z;
	direction = UP;
	joint = SHOULDER;
	shoulderX = 0; elbowX = 0; wristX = 0;
	shoulderY = 0; elbowY = 0; wristY = 0;
	shoulderZ = 0; elbowZ = 0; wristZ = 0;

}

void timeStep() {
	robotRevolution = (robotRevolution + 1) % ORBIT_SLICES;
	glutPostRedisplay();
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
	//glutKeyboardFunc(keyboard);
	//glutMouseFunc(mouse);
	glEnable(GL_DEPTH_TEST);
	glutIdleFunc(timeStep);

	//setMenus();
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