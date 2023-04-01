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

K9_Image *rgb_to_gray(K9_Image *gray, K9_Image *image, bool read){
    if (image->channels == 1){
        fprintf(stderr,"\e[1;33mWarning!\e[0m In function rgb_to_gray(), image is already single channel.\n");
        return gray;
    }
    int totalpixels = image->width * image->height;
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "rgb_to_gray";
        uint16_t conv_id = 440;
        size_t global_item_size = image->width * image->height * image->channels;
        if (image->mem_id == NULL)
			update_input_buffer(image, global_item_size);
		if (gray->mem_id == NULL)
            update_output_buffer(gray, gray->height * gray->width * gray->channels);
        if (gray->image == NULL && read)
            gray->image = (uint8_t *) malloc(totalpixels);
        read_cl_program(prog, conv_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, conv_id);
            strcpy(global.past_func, func);
		}
        set_main_args(image->mem_id, gray->mem_id);

        if (read)
            gray->image = run_kernel(global_item_size, *gray, totalpixels);
        else 
			run_kernel_no_return(global_item_size);
    } else {
        for (int g = 0; g < totalpixels; g++){
            int avg = (image->image[g*3] + image->image[g*3+1] + image->image[g*3+2])/3;
            gray->image[g] = avg;
        }
    }
    return gray;
}

K9_Image *rgb_to_hsv(K9_Image *hsv, K9_Image *image, bool read){
    if (image->channels == 1){
        fprintf(stderr,"\e[1;33mWarning!\e[0m In function rgb_to_hsv(), image is single channel.\n");
        return hsv;
    }
    int totalpixels = image->width * image->height;
    if (global.enable_gpu == true){
        uint16_t conv_id = 440;
        char prog[] = "./conversions/conversions.cl";
        char func[] = "rgb_to_hsv";
        size_t global_item_size = image->width * image->height * image->channels;
        if (image->mem_id == NULL)
			update_input_buffer(image, global_item_size);
		if (hsv->mem_id == NULL)
            update_output_buffer(hsv, hsv->height * hsv->width * hsv->channels);
        if (hsv->image == NULL && read)
            hsv->image = (uint8_t *) malloc(global_item_size);
        read_cl_program(prog, conv_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, conv_id);
            strcpy(global.past_func, func);
		}
        set_main_args(image->mem_id, hsv->mem_id);

        if (read)
            hsv->image = run_kernel(global_item_size, *hsv, totalpixels);
        else
            run_kernel_no_return(global_item_size);
    } else {
        float r, g, b, cmax, cmin, cdiff;
        float h = -1, s = -1, v;
        for (int a = 0; a < totalpixels; a++){
            r = (float) image->image[a*3]/255.0;
            g = (float) image->image[a*3+1]/255.0;
            b = (float) image->image[a*3+2]/255.0;
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
            hsv->image[a*3+2] = h/2;
            hsv->image[a*3+1] = s;
            hsv->image[a*3] = cmax * 255;
        }
    }
    return hsv;
}

K9_Image *invert(K9_Image *ret_img, K9_Image *image, bool read){
    int totalpixels = image->width * image->height * image->channels;
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "invert";
        uint16_t conv_id = 440;
        size_t global_item_size = image->width * image->height * image->channels;
        if (image->mem_id == NULL)
			update_input_buffer(image, global_item_size);
        if (ret_img->mem_id == NULL)
            update_output_buffer(ret_img, ret_img->height * ret_img->width * ret_img->channels);
        if (ret_img->image == NULL && read)
            ret_img->image = (uint8_t *) malloc(global_item_size);
        read_cl_program(prog, conv_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, conv_id);
            strcpy(global.past_func, func);
		}
        set_main_args(image->mem_id, ret_img->mem_id);

        if (read)
            ret_img->image = run_kernel(global_item_size, *ret_img, totalpixels);
        else
            run_kernel_no_return(global_item_size);
    } else {
        for (int i = 0; i < totalpixels; i++){
            ret_img->image[i] = 255 - image->image[i];
        }
    }
    return ret_img;
}

