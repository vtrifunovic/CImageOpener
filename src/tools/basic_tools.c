#include "basic_tools.h"
#include "../typename.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../global.h"

// OCL
K9_Image *blur(K9_Image *ret_img, K9_Image image, Kernel kern, int iterations){
    iterations = abs(iterations);
    int totalpixels = image.width * image.height * image.channels;
    ret_img->name = (char *) realloc(ret_img->name, 5);
    uint8_t kernelsize = kern.width * kern.height;
    strcpy(ret_img->name, "blur");
    if (global.enable_gpu == true){
        char prog[] = "./tools/basic_tools.cl";
        char func[] = "blur";
        uint16_t tool_id = 540;
        size_t global_item_size = image.width * image.height * image.channels;
        update_gpu_channels(image, global_item_size);
        global.totalsize = global_item_size;
        read_cl_program(prog, tool_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, tool_id);
            strcpy(global.past_func, func);
        }
        set_main_args();
        
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_uchar), (void *)&kernelsize);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_int), (void *)&image.width);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_uchar), (void *)&image.channels);

        for (int i = 0; i < iterations; i ++){
            ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
            update_gpu_channels(*ret_img, global_item_size);
        }
    } else {
        // creating temporary image to store data so we can do multiple iterations
        K9_Image tmpimg = { 
            .width = image.width,
            .height = image.height,
            .channels = image.channels,
            .name = (char *) malloc(4),
            .image = (uint8_t *) malloc(totalpixels),
        };
        strcpy(tmpimg.name, "tmp");
        memcpy(tmpimg.image, image.image, totalpixels);
        if (tmpimg.channels > 3)
            tmpimg.channels = 3;
        int comp_iters = 0;
        while (comp_iters != iterations){
            for (int x = 0; x < totalpixels; x++){
                int avg_col = 0, loops = 0;
                for (int y = 0; y < kern.width; y++){
                    for (int z = 0; z < kern.height; z++){
                        int curr_val = x + (y - (kern.width / 2))*tmpimg.channels + ((z - (kern.height / 2)) * tmpimg.width)*tmpimg.channels;
                        if (curr_val > totalpixels || curr_val < 0) //OOB MEM CHECK
                            continue; 
                        avg_col += tmpimg.image[curr_val];
                        loops++;
                    }
                }
                if (loops == 0)
                    ret_img->image[x] = 0;
                else
                    ret_img->image[x] = avg_col/loops;
            }
            if (comp_iters != iterations)
                memcpy(tmpimg.image, ret_img->image, totalpixels);
            comp_iters++;
        }
        free(tmpimg.name);
        free(tmpimg.image);
    }
    return ret_img;
}

K9_Image *subtract(K9_Image *ret_img, K9_Image *img1, K9_Image *img2){
    int totalpixels = img1->width * img1->height * img1->channels;
    ret_img->name = (char *)realloc(ret_img->name, 9);
    strcpy(ret_img->name, "subtract");
    if (global.enable_gpu == true){
        char prog[] = "./tools/basic_tools.cl";
        char func[] = "subtract";
        uint16_t tool_id = 540;
        size_t global_item_size = img1->width * img1->height * img1->channels;
        if (global_item_size != global.totalsize){
            update_gpu_channels(*img1, global_item_size);
            global.totalsize = global_item_size;
        }
        read_cl_program(prog, tool_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, tool_id);
            strcpy(global.past_func, func);
        }
        cl_mem img2_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, totalpixels * sizeof(uint8_t), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, img2_mem_obj, CL_TRUE, 0, totalpixels * sizeof(uint8_t), img2->image, 0, NULL, NULL);

        set_main_args();

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&img2_mem_obj);

        ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
        global.gpu_values.ret = clReleaseMemObject(img2_mem_obj);
    }else{
        for (int x = 0; x < totalpixels; x++){
            ret_img->image[x] = img1->image[x] - img2->image[x];
        }
    }
    return ret_img;
}

