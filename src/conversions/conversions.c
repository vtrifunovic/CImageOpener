#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "conversions.h"
#include "../render/render.h"
#include "../linmath.h"
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

K9_Image rgb_to_gray(K9_Image image){
    if (image.channels == 1){
        return image;
    }
    int totalpixels = image.width * image.height;
    K9_Image gray = {
        .name = (char *) malloc(strlen(image.name)+5),
        .height = image.height,
        .width = image.width,
        .channels = 1,
        .image = malloc(totalpixels),
    };
    gray.name = strcpy(gray.name, "gray_");
    gray.name = strcat(gray.name, image.name);
    for (int g = 0; g < totalpixels; g++){
        int avg = (image.image[g*3] + image.image[g*3+1] + image.image[g*3+2])/3;
        gray.image[g] = avg;
    }
    return gray;
}

K9_Image rgb_to_hsv(K9_Image image){
    if (image.channels == 1){
        return image;
    }
    int totalpixels = image.width * image.height;
    K9_Image hsv = {
        .name = (char *) malloc(strlen(image.name)+4),
        .height = image.height,
        .width = image.width,
        .channels = image.channels,
        .image = malloc(totalpixels*3),
    };
    hsv.name = strcpy(hsv.name, "hsv_");
    hsv.name = strcat(hsv.name, image.name);
    double r, g, b, cmax, cmin, cdiff;
    double h = -1, s = -1, v;
    for (int a = 0; a < totalpixels; a++){
        r = (double) image.image[a*3]/255.0;
        g = (double) image.image[a*3+1]/255.0;
        b = (double) image.image[a*3+2]/255.0;
        cmax = MAX(r, MAX(g, b));
        cmin = MIN(r, MIN(g, b));
        cdiff = cmax - cmin;
        if (cmax == cmin)
            h = 0;
        else if (cmax == r)
            h = fmod(60 * ((g - b)/cdiff) + 360, 360);
        else if (cmax == g)
            h = fmod(60 * ((b - r)/cdiff) + 120, 360);
        else if (cmax == b)
            h = fmod(60 * ((r - g)/cdiff) + 240, 360);
        
        if (cmax == 0)
            s = 0;
        else
            s = (1 - cmin/cmax)*255;
        v = cmax * 255;
        // OpenCV stores images as BGR so image is displayed as VSH
        // This stores images as RGB so image is displayed as HSV
        hsv.image[a*3] = h/2;
        hsv.image[a*3+1] = s;
        hsv.image[a*3+2] = v;
    }
    return hsv;
}