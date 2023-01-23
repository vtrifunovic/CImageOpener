#include <stdio.h>
#include <stdlib.h>
#include "masks.h"
#include "../linmath.h"
#include "../render/render.h"
#include "../global.h"

K9_Image rgb_mask(K9_Image image, int *lower_bound, int *higher_bound){
	// Similar to OpenCV's inRange() function
	if (image.channels == 1){
		fprintf(stderr, "\e[1;33mWarning!\e[0m Function rgb_mask() needs 3 channels, returning unmasked image.\n");
		return image;
	}
	int psize = image.width * image.height;
	K9_Image mask = {
		.channels = 1,
		.name = (char *)malloc(strlen(image.name) + 5),
		.height = image.height,
		.width = image.width,
		.image = (uint8_t *) malloc(psize),
	};
	strcpy(mask.name, "mask_");
    strcat(mask.name, image.name);
	if (global.enable_gpu == true){
		read_cl_program("./masks/rgb_mask_kernel.cl");
		cl_mem lower_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 3 * sizeof(int), NULL, &global.gpu_values.ret);
		cl_mem upper_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 3 * sizeof(int), NULL, &global.gpu_values.ret);

		global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, lower_mem_obj, CL_TRUE, 0, 3 * sizeof(int), lower_bound, 0, NULL, NULL);
		global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, upper_mem_obj, CL_TRUE, 0, 3 * sizeof(int), higher_bound, 0, NULL, NULL);

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 0, sizeof(cl_mem), (void *)&global.gpu_values.input_image);
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 1, sizeof(cl_mem), (void *)&global.gpu_values.output_image);
		
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&lower_mem_obj);
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_mem), (void *)&upper_mem_obj);

		size_t global_item_size = image.width*image.height*image.channels;
		size_t local_item_size = 1;
		global.gpu_values.ret = clEnqueueNDRangeKernel(global.gpu_values.command_queue, global.gpu_values.kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
		global.gpu_values.ret = clEnqueueReadBuffer(global.gpu_values.command_queue, global.gpu_values.output_image, CL_TRUE, 0, global_item_size/3 * sizeof(uint8_t), mask.image, 0, NULL, NULL);
		
	} else {
		for (int x = 0; x < psize; x++){
			for (int c = 0; c < 3; c++){
				if (lower_bound[c] <= image.image[x*3+c] && higher_bound[c] >= image.image[x*3+c])
					mask.image[x] = 255;
			}
		}
	}
	return mask;
}

void bitwiseAnd(K9_Image image, K9_Image mask){
	if (mask.channels > 1){
		fprintf(stderr, "\e[1;33mWarning!\e[0m Mask in bitwiseAnd() has more than one channel\n");
	}
	// Similar to OpenCV's bitwise_and
	int size = image.width * image.height;
	if (image.channels == 3){
		for (int x = 0; x < size; x++){
			if (mask.image[x] != 255){
				image.image[x*3] = 0;
				image.image[x*3+1] = 0;
				image.image[x*3+2] = 0;
			}
		}
	} else {
		for (int x = 0; x < size; x++){
			if (mask.image[x] != 255){
				image.image[x] = 0;
			}
		}
	}
}

void bitwiseNot(K9_Image image, K9_Image mask){
	if (mask.channels > 1){
		fprintf(stderr, "\e[1;33mWarning!\e[0m Mask in bitwiseNot() has more than one channel\n");
	}
	// Similar to OpenCV's bitwise_not
	int size = image.width * image.height;
	if (image.channels == 3){
		for (int x = 0; x < size; x++){
			if (mask.image[x] == 255){
				image.image[x*3] = 0;
				image.image[x*3+1] = 0;
				image.image[x*3+2] = 0;
			}
		}
	} else {
		for (int x = 0; x < size; x++){
			if (mask.image[x] == 255){
				image.image[x] = 0;
			}
		}
	}
}

void grayscale_mask(K9_Image image, int lower_bound, int higher_bound){
	if (image.channels > 1){
		fprintf(stderr, "\e[1;31mError!\e[0m Function grayscale_mask() requires single channel image.\n");
		exit(0);
	}
	int totalpixels = image.width * image.height;
	for (int g = 0; g < totalpixels; g++){
		if (lower_bound <= image.image[g] && higher_bound >= image.image[g])
			image.image[g] = 0;
	}
}