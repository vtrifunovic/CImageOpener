#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "conversions.h"
#include "../render/render.h"
#include "../linmath.h"
#include "../typename.h"
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
        .image = (uint8_t *) malloc(totalpixels),
    };

    strcpy(gray.name, "gray_");
    strcat(gray.name, image.name);
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
        .name = (char *)malloc(strlen(image.name) + 4),
        .height = image.height,
        .width = image.width,
        .channels = image.channels,
        .image = (uint8_t *) malloc(totalpixels * 3),
    };
    strcpy(hsv.name, "hsv_");
    strcat(hsv.name, image.name);
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
        // OpenCV stores images as BGR so image is displayed as VSH
        // This stores images as RGB so image is displayed as HSV
        hsv.image[a*3] = h/2;
        hsv.image[a*3+1] = s;
        hsv.image[a*3+2] = cmax * 255;
    }
    return hsv;
}

void invert(K9_Image image){
    for (int i = 0; i < image.width * image.height * image.channels; i++){
        image.image[i] = 255 - image.image[i];
    }
}

K9_Image resize_img(K9_Image image, vec2 scale, char *type){
    if (image.channels > 3)
        image.channels = 3;
    K9_Image ret_img = {
        .channels = image.channels,
        .width = image.width*scale[0],
        .height = image.height*scale[1],
        .name = (char *) malloc(strlen(image.name)+4),
        .image = (uint8_t *) malloc(image.width * scale[0] * image.height * scale[1] * image.channels),
    };
    strcpy(ret_img.name, "re_");
    strcat(ret_img.name, image.name);
    double sizex = image.height/(double)ret_img.height;
    double sizey = image.width/(double)ret_img.width;
    // nearest neighbor interpolation :: still has issues
    if (strcmp(type, K9_NEAREST) == 0){
        for (int x = 0; x < ret_img.height; x++){
            for (int y = 0; y < ret_img.width; y++){
                double posx = floor(y*sizex);
                double posy = floor(x*sizey);
                for (int z = 0; z < image.channels; z++){
                    ret_img.image[((x*ret_img.width)+y)*image.channels+z] = 
                    image.image[((int)(posy*image.width)+(int)posx)*image.channels+z];
                }
            }
        }
    }
    // not correct :: Work in progress
    else if (strcmp(type, K9_BILLINEAR)==0){
        for (int x = 0; x < ret_img.height; x++){
            for (int y = 0; y < ret_img.width; y++){
                double dx = floor((float)((y/x + 0.5)*scale[0] - 0.5));
                double dy = floor((float)((y/x + 0.5)*scale[1] - 0.5));
                for (int z = 0; z < image.channels; z++){
                    ret_img.image[((x * ret_img.width) + y) * image.channels + z] =
                        image.image[((int)(dy * image.width) + (int)dx) * image.channels + z];
                }
            }
        }
    }
    return ret_img;
}