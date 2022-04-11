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
double shiftleft = 0;
double shiftright = 0;
double hmove = 0;
double vmove = 0;
double hinc = -0.009;
double vinc = 0.0;
double pastmoves[60];
int scorel = 0;
int scorer = 0;

void render_score(float xpos, float ypos, int score)
{
	char *c;
	char *string = (char *)malloc(sizeof(score)*sizeof(int));
	sprintf(string, "%d", score);
	glColor3f(0.25,  0.75, 1);
	glRasterPos3f(xpos, ypos, 0.3);
	for (c = string; *c != '\0'; c++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
	}
}

void check_move()
{
	if (hmove > 1)
	{
		scorel += 1;
		hmove=0;
		hinc = 0.005;
		vinc = 0.0;
		vmove=0;
		shiftleft = 0;
		shiftright = 0;
	}
	else if(hmove < -1)
	{
		scorer += 1;
		hmove=0;
		vmove=0;
		hinc = -0.005;
		vinc = 0.0;
		shiftleft = 0;
		shiftright = 0;
	}
	if (fabs(vmove) >= 1)
		vinc = -vinc;
	// hitboxes of right paddle
	if (hmove+0.05 > 0.85 && hmove+0.05 < 0.9 && vmove-0.05 < 0.2+shiftright && vmove+0.05 > -0.2+shiftright)
	{
		hinc = -(hinc+0.001);
		vinc = (vmove-shiftright)/25;
	}
	// hitboxes of left paddle
	if (hmove-0.05 < -0.85 && hmove > -0.9 && vmove-0.05 < 0.2+shiftleft && vmove+0.05 > -0.2+shiftleft)
	{
		hinc = -(hinc-0.001);
		vinc = (vmove-shiftleft)/25;
	}
	double avg = 0;
	for (int x = 0; x < 59; x++)
	{
		pastmoves[x] = pastmoves[x+1];
		avg += pastmoves[x];
	}
	pastmoves[59] = vmove;
	avg += vmove;
	avg = avg/60;
	shiftright = (avg);//+(shiftleft*0.05);
	render_score(-0.2, 0.7, scorel);
	render_score(0.2, 0.7, scorer);
	// 0.035 MAX SPEED
	if (fabs(hinc) > 0.04)
	{
		hinc = (fabs(hinc)/hinc)*0.04;
	}		
}

// ----------------------------------------------------------
// display() Callback function
// ----------------------------------------------------------
void display(){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//glLoadIdentity();
	
	// BEGIN: BALL
	float twicePi = 2.0 * 3.1415;
	// fabs for float abs value
	glColor3f(fabs(hmove),  0.75, fabs(vmove));
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(hmove, vmove); // center of circle
		for(int i = 0; i <= 21;i++) { 
			glVertex2f(
		            hmove+(0.05 * cos(i *  twicePi / 20)), 
			    vmove+(0.05 * sin(i * twicePi / 20))
			);
		}
	glEnd();
	// BEGIN: LEFT PADDLE
	glColor3f(1,1,1);
	glRectf(-0.9,0.2+shiftleft,-0.85,-0.2+shiftleft);
	// BEGIN: RIGHT PADDLE
	glColor3f(1,1,1);
	glRectf(0.85,0.2+shiftright,0.9,-0.2+shiftright);
	// END
	hmove += hinc;
	vmove += vinc;
	check_move();
	glFlush();
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y ) {
	switch(key){
		case 'q':
			exit(0);
			break;
		// MOVEMENT FOR LEFT PADDLE
		case 'w':
			shiftleft += 0.03;
			break;
		case 's':
			shiftleft -= 0.03;
			break;
		case 'r':
			scorel = 0;
			scorer = 0;
			hmove=0;
			hinc = 0.005;
			vinc = 0.0;
			vmove=0;
			shiftleft = 0;
			break;
	glutPostRedisplay();
	}
}


int main(int argc, char* argv[]){
	srand(time(NULL));
	//  Initialize GLUT and process user parameters
	glutInit(&argc,argv);
	//  Request double buffered true color window with Z-buffer
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800,800);
	// Create window
	glutCreateWindow("Pong");
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
