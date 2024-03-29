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
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define checkImageWidth 1920
#define checkImageHeight 1080
GLubyte checkImage[checkImageHeight][checkImageWidth][3];
GLubyte checkImage_copy[checkImageHeight][checkImageWidth][3];

int globalw = 0;
int globalh = 0;
char filename[100];

void run_Shift()
{
	
	for (int x = 0; x < checkImageHeight; x++)
	{
		for (int y = 0; y < checkImageWidth; y++)
		{
			if (checkImage[x][y][0] == 0)
				checkImage[x][y][0] = 255;
			else
				checkImage[x][y][0] = -checkImage[x][y][0];
			if (checkImage[x][y][1] == 0)
				checkImage[x][y][1] = 255;
			else
				checkImage[x][y][1] = -checkImage[x][y][1];
			if (checkImage[x][y][2] == 0)
				checkImage[x][y][2] = 255;
			else
				checkImage[x][y][2] = -checkImage[x][y][2];
		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
	glFlush();
}

void hxm_T()
{
	for (int x = 0; x < checkImageHeight; x++)
	{
		for (int y = 0; y < checkImageWidth; y++)
		{
			if (checkImage[x][y][0] >= 250 && checkImage[x+1][y][0] >= 250 && checkImage[x][y+1][0] >= 250)
			{
				checkImage[x][y][0] = 255;
				checkImage[x][y][1] = 255;
				checkImage[x][y][2] = 255;
			}
			else
			{
				checkImage[x][y][0] = 0;
				checkImage[x][y][1] = 0;
				checkImage[x][y][2] = 0;
			}
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

void make_grayscale()
{
	for (int x = 0; x < checkImageHeight; x++)
	{
		for (int y = 0; y < checkImageWidth; y++)
		{
			int c = (int) (checkImage[x][y][0] + checkImage[x][y][1] + checkImage[x][y][2])/3;
			checkImage[x][y][0] = c;
			checkImage[x][y][1] = c;
			checkImage[x][y][2] = c;
			checkImage_copy[x][y][0] = c;
			checkImage_copy[x][y][1] = c;
			checkImage_copy[x][y][2] = c;

		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
	glFlush();
}

void dilate()
{
	/*
	int sum = 0, pixels = 0;
	for (int x = 0; x < checkImageHeight; x++)
	{
		for (int y = 0; y < checkImageWidth; y++)
		{
			sum += checkImage[x][y][0];
			sum += checkImage[x][y][1];
			sum += checkImage[x][y][2];
			pixels += 3;
		}
	}
	printf("Sum: %d\n", sum);
	printf("Total Pixels: %d\n", pixels);
	*/
	int avg;
	//avg = (int)sum/pixels;
	int new;
	//printf("Avg. grayscale value: %d\n", avg);
	//******************
	//* clean this up  *
	//******************
	for (int x = 0; x < checkImageHeight; x++)
	{
		for (int y = 0; y < checkImageWidth; y++)
		{
			//channel 0
			avg = 0;
			avg += checkImage[x+1][y][0];
			avg += checkImage[x][y+1][0];
			avg += checkImage[x-1][y][0];
			avg += checkImage[x][y-1][0];
			avg += checkImage[x+1][y+1][0];
			avg += checkImage[x-1][y-1][0];
			avg += checkImage[x+1][y-1][0];
			avg += checkImage[x-1][y+1][0];
			//
			avg += checkImage[x+2][y][0];
			avg += checkImage[x][y+2][0];
			avg += checkImage[x-2][y][0];
			avg += checkImage[x][y-2][0];
			//
			avg += checkImage[x+2][y+1][0];
			avg += checkImage[x-2][y-1][0];
			avg += checkImage[x+2][y-1][0];
			avg += checkImage[x-2][y+1][0];
			//
			avg += checkImage[x+1][y+2][0];
			avg += checkImage[x-1][y-2][0];
			avg += checkImage[x+1][y-2][0];
			avg += checkImage[x-1][y+2][0];
			//
			avg += checkImage[x+2][y+2][0];
			avg += checkImage[x-2][y-2][0];
			avg += checkImage[x+2][y-2][0];
			avg += checkImage[x-2][y+2][0];
			//
			avg += checkImage[x+3][y][0];
			avg += checkImage[x][y+3][0];
			avg += checkImage[x-3][y][0];
			avg += checkImage[x][y-3][0];
			//
			avg += checkImage[x+3][y+1][0];
			avg += checkImage[x-3][y-1][0];
			avg += checkImage[x+3][y-1][0];
			avg += checkImage[x-3][y+1][0];
			//
			avg += checkImage[x+3][y+2][0];
			avg += checkImage[x-3][y-2][0];
			avg += checkImage[x+3][y-2][0];
			avg += checkImage[x-3][y+2][0];
			//
			avg += checkImage[x+1][y+3][0];
			avg += checkImage[x-1][y-3][0];
			avg += checkImage[x+1][y-3][0];
			avg += checkImage[x-1][y+3][0];
			//
			avg += checkImage[x+2][y+3][0];
			avg += checkImage[x-2][y-3][0];
			avg += checkImage[x+2][y-3][0];
			avg += checkImage[x-2][y+3][0];
			//
			avg += checkImage[x+3][y+3][0];
			avg += checkImage[x-3][y-3][0];
			avg += checkImage[x+3][y-3][0];
			avg += checkImage[x-3][y+3][0];
			avg = (int) avg/88;
			
			if (checkImage[x][y][0] > avg)
			{
				new = checkImage[x][y][0] + 3;
				if (new > 255)
				{
					checkImage[x][y][0] = 255;
				}
				else
				{
					checkImage[x][y][0] += 3;
				}
				if (x+1 < checkImageHeight)
				{
					new = checkImage[x+1][y][0] + 1;
					if (new > 255)
					{
						checkImage[x+1][y][0] = 255;
					}
					else
					{
						checkImage[x+1][y][0] += 1;
					}
				}
				if (x-1 > 0){
					new = checkImage[x-1][y][0] + 1;
					if (new > 255)
					{
						checkImage[x-1][y][0] = 255;
					}
					else
					{
						checkImage[x-1][y][0] += 1;
					}
				}
				if (y+1 < checkImageWidth)
				{
					new = checkImage[x][y+1][0] + 1;
					if (new > 255)
					{
						checkImage[x][y+1][0] = 255;
					}
					else
					{
						checkImage[x][y+1][0] += 1;
					}
				}
				if (y-1 > 0)
				{
					new = checkImage[x][y-1][0] + 1;
					if (new > 255)
					{
						checkImage[x][y-1][0] = 255;
					}
					else
					{
						checkImage[x][y-1][0] += 1;
					}
				}
			}
			//channel 1
			if (checkImage[x][y][1] > avg)
			{
				new = checkImage[x][y][1] + 3;
				if (new > 255)
				{
					checkImage[x][y][1] = 255;
				}
				else
				{
					checkImage[x][y][1] += 3;
				}
				if (x+1 < checkImageHeight)
				{
					new = checkImage[x+1][y][1] + 1;
					if (new > 255)
					{
						checkImage[x+1][y][1] = 255;
					}
					else
					{
						checkImage[x+1][y][1] += 1;
					}
				}
				if (x-1 > 0){
					new = checkImage[x-1][y][1] + 1;
					if (new > 255)
					{
						checkImage[x-1][y][1] = 255;
					}
					else
					{
						checkImage[x-1][y][1] += 1;
					}
				}
				if (y+1 < checkImageWidth)
				{
					new = checkImage[x][y+1][1] + 1;
					if (new > 255)
					{
						checkImage[x][y+1][1] = 255;
					}
					else
					{
						checkImage[x][y+1][1] += 1;
					}
				}
				if (y-1 > 0)
				{
					new = checkImage[x][y-1][1] + 1;
					if (new > 255)
					{
						checkImage[x][y-1][1] = 255;
					}
					else
					{
						checkImage[x][y-1][1] += 1;
					}
				}
			}
			//channel 2
			if (checkImage[x][y][2] > avg)
			{
				new = checkImage[x][y][2] + 3;
				if (new > 255)
				{
					checkImage[x][y][2] = 255;
				}
				else
				{
					checkImage[x][y][2] += 3;
				}
				if (x+1 < checkImageHeight)
				{
					new = checkImage[x+1][y][2] + 1;
					if (new > 255)
					{
						checkImage[x+1][y][2] = 255;
					}
					else
					{
						checkImage[x+1][y][2] += 1;
					}
				}
				if (x-1 > 0){
					new = checkImage[x-1][y][2] + 1;
					if (new > 255)
					{
						checkImage[x-1][y][2] = 255;
					}
					else
					{
						checkImage[x-1][y][2] += 1;
					}
				}
				if (y+1 < checkImageWidth)
				{
					new = checkImage[x][y+1][2] + 1;
					if (new > 255)
					{
						checkImage[x][y+1][2] = 255;
					}
					else
					{
						checkImage[x][y+1][2] += 1;
					}
				}
				if (y-1 > 0)
				{
					new = checkImage[x][y-1][2] + 1;
					if (new > 255)
					{
						checkImage[x][y-1][2] = 255;
					}
					else
					{
						checkImage[x][y-1][2] += 1;
					}
				}
			}
		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
	glFlush();
}

void erode()
{
	/*
	int sum = 0, pixels = 0;
	for (int x = 0; x < checkImageHeight; x++)
	{
		for (int y = 0; y < checkImageWidth; y++)
		{
			sum += checkImage[x][y][0];
			sum += checkImage[x][y][1];
			sum += checkImage[x][y][2];
			pixels += 3;
		}
	}
	printf("Sum: %d\n", sum);
	printf("Total Pixels: %d\n", pixels);
	*/
	int avg;
	//avg = (int)sum/pixels;
	//printf("Avg. grayscale value: %d\n", avg);
	int new2;
	//*********************
	//* clean this up too *
	//*********************
	for (int x = 0; x < checkImageHeight; x++)
	{
		for (int y = 0; y < checkImageWidth; y++)
		{
			avg = 0;
			avg += checkImage[x+1][y][0];
			avg += checkImage[x][y+1][0];
			avg += checkImage[x-1][y][0];
			avg += checkImage[x][y-1][0];
			avg += checkImage[x+1][y+1][0];
			avg += checkImage[x-1][y-1][0];
			avg += checkImage[x+1][y-1][0];
			avg += checkImage[x-1][y+1][0];
			//
			avg += checkImage[x+2][y][0];
			avg += checkImage[x][y+2][0];
			avg += checkImage[x-2][y][0];
			avg += checkImage[x][y-2][0];
			//
			avg += checkImage[x+2][y+1][0];
			avg += checkImage[x-2][y-1][0];
			avg += checkImage[x+2][y-1][0];
			avg += checkImage[x-2][y+1][0];
			//
			avg += checkImage[x+1][y+2][0];
			avg += checkImage[x-1][y-2][0];
			avg += checkImage[x+1][y-2][0];
			avg += checkImage[x-1][y+2][0];
			//
			avg += checkImage[x+2][y+2][0];
			avg += checkImage[x-2][y-2][0];
			avg += checkImage[x+2][y-2][0];
			avg += checkImage[x-2][y+2][0];
			//
			avg += checkImage[x+3][y][0];
			avg += checkImage[x][y+3][0];
			avg += checkImage[x-3][y][0];
			avg += checkImage[x][y-3][0];
			//
			avg += checkImage[x+3][y+1][0];
			avg += checkImage[x-3][y-1][0];
			avg += checkImage[x+3][y-1][0];
			avg += checkImage[x-3][y+1][0];
			//
			avg += checkImage[x+3][y+2][0];
			avg += checkImage[x-3][y-2][0];
			avg += checkImage[x+3][y-2][0];
			avg += checkImage[x-3][y+2][0];
			//
			avg += checkImage[x+1][y+3][0];
			avg += checkImage[x-1][y-3][0];
			avg += checkImage[x+1][y-3][0];
			avg += checkImage[x-1][y+3][0];
			//
			avg += checkImage[x+2][y+3][0];
			avg += checkImage[x-2][y-3][0];
			avg += checkImage[x+2][y-3][0];
			avg += checkImage[x-2][y+3][0];
			//
			avg += checkImage[x+3][y+3][0];
			avg += checkImage[x-3][y-3][0];
			avg += checkImage[x+3][y-3][0];
			avg += checkImage[x-3][y+3][0];
			avg = (int) avg/88;
			
			if (checkImage[x][y][0] > avg)
			{
				new2 = checkImage_copy[x][y][0] - 3;
				if (new2 < 0)
				{
					checkImage_copy[x][y][0] = 0;
				}
				else
				{
					checkImage_copy[x][y][0] -= 3;
				}
				if (x+1 < checkImageHeight)
				{
					new2 = checkImage_copy[x+1][y][0] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x+1][y][0] = 0;
					}
					else
					{
						checkImage_copy[x+1][y][0] -= 1;
					}
				}
				if (x-1 > 0){
					new2 = checkImage_copy[x-1][y][0] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x-1][y][0] = 0;
					}
					else
					{
						checkImage_copy[x-1][y][0] -= 1;
					}
				}
				if (y+1 < checkImageWidth)
				{
					new2 = checkImage_copy[x][y+1][0] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x][y+1][0] = 0;
					}
					else
					{
						checkImage_copy[x][y+1][0] -= 1;
					}
				}
				if (y-1 > 0)
				{
					new2 = checkImage_copy[x][y-1][0] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x][y-1][0] = 0;
					}
					else
					{
						checkImage_copy[x][y-1][0] -= 1;
					}
				}
			}
			//channel 1
			if (checkImage[x][y][1] > avg)
			{
					
				new2 = checkImage_copy[x][y][1] - 3;
				if (new2 < 0)
				{
					checkImage_copy[x][y][1] = 0;
				}
				else
				{
					checkImage_copy[x][y][1] -= 3;
				}
				if (x+1 < checkImageHeight)
				{
					new2 = checkImage_copy[x+1][y][1] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x+1][y][1] = 0;
					}
					else
					{
						checkImage_copy[x+1][y][1] -= 1;
					}
				}
				if (x-1 > 0){
					new2 = checkImage_copy[x-1][y][1] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x-1][y][1] = 0;
					}
					else
					{
						checkImage_copy[x-1][y][1] -= 1;
					}
				}
				if (y+1 < checkImageWidth)
				{
					new2 = checkImage_copy[x][y+1][1] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x][y+1][1] = 0;
					}
					else
					{
						checkImage_copy[x][y+1][1] -= 1;
					}
				}
				if (y-1 > 0)
				{
					new2 = checkImage_copy[x][y-1][1] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x][y-1][1] = 0;
					}
					else
					{
						checkImage_copy[x][y-1][1] -= 1;
					}
				}
			}
			//channel 2
			if (checkImage[x][y][2] > avg)
			{
					
				new2 = checkImage_copy[x][y][2] - 3;
				if (new2 < 0)
				{
					checkImage_copy[x][y][2] = 0;
				}
				else
				{
					checkImage_copy[x][y][2] -= 3;
				}
				if (x+1 < checkImageHeight)
				{
					new2 = checkImage_copy[x+1][y][2] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x+1][y][2] = 0;
					}
					else
					{
						checkImage_copy[x+1][y][2] -= 1;
					}
				}
				if (x-1 > 0){
					new2 = checkImage_copy[x-1][y][2] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x-1][y][2] = 0;
					}
					else
					{
						checkImage_copy[x-1][y][2] -= 1;
					}
				}
				if (y+1 < checkImageWidth)
				{
					new2 = checkImage_copy[x][y+1][2] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x][y+1][2] = 0;
					}
					else
					{
						checkImage_copy[x][y+1][2] -= 1;
					}
				}
				if (y-1 > 0)
				{
					new2 = checkImage_copy[x][y-1][2] - 1;
					if (new2 < 0)
					{
						checkImage_copy[x][y-1][2] = 0;
					}
					else
					{
						checkImage_copy[x][y-1][2] -= 1;
					}
				}
			}
		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage_copy);
	glFlush();
}

