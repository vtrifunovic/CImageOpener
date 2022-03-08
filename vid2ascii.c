//import
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define buffer 0x8000
#define checkImageWidth 1920
#define checkImageHeight 1080

unsigned char frame[1920][1080][3] = {0};

char grayscale[] = ".V#IX=!:. ";

void run_vid(char* argv)
{
	int x, y, count;
	FILE *pipein = popen("ffmpeg -i WitchAMV.mp4 -loglevel error -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", "r");
	char block1[1760];
	float block2[1760];
	printf("\x1b[2J");
	int nsend = 0;
	int linecount;
	while(1)
	{
		memset(block1,32,1760);
	        memset(block2,0,1760);
	        printf("\x1b[H");
		count = fread(frame, 1, 1920*1080*3, pipein);
		if (count != 1920*1080*3) break;
		linecount = 0;
		printf("%d: ", linecount);
		for (x = 0; x < 1920; x++)
		{
			for (y=0; y<1080; y++)
			{
				if ((x+1)%24 == 0 && (y+1)%18==0) //24 & 18
				{
					//int loops = 0;
					int avg = 0;
					int xrows = x-5;
					int yrows = y-5;
					for (xrows; xrows <= (x+5); xrows++)
					{
						for (yrows; yrows <= (y+5); yrows++)
						{
							int g = (int) (frame[xrows+5][yrows][0] + frame[xrows+5][yrows][1] + frame[xrows+5][yrows][2])/3;
							avg += g;
						}
					}
					avg = (int) avg/10;
					printf("%c", grayscale[(int)((avg*9)/255)]);
					//printf("Total loops: %d\n", loops);
					nsend += 1;
				}
				if (nsend == 120) //120
				{
					linecount++;
					printf("\n%d: ", linecount);
					//printf("\n");
					nsend = 0;
				}
			}
		}
		printf("\n");
		usleep(30000);
	}
	fflush(pipein);
    	pclose(pipein);
}




int main(int argc, char** argv)
{
	run_vid(argv[1]);
	return 0;
}
