#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// ----------------------------------------------------------
// Global Variables
// ----------------------------------------------------------
float depth = 0;
int minefield[20][20];
int minefieldtotals[20][20];
int flags[20][20];
int DFS[20][20];
int totalmines = 0;
// ----------------------------------------------------------
// Function Prototypes
// ----------------------------------------------------------
void display();


void box(float x, float y, float size)
{
	glColor3f(1,1,1);
	glBegin(GL_LINE_LOOP);
		glVertex3f(x,y, 0);
		glVertex3f(x+size,y, 0);
		glVertex3f(x+size,y+size, 0);
		glVertex3f(x,y+size, 0);
	glEnd();
}

// Draws flags
void draw_flag(int x, int y, int state)
{
	float posx = ((float)x/10)-1;
	float posy = -(((float)y/10)-1)-0.1;
	if (state == 1){
		glColor3f(1, 0, 0.75);
		glBegin(GL_TRIANGLES);
		glVertex3f(posx+0.04, posy-0.01+0.1, 0.2+depth);
		glVertex3f(posx+0.08, posy-0.025+0.1, 0.2+depth);
		glVertex3f(posx+0.04, posy-0.05+0.1, 0.2+depth);
		glEnd();
		glColor3f(1, 0, 0.75);
		glBegin(GL_POLYGON);
		glVertex3f(posx+0.036, posy-0.01+0.1, 0.2+depth);
		glVertex3f(posx+0.03999, posy-0.01+0.1, 0.2+depth);
		glVertex3f(posx+0.03999, posy-0.09+0.1, 0.2+depth);
		glVertex3f(posx+0.036, posy-0.09+0.1, 0.2+depth);
		glEnd();
	}
	else{
		glColor3f(0, 0, 0);
		glBegin(GL_POLYGON);
		glVertex3f(posx+0.036, posy-0.01+0.1, 0.2+depth);
		glColor3f(0, 0, 0);
		glVertex3f(posx+0.03999, posy-0.01+0.1, 0.2+depth);
		glColor3f(0, 0, 0);
		glVertex3f(posx+0.03999, posy-0.09+0.1, 0.2+depth);
		glColor3f(0, 0, 0);
		glVertex3f(posx+0.036, posy-0.09+0.1, 0.2+depth);
		glEnd();
		glColor3f(1,0,0);
		glBegin(GL_TRIANGLES);
		glVertex3f(posx+0.04, posy-0.01+0.1, 0.2+depth);
		glVertex3f(posx+0.08, posy-0.025+0.1, 0.2+depth);
		glVertex3f(posx+0.04, posy-0.05+0.1, 0.2+depth);
		glEnd();
	}
	glFlush();
	glutSwapBuffers();
}

// Displays # of nearby mines
void minenumber(int x, int y)
{
	float posx = ((float)x/10)-1;
	float posy = -(((float)y/10)-1)-0.1;
	if (minefield[x][y-1] == 1)
	{
		char *c = "X";
		glColor3f(1,0,0);
		glRasterPos3f(posx+0.04, posy+0.04, 0.5);
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, *c);
	}
	else{		
		char *c;
		char *string = (char *)malloc(sizeof(minefieldtotals[x][y]));
		sprintf(string, "%d", minefieldtotals[x][y]);
		if (minefieldtotals[x][y] == 0)
			glColor3f(0,0,0);
		else if (minefieldtotals[x][y] == 1)
			glColor3f(0,1,0);
		else if (minefieldtotals[x][y] == 2)
			glColor3f(0,0,1);
		else
			glColor3f(0,1,1);
		glRasterPos3f(posx+0.04, posy+0.04, -0.7);
		for (c = string; *c != '\0'; c++)
		{
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, *c);
		}
		free(string);
	}
}

void show_mine(float x, float y, float size, int fail, int posx, int posy)
{
	minenumber(posx, posy);
	glBegin(GL_POLYGON);
	if (fail == 1){
		glColor3f(1, 0, 0);
		glVertex3f(x, y, 0);
		glVertex3f(x+size, y, 0);
		glVertex3f(x+size, y+size, 0);
		glVertex3f(x, y+size, 0);
		glEnd();
		glColor3f(0, 0, 0);
		glBegin(GL_TRIANGLES);
		glVertex3f(x+0.05, y+0.01, -0.001);
		glVertex3f(x+0.087, y+0.067, -0.001);
		glVertex3f(x+0.013, y+0.067, -0.001);
		glEnd();
		glBegin(GL_TRIANGLES);
		glVertex3f(x+0.05, y+0.09, -0.001);
		glVertex3f(x+0.087, y+0.033, -0.001);
		glVertex3f(x+0.013, y+0.033, -0.001);
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
		float twicePi = 2.0 * 3.1415;
		glVertex3f(x+0.05, y+0.05, -0.01); // center of circle
		for(int i = 0; i <= 21;i++) { 
			glVertex3f(
		            (x+0.05)+(0.03 * cos(i *  twicePi / 20)), 
			    (y+0.05)+(0.03 * sin(i * twicePi / 20)), -0.01
			);
		}
		glEnd();
		return;
	}
	else
		glColor3f(0,0,0);
	glVertex3f(x, y, 0);
	glVertex3f(x+size, y, 0);
	glVertex3f(x+size, y+size, 0);
	glVertex3f(x, y+size, 0);
	glEnd();	
}

