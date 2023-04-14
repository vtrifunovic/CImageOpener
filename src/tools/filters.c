#include "filters.h"
#include <string.h>

static uint8_t insertion_sort(uint8_t window[], uint8_t size){
    int j, key;
    for (uint8_t i = 1; i < size; i++){
        key = window[i];
        j = i - 1;
        while (j >= 0 && window[j] > key){
            window[j+1] = window[j];
            j = j - 1;
        }
        window[j+1] = key;
    }
    return window[size/2];
}

K9_Image *median_filter(K9_Image *ret_img, K9_Image *image, uint8_t order, bool read){
    size_t global_item_size = image->width * image->height * image->channels;
    if (global.enable_gpu == true){
        char prog[] = "./tools/filters.cl";
        char func[] = "median_filter";
        uint16_t filt_id = 145;
        if (image->mem_id == NULL)
            update_input_buffer(image, global_item_size);
        if (ret_img->mem_id == NULL)
            update_output_buffer(ret_img, global_item_size);
        if (ret_img->image == NULL && read)
            ret_img->image = (uint8_t *) malloc(global_item_size);
        read_cl_program(prog, filt_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, filt_id);
            strcpy(global.past_func, func);
        }
        // window won't work properly if workgroups aren't size of 1
        recalculate_local_workgroups(global_item_size, 1);

        set_main_args(image->mem_id, ret_img->mem_id);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_uchar)*order*order, NULL);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_uchar), (void *)&order);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_int), (void *)&image->width);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 5, sizeof(cl_uchar), (void *)&image->channels);

        if (read)
            ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
        else
            run_kernel_no_return(global_item_size);
        // not time effective, but saves speed by resetting back to original
        recalculate_local_workgroups(global_item_size, 0);
    } else {
        uint8_t *window = (uint8_t *)malloc(order*order);
        uint8_t *new_window = (uint8_t *) malloc(order*order);
        int8_t p_back = order/2;
        int8_t start = p_back - order + 1;
        int8_t shift = abs(p_back - order + 1);
        for (int y = 0; y < global_item_size; y++){
            for (int8_t x = start; x <= p_back; x++){
                for (int8_t z = start; z <= p_back; z++){
                    if (y+z*3+image->width*x*3 > global_item_size) // OOB Check
                        continue;
                    window[order*(x+shift)+(z+shift)] = image->image[y+z*image->channels+image->width*x*image->channels];
                }
            }
        ret_img->image[y] = insertion_sort(window, order * order);
        }
        free(window);
        free(new_window);
    }
    return ret_img;
}

K9_Image *convolve(K9_Image *ret_img, K9_Image *image, Kernel kern, bool read){
    size_t global_item_size = image->width * image->height *image->channels;
    if (global.enable_gpu == true){
        char prog[] = "./tools/filters.cl";
        char func[] = "convolve";
        uint16_t filt_id = 145;
        if (image->mem_id == NULL)
            update_input_buffer(image, global_item_size);
        if (ret_img->mem_id == NULL)
            update_output_buffer(ret_img, global_item_size);
        if (ret_img->image == NULL && read)
            ret_img->image = (uint8_t *)malloc(global_item_size);
        read_cl_program(prog, filt_id);
        if (strcmp(global.past_func, func) != 0){
            bind_cl_function(func, filt_id);
            strcpy(global.past_func, func);
        }

        cl_mem kern_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, kern.width * kern.height * sizeof(int16_t), NULL, &global.gpu_values.ret);

        global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, kern_mem_obj, CL_TRUE, 0, kern.width * kern.height * sizeof(int16_t), kern.kernel, 0, NULL, NULL);

        set_main_args(image->mem_id, ret_img->mem_id);
        
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&kern_mem_obj);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_uchar), (void *)&kern.width);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 4, sizeof(cl_int), (void *)&image->width);
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 5, sizeof(cl_uchar), (void *)&image->channels);

        if (read)
            ret_img->image = run_kernel(global_item_size, *ret_img, global_item_size);
        else
            run_kernel_no_return(global_item_size);

        global.gpu_values.ret = clReleaseMemObject(kern_mem_obj);

    } else {
        int8_t p_back = kern.width / 2;
        int8_t start = p_back - kern.width + 1;
        int8_t shift = abs(p_back - kern.width + 1);
        for (int y = 0; y < global_item_size; y++){
            int total = 0;
            for (int8_t x = start; x <= p_back; x++){
                for (int8_t z = start; z <= p_back; z++){
                    if (y+z*image->channels+image->width*x*image->channels < 0)
                        continue;
                    if (y+z*image->channels+image->width*x*image->channels > global_item_size)
                        continue;
                    total += kern.kernel[kern.width*(x+shift)+(z+shift)] * image->image[y+z*image->channels+image->width*x*image->channels];
                }
            }
            total = total > 255 ? 255 : total;
            ret_img->image[y] = total < 0 ? 0 : total;
        }
    }
    return ret_img;
}