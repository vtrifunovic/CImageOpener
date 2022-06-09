// Conway's Game of Life in C using opengl
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// how many boxes we want
#define amount 100
// int to determine when to start simulation
int run = 0;

// cells that store alive cells as 1
int cells[amount][amount];
int cellscopy[amount][amount];
// this func applies the rules when simulation is running
void n_check(int x, int y)
{
	// check N8 neighbors to find totals
	int totals = 0;
	totals += cells[x - 1][y - 1];
	totals += cells[x - 1][y];
	totals += cells[x - 1][y + 1];
	totals += cells[x][y - 1];
	totals += cells[x][y + 1];
	totals += cells[x + 1][y - 1];
	totals += cells[x + 1][y];
	totals += cells[x + 1][y + 1];
	// 1st rule:
	// Any live cell with fewer than two live neighbours dies, as if by underpopulation.
	if (totals < 2 && cells[x][y] == 1)
	{
		cellscopy[x][y] = 0;
	}
	// 2nd rule:
	// Any live cell with two or three live neighbours lives on to the next generation.
	if ((totals == 2 || totals == 3) && cells[x][y] == 1)
	{
		cellscopy[x][y] = 1;
	}
	// 3rd rule:
	// Any live cell with more than three live neighbours dies, as if by overpopulation.
	if (totals > 3 && cells[x][y] == 1)
	{
		cellscopy[x][y] = 0;
	}
	// 4th rule:
	// Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
	if (totals == 3 && cells[x][y] == 0)
	{
		cellscopy[x][y] = 1;
	}
}

// draws our boxes
void box(float xpos, float ypos, float dist)
{
	glColor3f(1, 1, 1);
	glBegin(GL_LINE_LOOP);
	glVertex3f(xpos, ypos, 0);
	glVertex3f(xpos + dist, ypos, 0);
	glVertex3f(xpos + dist, ypos + dist, 0);
	glVertex3f(xpos, ypos + dist, 0);
	glEnd();
}

void render_totals(int x, int y, float xpos, float ypos, float boxdist)
{
	char t[2];
	int totals = 0;
	totals += cells[x - 1][y - 1];
	totals += cells[x - 1][y];
	totals += cells[x - 1][y + 1];
	totals += cells[x][y - 1];
	totals += cells[x][y + 1];
	totals += cells[x + 1][y - 1];
	totals += cells[x + 1][y];
	totals += cells[x + 1][y + 1];
	sprintf(t, "%d", totals);
	glColor3f(1, 0, 0);
	glRasterPos3f(xpos + boxdist, ypos + boxdist, -0.7);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, *t);
}

// fills the cells with yellow color
void fill_cell(float xpos, float ypos, float dist)
{
	glColor3f(1, 1, 0);
	glBegin(GL_POLYGON);
	glVertex3f(xpos, ypos, 0);
	glVertex3f(xpos + dist, ypos, 0);
	glVertex3f(xpos + dist, ypos + dist, 0);
	glVertex3f(xpos, ypos + dist, 0);
	glEnd();
}

// main rendering loop
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float boxdist = 2 / (float)amount;
	for (int x = 0; x < amount; x++)
	{
		for (int y = 0; y < amount; y++)
		{
			float xpos = (float)x / ((float)amount / 2);
			float ypos = (float)y / ((float)amount / 2);
			box(xpos - 1, ypos - 1, boxdist);
			render_totals(x, y, xpos - 1, ypos - 1, boxdist / 3);
			if (cells[x][y] == 1)
			{
				fill_cell(xpos - 1, ypos - 1, boxdist);
			}
			if (run)
			{
				n_check(x, y);
			}
		}
	}
	if (run)
	{
		for (int g = 0; g < amount; g++)
		{
			for (int h = 0; h < amount; h++)
			{
				cells[g][h] = cellscopy[g][h];
			}
		}
		usleep(100000);
	}
	memset(cellscopy, 0, sizeof(cellscopy));
	glFlush();
	glutSwapBuffers();
}

void reset()
{
	memset(cells, 0, sizeof(cells));
	memset(cellscopy, 0, sizeof(cellscopy));
	run = 0;
}
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		exit(0);
		break;
	case 's':
		run = -abs(run) + 1;
		break;
	case 'r':
		reset();
		break;
		glutPostRedisplay();
	}
}
void mouse(int button, int state, int x, int y)
{
	if (run == 0)
	{
		if (state)
		{
			float xnorm = (float)x / 800 * amount;
			float ynorm = (float)y / 800 * amount;
			cells[(int)xnorm][amount - (int)ynorm - 1] = -abs(cells[(int)xnorm][amount - (int)ynorm - 1]) + 1;
		}
	}
	else
	{
		printf("Click Disabled!\n");
	}
}

int main(int argc, char *argv[])
{
	//  Initialize GLUT and process user parameters
	glutInit(&argc, argv);
	//  Request double buffered true color window with Z-buffer
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	// Create window
	glutCreateWindow("GOL");
	//  Enable Z-buffer depth test
	glEnable(GL_DEPTH_TEST);
	// Callback functions
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	// Pass control to GLUT for events
	glutIdleFunc(display);
	glutMainLoop();
	//  Return to OS
	return 0;
}
