#include "gpu_setup.h"
#include "../global.h"
#include <stdio.h>
#include <string.h>

#define MAX_SOURCE_SIZE (0x100000)

GPU_Values g;

static void set_local_workgroup(int siz){
    for (int i = CL_DEVICE_LOCAL_MEM_SIZE/8; i > 0; i--){
        if (siz % i == 0){
            g.localsize = i;
            return;
        }
    }
    fprintf(stderr, "Couldn't find local group size... terminating.\n");
    exit(0);
}

void init_gpu(K9_Image image){
    char prog[] = "init";
    strcpy(global.past_prog, prog);
    strcpy(global.past_func, prog);
    cl_platform_id platform_id = NULL;
    g.device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    global.gpu_values.ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    global.gpu_values.ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &g.device_id, &ret_num_devices);
    global.gpu_values.context = clCreateContext(NULL, 1, &g.device_id, NULL, NULL, &global.gpu_values.ret);
    global.gpu_values.command_queue = clCreateCommandQueue(global.gpu_values.context,g.device_id, 0, &global.gpu_values.ret);
    global.totalsize = image.width * image.height * image.channels;
    // Note to self: There's two of these in use
    set_local_workgroup(global.totalsize);
    g.input_image = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, global.totalsize * sizeof(uint8_t), NULL, &global.gpu_values.ret);
    g.output_image = clCreateBuffer(global.gpu_values.context, CL_MEM_WRITE_ONLY, global.totalsize * sizeof(uint8_t), NULL, &global.gpu_values.ret);
    global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, g.input_image, CL_TRUE, 0, global.totalsize * sizeof(uint8_t), image.image, 0, NULL, NULL);
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
    source_str = realloc(source_str, strlen(source_str) * sizeof(char)+1);
    fclose(fp);

   g.program = clCreateProgramWithSource(global.gpu_values.context, 1, (const char **)&source_str, (const size_t *)&source_size, &global.gpu_values.ret);
   global.gpu_values.ret = clBuildProgram(g.program, 1, &g.device_id, NULL, NULL, NULL);

   free(source_str);
}

void bind_cl_function(char *function){
   global.gpu_values.kernel = clCreateKernel(g.program, function, &global.gpu_values.ret);
}

void update_gpu_channels(K9_Image image, int totalpixels){
    global.totalsize = totalpixels;
    global.gpu_values.ret = clReleaseMemObject(g.input_image);
    g.input_image = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, totalpixels * sizeof(uint8_t), NULL, &global.gpu_values.ret);
    set_local_workgroup(totalpixels);
    global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, g.input_image, CL_TRUE, 0, totalpixels * sizeof(uint8_t), image.image, 0, NULL, NULL);
}

void set_main_args(void){
    global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 0, sizeof(cl_mem), (void *)&g.input_image);
    global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 1, sizeof(cl_mem), (void *)&g.output_image);
}

uint8_t *run_kernel(size_t global_item_size, K9_Image ret_img, size_t return_size){
    global.gpu_values.ret = clEnqueueNDRangeKernel(global.gpu_values.command_queue, global.gpu_values.kernel, 1, NULL, &global_item_size, &g.localsize, 0, NULL, NULL);

    global.gpu_values.ret = clEnqueueReadBuffer(global.gpu_values.command_queue, g.output_image, CL_TRUE, 0, return_size * sizeof(uint8_t), ret_img.image, 0, NULL, NULL);

    return ret_img.image;
}