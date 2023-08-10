#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include "conversions.h"
#include "../render/render.h"
#include "../linmath.h"
#include "../typename.h"
#include "../global.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

K9_Image *convert_channels(K9_Image *ret_img, K9_Image *image, int type, bool read){
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "rgb_to_gray";
        uint16_t conv_id = 440;
        if (type == 0xA9)
            strcpy(func, "rgb_to_hsv");
        else if (type == 0xAC)
            strcpy(func, "hsv_to_rgb");
        else if (type == 0xAB)
            strcpy(func, "invert");
        else if (type == 0xAD)
            strcpy(func, "rgb_to_yuv");
        else if (type == 0xAE)
            strcpy(func, "yuv_to_rgb");

        mem_check_gpu(image, ret_img, prog, func, conv_id, image->tp, read);
        set_main_args(image->mem_id, ret_img->mem_id);

        if (read)
            ret_img->image = run_kernel(image->tp, *ret_img, ret_img->tp);
        else
            run_kernel_no_return(image->tp);
    } else {
        if (type == 0xAA){
            for (int g = 0; g < ret_img->tp; g++){
                int avg = (image->image[g*3] + image->image[g*3+1] + image->image[g*3+2])/3;
                ret_img->image[g] = avg;
            }
        } else if (type == 0xA9){
            float r, g, b, cmax, cmin, cdiff;
            float h = -1, s = -1, v;
            for (int a = 0; a < ret_img->tp/3; a++){
                r = (float)image->image[a*3]/255.0;
                g = (float)image->image[a*3+1]/255.0;
                b = (float)image->image[a*3+2]/255.0;
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
                ret_img->image[a*3+2] = h/2;
                ret_img->image[a*3+1] = s;
                ret_img->image[a*3] = cmax * 255;
            }
        } else if (type == 0xAB){
            for (int i = 0; i < ret_img->tp; i++){
                ret_img->image[i] = 255 - image->image[i];
            }
        } else if (type == 0xAC){
            float hh, p, q, t, ff;
            int i;
            for (int a = 0; a < ret_img->tp / 3; a++){
                if (image->image[a*3+1] == 0){
                    ret_img->image[a*3] =   image->image[a*3];
                    ret_img->image[a*3+1] = image->image[a*3];
                    ret_img->image[a*3+2] = image->image[a*3];
                }
                hh = image->image[a*3+2]*2;
                if (hh >= 360.0)
                    hh = 0;
                hh /= 60.0;
                i = (int)hh;
                ff = hh - i;
                p = image->image[a*3] * (1.0 - (float)image->image[a*3+1]/255);
                q = image->image[a*3] * (1.0 - ((float)image->image[a*3+1]/255 * ff));
                t = image->image[a*3] * (1.0 - ((float)image->image[a*3+1]/255 * (1.0 - ff)));
                switch(i) {
                    case 0:
                        ret_img->image[a*3] = image->image[a*3];
                        ret_img->image[a*3+1] = t;
                        ret_img->image[a*3+2] = p;
                        break;
                    case 1:
                        ret_img->image[a*3] = q;
                        ret_img->image[a*3+1] = image->image[a*3];
                        ret_img->image[a*3+2] = p;
                        break;
                    case 2:
                        ret_img->image[a*3] = p;
                        ret_img->image[a*3+1] = image->image[a*3];
                        ret_img->image[a*3+2] = t;
                        break;

                    case 3:
                        ret_img->image[a*3] = p;
                        ret_img->image[a*3+1] = q;
                        ret_img->image[a*3+2] = image->image[a*3];
                        break;
                    case 4:
                        ret_img->image[a*3] = t;
                        ret_img->image[a*3+1] = p;
                        ret_img->image[a*3+2] = image->image[a*3];
                        break;
                    case 5:
                    default:
                        ret_img->image[a*3] = image->image[a*3];
                        ret_img->image[a*3+1] = p;
                        ret_img->image[a*3+2] = q;
                        break;
                }
            }
        }
    }
    return ret_img;
}

K9_Image *rgb_to_gray(K9_Image *gray, K9_Image *image, bool read){
    // deprecated
}

