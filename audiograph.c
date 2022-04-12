// Import Statements
// Program can only do .wav files for now
// Stats -> 0% cpu (5950x) :: 8MB RAM :: 2MB GPU Memory (desktop)
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <pthread.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <complex.h>
#include <pulse/simple.h>
#include <pulse/error.h>
// Compiled with -> gcc audiograph.c -lGL -lglut -lm -lglut -lpthread -lpulse-simple -lpulse -lm

// ----------------------------------
// Definitions
// ----------------------------------
#define BUFSIZE 1024 // this is the buffer size of our audio
#define SMSIZE 5 // this is to smoothen our bars from looking jittery
// ---------------------------------
// Global variables for pulseaudio
// ---------------------------------
pthread_t thread1;
double PI;
typedef double complex cplx;
cplx buffer[1024];
double arr[4];

// --------------------------------
// Global variables for opengl
// --------------------------------
float sq1 = 0;
float sm1[SMSIZE];
float sq2 = 0;
float sm2[SMSIZE];
float sq3 = 0;
float sm3[SMSIZE];
float sq4 = 0;
float sm4[SMSIZE];
float sq5 = 0;
float sm5[SMSIZE];
float sq6 = 0;
float sm6[SMSIZE];
float sq7 = 0;
float sm7[SMSIZE];

// global for pthread
int counter = 0;

// 2nd step of fft
void _fft(cplx buffer[], cplx out[], int n, int step)
{
	if (step < n) {
		_fft(out, buffer, n, step * 2);
		_fft(out + step, buffer + step, n, step * 2);
 
		for (int i = 0; i < n; i += 2 * step) {
			cplx t = cexp(-I * PI * i / n) * out[i + step];
			buffer[i / 2]     = out[i] + t;
			buffer[(i + n)/2] = out[i] - t;
		}
	}
}

// main step of fft
void fft(cplx buffer[], int n)
{
	cplx out[n];
	for (int i = 0; i < n; i++) out[i] = buffer[i];
 
	_fft(buffer, out, n, 1);
}

// Old function to show the values of fft, now its used to pull one frequency value
void show(const char * s, cplx buffer[]) {
	for (int x = 0; x < 4; x++)
		arr[x] = 0;
	int count = 0;
	for (int i = 0; i < 1024; i++)
		if (!cimag(buffer[i]))
		{
			float val = creal(buffer[i])*1024/48000/2;
			if (count < 3)
				arr[count] = val;
			count += 1;
		}
}

// opengl function to display our audio graph
void display(){
	int gg;
	sq1 -= 0.01;
	float sq1avg = 0;
	if (sq1 < 0) sq1 = 0;
	for (gg = 0; gg < SMSIZE-1; gg++)
	{
		sm1[gg] = sm1[gg+1];
		sq1avg += sm1[gg];
	}
	sm1[SMSIZE-1] = sq1;
	sq1avg += sq1;
	sq1avg = sq1avg/SMSIZE;
	sq2 -= 0.02;
	if (sq2 < 0) sq2 = 0;
	float sq2avg = 0;
	for (gg = 0; gg < SMSIZE-1; gg++)
	{
		sm2[gg] = sm2[gg+1];
		sq2avg += sm2[gg];
	}
	sm2[SMSIZE-1] = sq2;
	sq2avg += sq2;
	sq2avg = sq2avg/SMSIZE;
	sq3 -= 0.01;
	if (sq3 < 0) sq3 = 0;
	float sq3avg = 0;
	for (gg = 0; gg < SMSIZE-1; gg++)
	{
		sm3[gg] = sm3[gg+1];
		sq3avg += sm3[gg];
	}
	sm3[SMSIZE-1] = sq3;
	sq3avg += sq3;
	sq3avg = sq3avg/SMSIZE;
	sq4 -= 0.01;
	if (sq4 < 0) sq4 = 0;
	float sq4avg = 0;
	for (gg = 0; gg < SMSIZE-1; gg++)
	{
		sm4[gg] = sm4[gg+1];
		sq4avg += sm4[gg];
	}
	sm4[SMSIZE-1] = sq4;
	sq4avg += sq4;
	sq4avg = sq4avg/SMSIZE;
	sq5 -= 0.01;
	if (sq5 < 0) sq5 = 0;
	float sq5avg = 0;
	for (gg = 0; gg < SMSIZE-1; gg++)
	{
		sm5[gg] = sm5[gg+1];
		sq5avg += sm5[gg];
	}
	sm5[SMSIZE-1] = sq5;
	sq5avg += sq5;
	sq5avg = sq5avg/SMSIZE;
	sq6 -= 0.01;
	if (sq6 < 0) sq6 = 0;
	float sq6avg = 0;
	for (gg = 0; gg < SMSIZE-1; gg++)
	{
		sm6[gg] = sm6[gg+1];
		sq6avg += sm6[gg];
	}
	sm6[SMSIZE-1] = sq6;
	sq6avg += sq6;
	sq6avg = sq6avg/SMSIZE;
	sq7 -= 0.01;
	if (sq7 < 0) sq7 = 0;
	float sq7avg = 0;
	for (gg = 0; gg < SMSIZE-1; gg++)
	{
		sm7[gg] = sm7[gg+1];
		sq7avg += sm7[gg];
	}
	sm7[SMSIZE-1] = sq7;
	sq7avg += sq7;
	sq7avg = sq7avg/SMSIZE;
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glColor3f(sq1avg,1-sq1avg,1-0.5*sq1avg);
	glRectf(-0.9,-0.9,-0.7, -0.8+sq1avg); //sq1
	glColor3f(sq2avg,1-sq2avg,1-0.5*sq2avg);
	glRectf(-0.65,-0.9,-0.45, -0.8+sq2avg); //sq2
	glColor3f(sq3avg,1-sq3avg,1-0.5*sq3avg);
	glRectf(-0.4,-0.9,-0.2, -0.8+sq3avg); //sq3
	glColor3f(sq4avg,1-sq4avg,1-0.5*sq4avg);
	glRectf(-0.15,-0.9,0.05, -0.8+sq4avg); //sq4
	glColor3f(sq5avg,1-sq5avg,1-0.5*sq5avg);
	glRectf(0.1,-0.9, 0.3, -0.8+sq5avg); //sq5
	glColor3f(sq6avg,1-sq6avg,1-0.5*sq6avg);
	glRectf(0.35,-0.9,0.55, -0.8+sq6avg); //sq6
	glColor3f(sq7avg,1-sq7avg,1-0.5*sq7avg);
	glRectf(0.6,-0.9,0.8, -0.8+sq7avg); //sq7
	glFlush();
	glutSwapBuffers();
}
  