K9_Image *add(K9_Image *ret_img, K9_Image img1, K9_Image img2){
    int totalpixels = img1.width * img1.height * img1.channels;
    ret_img->name = (char *)realloc(ret_img->name, 4);
    strcpy(ret_img->name, "add");
    if (global.enable_gpu == true){
        char prog[] = "./tools/basic_tools.cl";
        char func[] = "add";
        uint16_t tool_id = 540;
        size_t global_item_size = img1.width * img1.height * img1.channels;
        // if (global_item_size != global.totalsize){
        update_gpu_channels(img1, global_item_size);
        global.totalsize = global_item_size;
        //}
        read_cl_program(prog, tool_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, tool_id);
            strcpy(global.past_func, func);
        }
        cl_mem img2_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, totalpixels * sizeof(uint8_t), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, img2_mem_obj, CL_TRUE, 0, totalpixels * sizeof(uint8_t), img2.image, 0, NULL, NULL);

        set_main_args();

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&img2_mem_obj);

        ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
    } else {
        for (int x = 0; x < totalpixels; x++){
            ret_img->image[x] = img1.image[x] + img2.image[x];
        }
    }
    return ret_img;
}

// Fix this to be compatible with 2/2 updates
// NO OCL
K9_Split split_channels(K9_Image image){
    // Thoughts: 3 outputs edit the pointers
    // 1 input for the image that gets split??
    int totalpixels = image.width*image.height;
    if (image.channels < 3){
        fprintf(stderr, "\e[1;31mError!\e[0m Function split_channels() cannot split less than 3 channels.\n");
        exit(0);
    }
    K9_Split ret_img = {
        .r = {
            .width = image.width,
            .height = image.height,
            .channels = 1,
            .image = (uint8_t *)malloc(totalpixels),
            .name = (char *)malloc(2)
        },
        .g = {
            .width = image.width,
            .height = image.height,
            .channels = 1,
            .image = (uint8_t *)malloc(totalpixels),
            .name = (char *)malloc(2)
        },
        .b = {
            .width = image.width,
            .height = image.height,
            .channels = 1,
            .image = (uint8_t *)malloc(totalpixels),
            .name = (char *)malloc(2)
        },

    };
    strcpy(ret_img.r.name, "r");
    strcpy(ret_img.g.name, "g");
    strcpy(ret_img.b.name, "b");
    for (int x = 0; x < totalpixels; x++){
        ret_img.r.image[x] = image.image[x*3];
        ret_img.g.image[x] = image.image[x*3+1];
        ret_img.b.image[x] = image.image[x*3+2];
    }
    return ret_img;
}

// Fix this to be compatible with 2/2 updates
// NO OCL
K9_Image merge_channels(K9_Image r, K9_Image g, K9_Image b){
    if (r.height != g.height || g.height != b.height || r.width != g.width || g.width != b.width)
        fprintf(stderr,"\e[1;33mWarning!\e[0m In function call merge_channels() channel sizes do not match.\n");
    if (r.channels > 1 || g.channels > 1 || b.channels > 1){
        fprintf(stderr, "\e[1;31mAll images must be single channel\e[0m\n");
        exit(0);
    }
    int totalpixels = r.height * r.width;
    K9_Image ret_img = {
        .height = r.height,
        .width = r.width,
        .channels = 3,
        .name = (char *)malloc(7),
        .image = (uint8_t *)malloc(totalpixels*3),
    };
    strcpy(ret_img.name, "Merged");
    for (int x = 0; x < totalpixels; x++){
        ret_img.image[x*3] = r.image[x];
        ret_img.image[x*3+1] = g.image[x];
        ret_img.image[x*3+2] = b.image[x];
    }
    return ret_img;
}

static int checkbounds(vec2 xcrop, vec2 ycrop, int x, int y){
    return (x > xcrop[0] && x < xcrop[1] && y > ycrop[0] && y < ycrop[1]);
}

