#pragma once
#include "../render/render.h"

typedef struct gpu_values {
    cl_int ret;
    cl_context context;
    cl_device_id device_id;
    cl_command_queue command_queue;
    cl_kernel kernel;
    size_t localsize;
    size_t totalpixels;
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
void init_gpu(K9_Image *image);


void read_cl_program(char *path, uint16_t sid);
void bind_cl_function(char *function, uint16_t sid);
void update_output_buffer(K9_Image *image, size_t totalsize);
void update_input_buffer(K9_Image *image, int totalpixels);
void set_main_args(cl_mem input, cl_mem output);
uint8_t *run_kernel(size_t global_item_size, K9_Image ret_img, size_t return_size);
void run_kernel_no_return(size_t global_item_size);

/*! @brief Reads the image buffer stored on the GPU back into Ram.
 *  
 * @param[in] image The image whose data will be read back into memory
 * @return Same image, but with updated image data stored in Ram.
 */
uint8_t *read_mem_buffer(K9_Image image);

void recalculate_local_workgroups(size_t size, int override);

// Frees the stored gpu values. Include at the end of program.
void K9_free_gpu(void);