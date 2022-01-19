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
double size_s=1.0;
double translate_x = 0;
double translate_y = 0;
double translate_z = 0;

// ----------------------------------------------------------
// display() Callback function
// ----------------------------------------------------------
void display(){

  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();

  glRotatef( rotate_x, 1.0, 0.0, 0.0 );
  glRotatef( rotate_y, 0.0, 1.0, 0.0 );
  glScalef( size_s, size_s, size_s );
  glTranslatef( translate_x, translate_y, translate_z );

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
	  case 'a':
		translate_x += 0.01;
		rotate_y -= 1;
		printf("Updating d\n");
		break;
	  case 'd':
		translate_x -= 0.01;
		rotate_y += 1;
		printf("Updating a\n");
		break;
	  case 's':
		printf("Updating w\n");
		translate_z += 0.01;
		rotate_x += 1;
		break;
	  case 'w':
		rotate_x -= 1;
		translate_z -= 0.01;
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
