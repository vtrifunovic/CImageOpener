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
		.name = (char *)malloc(5),
		.height = image.height,
		.width = image.width,
		.image = (uint8_t *) malloc(psize),
	};
	strcpy(mask.name, "mask");
	if (global.enable_gpu == true){
		char prog[] = "./masks/rgb_mask_kernel.cl"; 
		char func[] = "rgb_mask";
		size_t global_item_size = image.width * image.height * image.channels;
		size_t local_item_size = 128;
		if (image.channels != global.channels){
			update_gpu_channels(image, global_item_size);
			global.channels = image.channels;
		}
		if (strcmp(global.past_prog, prog) != 0){
			strcpy(global.past_prog, prog);
			read_cl_program(prog);
		}
		if (strcmp(global.past_func, func) != 0){
			bind_cl_function(func);
			strcpy(global.past_func, func);
		}
		cl_mem lower_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 3 * sizeof(int), NULL, &global.gpu_values.ret);
		cl_mem upper_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 3 * sizeof(int), NULL, &global.gpu_values.ret);

		global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, lower_mem_obj, CL_TRUE, 0, 3 * sizeof(int), lower_bound, 0, NULL, NULL);
		global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, upper_mem_obj, CL_TRUE, 0, 3 * sizeof(int), higher_bound, 0, NULL, NULL);

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 0, sizeof(cl_mem), (void *)&global.gpu_values.input_image);
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 1, sizeof(cl_mem), (void *)&global.gpu_values.output_image);
		
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&lower_mem_obj);
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_mem), (void *)&upper_mem_obj);

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

K9_Image bitwiseAnd(K9_Image image, K9_Image mask){
	// might need to work on 1:1 channel masks. May be broken with GPU update
	if (mask.channels > 1){
		fprintf(stderr, "\e[1;33mWarning!\e[0m Mask in bitwiseAnd() has more than one channel\n");
	}
	// Similar to OpenCV's bitwise_and
	int size = image.width * image.height;
	K9_Image ret_img = {
		.channels = 3,
		.name = (char *)malloc(6),
		.height = image.height,
		.width = image.width,
		.image = (uint8_t *)malloc(size*3),
	};
	strcpy(ret_img.name, "bwAnd");
	if (global.enable_gpu == true){
		char prog[] = "./masks/rgb_mask_kernel.cl";
		char func[] = "bitwiseAnd";
		size_t global_item_size = image.width * image.height * image.channels;
		size_t local_item_size = 128;
		if (image.channels != global.channels){
			update_gpu_channels(image, global_item_size);
			global.channels = image.channels;
		}
		if (strcmp(global.past_prog, prog) != 0){
			strcpy(global.past_prog, prog);
			read_cl_program(prog);
		}
		if (strcmp(global.past_func, func) != 0){
			bind_cl_function(func);
			strcpy(global.past_func, func);
		}
		cl_mem mask_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, size * sizeof(uint8_t), NULL, &global.gpu_values.ret);
		global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, mask_mem_obj, CL_TRUE, 0, size * sizeof(uint8_t), mask.image, 0, NULL, NULL);

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 0, sizeof(cl_mem), (void *)&global.gpu_values.input_image);
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 1, sizeof(cl_mem), (void *)&global.gpu_values.output_image);

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&mask_mem_obj);
		global.gpu_values.ret = clEnqueueNDRangeKernel(global.gpu_values.command_queue, global.gpu_values.kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
		global.gpu_values.ret = clEnqueueReadBuffer(global.gpu_values.command_queue, global.gpu_values.output_image, CL_TRUE, 0, global_item_size * sizeof(uint8_t), ret_img.image, 0, NULL, NULL);
	} else {
		if (image.channels == 3){
			for (int x = 0; x < size; x++){
				if (mask.image[x] == 255){
					ret_img.image[x*3] = image.image[x*3];
					ret_img.image[x*3+1] = image.image[x*3+1];
					ret_img.image[x*3+2] = image.image[x*3+2];
				}
			}
		} else {
			for (int x = 0; x < size; x++){
				if (mask.image[x] == 255){
					ret_img.image[x] = image.image[x];
				}
			}
		}
	}
	return ret_img;
}

