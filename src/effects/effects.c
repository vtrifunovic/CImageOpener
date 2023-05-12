#include <string.h>
#include "effects.h"

// Not really useful for anything, but they make cool video effects :3

K9_Image *pixel_lag(K9_Image *ret_img, K9_Image *image, int chance, int chunk_size, bool read){
    size_t global_item_size = image->width * image->height * image->channels;
    char prog[] = "./effects/effects.cl";
    char func[] = "pixel_lag";
    uint16_t effects_id = 230;
    if (image->mem_id == NULL)
        update_input_buffer(image, global_item_size);
    if (ret_img->mem_id == NULL)
        update_output_buffer(ret_img, global_item_size);
    if (ret_img->image == NULL && read)
        ret_img->image = (uint8_t *)malloc(global_item_size);
    read_cl_program(prog, effects_id);
    if (strcmp(global.past_func, func) != 0){
        bind_cl_function(func, effects_id);
        strcpy(global.past_func, func);
    }
    set_main_args(image->mem_id, ret_img->mem_id);

    int value = rand() % (chance + 1);

    global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_int), (void *)&chance);
    global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_int), (void *)&chunk_size);
    global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_int), (void *)&value);

    if (read)
        ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
    else
        run_kernel_no_return(global_item_size);

    return ret_img;
}

K9_Image *color_decay(K9_Image *ret_img, K9_Image *image, int kern, int iterations, bool read){
    iterations = abs(iterations);
    size_t totalpixels = image->width * image->height * image->channels;
    uint8_t kernelsize = kern*kern;
    // creating temporary image to store data so we can do multiple iterations
    K9_Image *tmpimg = malloc(sizeof(K9_Image));
    tmpimg->width = image->width;
    tmpimg->height = image->height;
    tmpimg->channels = image->channels;
    tmpimg->mem_id = NULL;
    tmpimg->image = (uint8_t *)malloc(totalpixels);
    memcpy(tmpimg->image, image->image, totalpixels);
    if (global.enable_gpu == true){
        char prog[] = "./effects/effects.cl";
        char func[] = "color_decay";
        uint16_t effects_id = 230;
        if (tmpimg->mem_id == NULL)
            update_input_buffer(tmpimg, totalpixels);
        if (ret_img->mem_id == NULL)
            update_output_buffer(ret_img, totalpixels);
        if (ret_img->image == NULL && read)
            ret_img->image = (uint8_t *)malloc(totalpixels);
        read_cl_program(prog, effects_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, effects_id);
            strcpy(global.past_func, func);
		}
        set_main_args(tmpimg->mem_id, ret_img->mem_id);

        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_uchar), (void *)&kernelsize);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_int), (void *)&tmpimg->width);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_uchar), (void *)&tmpimg->channels);

        for (int i = 0; i < iterations; i++){
            run_kernel_no_return(totalpixels);
            if (i%2==0)
                set_main_args(ret_img->mem_id, tmpimg->mem_id);
            else
                set_main_args(tmpimg->mem_id, ret_img->mem_id);
        }
        if (read)
            ret_img->image = read_mem_buffer(*ret_img);
        global.gpu_values.ret = clReleaseMemObject(tmpimg->mem_id);
    }
    free(tmpimg->image);
    free(tmpimg);
    return ret_img;
}

K9_Image *buffer_kill(K9_Image *ret_img, K9_Image *image, uint8_t buffer_overwrite, bool read){
    size_t global_item_size = image->width * image->height * image->channels;
    char prog[] = "./effects/effects.cl";
    char func[] = "buffer_kill";
    uint16_t effects_id = 230;
    if (image->mem_id == NULL)
        update_input_buffer(image, global_item_size);
    if (ret_img->mem_id == NULL)
        update_output_buffer(ret_img, global_item_size);
    if (ret_img->image == NULL && read)
        ret_img->image = (uint8_t *)malloc(global_item_size);
    read_cl_program(prog, effects_id);
    if (strcmp(global.past_func, func) != 0){
        bind_cl_function(func, effects_id);
        strcpy(global.past_func, func);
    }

    set_main_args(image->mem_id, ret_img->mem_id);
    global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_uchar), (void *)&buffer_overwrite);

    if (read)
        ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
    else
        run_kernel_no_return(global_item_size);

    return ret_img;
}
