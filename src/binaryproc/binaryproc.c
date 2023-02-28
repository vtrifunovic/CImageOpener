#include "binaryproc.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "../global.h"

K9_Image *hit_x_miss(K9_Image *ret_img, K9_Image *image, Kernel kern){
    if (image->channels > 1){
        fprintf(stderr, "\e[1;31m Error!\e[0m Function hit_x_miss() needs a single channel image\n");
        printf("Channels given: %d\n", image->channels);
        exit(0);
    };
    uint8_t kernelsize = kern.height * kern.width;
    ret_img->name = (char *)realloc(ret_img->name, 4);
    strcpy(ret_img->name, "hxm");
    if (global.enable_gpu == true){
        char prog[] = "./binaryproc/binaryproc.cl";
        char func[] = "hit_x_miss";
        uint16_t bin_id = 640;
        size_t global_item_size = image->width * image->height * image->channels;
        //if (global_item_size != global.totalsize){
        update_gpu_channels(*image, global_item_size);
        global.totalsize = global_item_size;
        //}
        read_cl_program(prog, bin_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, bin_id);
            strcpy(global.past_func, func);
        }

        cl_mem kern_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, kernelsize * sizeof(int16_t), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, kern_mem_obj, CL_TRUE, 0, kernelsize*sizeof(int16_t), kern.kernel, 0, NULL, NULL);

        set_main_args();

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&kern_mem_obj);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_uchar), (void *)&kernelsize);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_int), (void *)&image->width);

        ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
        global.gpu_values.ret = clReleaseMemObject(kern_mem_obj);
    } else {
        int length = image->width * image->height;
        int count_trues = 0, nxtline = 0;
        for (int x = 0; x < length; x++){
            uint8_t h = sqrt((float)kernelsize)-1;
            bool r = false;
            for (uint8_t i = 0; i < h; i++){
                for (uint8_t j = 0; j < h+1; j++){
                    if (i == 0 && j == h)
                        continue;
                    else if (x-(i*image->width-i)-j < 0)
                        r = true;
                    if (kern.kernel[kernelsize/2-i*h-j] != image->image[x-(i*image->width-i)-j] && kern.kernel[kernelsize/2-i*h-j] >= 0)
                        r = true;
                    if (kern.kernel[kernelsize/2+i*h+j] != image->image[x+(i*image->width-i)+j] && kern.kernel[kernelsize/2+i*h+j] >= 0)
                        r = true;
                }
            }
            if (r)
                ret_img->image[x] = 0;
            else
                ret_img->image[x] = 255;
        }
    }
    return ret_img;
}

K9_Image *bin_dilation(K9_Image *ret_img, K9_Image image, Kernel kern){
    if (image.channels > 1){
        fprintf(stderr, "\e[1;31mError!\e[0m Function bin_dilation() needs single channel image\n");
        return ret_img;
    }
    uint8_t kernelsize = kern.width * kern.height;
    ret_img->name = (char *) realloc(ret_img->name, 4);
    strcpy(ret_img->name, "dil");
    if (global.enable_gpu == true){
        char prog[] = "./binaryproc/binaryproc.cl";
        char func[] = "bin_dilation";
        uint16_t bin_id = 640;
        size_t global_item_size = image.width * image.height;
        if (global_item_size != global.totalsize){
            update_gpu_channels(image, global_item_size);
            global.totalsize = global_item_size;
        }
        read_cl_program(prog, bin_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, bin_id);
            strcpy(global.past_func, func);
        }
        cl_mem kern_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, kernelsize * sizeof(int16_t), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, kern_mem_obj, CL_TRUE, 0, kernelsize * sizeof(int16_t), kern.kernel, 0, NULL, NULL);

        set_main_args();

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&kern_mem_obj);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_uchar), (void *)&kernelsize);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_int), (void *)&image.width);

        ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
        global.gpu_values.ret = clReleaseMemObject(kern_mem_obj);
    } else {
        int center = kern.width * kern.height/ 2;
        int length = image.width * image.height;
        for (int j = 0; j < length; j++){
            if (kern.kernel[center] == image.image[j]){
                ret_img->image[j] = 255;
                int inc = 1;
                while (j-inc >= 0 && kern.kernel[center-inc] == 255 && inc <= kern.width/2){
                    ret_img->image[j-inc] = 255;
                    inc++;
                }
                inc = 1;
                while (j + inc <= length && kern.kernel[center + inc] == 255 && inc <= kern.width/2){
                    ret_img->image[j+inc] = 255;
                    inc++;
                }
                inc = 1;
                int side = 1;
                while (j-inc*image.width >= 0 && kern.kernel[center + inc * kern.width] == 255){
                    ret_img->image[j - inc*image.width] = 255;
                    while (j - inc * image.width - side >= 0 && kern.kernel[center - inc - side * kern.width] == 255)
                    {
                        ret_img->image[j - side - inc * image.width] = 255;
                        side++;
                    }
                    side = 1;
                    while (j - inc * image.width - side >= 0 && kern.kernel[center - inc + side * kern.width] == 255)
                    {
                        ret_img->image[j + side - inc * image.width] = 255;
                        side++;
                    }
                    inc++;
                }
                inc = 1;
                side = 1;
                while (j + inc * image.width <= length && kern.kernel[center - inc * kern.width] == 255){
                    ret_img->image[j + inc * image.width] = 255;
                    while (j + inc * image.width - side >= 0 && kern.kernel[center - inc - side * kern.width] == 255)
                    {
                        ret_img->image[j - side + inc * image.width] = 255;
                        side++;
                    }
                    side = 1;
                    while (j + inc * image.width - side >= 0 && kern.kernel[center - inc + side * kern.width] == 255)
                    {
                        ret_img->image[j + side + inc * image.width] = 255;
                        side++;
                    }
                    inc++;
                }
            }
        }
    }
    return ret_img;
}

