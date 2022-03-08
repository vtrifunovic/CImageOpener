//import
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
//OpenGL imports
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define checkImageWidth 1920
#define checkImageHeight 1080

unsigned char frame[1080][1920][3] = {0};
GLubyte checkImage[checkImageHeight][checkImageWidth][3];
char grayscale[] = ".V#IX=!:. ";

void makeCheckImage(int width, int height)
{
   int i, j;
   
   printf("%d, %d\n", width, height);
   
   for (i = 0; i < height; i++) {
      for (j = 0; j < width; j++) {
         //c = ((((i&0x8)==0)^((j&0x8))==0))*255;
         checkImage[i][j][0] = 255;
         checkImage[i][j][1] = 255;
         checkImage[i][j][2] = 255;
         }
   }
}

void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
      case 'q':
         exit(0);
         break;
   }
}

void init(int width, int height)
{    
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel(GL_FLAT);
   makeCheckImage(1920, 1080);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void display(void)
{
  	glClear(GL_COLOR_BUFFER_BIT);
	int x, y, count;
	FILE *pipein = popen("ffmpeg -i WitchAMV.mp4 -loglevel error -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", "r");
	char block1[1760];
	float block2[1760];
	printf("\x1b[2J");
	int nsend = 0;
	while(1)
	{
		memset(block1,32,1760);
		memset(block2,0,1760);
		printf("\x1b[H");
		count = fread(frame, 1, 1920*1080*3, pipein);
		if (count != 1920*1080*3) break;
		for (x = 0; x < 1080; x++)
		{	

			for (y=0; y<1920; y++)
			{
				checkImage[x][y][0] = frame[1080-x][y][0];
				checkImage[x][y][1] = frame[1080-x][y][1];
				checkImage[x][y][2] = frame[1080-x][y][2];
				if ((y+1)%10 == 0 && (x+1)%20==0) //24 & 18
				{
					int avg = 0;
					int xrows = x-5;
					int yrows = y-5;
					for (xrows; xrows <= (x+5); xrows++)
					{
						for (yrows; yrows <= (y+5); yrows++)
						{
							int g = (int) (frame[xrows][yrows][0] + frame[xrows][yrows][1] + frame[xrows][yrows][2])/3;
							avg += g;
						}
					}
					avg = (int) avg/10;
					printf("%c", grayscale[(int)((avg*9)/255)]);
					nsend += 1;
				}
				if (nsend == 192) //120
				{
					printf("\n");
					nsend = 0;
				}
			}
		}
		printf("\n");
		usleep(30000);
		glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
  		glFlush();
	}
	fflush(pipein);
	exit(0);
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
   glutInitWindowSize(1920, 1080);
   glutInitWindowPosition(0, 0);
   glutCreateWindow(argv[1]);
   init(1920, 1080);
   glutKeyboardFunc(keyboard);
   glutDisplayFunc(display);
   glutMainLoop();
   return 0; 
}
