#include "filters.h"
#include "../typename.h"
#include <math.h>
#include <string.h>
#include <malloc.h>

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

static int calculate_divisor(Kernel kern){
    int sum = 0;
    for (int i = 0; i < kern.width*kern.height; i++){
        sum += kern.kernel[i];
    }
    return sum == 0 ? 1 : sum;
}

K9_Image *gaussian_blur(K9_Image *ret_img, K9_Image *image, Kernel kern, bool read, int divisor){
    size_t global_item_size = image->width * image->height * image->channels;
    if (divisor == 0xFFFFFF)
        divisor = calculate_divisor(kern);
        
    if (global.enable_gpu == true){
        char prog[] = "./tools/filters.cl";
        char func[] = "g_blur";
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
        global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 6, sizeof(cl_int), (void *)&divisor);

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
            total = total/divisor > 255 ? 255 : total/divisor;
            ret_img->image[y] = total < 0 ? 0 : total;
        }
    }
    return ret_img;
}

static void df_search(K9_Image *image, char *searched, int x, Contour *first, int type){
    if (x > image->width*image->height || x < 0) // OOB mem check
        return;
    searched[x] = 'c';
    first->pixels[first->length] =  x;
    first->length += 1;
    // N4 Search
    if (image->image[x+1] == 255 && searched[x+1] == 'x')
        df_search(image, searched, x+1, first, type);
    if (image->image[x-1] == 255 && searched[x-1] == 'x')
        df_search(image, searched, x-1, first, type);
    if (image->image[x-image->width] == 255 && searched[x-image->width] == 'x')
        df_search(image, searched, x-image->width, first, type);
    if (image->image[x+image->width] == 255 && searched[x+image->width] == 'x')
        df_search(image, searched, x+image->width, first, type);
    // N8 Search
    if (type == K9_N8){
        if (image->image[x+image->width+1] == 255 && searched[x+image->width+1] == 'x')
            df_search(image, searched, x+image->width+1, first, type);        
        if (image->image[x+image->width-1] == 255 && searched[x+image->width-1] == 'x')
            df_search(image, searched, x+image->width-1, first, type);
        if (image->image[x-image->width+1] == 255 && searched[x-image->width+1] == 'x')
            df_search(image, searched, x-image->width+1, first, type);
        if (image->image[x-image->width-1] == 255 && searched[x-image->width-1] == 'x')
            df_search(image, searched, x-image->width-1, first, type);
    } 
}
// 1,166,400
// 174,569

static Contour *dfs_new_contour(K9_Image *image, char *searched, int x, Contour *first, int type){
    size_t totalpixels = image->width * image->height * image->channels;
    Contour *link = (Contour *)malloc(sizeof(Contour));
    link->length = 0;
    link->pixels = (int *)malloc(totalpixels * sizeof(int));
    link->next = first;
    first = link;
    df_search(image, searched, x, first, type);
    first->pixels = (int *)realloc(first->pixels, first->length*sizeof(int));
    return first;
}

Contour *detect_contours(K9_Image *image, int type, bool debug){
    if (image->channels > 1){
        fprintf(stderr, "\e[1;31mError!\e[0m Function detect_contours() only accepts single channel images\n");
        exit(0);
    }
    Contour *first = NULL;
    size_t totalpixels = image->width * image->height * image->channels;
    char *searched = (char *)malloc(totalpixels);
    memset(searched, 'x', totalpixels);
    int total_contours = 0;
    for (int x = 0; x < totalpixels; x++){
        if (debug)
            printf("Searching pixel[\e[1;36m%d\e[0m]: \tTotal Contours: %d ", x, total_contours);
        if (image->image[x] != 255){
            if (debug)
                printf("\t\e[1;31mNot Valid!\e[0m\n");
            continue;
        }
        if (searched[x] != 'x'){
            if (debug)
                printf("\t\e[1;33mPixel has already been searched\e[0m\n");
            continue;
        }
        if (debug)
            printf("\t\e[1;32mNew contour found!\e[0m\n");
        first = dfs_new_contour(image, searched, x, first, type);
        total_contours++;
    }
    free(searched);

    if (debug)
        printf("Total contours found: %d\n", total_contours);
    return first;
}

K9_Image *viz_contour_by_index(K9_Image *original, int index, Contour *first){
    size_t totalpixels = original->width * original->height * original->channels;
    //memset(original->image, 0, totalpixels);
    int cnt = 0;
    while (first->next != NULL && cnt != index){
        first = first->next;
        cnt++;
    }
    printf("Reconstructing contour \e[1;36m%d\e[0m\tSize: %d\n", cnt, first->length);
    for (int i = 0; i < first->length; i++){
        original->image[first->pixels[i]] = 255;
    }
    return original;
}