K9_Image *resize_img(K9_Image *ret_img, K9_Image *image, vec2 scale, int type, bool read){
    ret_img->width = scale[0] * image->width;
    ret_img->height = scale[1] * image->height;
    ret_img->channels = image->channels;
    if (global.enable_gpu == true){
        // gpu bound re-sizing makes 1080x1080 jpgs buffer fail???
        char prog[] = "./conversions/conversions.cl";
        char func[] = "resize_img_billinear";
        if (type == K9_NEAREST)
            strcpy(func, "resize_img_nearest");
        uint16_t conv_id = 440;
        size_t global_item_size = image->width * image->height * image->channels;
        size_t return_item_size = ret_img->width * ret_img->height * ret_img->channels;
        if (image->mem_id == NULL)
			update_input_buffer(image, global_item_size);
        if (ret_img->mem_id == NULL)
            update_output_buffer(ret_img, return_item_size);
        if (ret_img->image == NULL && read)
            ret_img->image = (uint8_t *)malloc(return_item_size);
        read_cl_program(prog, conv_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, conv_id);
            strcpy(global.past_func, func);
		}
        double scales[] = {image->width/(double)ret_img->width, image->height/(double)ret_img->height};
        int sizes[] = {ret_img->width, ret_img->height, ret_img->channels, image->width};

        cl_mem scale_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 2 * sizeof(double), NULL, &global.gpu_values.ret);
        cl_mem sizes_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 4 * sizeof(int), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, scale_mem_obj, CL_TRUE, 0, 2 * sizeof(double), scales, 0, NULL, NULL);
        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, sizes_mem_obj, CL_TRUE, 0, 4 * sizeof(int), sizes, 0, NULL, NULL);

        set_main_args(image->mem_id, ret_img->mem_id);

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&scale_mem_obj);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_mem), (void *)&sizes_mem_obj);

        if (return_item_size > global_item_size){
            global_item_size = return_item_size;
            recalculate_local_workgroups(return_item_size);
        }

        if (read)
            ret_img->image = run_kernel(global_item_size, *ret_img, return_item_size);
        else 
            run_kernel_no_return(global_item_size);

        global.gpu_values.ret = clReleaseMemObject(scale_mem_obj);
        global.gpu_values.ret = clReleaseMemObject(sizes_mem_obj);
    } else {
        ret_img->image = (uint8_t *)realloc(ret_img->image, ret_img->width * ret_img->height * ret_img->channels);
        if (type == K9_NEAREST){
            double sizex = image->width / (double)ret_img->width;
            double sizey = image->height / (double)ret_img->height;
            for (int x = 0; x < ret_img->height; x++){
                for (int y = 0; y < ret_img->width; y++){
                    double posx = floor(y*sizex);
                    double posy = floor(x*sizey);
                    for (int z = 0; z < image->channels; z++){
                        ret_img->image[GET_PIXEL(x, y, z, ret_img->width, ret_img->channels)] = 
                        image->image[GET_PIXEL((int)posy, (int)posx, z, image->width, image->channels)];
                    }
                }
            }
        } else if (type == K9_BILINEAR){
            float x_ratio = image->width / (double)ret_img->width;
            float y_ratio = image->height / (double)ret_img->height;
            for (int x = 0; x < ret_img->height; x++){
                for (int y = 0; y < ret_img->width; y++){
                    float sizex = (x + 0.5) * (x_ratio)-0.5;
                    float sizey = (y + 0.5) * (y_ratio)-0.5;
                    int xpoi = (int)sizex;
                    int ypoi = (int)sizey;
                    float x_dist = sizex - xpoi;
                    float y_dist = sizey - ypoi;
                    for (int z = 0; z < ret_img->channels; z++){
                        uint8_t a = image->image[GET_PIXEL(xpoi, ypoi, z, image->width, image->channels)];
                        uint8_t b = image->image[GET_PIXEL((xpoi+1), ypoi, z, image->width, image->channels)];
                        uint8_t c = image->image[GET_PIXEL(xpoi, (ypoi+1), z, image->width, image->channels)];
                        uint8_t d = image->image[GET_PIXEL((xpoi+1), (ypoi+1), z, image->width, image->channels)];
                        ret_img->image[GET_PIXEL(x,y,z,ret_img->width,ret_img->channels)]  = a*(1-x_dist)*(1-y_dist) +b*(x_dist)*(1-y_dist) + c*(1-x_dist)*(y_dist) + d*(x_dist)*y_dist;
                    }
                }
            }
        }
    }
    return ret_img;
}