void uncovermine(float x, float y, float size, int fail)
{
	int posx = x*10+10;
	int posy = 19-(y*10+10);
	show_mine(x, y, size, fail, posx, posy);
	if (fail == 1)
		return;
	DFS[posx][posy] = 1;
	if (minefieldtotals[posx][posy] != 0)
		return;
	// REGULAR MINESWEEPER DOES AN N8 SEARCH
	// Uncomment for cool animation --> does cause bugs tho
	//glFlush();
	//glutSwapBuffers();
	//usleep(30000); // <--- this is needed to stop flashing
	for (int a = -1; a < 2; a++){
		for (int b = -1; b < 2; b++){
			if (minefieldtotals[posx][posy+b] == 0 && fabs(x) < 1 && fabs(y) < 1 && DFS[posx+a][posy+b] == 0)
				{
				DFS[posx+a][posy+b] = 1;
				uncovermine((float)(posx+a)/10-1,-((float)(posy+b)/10-1)-0.1,0.1, 0);}
			else if (minefield[posx+a][posy+b-1] != 1 && fabs(x) < 1 && fabs(y) < 1 && DFS[posx+a][posy+b] == 0)
				{DFS[posx+a][posy+b] = 1;
				show_mine((float)(posx+a)/10-1, -((float)(posy+b)/10-1)-0.1, size, fail, posx+a, posy+b);}
		}
	}
}

void covermine(float x, float y, float size)
{
	glBegin(GL_POLYGON);
	glColor3f(1,0,0.75);
	glVertex3f(x, y, 0.5);
	glVertex3f(x+size, y, 0.5);
	glVertex3f(x+size, y+size, 0.5);
	glVertex3f(x, y+size, 0.5);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(1, 0, 1);
	glVertex3f(x, y, 0.49);
	glVertex3f(x+size, y, 0.49);
	glVertex3f(x+size-0.01, y+0.01, 0.49);
	glVertex3f(x+0.01, y+0.01, 0.49);
	glVertex3f(x+0.01, y+size-0.01, 0.49);
	glVertex3f(x, y+size, 0.49);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0.75, 0, 0.75);
	glVertex3f(x+size, y+size, 0.49);
	glVertex3f(x, y+size, 0.49);
	glVertex3f(x+0.01, y+size-0.01, 0.49);
	glVertex3f(x+size-0.01, y+size-0.01, 0.49);
	glVertex3f(x+size-0.01, y+0.01, 0.49);
	glVertex3f(x+size, y, 0.49);
	glEnd();
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	srand(time(NULL));
	for (int x = 0; x < 20; x++){
		for (int y = 0; y < 20; y++){
			if (y < 19 && y > -1){
				if (rand()%7+1==1)
				{
					minefield[x][y] = 1;
					totalmines += 1;
				}
			}
		}
	}
	printf("Total mines created: %d\n", totalmines);
	for (int x = 0; x < 20; x++){
		for (int y = 0; y < 20; y++){
			float xpos = (float)x/10;
			float ypos = (float)y/10;
			covermine(xpos-1, ypos-1, 0.1);
			box(xpos-1,ypos-1,0.1);
			int tot = 0;
			for (int a = -1; a < 2; a++){
				for (int b = -2; b < 1; b++){
					// preventing out of bounds addition
					if (y+b < 19 && y+b > -1)
						if (minefield[x+a][y+b] == 1)
							tot += 1;
				}
			}
			minefieldtotals[x][y] = tot;
		}
	}
	glFlush();
	glutSwapBuffers();
}

void check_win(){
	int uncoveredmines = 0;
	for (int j = 0; j < 20; j++){
		for (int k = 0; k < 20; k++){
			uncoveredmines += DFS[j][k];
		}
	}
	printf("Totalmines :: %d ==> Minesleft: %d\n", totalmines, 400-uncoveredmines);
	if (400-uncoveredmines == totalmines)
		printf("You won!!\n");
}

// TO DO -->
// Make flags pretty
void set_flag(int x, int y)
{
	depth -= 0.0001;
	draw_flag(x,y, flags[x][y-1]);
	flags[x][y-1] = -abs(flags[x][y-1]) + 1;
}

void mine_check(int x, int y)
{
	if (flags[x][y-1] == 1)
		return;
	else if (minefield[x][y-1] != 1){
		uncovermine((float)x/10-1,-((float)y/10-1)-0.1,0.1, 0);
		check_win();
	}
	else{
		printf("Game over!\n");
		uncovermine((float)x/10-1,-((float)y/10-1)-0.1,0.1, 1);
	}
	// For Classic Animation
	glFlush();
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y ) {
	int row = 0;
	switch(key){
		case 'q':
			for (int x = 0; x < 20; x++){
				printf("Row %d: ", row);
				row++;
				for (int y = 0; y < 20; y++){
					printf("%d ", DFS[x][y]); 
				}
				printf("\n");
			} 
			exit(0);
			break;
		case 'r':
			depth = 0;
			memset(minefield, 0, sizeof(minefield));
			memset(minefieldtotals, 0, sizeof(minefieldtotals));
			memset(flags, 0, sizeof(flags));
			memset(DFS, 0, sizeof(DFS));
			totalmines = 0;
			display();
			break;
	glutPostRedisplay();
	}
}

void mouse(int button, int state, int x, int y)
{
	if (button == 0 && state == 1 && x > 0 && y > 0 && x < 800 && y < 800)
	{
		mine_check(x/40, y/40);
	}
	else if (button == 2 && state == 1 && x > 0 && y > 0 && x < 800 && y < 800)
		set_flag(x/40, y/40);
}


int main(int argc, char* argv[]){
	srand(time(NULL));
	//  Initialize GLUT and process user parameters
	glutInit(&argc,argv);
	//  Request double buffered true color window with Z-buffer
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800,800);
	// Create window
	glutCreateWindow("MineSweeper");
	//  Enable Z-buffer depth test
	glEnable(GL_DEPTH_TEST);
	// Callback functions
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	// Pass control to GLUT for events
	//glutIdleFunc(display);
	glutMainLoop();
	//  Return to OS
	return 0;
 
}