K9_Image *bin_erosion(K9_Image *ret_img, K9_Image image, Kernel kern){
    if (image.channels > 1){
        fprintf(stderr, "\e[1;31mError!\e[0m Function bin_erosion() needs single channel image\n");
        return ret_img;
    }
    ret_img->name = (char *)realloc(ret_img, 6);
    strcpy(ret_img->name, "erode");
    uint8_t kernelsize = kern.width * kern.height;
    if (global.enable_gpu == true){
        char prog[] = "./binaryproc/binaryproc.cl";
        char func[] = "bin_erosion";
        uint16_t bin_id = 640;
        size_t global_item_size = image.width * image.height * image.channels;
        if (global_item_size != global.totalsize){
            update_gpu_channels(image, global_item_size);
            global.totalsize = global_item_size;
        }
        read_cl_program(prog, bin_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, bin_id);
            strcpy(global.past_func, func);
        }
        cl_mem kern_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, kernelsize * sizeof(int16_t), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, kern_mem_obj, CL_TRUE, 0, kernelsize * sizeof(int16_t), kern.kernel, 0, NULL, NULL);

        set_main_args();

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&kern_mem_obj);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_uchar), (void *)&kernelsize);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_int), (void *)&image.width);

        ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
        global.gpu_values.ret = clReleaseMemObject(kern_mem_obj);
    }
    memcpy(ret_img->image, image.image, image.width * image.height);
    int center = kern.width * kern.height / 2;
    int length = image.width * image.height;
    for (int j = 0; j < length; j++)
    {
        if (kern.kernel[center] != image.image[j])
        {
            ret_img->image[j] = 0;
            int inc = 1;
            while (j - inc >= 0 && kern.kernel[center - inc] == 255 && inc <= kern.width/2)
            {
                ret_img->image[j - inc] = 0;
                inc++;
            }
            inc = 0;
            while (j + inc <= length && kern.kernel[center + inc] == 255 && inc <= kern.width/2)
            {
                ret_img->image[j + inc] = 0;
                inc++;
            }
            inc = 1;
            int side = 1;
            while (j - inc * image.width >= 0 && kern.kernel[center + inc * kern.width] == 255)
            {
                ret_img->image[j - inc * image.width] = 0;
                while (j - inc * image.width - side >= 0 && kern.kernel[center - inc - side * kern.width] == 255)
                {
                    ret_img->image[j - side - inc * image.width] = 0;
                    side++;
                }
                side = 1;
                while (j - inc * image.width - side >= 0 && kern.kernel[center - inc + side * kern.width] == 255)
                {
                    ret_img->image[j + side - inc * image.width] = 0;
                    side++;
                }
                inc++;
            }
            side = 1;
            inc = 1;
            while (j + inc * image.width <= length && kern.kernel[center - inc * kern.width] == 255)
            {
                ret_img->image[j + inc * image.width] = 0;
                while (j + inc * image.width - side >= 0 && kern.kernel[center - inc - side * kern.width] == 255)
                {
                    ret_img->image[j - side + inc * image.width] = 0;
                    side++;
                }
                side = 1;
                while (j + inc * image.width - side >= 0 && kern.kernel[center - inc + side * kern.width] == 255)
                {
                    ret_img->image[j + side + inc * image.width] = 0;
                    side++;
                }
                inc++;
            }
        }
    }
    return ret_img;
}