// OCL
K9_Image *crop(K9_Image *ret_img, K9_Image image, vec2 xcrop, vec2 ycrop, int type){
    // needs warning to not crop oob memory
    if (xcrop[1] - xcrop[0] > image.width){
        fprintf(stderr, "\e[1;33mWarning!!!\e[0m In function call crop() x-axis is larger than image width. X-axis will not be cropped\n");
        xcrop[0] = 0;
        xcrop[1] = image.width;
    }
    if (ycrop[1] - ycrop[0] > image.height){
        fprintf(stderr, "\e[1;33mWarning!!!\e[0m In function call crop() x-axis is larger than image width. X-axis will not be cropped\n");
        ycrop[0] = 0;
        ycrop[1] = image.height;
    }
    ret_img->name = (char *) realloc(ret_img->name, 5);
    strcpy(ret_img->name, "crop");
    if (type == K9_FILL){
        ret_img->height = image.height;
        ret_img->width = image.width;
        for (int x = 0; x < image.width; x++){
            for (int y = 0; y < image.height; y++){
                for (int c = 0; c < image.channels; c++){
                    if (checkbounds(xcrop, ycrop, x, y))
                        ret_img->image[(y*image.width+x)*image.channels+c] = image.image[(y*image.width+x)*image.channels+c];
                    else
                        ret_img->image[(y*image.width+x)*image.channels+c] = 0;
                }
            }
        }
    }
    if (type == K9_NOFILL){
        int xwid = xcrop[1] - xcrop[0];
        ret_img->width = xwid;
        int ywid = ycrop[1] - ycrop[0];
        ret_img->height = ywid;
        ret_img->image = (uint8_t *) realloc(ret_img->image, xwid * ywid * image.channels);
        for (int x = 0; x < xwid; x++){
            for (int y = 0; y < ywid; y++){
                for (int c = 0; c < image.channels; c++){
                    ret_img->image[((y*xwid)+x)*image.channels+c] = image.image[((y+(int)ycrop[0])*image.width+(x+(int)xcrop[0]))*image.channels+c];
                }
            }
        }
    }
    return ret_img;
}

// Fix this to be compatible with 2/2 updates
// OCL
K9_Image blend_img(K9_Image image, K9_Image b, float alpha){
    if (alpha > 1){
        alpha = 1;
    }
    float beta = 1 - alpha;
    int totalpixels = image.width * image.height * image.channels;
    K9_Image ret_img = {
        .width = image.width,
        .height = image.height,
        .channels = image.channels,
        .name = (char *) malloc(8),
        .image = (uint8_t *) malloc(totalpixels),
    };
    strcpy(ret_img.name, "blended");
    // same channels blend
    if (image.channels == b.channels){
        for (int x = 0; x < totalpixels; x++){
            ret_img.image[x] = image.image[x] * alpha + b.image[x] * beta;
        }
    // multi-channel blended into single channel
    } else if (image.channels < b.channels){
        for (int x = 0; x < totalpixels; x++){
            ret_img.image[x] = image.image[x] * alpha + b.image[x*b.channels] * beta;
        }
    // single channel blended into multi-channel
    } else {
        for (int x = 0; x < image.height; x++){
            for (int y =0; y < image.width; y++){
                for (int z = 0; z < image.channels; z++){
                    ret_img.image[(y*image.width+x)*image.channels+z] = image.image[(y*image.width+x)*image.channels+z] * alpha + b.image[y*image.width+x] * beta;
                }
            }
        }
    }
    return ret_img;
}

bool compare(K9_Image img1, K9_Image img2){
    if (img1.channels != img2.channels)
        return false;
    if (img1.height != img2.height)
        return false;
    if (img1.width != img2.width)
        return false;
    int totalpixels = img1.width * img1.height * img1.channels;
    for (int i = 0; i < totalpixels; i++){
        if (img1.image[i] != img2.image[i])
            return false;
    }
    return true;
}

void K9_free_split(K9_Split image){
    free(image.r.name);
    free(image.r.image);
    free(image.g.name);
    free(image.g.image);
    free(image.b.name);
    free(image.b.image);
}

K9_Image *gray_morph(K9_Image *ret_img, K9_Image *image, Kernel kern, int type){
    size_t global_item_size = image->width * image->height * image->channels;
    size_t kernelsize = kern.width * kern.height;
    ret_img->name = (char *)realloc(ret_img->name, 6);
    strcpy(ret_img->name, "grmph");
    if (global.enable_gpu == true){
        char prog[] = "./tools/basic_tools.cl";
        char func[] = "g_dilated";
        if (type == K9_EROSION)
            strcpy(func, "g_eroded");
        uint16_t tool_id = 540;
        update_gpu_channels(*image, global_item_size);
        global.totalsize = global_item_size;
        read_cl_program(prog, tool_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, tool_id);
            strcpy(global.past_func, func);
        }
        cl_mem kern_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, kernelsize * sizeof(int16_t), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, kern_mem_obj, CL_TRUE, 0, kernelsize * sizeof(int16_t), kern.kernel, 0, NULL, NULL);

        set_main_args();

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&kern_mem_obj);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_uchar), (void *)&kernelsize);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_int), (void *)&image->width);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 5, sizeof(cl_uchar), (void *)&image->channels);

        ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
        global.gpu_values.ret = clReleaseMemObject(kern_mem_obj);
    }
    return ret_img;
}