K9_Image *rgb_to_hsv(K9_Image *hsv, K9_Image *image, bool read){
    // deprecated
}

K9_Image *invert(K9_Image *ret_img, K9_Image *image, bool read){
    // deprecated
}

K9_Image *resize_img(K9_Image *ret_img, K9_Image *image, vec2 scale, int type, bool read){
    ret_img->width = scale[0] * image->width;
    ret_img->height = scale[1] * image->height;
    ret_img->channels = image->channels;
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "resize_img_billinear";
        if (type == K9_NEAREST)
            strcpy(func, "resize_img_nearest");
        uint16_t conv_id = 440;
        size_t global_item_size = image->width * image->height * image->channels;
        size_t return_item_size = ret_img->width * ret_img->height * ret_img->channels;

        mem_check_gpu(image, ret_img, prog, func, conv_id, global_item_size, read);

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
            recalculate_local_workgroups(return_item_size, 0);
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
                        ret_img->image[GET_PIXEL(x, y, z, ret_img)] = 
                        image->image[GET_PIXEL((int)posy, (int)posx, z, image)];
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
                        uint8_t a = image->image[GET_PIXEL(xpoi, ypoi, z, image)];
                        uint8_t b = image->image[GET_PIXEL((xpoi+1), ypoi, z, image)];
                        uint8_t c = image->image[GET_PIXEL(xpoi, (ypoi+1), z, image)];
                        uint8_t d = image->image[GET_PIXEL((xpoi+1), (ypoi+1), z, image)];
                        ret_img->image[GET_PIXEL(x,y,z,ret_img)]  = a*(1-x_dist)*(1-y_dist) +b*(x_dist)*(1-y_dist) + c*(1-x_dist)*(y_dist) + d*(x_dist)*y_dist;
                    }
                }
            }
        }
    }
    return ret_img;
}

K9_Image *translate_image(K9_Image *ret_img, K9_Image *image, int x, int y, bool read){
    // make x&y respective to regular +x(left) and -y(down) type axis
    x = -x;
    y = -y;
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "translate";
        uint16_t conv_id = 440;

        mem_check_gpu(image, ret_img, prog, func, conv_id, image->tp, read);
        int dims[] = {image->width, image->height, image->channels};

        cl_mem dim_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 3 * sizeof(int), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, dim_mem_obj, CL_TRUE, 0, 3 * sizeof(int), dims, 0, NULL, NULL);

        set_main_args(image->mem_id, ret_img->mem_id);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&dim_mem_obj);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_int), (void *)&x);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_int), (void *)&y);

        if (read)
            ret_img->image = run_kernel(ret_img->tp, *ret_img, ret_img->tp);
        else
            run_kernel_no_return(ret_img->tp);

        global.gpu_values.ret = clReleaseMemObject(dim_mem_obj);
    } else {
        for (int i = 0; i < image->width; i++){
            for (int j = 0; j < image->height; j++){
                for (int k = 0; k < image->channels; k++){
                    if (GET_PIXEL((j+y), (i+x), k, image) >= 0 && GET_PIXEL((j+y), (i+x), k, image) < image->tp){
                        if (x+i < 0 || x+i > image->width)
                            ret_img->image[GET_PIXEL(j, i, k, image)] = 0;
                        else if (y+j > image->height || y+j < 0)
                            ret_img->image[GET_PIXEL(j, i, k, image)] = 0;
                        else
                            ret_img->image[GET_PIXEL(j, i, k, image)] = image->image[GET_PIXEL((j+y), (i+x), k, image)];
                    }
                    else{
                        ret_img->image[GET_PIXEL(j, i, k, image)] = 0;
                    }
                }
            }
        }
    }
    return ret_img;
}

