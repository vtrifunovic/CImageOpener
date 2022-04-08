#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// ----------------------------------------------------------
// Function Prototypes
// ----------------------------------------------------------
void display();

// ----------------------------------------------------------
// Global Variables
// ----------------------------------------------------------
double red = 1.0;
double green = 1.0;
double blue = 1.0;
double shiftside = 0;
double shiftup = 0;
double rotate_x = 0;
double rotate_y = 0;
double valuex;
double valuey;
// ----------------------------------------------------------
// display() Callback function
// ----------------------------------------------------------
void display(){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	glRotatef( rotate_x, 1.0, 0.0, 0.0 );
	glRotatef( rotate_y, 0.0, 1.0, 0.0 );

	/*
	GLfloat lightpos[] = {0.5, 1.0, -0.7, 0.};
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	*/
	if (red>1.0)
	{red = 0.0;}
	else if (red < 0.0)
	{red = 1.0;}
	if (green>1.0)
	{green = 0.0;}
	else if (green < 0.0)
	{green = 1.0;}
	if (blue>1.0)
	{blue = 0.0;}
	else if (blue < 0.0)
	{blue = 1.0;}
	float twicePi = 2.0 * 3.1415;
	glColor3f( red,  green, blue);
	glRectf( 0.25+shiftside, 0.2+shiftup, -0.25+shiftside, -0.2+shiftup);    
	glRectf( 0.2+shiftside, 0.25+shiftup, -0.2+shiftside, -0.25+shiftup);
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(0.2+shiftside, 0.2+shiftup); // center of circle
		for(int i = 0; i <= 20;i++) { 
			glVertex2f(
		            0.2 + (0.05 * cos(i *  twicePi / 20))+shiftside, 
			    0.2 + (0.05 * sin(i * twicePi / 20))+shiftup
			);
		}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(0.2+shiftside, -0.2+shiftup); // center of circle
		for(int i = 0; i <= 21;i++) { 
			glVertex2f(
		            0.2 + (0.05 * cos(i *  twicePi / 20))+shiftside, 
			    -0.2 + (0.05 * sin(i * twicePi / 20))+shiftup
			);
		}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(-0.2+shiftside, -0.2+shiftup); // center of circle
		for(int i = 0; i <= 20;i++) { 
			glVertex2f(
		            -0.2 + (0.05 * cos(i *  twicePi / 20))+shiftside, 
			    -0.2 + (0.05 * sin(i * twicePi / 20))+shiftup
			);
		}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(-0.2+shiftside, 0.2+shiftup); // center of circle
		for(int i = 0; i <= 20;i++) { 
			glVertex2f(
		            -0.2 + (0.05 * cos(i *  twicePi / 20))+shiftside, 
			    0.2 + (0.05 * sin(i * twicePi / 20))+shiftup
			);
		}
	glEnd();
	glFlush();
	glutSwapBuffers();
	
	shiftup += valuey;
	shiftside += valuex;
	red += 0.002;
	green -= 0.004;
	blue -= 0.003;
	if (shiftup > 0.75 || shiftup < -0.75)
	{
		valuey = -valuey;
		red += 0.2;
		green -= 0.4;
		blue -= 0.3;
	}
	if (shiftside < -0.75 || shiftside > 0.75)
	{
		valuex = -valuex;
		red += 0.2;
		green -= 0.4;
		blue -= 0.3;
	}
}

void keyboard(unsigned char key, int x, int y ) {
	switch(key){
		case 'q':
			exit(0);
			break;
	glutPostRedisplay();
	}
}


int main(int argc, char* argv[]){
	srand(time(NULL));
	valuex = rand()%25+1;
	valuex = valuex/2000;
	valuey = rand()%25+1;
	valuey = valuey/2000;
	printf("%f\n",valuex); 
	printf("%f\n",valuey); 
	printf("Reminder: get a new pen :3\n");
	//  Initialize GLUT and process user parameters
	glutInit(&argc,argv);
	//  Request double buffered true color window with Z-buffer
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800,600);
	// Create window
	glutCreateWindow("DVDMenu");
	//  Enable Z-buffer depth test
	glEnable(GL_DEPTH_TEST);
	// Callback functions
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	// Pass control to GLUT for events
	glutIdleFunc(display);
	glutMainLoop();
	//  Return to OS
	return 0;
 
}
