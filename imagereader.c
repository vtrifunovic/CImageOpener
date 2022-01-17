//OpenGL imports
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
//Regular imports
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
//STB_Image import
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define checkImageWidth 1920
#define checkImageHeight 1080
GLubyte checkImage[checkImageHeight][checkImageWidth][3];

void run_Shift()
{
	//printf("%d, %d\n", width, height);
	
	for (int x = 0; x < checkImageHeight; x++)
	{
		for (int y = 0; y < checkImageWidth; y++)
		{
			checkImage[x][y][0] += (GLubyte) 10;
			checkImage[x][y][1] += (GLubyte) 20;
			checkImage[x][y][2] += (GLubyte) 30;
		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
	glFlush();
}

void run_Split()
{
	
	for (int i = 0; i+5 < checkImageHeight-5; i++)
	{
		for (int j = 0; j+5 < checkImageWidth-5; j++)
		{
			checkImage[i][j][0] = (GLubyte) checkImage[i+5][j][0];
			checkImage[i][j][2] = (GLubyte) checkImage[i][j+5][2];
		}
	}
	
	for (int i = checkImageHeight-1; i+5 > 5; i--)
	{
		for (int j = checkImageWidth-1; j+5 > 5; j--)
		{
			checkImage[i][j][1] = (GLubyte) checkImage[i-3][j-1][1];
		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
	glFlush();
}

void dilate()
{
	for (int x = 0; x < checkImageHeight; x++)
	{
		for (int y = 0; y < checkImageWidth; y++)
		{
			checkImage[x][y][0] += (GLubyte) 3;
			if (x+1 < checkImageHeight)
			{
				checkImage[x+1][y][0] += (GLubyte) 1;
			}
			if (x-1 > 0){
				checkImage[x-1][y][0] += (GLubyte) 1;
			}
			if (y+1 < checkImageWidth)
			{
				checkImage[x][y+1][0] += (GLubyte) 1;
			}
			if (y-1 > 0)
			{
				checkImage[x][y-1][0] += (GLubyte) 1;
			}
			
			checkImage[x][y][1] += (GLubyte) 3;
			if (x+1 < checkImageHeight)
			{
				checkImage[x+1][y][1] += (GLubyte) 1;
			}
			if (x-1 > 0){
				checkImage[x-1][y][1] += (GLubyte) 1;
			}
			if (y+1 < checkImageWidth)
			{
				checkImage[x][y+1][1] += (GLubyte) 1;
			}
			if (y-1 > 0)
			{
				checkImage[x][y-1][1] += (GLubyte) 1;
			}
			
			checkImage[x][y][2] += (GLubyte) 3;
			if (x+1 < checkImageHeight)
			{
				checkImage[x+1][y][2] += (GLubyte) 1;
			}
			if (x-1 > 0){
				checkImage[x-1][y][2] += (GLubyte) 1;
			}
			if (y+1 < checkImageWidth)
			{
				checkImage[x][y+1][2] += (GLubyte) 1;
			}
			if (y-1 > 0)
			{
				checkImage[x][y-1][2] += (GLubyte) 1;
			}

		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
	glFlush();
}


void makeCheckImage(int width, int height,uint8_t* rgb_image)
{
   int i, j, c;
   
   printf("%d, %d\n", width, height);
   
   for (i = 0; i < height; i++) {
      for (j = 0; j < width; j++) {
         //c = ((((i&0x8)==0)^((j&0x8))==0))*255;
         checkImage[i][j][0] = (GLubyte) rgb_image[((width*height-width+j)-(i*width))*3];
         checkImage[i][j][1] = (GLubyte) rgb_image[((width*height-width+j)-(i*width))*3+1];
         checkImage[i][j][2] = (GLubyte) rgb_image[((width*height-width+j)-(i*width))*3+2];

      }
   }
}

void init(int width, int height,uint8_t* rgb_image)
{    
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel(GL_FLAT);
   makeCheckImage(width, height, rgb_image);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
   glFlush();
}

void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
      case 'q':
         exit(0);
         break;
      case 's':
		run_Shift();
		break;
      case 'a':
		run_Split();
		break;
      case 'd':
		dilate();
		break;
   }
}


int main(int argc, char** argv)
{   
   int width, height, bpp, row;

   uint8_t* rgb_image = stbi_load(argv[1], &width, &height, &bpp, 3);
   
   if (width > checkImageWidth || height > checkImageHeight)
   {
	   puts("Image too big, closing program");
	   return 0;
   }
   
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutInitWindowPosition(100, 100);
   glutCreateWindow(argv[1]);
   init(width, height, rgb_image);
   stbi_image_free(rgb_image);
   glutKeyboardFunc(keyboard);
   glutDisplayFunc(display);
   glutMainLoop();
   return 0; 
}