K9_Image bitwiseNot(K9_Image image, K9_Image mask){
	// might need to work on 1:1 channel masks. May be broken with GPU update
	if (mask.channels > 1){
		fprintf(stderr, "\e[1;33mWarning!\e[0m Mask in bitwiseNot() has more than one channel\n");
	}
	// Similar to OpenCV's bitwise_not
	int size = image.width * image.height;
	K9_Image ret_img = {
		.channels = 3,
		.name = (char *)malloc(6),
		.height = image.height,
		.width = image.width,
		.image = (uint8_t *)malloc(size * 3),
	};
	strcpy(ret_img.name, "bwNot");
	if (global.enable_gpu == true)
	{
		char prog[] = "./masks/rgb_mask_kernel.cl";
		char func[] = "bitwiseNot";
		size_t global_item_size = image.width * image.height * image.channels;
		size_t local_item_size = 128;
		if (image.channels != global.channels){
			update_gpu_channels(image, global_item_size);
			global.channels = image.channels;
		}
		if (strcmp(global.past_prog, prog) != 0){
			strcpy(global.past_prog, prog);
			read_cl_program(prog);
		}
		if (strcmp(global.past_func, func) != 0){
			bind_cl_function(func);
			strcpy(global.past_func, func);
		}
		cl_mem mask_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, size * sizeof(uint8_t), NULL, &global.gpu_values.ret);
		global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, mask_mem_obj, CL_TRUE, 0, size * sizeof(uint8_t), mask.image, 0, NULL, NULL);

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 0, sizeof(cl_mem), (void *)&global.gpu_values.input_image);
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 1, sizeof(cl_mem), (void *)&global.gpu_values.output_image);

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&mask_mem_obj);

		global.gpu_values.ret = clEnqueueNDRangeKernel(global.gpu_values.command_queue, global.gpu_values.kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
		global.gpu_values.ret = clEnqueueReadBuffer(global.gpu_values.command_queue, global.gpu_values.output_image, CL_TRUE, 0, global_item_size * sizeof(uint8_t), ret_img.image, 0, NULL, NULL);
	} else {
		if (image.channels == 3)
		{
			for (int x = 0; x < size; x++)
			{
				if (mask.image[x] != 255)
				{
					ret_img.image[x * 3] = image.image[x * 3];
					ret_img.image[x * 3 + 1] = image.image[x * 3 + 1];
					ret_img.image[x * 3 + 2] = image.image[x * 3 + 2];
				}
			}
		}
		else
		{
			for (int x = 0; x < size; x++)
			{
				if (mask.image[x] != 255)
				{
					image.image[x] = 0;
				}
			}
		}
	}
	return ret_img;
}

K9_Image grayscale_mask(K9_Image image, uint8_t lower_bound, uint8_t higher_bound){
	if (image.channels > 1){
		fprintf(stderr, "\e[1;31mError!\e[0m Function grayscale_mask() requires single channel image.\n");
		exit(0);
	}
	int totalpixels = image.width * image.height;
	K9_Image ret_img = {
		.channels = 1,
		.name = (char *)malloc(6),
		.height = image.height,
		.width = image.width,
		.image = (uint8_t *)malloc(totalpixels),
	};
	strcpy(ret_img.name, "gMask");
	if (global.enable_gpu == true){
		char prog[] = "./masks/rgb_mask_kernel.cl";
		char func[] = "grayscale_mask";
		size_t global_item_size = image.width * image.height;
		size_t local_item_size = 128;
		if (image.channels != global.channels){
			update_gpu_channels(image, global_item_size);
			global.channels = image.channels;
		}
		if (strcmp(global.past_prog, prog) != 0){
			strcpy(global.past_prog, prog);
			read_cl_program(prog);
		}
		if (strcmp(global.past_func, func) != 0){
			bind_cl_function(func);
			strcpy(global.past_func, func);
		}

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 0, sizeof(cl_mem), (void *)&global.gpu_values.input_image);
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 1, sizeof(cl_mem), (void *)&global.gpu_values.output_image);

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_uchar), (void *)&lower_bound);
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_uchar), (void *)&higher_bound);

		global.gpu_values.ret = clEnqueueNDRangeKernel(global.gpu_values.command_queue, global.gpu_values.kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

		global.gpu_values.ret = clEnqueueReadBuffer(global.gpu_values.command_queue, global.gpu_values.output_image, CL_TRUE, 0, global_item_size * sizeof(uint8_t), ret_img.image, 0, NULL, NULL);
	} else {
		for (int g = 0; g < totalpixels; g++){
			if (lower_bound <= image.image[g] && higher_bound >= image.image[g])
				ret_img.image[g] = image.image[g];
		}
	}
	return ret_img;
}