void edge_detect()
{
	int xyz;
	dilate();
	erode();
	for (int x = 0; x < checkImageHeight; x++)
	{
		for (int y = 0; y < checkImageWidth; y++)
		{
			xyz = checkImage[x][y][0] - checkImage_copy[x][y][0];
			checkImage[x][y][0] = xyz;
			xyz = checkImage[x][y][1] - checkImage_copy[x][y][1];
			checkImage[x][y][1] = xyz;
			xyz = checkImage[x][y][2] - checkImage_copy[x][y][2];
			checkImage[x][y][2] = xyz;
		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
	glFlush();
}

void v_blur()
{
	int avg;
	
	for (int i=1; i < checkImageHeight-1; i ++)
	{
		for (int j=1; j < checkImageWidth-1; j++)
		{
			//n4c0
			 avg = 0;
			 avg += checkImage[i+1][j][0];
			 avg += checkImage[i-1][j][0];
			 avg += checkImage[i][j+1][0];
			 avg += checkImage[i][j-1][0];
			 //n8c0
			 avg += checkImage[i-1][j-1][0];
			 avg += checkImage[i+1][j-1][0];
			 avg += checkImage[i-1][j+1][0];
			 avg += checkImage[i+1][j+1][0];
			 avg = (int)avg/8;
			 checkImage[i][j][0] = avg;
			 //n4c1
			 avg = 0;
			 avg += checkImage[i+1][j][1];
			 avg += checkImage[i-1][j][1];
			 avg += checkImage[i][j+1][1];
			 avg += checkImage[i][j-1][1];
			 //n8c1
			 avg += checkImage[i-1][j-1][1];
			 avg += checkImage[i+1][j-1][1];
			 avg += checkImage[i-1][j+1][1];
			 avg += checkImage[i+1][j+1][1];
			 avg = (int)avg/8;
			 checkImage[i][j][1] = avg;
			 //n4c2
			 avg = 0;
			 avg += checkImage[i+1][j][2];
			 avg += checkImage[i-1][j][2];
			 avg += checkImage[i][j+1][2];
			 avg += checkImage[i][j-1][2];
			 //n8c2
			 avg += checkImage[i-1][j-1][2];
			 avg += checkImage[i+1][j-1][2];
			 avg += checkImage[i-1][j+1][2];
			 avg += checkImage[i+1][j+1][2];
			 avg = (int)avg/8;
			 checkImage[i][j][2] = avg;
		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
	glFlush();
}

void mask(){
	int lgs, hgs;
	printf("Enter lower grayscale value: ");
	scanf("%3d", &lgs);
	printf("Enter higher grayscale value: ");
	scanf("%3d", &hgs);
	for (int i = 0; i < checkImageHeight; i++) {
		for (int j = 0; j < checkImageWidth; j++) {
			if (checkImage[i][j][0] >= lgs && checkImage[i][j][0] <= hgs)
			{
				checkImage[i][j][0] = 0;
				checkImage[i][j][1] = 0;
				checkImage[i][j][2] = 0;
			}
			else
			{
				checkImage[i][j][0] = 255;
				checkImage[i][j][1] = 255;
				checkImage[i][j][2] = 255;
			}
		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
	glFlush();	
}

void reset()
{
   int i, j;
   
   int width2, height2, bpp2;

   uint8_t* rgb_image = stbi_load("test.JPG", &width2, &height2, &bpp2, 3);
   
   printf("Resetting!... %d:%d\n", width2, height2);
   
   for (i = 0; i < height2; i++) {
      for (j = 0; j < width2; j++) {
         //c = ((((i&0x8)==0)^((j&0x8))==0))*255;
         checkImage[i][j][0] = (GLubyte) rgb_image[((width2*height2-width2+j)-(i*width2))*3];
         checkImage[i][j][1] = (GLubyte) rgb_image[((width2*height2-width2+j)-(i*width2))*3+1];
         checkImage[i][j][2] = (GLubyte) rgb_image[((width2*height2-width2+j)-(i*width2))*3+2];
         
         checkImage_copy[i][j][0] = (GLubyte) rgb_image[((width2*height2-width2+j)-(i*width2))*3];
         checkImage_copy[i][j][1] = (GLubyte) rgb_image[((width2*height2-width2+j)-(i*width2))*3+1];
         checkImage_copy[i][j][2] = (GLubyte) rgb_image[((width2*height2-width2+j)-(i*width2))*3+2];
      }
   }
   stbi_image_free(rgb_image);
   glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
   glFlush();	
}

void v_canny()
{
	for (int x = 0; x < checkImageHeight-2; x++)
	{
		for (int y = 0; y < checkImageWidth-2; y++)
		{	
			if (checkImage[x][y][0] - checkImage[x+1][y][0] > 10 || checkImage[x][y][0] - checkImage[x][y+1][0] > 10)
			{
				checkImage[x][y][0] = 255;
				checkImage[x][y][1] = 255;
				checkImage[x][y][2] = 255;
			}
			else
			{
				checkImage[x][y][0] = 0;
				checkImage[x][y][1] = 0;
				checkImage[x][y][2] = 0;
			}
		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
   	glFlush();					
}

void save_image()
{	
	int w = globalw, h = globalh;
	uint8_t* write_data;
	write_data = (uint8_t* ) malloc(w*h*3);
	printf("creating write_data\n");
	int maxpixel = 0;
	
	for (int i = 0; i < globalh; i++)
	{
		for (int j = 0; j < globalw; j++)
		{
			write_data[((w*h-w+j)-(i*w))*3] = (int) checkImage[i][j][0]; 
			write_data[((w*h-w+j)-(i*w))*3+1] = (int) checkImage[i][j][1]; 
			write_data[((w*h-w+j)-(i*w))*3+2] = (int) checkImage[i][j][2];
			if (maxpixel < ((w*h-w+j)-(i*w))*3)
			{
				maxpixel = ((w*h-w+j)-(i*w))*3;
			}
		}
	}
	printf("Maxpixel = %d\n", maxpixel);
	//free will crash, or create segmentation fault if data is incorrectly saved
	//                  name         w  w  rgb    data  quality
	stbi_write_jpg("test-saved.jpg", w, h, 3, write_data, 100);
	free(write_data);		
}

void find_contours()
{
	//ctest = (uint8_t* )malloc(globalw*globalh);
	for (int i = 0; i < globalh; i++)
	{
		for (int j = 0; j < globalw; j++)
		{
			if (checkImage[i][j][0] == 255)
			{
				//ctest[((w*h-w+j)-(i*w))*3] = (int) 255;
				if (checkImage[i-1][j][0] != 255)
				{
					checkImage[i][j][0] = 0;
					checkImage[i][j][1] = 255;
					checkImage[i][j][2] = 0;
				}
				else
				{
					checkImage[i][j][0] = 255;
					checkImage[i][j][1] = 255;
					checkImage[i][j][2] = 255;
				}
			}
			else
			{
				//ctest[((w*h-w+j)-(i*w))*3] = (int) 0;
				checkImage[i][j][0] = 0;
				checkImage[i][j][1] = 0;
				checkImage[i][j][2] = 0;
			}
		}
	}
	glDrawPixels(checkImageWidth, checkImageHeight, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
   	glFlush();	
	
}
void makeCheckImage(int width, int height,uint8_t* rgb_image)
{
   int i, j;
   
   printf("%d, %d\n", globalw, globalh);
   
   for (i = 0; i < height; i++) {
      for (j = 0; j < width; j++) {
         //c = ((((i&0x8)==0)^((j&0x8))==0))*255;
         checkImage[i][j][0] = (GLubyte) rgb_image[((width*height-width+j)-(i*width))*3];
         checkImage[i][j][1] = (GLubyte) rgb_image[((width*height-width+j)-(i*width))*3+1];
         checkImage[i][j][2] = (GLubyte) rgb_image[((width*height-width+j)-(i*width))*3+2];
         
         checkImage_copy[i][j][0] = (GLubyte) rgb_image[((width*height-width+j)-(i*width))*3];
         checkImage_copy[i][j][1] = (GLubyte) rgb_image[((width*height-width+j)-(i*width))*3+1];
         checkImage_copy[i][j][2] = (GLubyte) rgb_image[((width*height-width+j)-(i*width))*3+2];
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
      case 'e':
		erode();
		break;
      case 'm':
		mask();
		break;
      case 'r':
		reset();
		break;
      case 'g':
		make_grayscale();
		break;
      case 'p':
		edge_detect();
		break;
      case 'l':
      	save_image();
      	break;
      case 'v':
      	v_canny();
      	break;
      case 'b':
      	v_blur();
      	break;
      case 'f':
		find_contours();
		break;
      case 'x':
      	hxm_T();
      	break;
   }
}


int main(int argc, char** argv)
{   
   int width, height, bpp;
   //gname = (char) argv[1];
   //filename = strcpy(argv[1]);
   uint8_t* rgb_image = stbi_load(argv[1], &width, &height, &bpp, 3);
   globalw = width;
   globalh = height;
   
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
