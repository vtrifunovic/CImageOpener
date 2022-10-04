//import
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define checkImageWidth 1920
#define checkImageHeight 1080

unsigned char frame[1080][1920][3] = {0};

char grayscale[] = " .+#*X=!: ";

void run_vid(int argc, char* argv)
{
	int x, y, count;
	if (argc < 2)
	{
		fprintf(stderr, "No video file given.\n");
		return;
	}
	int msize = strlen(argv);
	char beginning[] = "ffmpeg -i ";
	msize += strlen(beginning);
	char end[] = " -loglevel error -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -c:a libfdk_aac -";
	msize += strlen(end);
	char *pipedata = malloc(msize*sizeof(char));
	strcat(pipedata, beginning); 
	strcat(pipedata, argv); 
	strcat(pipedata, end); 
	FILE *pipein = popen(pipedata, "r");
	char block1[3520];
	float block2[3520];
	printf("\x1b[2J");
	int nsend = 0;
	while(1)
	{
		memset(block1,32,1760);
		memset(block2,32,1760);
		printf("\x1b[H");
		count = fread(frame, 1, 1920*1080*3, pipein);
		if (count != 1920*1080*3) break;
		for (x = 0; x < 1080; x++)
		{
			for (y=0; y<1920; y++)
			{
				if ((y+1)%10== 0 && (x+1)%20==0) //5 % 10 -> 6 pt font//regular -> 10 & 20
				{
					//int loops = 0;
					int avgr = 0, avgg = 0, avgb = 0, iterations = 0;
					int xrows = x-5;
					int yrows = y-5;
					for(; xrows <= (x+5); xrows++)
					{
						for(; yrows <= (y+5); yrows++)
						{
							avgr += (int) frame[xrows][yrows][0];
							avgg += (int) frame[xrows][yrows][1];
							avgb += (int) frame[xrows][yrows][2];
							iterations++;
						}
					}
					if (argc > 2)
					{
						avgr = (int) avgr/iterations;
						avgg = (int) avgg/iterations;
						avgb = (int) avgb/iterations;
						//int charchoice = (int)((avg*9)/255);
						if (avgb > avgr && avgb > avgg)
							printf("\e[0;104m"); //Blue
						else if (avgb > avgr && avgb > avgg && avgb < 200)
							printf("\e[0;106m"); //Cyan
						else if (avgg > avgb && avgg > avgr)
							printf("\e[0;102m"); //Green
						else if (avgr > avgb && avgg > avgb && avgb < 100)
							printf("\e[0;103m"); //Yellow
						else if (avgr > avgb+25 && avgr > avgg+25)
							printf("\e[0;101m"); //Red
						else if (avgr > avgb+25 && avgr > avgg+25 && avgr < 200)
							printf("\e[0;105m"); //Magenta
						else if (avgg > 200 && avgr > 200 && avgb > 200)
							printf("\e[0;107m"); //White
						else if (avgg < 100 && avgr < 100 && avgb < 100)
							printf("\e[1;90m"); //Black
						else
							{printf("\e[0;100m");} //Gray
						printf(" ");
						printf("\033[0m");
						//printf("Iterations: %d\n", iterations);
						nsend += 1;
					}
					else
					{
						avgr = (int) (avgr+avgb+avgg)/(iterations*3);
						int charchoice = (int)((avgr*9)/255);
						printf("%c", grayscale[charchoice]);
						//printf("%d", charchoice);
						nsend += 1;
					}
				}
				if (nsend == 192) //384 -> 6 font //192 -> regular
				{
					printf("\n");
					nsend = 0;
				}
			}
		}
		printf("\n");
		usleep(15000);
	}
	fflush(pipein);
    pclose(pipein);
	free(pipedata);
}




int main(int argc, char** argv)
{
	run_vid(argc, argv[1]);
	return 0;
}
