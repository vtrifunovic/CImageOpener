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
        if (global_item_size != global.totalsize){
            update_gpu_channels(image, global_item_size);
            global.totalsize = global_item_size;
        }
        if (strcmp(global.past_prog, prog) != 0){
            strcpy(global.past_prog, prog);
            read_cl_program(prog);
        }
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func);
            strcpy(global.past_func, func);
        }
        set_main_args();
        gray.image = run_kernel(global_item_size, gray, 1);

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
        .image = (uint8_t *) malloc(totalpixels*3),
    };
    strcpy(hsv.name, "hsv");
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "rgb_to_hsv";
        size_t global_item_size = image.width * image.height * image.channels;
        if (global_item_size != global.totalsize){
            update_gpu_channels(image, global_item_size);
            global.totalsize = global_item_size;
        }
        if (strcmp(global.past_prog, prog) != 0){
            strcpy(global.past_prog, prog);
            read_cl_program(prog);
        }
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func);
            strcpy(global.past_func, func);
        }
        set_main_args();
        hsv.image = run_kernel(global_item_size, hsv, 1);

    } else {
        float r, g, b, cmax, cmin, cdiff;
        float h = -1, s = -1, v;
        for (int a = 0; a < totalpixels; a++){
            r = (float) image.image[a*3]/255.0;
            g = (float) image.image[a*3+1]/255.0;
            b = (float) image.image[a*3+2]/255.0;
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
            // Storing image as V,S,H
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
        .image = (uint8_t *)malloc(totalpixels),
    };
    strcpy(ret_img.name, "inverted");
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "invert";
        size_t global_item_size = image.width * image.height * image.channels;
        if (global_item_size != global.totalsize){
            update_gpu_channels(image, global_item_size);
            global.totalsize = global_item_size;
        }
        if (strcmp(global.past_prog, prog) != 0){
            strcpy(global.past_prog, prog);
            read_cl_program(prog);
        }
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func);
            strcpy(global.past_func, func);
        }
        set_main_args();
        ret_img.image = run_kernel(global_item_size, ret_img, 1);

    } else {
        for (int i = 0; i < totalpixels; i++){
            ret_img.image[i] = 255 - image.image[i];
        }
    }
    return ret_img;
}


K9_Image resize_img(K9_Image image, vec2 scale, char *type){
    if (image.channels > 3)
        image.channels = 3;
    K9_Image ret_img = {
        .channels = image.channels,
        .width = image.width*scale[0],
        .height = image.height*scale[1],
        .name = (char *) malloc(8),
        .image = (uint8_t *) malloc(image.width * scale[0] * image.height * scale[1] * image.channels),
    };
    strcpy(ret_img.name, "resized");
    double sizex = image.height/(double)ret_img.height;
    double sizey = image.width/(double)ret_img.width;
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "resize_img_nearest";
        size_t global_item_size = ret_img.width * ret_img.height * ret_img.channels;
        if (global_item_size != global.totalsize){
            update_gpu_channels(image, global_item_size);
            global.totalsize = global_item_size;
        }
        if (strcmp(global.past_prog, prog) != 0){
            strcpy(global.past_prog, prog);
            read_cl_program(prog);
        }
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func);
            strcpy(global.past_func, func);
        }
        double scales[] = {sizex, sizey};
        double sizes[] = {ret_img.width, ret_img.height, ret_img.channels};

        cl_mem scale_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 2 * sizeof(int), NULL, &global.gpu_values.ret);
        cl_mem sizes_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 3 * sizeof(int), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, scale_mem_obj, CL_TRUE, 0, 2 * sizeof(int), scales, 0, NULL, NULL);
        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, sizes_mem_obj, CL_TRUE, 0, 3 * sizeof(int), sizes, 0, NULL, NULL);

        set_main_args();

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&scale_mem_obj);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_mem), (void *)&sizes_mem_obj);

        ret_img.image = run_kernel(global_item_size, ret_img, 1);
    }
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