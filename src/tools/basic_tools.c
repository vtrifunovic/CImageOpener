#include "basic_tools.h"
#include "../typename.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../global.h"

K9_Image *blur(K9_Image *ret_img, K9_Image *image, Kernel kern, int iterations, bool read){
    iterations = abs(iterations);
    uint8_t kernelsize = kern.width * kern.height;
    // creating temporary image to store data so we can do multiple iterations
    K9_Image *tmpimg = create_img_template(image, true);
    memcpy(tmpimg->image, image->image, tmpimg->tp);
    if (global.enable_gpu == true){
        char prog[] = "./tools/basic_tools.cl";
        char func[] = "blur";
        uint16_t tool_id = 540;

        mem_check_gpu(tmpimg, ret_img, prog, func, tool_id, ret_img->tp, read);

        set_main_args(tmpimg->mem_id, ret_img->mem_id);

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_uchar), (void *)&kernelsize);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_int), (void *)&tmpimg->width);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_uchar), (void *)&tmpimg->channels);

        for (int i = 0; i < iterations; i++){
            run_kernel_no_return(ret_img->tp);
            if (i%2==0)
                set_main_args(ret_img->mem_id, tmpimg->mem_id);
            else
                set_main_args(tmpimg->mem_id, ret_img->mem_id);
        }
        if (read)
            ret_img->image = read_mem_buffer(*ret_img);
        global.gpu_values.ret = clReleaseMemObject(tmpimg->mem_id);
    } else {
        int comp_iters = 0;
        while (comp_iters != iterations){
            for (int x = 0; x < ret_img->tp; x++){
                int avg_col = 0, loops = 0;
                for (int y = 0; y < kern.width; y++){
                    for (int z = 0; z < kern.height; z++){
                        int curr_val = x + (y - (kern.width / 2))*tmpimg->channels + ((z - (kern.height / 2)) * tmpimg->width)*tmpimg->channels;
                        if (curr_val > ret_img->tp || curr_val < 0) //OOB MEM CHECK
                            continue; 
                        avg_col += tmpimg->image[curr_val];
                        loops++;
                    }
                }
                if (loops == 0)
                    ret_img->image[x] = 0;
                else
                    ret_img->image[x] = avg_col/loops;
            }
            if (comp_iters != iterations)
                memcpy(tmpimg->image, ret_img->image, ret_img->tp);
            comp_iters++;
        }
    }
    free(tmpimg->image);
    free(tmpimg);
    return ret_img;
}

K9_Image *subtract(K9_Image *ret_img, K9_Image *img1, K9_Image *img2, bool read){
    if (global.enable_gpu == true){
        char prog[] = "./tools/basic_tools.cl";
        char func[] = "subtract";
        uint16_t tool_id = 540;
        if (img1->mem_id == NULL)
            update_input_buffer(img1);
        if (img2->mem_id == NULL)
            update_input_buffer(img2);
        if (ret_img->mem_id == NULL)
            update_output_buffer(ret_img);
        if (ret_img->image == NULL && read)
            ret_img->image = (uint8_t *)malloc(img1->tp);
        read_cl_program(prog, tool_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, tool_id);
            strcpy(global.past_func, func);
		}
        set_main_args(img1->mem_id, ret_img->mem_id);

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&img2->mem_id);
        
        if (read)
            ret_img->image = run_kernel(img1->tp, *ret_img, img1->tp, false);
        else
            run_kernel_no_return(img1->tp);
    }else{
        for (int x = 0; x < img1->tp; x++){
            ret_img->image[x] = img1->image[x] - img2->image[x];
        }
    }
    return ret_img;
}

K9_Image *add(K9_Image *ret_img, K9_Image *img1, K9_Image *img2, bool read){
    if (global.enable_gpu == true){
        char prog[] = "./tools/basic_tools.cl";
        char func[] = "add";
        uint16_t tool_id = 540;
        size_t global_item_size = img1->width * img1->height * img1->channels;
        if (img1->mem_id == NULL)
            update_input_buffer(img1);
        if (img2->mem_id == NULL)
            update_input_buffer(img2);
        if (ret_img->mem_id == NULL)
            update_output_buffer(ret_img);
        if (ret_img->image == NULL && read)
            ret_img->image = (uint8_t *)malloc(global_item_size);
        read_cl_program(prog, tool_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, tool_id);
            strcpy(global.past_func, func);
		}
        cl_mem img2_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, img1->tp * sizeof(uint8_t), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, img2_mem_obj, CL_TRUE, 0, img1->tp * sizeof(uint8_t), img2->image, 0, NULL, NULL);

        set_main_args(img1->mem_id, ret_img->mem_id);

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&img2_mem_obj);

        if (read)
            ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size, false);
        else
            run_kernel_no_return(global_item_size);

        global.gpu_values.ret = clReleaseMemObject(img2_mem_obj);
    } else {
        for (int x = 0; x < img1->tp; x++){
            ret_img->image[x] = img1->image[x] + img2->image[x];
        }
    }
    return ret_img;
}

void split_channels(K9_Image *r, K9_Image *g, K9_Image *b, K9_Image image){
    int totalpixels = image.width*image.height;
    if (image.channels < 3){
        fprintf(stderr, "\e[1;31mError!\e[0m Function split_channels() cannot split less than 3 channels.\n");
        exit(0);
    }
    for (int x = 0; x < totalpixels; x++){
        r->image[x] = image.image[x*3];
        g->image[x] = image.image[x*3+1];
        b->image[x] = image.image[x*3+2];
    }
}

