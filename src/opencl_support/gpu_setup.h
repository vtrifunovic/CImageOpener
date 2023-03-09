#pragma once
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>
#include "../render/render.h"

typedef struct gpu_values {
    cl_int ret;
    cl_mem input_image;
    cl_mem output_image;
    cl_context context;
    cl_device_id device_id;
    cl_command_queue command_queue;
    cl_kernel kernel;
    size_t localsize;
} GPU_Values;

struct node {
    uint16_t id;
    cl_program prog;
    struct node *next;
};

/*! @brief Initializes gpu processing. Uses any type of K9_Image struct (can be empty one) to run OpenCL boilerplate and initialize some buffers. These buffers will later self-readjust, but this gets them started.
 * 
 * @param[in] image Needs any type of image to initialize the buffers.
 */
void init_gpu(K9_Image image);


void read_cl_program(char *path, uint16_t sid);
void bind_cl_function(char *function, uint16_t sid);
void update_gpu_channels(K9_Image image, int totalpixels);
void set_main_args(void);
uint8_t *run_kernel(size_t global_item_size, K9_Image ret_img, size_t divisor);

// Frees the stored gpu values. Include at the end of program.
void K9_free_gpu(void);