#include "gpu_setup.h"
#include "../global.h"
#include <stdio.h>
#include <string.h>

#define MAX_SOURCE_SIZE (0x100000)

void init_gpu(K9_Image image){
    cl_platform_id platform_id = NULL;
    global.gpu_values.device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    global.gpu_values.ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    global.gpu_values.ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &global.gpu_values.device_id, &ret_num_devices);
    global.gpu_values.context = clCreateContext(NULL, 1, &global.gpu_values.device_id, NULL, NULL, &global.gpu_values.ret);
    global.gpu_values.command_queue = clCreateCommandQueue(global.gpu_values.context, global.gpu_values.device_id, 0, &global.gpu_values.ret);
    int totalpixels = image.width * image.height * image.channels;
    global.gpu_values.input_image = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, totalpixels * sizeof(uint8_t), NULL, &global.gpu_values.ret);
    global.gpu_values.output_image = clCreateBuffer(global.gpu_values.context, CL_MEM_WRITE_ONLY, totalpixels * sizeof(uint8_t), NULL, &global.gpu_values.ret);
    global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, global.gpu_values.input_image, CL_TRUE, 0, totalpixels * sizeof(uint8_t), image.image, 0, NULL, NULL);
}
void read_cl_program(char *path){
    FILE *fp;
    char *source_str;
    size_t source_size;
    fp = fopen(path, "r");
    if (!fp){
        fprintf(stderr, "\e[1;31mError!\e[0m Failed to load kernel %s.\n", path);
        exit(1);
    }
    source_str = (char *)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    source_str = realloc(source_str, strlen(source_str) * sizeof(char));
    fclose(fp);

    global.gpu_values.program = clCreateProgramWithSource(global.gpu_values.context, 1, (const char **)&source_str, (const size_t *)&source_size, &global.gpu_values.ret);
    global.gpu_values.ret = clBuildProgram(global.gpu_values.program, 1, &global.gpu_values.device_id, NULL, NULL, NULL);
    global.gpu_values.kernel = clCreateKernel(global.gpu_values.program, "rgb_mask", &global.gpu_values.ret);
    free(source_str);
}