// bad name, but this is the audio driver
void* random_print()
{
 
    /* The Sample format to use */
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 48000, // this will have to change depending on the song sample rate
        .channels = 2 // this might have to change for mp3 files
    };
 
    pa_simple *s = NULL;
    int error;
 
    // this is where we open our audio file
    if (2 > 1) {
        int fd;
 
        if ((fd = open("RosesRemix.wav", O_RDONLY)) < 0) {
            fprintf(stderr, __FILE__": open() failed: %s\n", strerror(errno));
            goto finish;
        }
 
        if (dup2(fd, STDIN_FILENO) < 0) {
            fprintf(stderr, __FILE__": dup2() failed: %s\n", strerror(errno));
            goto finish;
        }
 
        close(fd);
    }
 
    /* Create a new playback stream */
    if (!(s = pa_simple_new(NULL, "a.out", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        goto finish;
    }
    
    PI = atan2(1, 1) * 4;
    for (;;) {
        uint8_t buf[BUFSIZE];
        ssize_t r;
 
        /* Read some data ... */
        if ((r = read(STDIN_FILENO, buf, sizeof(buf))) <= 0) {
        	printf("%ld", r); //r is a ssize_t (long int)
            if (r == 0) /* EOF */
                break;
 
            fprintf(stderr, __FILE__": read() failed: %s\n", strerror(errno));
            goto finish;
        }
        int val = 0;
        //cmplx buffer[1024];
        for (int xy = 0; xy < BUFSIZE; xy++)
        {
        	val += buf[xy];
        	buffer[xy] = buf[xy];
        }
        fft(buffer, BUFSIZE);
        show("Freq : \n", buffer);
        int free2;
        free2 = arr[1];
        
        //1 :: Split frequencies into proper bar
        if (free2 < 50 && free2 > 0)
        	sq7 += 0.06;
        if (sq7 > 0.7) sq7 = 0.7;
        //2
        if (free2 > 50 && free2 < 75)
        	sq6 += 0.04;
        if (sq6 > 1.7) sq6 = 1.7;
        //3
        if (free2 > 75 && free2 < 125)
         	sq5 += 0.035;
        if (sq5 > 1.7) sq5 = 1.7;
        //4
        if (free2 > 158 && free2 < 200)
         	sq4 += 0.045;
        if (sq4 > 1.7) sq4 = 1.7;
        //5
        if (free2 > 200 && free2 < 300)
         	sq3 += 0.045;
        if (sq3 > 1.7) sq3 = 1.7;
        // 6
        if (free2 > 300 && free2 < 500)
         	sq2 += 0.09;
        if (sq2 > 0.7) sq2 = 0.7;
        // 7
        if (free2 > 500)
        	sq1 += 0.09;
        if (sq1 > 1.7) sq1 = 1.7;;
        /* ... and play it */
        if (pa_simple_write(s, buf, (size_t) r, &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
            //printf("%ld", sizeof(buf));
            goto finish;
        } 
    }
 
    /* Make sure that every single sample was played */
    if (pa_simple_drain(s, &error) < 0) {
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
        goto finish;
    }
 
finish:
 
    if (s)
        pa_simple_free(s);
    printf("Done.\n");
    counter = 3;
    return NULL;
}

// opengl runs into this and this is where we create our thread to play music
void run_all(void)
{
	display();
	if (counter == 0)
	{
		int error;

		error = pthread_create(&thread1, NULL, &random_print, NULL);
		if (error != 0)
			{printf("Thread can't be created\n");}
	}
	else if (counter == 3)
    	{pthread_join(thread1, NULL);
    	printf("Thread joined\n");
    	exit(0);}
    counter = 1;

}


// Main function
int main(int argc, char** argv){
	srand(time(NULL));
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800,800);
	glutCreateWindow("Music");
	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(display);
	glutIdleFunc(run_all);
	glutMainLoop();
	return 0;
}
