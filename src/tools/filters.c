#include "filters.h"
#include <math.h>
#include <string.h>

struct contour *first = NULL;
struct contour *now = NULL;

int total_contours = 0;

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

static bool n4_check(int pix, int p1, int p2, int p3, int p4){
    return (pix == p1 || pix == p2 || pix == p3 || pix == p4);
}

static int find_existing_contours(int x, int width){
    int count = 0;
    struct contour *now = first;
    while (now != NULL){
        for (int i = 0; i < now->length; i++){
            //printf("Pixel[%d]: %d, x: %d\n", i, now->pixels[i], x);
            if (n4_check(now->pixels[i], x+1, x-1, x+width, x-width)){
                count++;
                i = now->length;
            }
        }
        now = now->next;
    }
    return count;
}

static void append_to_existing(int x, int width){
    //printf("Appending pixel: %d\n", x);
    struct contour *now = first;
    while (now != NULL){
        for (int i = 0; i < now->length; i++){
            if (n4_check(now->pixels[i], x+1, x-1, x+width, x-width)){
                now->pixels = realloc(now->pixels, (now->length+1)*sizeof(int));
                now->length += 1;
                now->pixels[now->length-1] = x;
                return;
            }
        }
        now = now->next;
    }
    fprintf(stderr, "Pixel[%d] found but not appended!!\n", x);
}

static void create_new_contour(int pixel){
    total_contours++;
    struct contour *link = (struct contour *)malloc(sizeof(struct contour));
    link->length = 1;
    link->pixels = (int *)malloc(1*sizeof(int));
    link->pixels[0] = pixel;
    //printf("Starting pixel: %d\n", pixel);
    link->next = first;
    first = link;
}

static void combine_contour_list(int x, int width){
    struct contour *now = first;
    struct contour *keep = NULL;
    struct contour *prev = NULL;
    while (now != NULL){
        for (int i = 0; i < now->length; i++){
            if (n4_check(now->pixels[i], x+1, x-1, x+width, x-width) && keep == NULL){
                keep = now;
                i = now->length;
            } else if (n4_check(now->pixels[i], x+1, x-1, x+width, x-width)){
                total_contours--;
                keep->pixels = realloc(keep->pixels, (keep->length+now->length+1)*sizeof(int));
                for (int x = 0; x < now->length; x++){
                    keep->pixels[x+keep->length-1] = now->pixels[x];
                }
                keep->length += now->length;
                keep->pixels[keep->length-1] = x;
                if (now->next == NULL){
                    prev->next = NULL;
                    free(now);
                    return;
                }
                struct contour *temp_node = prev->next;
                prev->next = temp_node->next;
                free(temp_node);
                return;
            }
        }
        prev = now;
        now = now->next;
    }
}

void detect_contours(K9_Image *image, bool debug){
    size_t totalpixels = image->width * image->height * image->channels;
    for (int x = 0; x < totalpixels; x++){
        if (debug)
            printf("Scanning pixel[\e[1;36m%d\e[0m]: \t", x);
        if (image->image[x] != 255){
            if (debug)
                printf("\e[1;31mNot valid!\e[0m\n");
            continue;
        }
        if (!n4_check(image->image[x], image->image[x-1], image->image[x+1], image->image[x+image->width], image->image[x - image->width])){
            if (debug)
                printf("\e[1;31mNot valid!\e[0m\n");
            continue;
        }
        int finds = find_existing_contours(x, image->width);
        if (debug)
            printf("Found \e[1;32m%d\e[0m matches...\n", finds);
        if (finds == 0)
            create_new_contour(x);
        else if (finds == 1)
            append_to_existing(x, image->width);
        else if (finds > 1)
            combine_contour_list(x, image->width);
    }
    if (debug)
        printf("Total contours found: %d\n", total_contours);
}

K9_Image *viz_contour_by_index(K9_Image *original, int index){
    //printf("Vizualizing contour %d\n", index);
    size_t totalpixels = original->width * original->height * original->channels;
    memset(original->image, 0, totalpixels);
    struct contour *fc = first;
    int cnt = 0;
    while (fc->next != NULL && cnt != index){
        fc = fc->next;
        cnt++;
    }
    for (int i = 0; i < fc->length; i++){
        original->image[fc->pixels[i]] = 255;
    }
    return original;
}