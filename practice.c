#include <stdio.h>
#include <stdarg.h>
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
double rotate_y=0; 
double rotate_x=0;

// ----------------------------------------------------------
// display() Callback function
// ----------------------------------------------------------
void display(){

  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();

  glRotatef( rotate_x, 1.0, 0.0, 0.0 );
  glRotatef( rotate_y, 0.0, 1.0, 0.0 );

  glBegin(GL_LINES);
  glColor3f( 1.0, 1.0, 1.0 );
  glVertex3f( 0, 0.6, 0 );    
  glVertex3f(-0.6, -0.6, 0.5 );
  glEnd();
  
  glBegin(GL_LINES);
  glColor3f(   1.0,  1.0, 1.0 );
  glVertex3f(  0,  0.6, 0 );
  glVertex3f(  0.6, -0.6, 0.5 );
  glEnd();
 
  // Purple side - RIGHT
  glBegin(GL_LINES);
  glColor3f(  1.0,  1.0,  1.0 );
  glVertex3f( 0.6, -0.6, 0.5 );
  glVertex3f( -0.6,  -0.6, 0.5 );
  glEnd();
  
  glBegin(GL_LINES);
  glColor3f(  1.0,  1.0,  1.0 );
  glVertex3f( 0, 0.6, 0 );
  glVertex3f( -0.6,  -0.6, -0.5 );
  glEnd();
  
  glBegin(GL_LINES);
  glColor3f(  1.0,  1.0,  1.0 );
  glVertex3f( 0, 0.6, 0 );
  glVertex3f( 0.6,  -0.6, -0.5 );
  glEnd();

  glBegin(GL_LINES);
  glColor3f(  1.0,  1.0,  1.0 );
  glVertex3f( 0.6, -0.6, 0.5 );
  glVertex3f( 0.6,  -0.6, -0.5 );
  glEnd();
  
  glBegin(GL_LINES);
  glColor3f(  1.0,  1.0,  1.0 );
  glVertex3f( -0.6, -0.6, 0.5 );
  glVertex3f( -0.6,  -0.6, -0.5 );
  glEnd();
  
  glBegin(GL_LINES);
  glColor3f(  1.0,  1.0,  1.0 );
  glVertex3f( 0.6, -0.6, -0.5 );
  glVertex3f( -0.6,  -0.6, -0.5 );
  glEnd();


 
  glFlush();
  glutSwapBuffers();
 
}


void keyboard(unsigned char key, int x, int y ) {
 
  switch(key){
	  case 'q':
		exit(0);
		break;
	  case 'd':
		rotate_y += 5;
		printf("Updating d\n");
		break;
	  case 'a':
		rotate_y -= 5;
		printf("Updating a\n");
		break;
	  case 'w':
		rotate_x += 5;
		printf("Updating w\n");
		break;
	  case 's':
		rotate_x -= 5;
		printf("Updating s\n");
		break;
	}
	glutPostRedisplay();
 
}


int main(int argc, char* argv[]){
 
  //  Initialize GLUT and process user parameters
  glutInit(&argc,argv);
  //  Request double buffered true color window with Z-buffer
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(600, 600);
  // Create window
  glutCreateWindow("Practice");
  //  Enable Z-buffer depth test
  glEnable(GL_DEPTH_TEST);
  // Callback functions
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  //  Pass control to GLUT for events
  glutMainLoop();
 
  //  Return to OS
  return 0;
 
}
