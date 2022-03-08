//import
#include <stdio.h>
#include <unistd.h>
#include <string.h>
//OpenGL imports
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define buffer 0x8000
#define checkImageWidth 1920
#define checkImageHeight 1080
unsigned char frame[1920][1080][3] = {0};
GLubyte checkImage[1920][1080][3];

char grayscale[] = ".V#IX=!:. ";

void run_vid()
{
	int x, y, count;
	FILE *pipein = popen("ffmpeg -i ko.mp4  -loglevel error -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", "r");
	char xy[1760];
	float zy[1760];
	printf("\x1b[2J");
	int count2 = 0;
	while(1)
	{
		memset(xy,32,1760);
	        memset(zy,0,1760);
	        printf("\x1b[H");
		count = fread(frame, 1, 1920*1080*3, pipein);
		if (count != 1920*1080*3) break;
		printf("Count: %d\n", count2);
		for (x = 0; x < 1920; x++)
		{
			for (y=0; y<1080; y++)
			{
				int c = (int) (frame[x][y][0] + frame[x][y][1] + frame[x][y][2])/3;
				checkImage[x][y][0] = c; //frame[x][y][0];
				checkImage[x][y][1] = c; //frame[x][y][1];
				checkImage[x][y][2] = c; //frame[x][y][2];
				if ((x+1)%10 == 0 && (y+1)%27==0)
				{
					int avg = 0;
					int a = x;
					int b = y;
					for (a; a <= (x+10); a++)
					{
						for (b; b <= (y+10); b++)
						{
							int g = (int) (frame[a-5][b-5][0] + frame[a-5][b-5][1] + frame[a-5][b-5][2])/3;
							avg += g;
						}
					}
					avg = (int) avg/10;
					printf("%c", grayscale[(int)((avg*9)/255)]);
					count2 += 1;
				}
				if (count2 == 192)
				{
					printf("\n");
					count2 = 0;
				}
			}
		}
		printf("\n");
		glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
		glFlush();
		//usleep(50000);
	}
	fflush(pipein);
    	pclose(pipein);
}

void makeCheckImage(int width, int height)
{
   int i, j;
   
   printf("%d, %d\n", checkImageWidth, checkImageHeight);
   
   for (i = 0; i < checkImageWidth; i++) {
      for (j = 0; j < checkImageHeight; j++) {
         //c = ((((i&0x8)==0)^((j&0x8))==0))*255;
         checkImage[i][j][0] = (GLubyte) 255;
         checkImage[i][j][1] = (GLubyte) 255;
         checkImage[i][j][2] = (GLubyte) 255;
         
      }
   }
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 'q':
			exit(0);
			break;
		case 'r':
			run_vid();
			break;
	}
}

void init(int width, int height) //,uint8_t* rgb_image)
{    
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel(GL_FLAT);
   makeCheckImage(width, height);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
   glFlush();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(1920, 1080);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(argv[1]);
	init(1920, 1080); //, rgb_image);
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}
