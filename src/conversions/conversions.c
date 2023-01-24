#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "conversions.h"
#include "../render/render.h"
#include "../linmath.h"
#include "../typename.h"
#include "../global.h"
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

K9_Image rgb_to_gray(K9_Image image){
    if (image.channels == 1){
        fprintf(stderr,"\e[1;33mWarning!\e[0m In function rgb_to_gray(), image is already single channel.\n");
        return image;
    }
    int totalpixels = image.width * image.height;
    K9_Image gray = {
        .name = (char *) malloc(5),
        .height = image.height,
        .width = image.width,
        .channels = 1,
        .image = (uint8_t *) malloc(totalpixels*3),
    };
    strcpy(gray.name, "gray");
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "rgb_to_gray";
        size_t global_item_size = image.width * image.height * image.channels;
        size_t local_item_size = 128;
        if (image.channels != global.channels){
            update_gpu_channels(image, global_item_size);
            global.channels = image.channels;
        }
        if (strcmp(global.past_prog, prog) != 0){
            strcpy(global.past_prog, prog);
            read_cl_program(prog);
        }
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func);
            strcpy(global.past_func, func);
        }
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 0, sizeof(cl_mem), (void *)&global.gpu_values.input_image);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 1, sizeof(cl_mem), (void *)&global.gpu_values.output_image);

        global.gpu_values.ret = clEnqueueNDRangeKernel(global.gpu_values.command_queue, global.gpu_values.kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
        global.gpu_values.ret = clEnqueueReadBuffer(global.gpu_values.command_queue, global.gpu_values.output_image, CL_TRUE, 0, global_item_size * sizeof(uint8_t), gray.image, 0, NULL, NULL);
    } else {
        for (int g = 0; g < totalpixels; g++){
            int avg = (image.image[g*3] + image.image[g*3+1] + image.image[g*3+2])/3;
            gray.image[g] = avg;
        }
    }
    return gray;
}

K9_Image rgb_to_hsv(K9_Image image){
    if (image.channels == 1){
        fprintf(stderr,"\e[1;33mWarning!\e[0m In function rgb_to_hsv(), image is single channel.\n");
        return image;
    }
    int totalpixels = image.width * image.height;
    K9_Image hsv = {
        .name = (char *)malloc(4),
        .height = image.height,
        .width = image.width,
        .channels = image.channels,
        .image = (uint8_t *) malloc(totalpixels * 3),
    };
    strcpy(hsv.name, "hsv");
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "rgb_to_hsv";
        size_t global_item_size = image.width * image.height * image.channels;
        size_t local_item_size = 128;
        if (image.channels != global.channels){
            update_gpu_channels(image, global_item_size);
            global.channels = image.channels;
        }
        if (strcmp(global.past_prog, prog) != 0){
            strcpy(global.past_prog, prog);
            read_cl_program(prog);
        }
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func);
            strcpy(global.past_func, func);
        }
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 0, sizeof(cl_mem), (void *)&global.gpu_values.input_image);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 1, sizeof(cl_mem), (void *)&global.gpu_values.output_image);

        global.gpu_values.ret = clEnqueueNDRangeKernel(global.gpu_values.command_queue, global.gpu_values.kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
        global.gpu_values.ret = clEnqueueReadBuffer(global.gpu_values.command_queue, global.gpu_values.output_image, CL_TRUE, 0, global_item_size * sizeof(uint8_t), hsv.image, 0, NULL, NULL);
    } else {
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
            hsv.image[a*3+2] = h/2;
            hsv.image[a*3+1] = s;
            hsv.image[a*3] = cmax * 255;
        }
    }
    return hsv;
}

K9_Image invert(K9_Image image){
    int totalpixels = image.width * image.height * image.channels;
    K9_Image ret_img = {
        .name = (char *)malloc(9),
        .height = image.height,
        .width = image.width,
        .channels = image.channels,
        .image = (uint8_t *)malloc(totalpixels * 3),
    };
    strcpy(ret_img.name, "inverted");
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "invert";
        size_t global_item_size = image.width * image.height * image.channels;
        size_t local_item_size = 128;
        if (image.channels != global.channels){
            update_gpu_channels(image, global_item_size);
            global.channels = image.channels;
        }
        if (strcmp(global.past_prog, prog) != 0){
            strcpy(global.past_prog, prog);
            read_cl_program(prog);
        }
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func);
            strcpy(global.past_func, func);
        }
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 0, sizeof(cl_mem), (void *)&global.gpu_values.input_image);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 1, sizeof(cl_mem), (void *)&global.gpu_values.output_image);

        global.gpu_values.ret = clEnqueueNDRangeKernel(global.gpu_values.command_queue, global.gpu_values.kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
        global.gpu_values.ret = clEnqueueReadBuffer(global.gpu_values.command_queue, global.gpu_values.output_image, CL_TRUE, 0, global_item_size * sizeof(uint8_t), ret_img.image, 0, NULL, NULL);
    } else {
        for (int i = 0; i < totalpixels; i++){
            ret_img.image[i] = 255 - image.image[i];
        }
    }
    return ret_img;
}

// Last function in conversions.c to add OpenCL function for
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