K9_Image *rotate_image(K9_Image *ret_img, K9_Image *image, float deg, bool rsz, bool read){
    double rad = deg*3.14/180;
    double rel_rad = fabs(deg);
    while (rel_rad-180 >= 0)
        rel_rad -= 180;
    rel_rad = rel_rad*3.14/180;
    float side_x = sqrt((image->width/2)*(image->width/2) + (image->height/2)*(image->height/2));
    float t1 = acos((image->width/2)/side_x);
    float t1_h = acos((image->height/2)/side_x);
    float t2 = t1 - fabs(rel_rad);
    if (rel_rad > 1.5708)
        t2 = t1 + fabs(rel_rad);
    float t2_h = t1_h - fabs(rel_rad);
    if (rel_rad > 1.5708)
        t2_h = t1_h + fabs(rel_rad);
    int new_width = abs(cos(t2)*side_x*2);
    int new_height = abs(cos(t2_h)*side_x*2);
    if (rsz){
        ret_img->width = new_width;
        ret_img->height = new_height;
        ret_img->tp = ret_img->height * ret_img->width * image->channels;
        ret_img->image = realloc(ret_img->image, ret_img->tp);
    }
    if (global.enable_gpu == true){
        char prog[] = "./conversions/conversions.cl";
        char func[] = "resize_rotate";
        if (!rsz)
            strcpy(func, "rotate_img");
        uint16_t conv_id = 440;

        mem_check_gpu(image, ret_img, prog, func, conv_id, image->tp, read);
        int dims[] = {image->width, image->height, image->channels};
        int nw_dims[] = {ret_img->width, ret_img->height, ret_img->channels};

        cl_mem dim_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 3 * sizeof(int), NULL, &global.gpu_values.ret);

        if (rsz)
            update_output_buffer(ret_img);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, dim_mem_obj, CL_TRUE, 0, 3 * sizeof(int), dims, 0, NULL, NULL);
        set_main_args(image->mem_id, ret_img->mem_id);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&dim_mem_obj);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_double), (void *)&rad);
        cl_mem rsz_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 3 * sizeof(int), NULL, &global.gpu_values.ret);
        if (rsz){
            global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, rsz_mem_obj, CL_TRUE, 0, 3 * sizeof(int), nw_dims, 0, NULL, NULL);
            global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_mem), (void *)&rsz_mem_obj);
        }

        if (read)
            ret_img->image = run_kernel(image->tp, *ret_img, ret_img->tp);
        else
            run_kernel_no_return(ret_img->tp);

        global.gpu_values.ret = clReleaseMemObject(dim_mem_obj);
        global.gpu_values.ret = clReleaseMemObject(rsz_mem_obj);
    } else {
        memset(ret_img->image, 0, ret_img->tp);
        for (int i = 0; i < image->width; i++){
            for (int j = 0; j < image->height; j++){
                for (int k = 0; k < image->channels; k++){
                    int abc, ab, c;
                    abc = i - (image->width/2);
                    if (abc >= 0){
                        c  = j - (image->height/2);
                        ab = i - (image->width /2);
                    } else {
                        ab = i - (image->width /2);
                        c  = j - (image->height/2);
                    }
                    double z = sqrt((float)(ab * ab) + (c * c));
                    double theta1 = asin((float)c / (float)z);
                    double d, a;
                    if (abc >= 0){
                        d = z*sin(theta1+rad);
                        a = z*cos(theta1+rad);
                    } else {
                        d = z*sin(rad-theta1);
                        a = z*cos(rad-theta1);
                    }
                    int y2, y1;
                    if (abc >=0){
                        y2 = (ret_img->height/2)+d;
                        y1 = (ret_img->width / 2) + a;
                    } else {
                        y2 = (ret_img->height / 2) - d;
                        y1 = (ret_img->width / 2) - a;
                    }
                    if (rsz){
                        if (GET_PIXEL(y2, y1, k, ret_img) >= 0 && GET_PIXEL(y2, y1, k, ret_img)  < ret_img->tp)
                            ret_img->image[GET_PIXEL(y2, y1, k, ret_img)] = image->image[GET_PIXEL(j, i, k, image)];
                    }
                    else if (GET_PIXEL(y2, y1, k, image) >= 0 && GET_PIXEL(y2, y1, k, image) < image->tp){
                        // prevent side looping
                        if (fabs(a) < image->width/2 && fabs(d) < image->height/2)
                            ret_img->image[GET_PIXEL(y2, y1, k, ret_img)] = image->image[GET_PIXEL(j, i, k, image)];
                    }
                }
            }
        }
    }
    return ret_img;
}