// OCL
K9_Image *merge_channels(K9_Image *ret_img, K9_Image *r, K9_Image *g, K9_Image *b, bool destroy){
    if (r->height != g->height || g->height != b->height || r->width != g->width || g->width != b->width)
        fprintf(stderr,"\e[1;33mWarning!\e[0m In function call merge_channels() channel sizes do not match.\n");
    if (r->channels > 1 || g->channels > 1 || b->channels > 1){
        fprintf(stderr, "\e[1;31mAll images must be single channel\e[0m\n");
        exit(0);
    }
    int totalpixels = r->height * r->width;
    for (int x = 0; x < totalpixels; x++){
        ret_img->image[x*3] = r->image[x];
        ret_img->image[x*3+1] = g->image[x];
        ret_img->image[x*3+2] = b->image[x];
    }
    if (destroy){
        K9_free(r);
        K9_free(g);
        K9_free(b);
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

K9_Image *blend_img(K9_Image *ret_img, K9_Image *image, K9_Image *b, float alpha, bool read){
    if (alpha > 1){
        alpha = 1;
    }
    if (global.enable_gpu){
        char prog[] = "./tools/basic_tools.cl";
        char func[] = "blend_img";
        uint16_t tool_id = 540;

        mem_check_gpu(image, ret_img, prog, func, tool_id, image->tp, read);
        if (b->mem_id == NULL)
            update_input_buffer(b);

        cl_mem b_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, b->tp * sizeof(uint8_t), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, b_mem_obj, CL_TRUE, 0, b->tp * sizeof(uint8_t), b->image, 0, NULL, NULL);

        set_main_args(image->mem_id, ret_img->mem_id);

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&b_mem_obj);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_float), (void *)&alpha);

        if (read)
            ret_img->image = run_kernel(ret_img->tp, *ret_img, ret_img->tp, false);
        else
            run_kernel_no_return(ret_img->tp);

        global.gpu_values.ret = clReleaseMemObject(b_mem_obj);
    } else {
        float beta = 1 - alpha;
        // same channels blend
        if (image->channels == b->channels){
            for (int x = 0; x < image->tp; x++){
                ret_img->image[x] = image->image[x] * alpha + b->image[x] * beta;
            }
        // multi-channel blended into single channel
        } else if (image->channels < b->channels){
            for (int x = 0; x < image->tp; x++){
                ret_img->image[x] = image->image[x] * alpha + b->image[x * b->channels] * beta;
            }
        // single channel blended into multi-channel
        } else {
            for (int x = 0; x < image->height; x++){
                for (int y =0; y < image->width; y++){
                    for (int z = 0; z < image->channels; z++){
                        ret_img->image[(y * image->width + x) * image->channels + z] = image->image[(y * image->width + x) * image->channels + z] * alpha + b->image[y * image->width + x] * beta;
                    }
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
    for (int i = 0; i < img1.tp; i++){
        if (img1.image[i] != img2.image[i])
            return false;
    }
    return true;
}

K9_Image *gray_morph(K9_Image *ret_img, K9_Image *image, Kernel kern, int type, bool read){
    size_t kernelsize = kern.width * kern.height;
    if (global.enable_gpu == true){
        char prog[] = "./tools/basic_tools.cl";
        char func[] = "g_dilated";
        if (type == K9_EROSION)
            strcpy(func, "g_eroded");
        uint16_t tool_id = 540;

        mem_check_gpu(image, ret_img, prog, func, tool_id, ret_img->tp, read);

        cl_mem kern_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, kernelsize * sizeof(int16_t), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, kern_mem_obj, CL_TRUE, 0, kernelsize * sizeof(int16_t), kern.kernel, 0, NULL, NULL);

        set_main_args(image->mem_id, ret_img->mem_id);

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&kern_mem_obj);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_uchar), (void *)&kernelsize);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_int), (void *)&image->width);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 5, sizeof(cl_uchar), (void *)&image->channels);

        if (read)
            ret_img->image = run_kernel(ret_img->tp, *ret_img, ret_img->tp, false);
        else
            run_kernel_no_return(ret_img->tp);

        global.gpu_values.ret = clReleaseMemObject(kern_mem_obj);
    } else {
        if (type == K9_EROSION){
            for (int x = 0; x < ret_img->tp; x++){
                uint8_t min = 255;
                int8_t order = sqrt((float)kernelsize);
                int8_t p_back = order/2;
                int8_t start = p_back-order+1;
                int8_t shift = abs(p_back-order+1);
                for (int8_t y = start; y <= p_back; y++){
                    for (int8_t z = start; z <= p_back; z++){
                        if (x+z*image->channels+image->width*y*image->channels < 0)
                            continue;
                        if (x+z*image->channels+image->width*y*image->channels >= ret_img->tp)
                            continue;
                        if (image->image[x+z*image->channels+image->width*y*image->channels] < min)
                            min = image->image[x+z*image->channels+image->width*y*image->channels];
                    }
                }
                ret_img->image[x]  = min;
            }
        } else {
            for (int x = 0; x < ret_img->tp; x++){
                uint8_t max = 0;
                int8_t order = sqrt((float)kernelsize);
                int8_t p_back = order/2;
                int8_t start = p_back-order+1;
                int8_t shift = abs(p_back-order+1);
                for (int8_t y = start; y <= p_back; y++){
                    for (int8_t z = start; z <= p_back; z++){
                        if (x+z*image->channels+image->width*y*image->channels < 0)
                            continue;
                        if (x+z*image->channels+image->width*y*image->channels>= ret_img->tp)
                            continue;
                        if (image->image[x+z*image->channels+image->width*y*image->channels] > max)
                            max = image->image[x+z*image->channels+image->width*y*image->channels];
                    }
                }
                ret_img->image[x]  = max;
            }   
        }
    }
    return ret_img;
}
