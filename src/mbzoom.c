#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>
#include "linmath.h"
#include "global.h"
#include "typename.h"
#include "effects/effects.h"

int press = 0, held = 0;

int main(void){
    struct timeval stop, start;
    double scale = 1;
    //double cenX = -1.253250005;
    //double cenY = -0.344136949;
    double cenX = -0.02532307;
    double cenY = -0.65226512951;
    K9_Image *img  = create_img(1200, 800, 1);
    K9_Image *dimg = create_img(1200, 800, 3);
    init_gpu(img);    
    mbzoom(dimg, scale, cenX, cenY, true);
    GLFWwindow *win = init_window(*img, "test");
    gettimeofday(&start, NULL);
    while (!handle_inputs(win)){
        show_image(win, *dimg, false);
        scale *= 1.05;
        mbzoom(dimg, scale, cenX, cenY, true);
        gettimeofday(&stop, NULL);
        if (33333 > stop.tv_usec-start.tv_usec && stop.tv_usec-start.tv_usec > 0)
            usleep(stop.tv_usec - start.tv_usec);
        start = stop;
    }
}