static bool compare(K9_Image img1, K9_Image img2){
    int totalpixels = img1.width * img1.height;
    for (int i = 0; i < totalpixels; i++){
        if (img1.image[i] != img2.image[i])
            return false;
    }
    return true;
}

static uint8_t A(uint8_t a1, uint8_t a2){
    return(!a1 && a2);
}

K9_Image *thinning(K9_Image *ret_img, K9_Image image){
    int totalpixels = image.width * image.height * image.channels;
    ret_img->name = (char *)malloc(5);
    K9_Image tmp = {
        .width = image.width,
        .height = image.height,
        .channels = image.channels,
        .image = (uint8_t *)malloc(totalpixels)
    };
    strcpy(ret_img->name, "thin");
    if (global.enable_gpu == true){
        char prog[] = "./binaryproc/binaryproc.cl";
        char func[] = "gh_thin";
        uint16_t bin_id = 640;
        size_t global_item_size = image.width * image.height * image.channels;
        // if (global_item_size != global.totalsize){
        update_gpu_channels(image, global_item_size);
        global.totalsize = global_item_size;
        //}

        read_cl_program(prog, bin_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, bin_id);
            strcpy(global.past_func, func);
        }
        uint8_t iter = 0;
        bool stop = false;
        while (!stop){
            memcpy(tmp.image, ret_img->image, totalpixels);
            set_main_args();
            global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_int), (void *)&image.width);
            global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_uchar), (void *)&iter);
            ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
            iter++;
            update_gpu_channels(*ret_img, global_item_size);
            set_main_args();
            global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_int), (void *)&image.width);
            global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_uchar), (void *)&iter);
            ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
            update_gpu_channels(*ret_img, global_item_size);
            iter = 0;
            stop = compare(*ret_img, tmp);
        }
        free(tmp.name);
        free(tmp.image);
    } else {
        int totalpixels = image.width * image.height;
        bool stop = false;
        memcpy(tmp.image, image.image, totalpixels);
        while (!stop){
            for (int x = 0; x <  totalpixels; x++){
                for (uint8_t loops = 0; loops < 2; loops++){
                    uint8_t p1 = tmp.image[x]/255;
                    uint8_t p2 = tmp.image[x-image.width]/255;
                    uint8_t p3 = tmp.image[x-image.width+1]/255;
                    uint8_t p4 = tmp.image[x+1]/255;
                    uint8_t p5 = tmp.image[x+1+image.width]/255;
                    uint8_t p6 = tmp.image[x+image.width]/255;
                    uint8_t p7 = tmp.image[x+image.width-1]/255;
                    uint8_t p8 = tmp.image[x-1]/255;
                    uint8_t p9 = tmp.image[x-1-image.width]/255;

                    uint8_t check1 = p2+p3+p4+p5+p6+p7+p8+p9;
                    check1 = check1 >= 2 ? check1 : 0;
                    check1 = check1 <= 6 ? check1 : 0;
                    uint8_t check2, check3;
                    if (loops == 1){
                        check2 = p2*p4*p8;
                        check3 = p2*p6*p8;
                    } else {
                        check2 = p2*p4*p6;
                        check3 = p4*p6*p8;
                    }

                    uint8_t check4 = A(p2, p3) + A(p3, p4) + A(p4, p5) + A(p5, p6) + A(p6, p7) + A(p7, p8) + A(p8, p9) + A(p9, p2);
                    if (check1 != 0 && check2 == 0 && check3 == 0 && check4 == 1){
                        ret_img->image[x] = 0;
                    } else {
                        ret_img->image[x] = tmp.image[x];
                    }
                }
            }
            stop = compare(*ret_img, tmp);
            memcpy(tmp.image, ret_img->image, totalpixels);
        }
    }
    return ret_img;
}

Kernel create_kernel(int *a, size_t size){
    // only works with 1x1, 3x3, 5x5 ... NxN kernels
    Kernel kern = {
        .height = sqrt(size), // fix this
        .width = sqrt(size),  // and this
        .kernel = (int16_t *)malloc(size * sizeof(int16_t)),
    };
    for (int j = 0; j < size; j++){
        kern.kernel[j] = a[j]*255;
    }
    return kern;
}

Kernel quick_kernel(int w, int h){
    Kernel kern = {
        .width = w,
        .height = h,
        .kernel = (int16_t *) malloc(w*h*sizeof(int16_t)),
    };
    for (int x = 0; x < w*h; x++)
        kern.kernel[x] = 255;
    return kern;
}