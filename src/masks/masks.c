#include <stdio.h>
#include <stdlib.h>
#include "masks.h"
#include "../linmath.h"
#include "../render/render.h"
#include "../global.h"

K9_Image *rgb_mask(K9_Image *mask, K9_Image *image, int *lower_bound, int *higher_bound, bool read){
	if (image->channels == 1){
		fprintf(stderr, "\e[1;33mWarning!\e[0m Function rgb_mask() needs 3 channels, returning unmasked image.\n");
		return mask;
	}
	if (global.enable_gpu == true){
		char prog[] = "./masks/rgb_mask_kernel.cl"; 
		char func[] = "rgb_mask";
		uint16_t mask_id = 340;
		
		mem_check_gpu(image, mask, prog, func, mask_id, image->tp, read);

		cl_mem lower_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 3 * sizeof(int), NULL, &global.gpu_values.ret);
		cl_mem upper_mem_obj = clCreateBuffer(global.gpu_values.context, CL_MEM_READ_ONLY, 3 * sizeof(int), NULL, &global.gpu_values.ret);

		global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, lower_mem_obj, CL_TRUE, 0, 3 * sizeof(int), lower_bound, 0, NULL, NULL);
		global.gpu_values.ret = clEnqueueWriteBuffer(global.gpu_values.command_queue, upper_mem_obj, CL_TRUE, 0, 3 * sizeof(int), higher_bound, 0, NULL, NULL);

		set_main_args(image->mem_id, mask->mem_id);

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&lower_mem_obj);
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_mem), (void *)&upper_mem_obj);

		if (read)
			mask->image = run_kernel(image->tp, *mask, mask->tp, false);
		else
			run_kernel_no_return(image->tp);

		global.gpu_values.ret = clReleaseMemObject(upper_mem_obj);
		global.gpu_values.ret = clReleaseMemObject(lower_mem_obj);
	} else {
		for (int x = 0; x < mask->tp; x++){
			for (int c = 0; c < 3; c++){
				if (lower_bound[c] <= image->image[x*3+c] && higher_bound[c] >= image->image[x*3+c])
					mask->image[x] = 255;
			}
		}
	}
	return mask;
}

K9_Image *bitwiseAnd(K9_Image *ret_img, K9_Image *image, K9_Image *mask, bool read){
	// might need to work on 1:1 channel masks. May be broken with GPU update
	if (mask->channels > 1){
		fprintf(stderr, "\e[1;33mWarning!\e[0m Mask in bitwiseAnd() has more than one channel\n");
	}
	// Similar to OpenCV's bitwise_and
	if (global.enable_gpu == true){
		uint16_t mask_id = 340;
		char prog[] = "./masks/rgb_mask_kernel.cl";
		char func[] = "bitwiseAnd";
		size_t global_item_size = image->width * image->height * image->channels;
		if (image->mem_id == NULL)
			update_input_buffer(image);
		if (ret_img->mem_id == NULL)
			update_output_buffer(ret_img);
		if (mask->mem_id == NULL)
			update_input_buffer(mask);
		if (ret_img->image == NULL && read)
			ret_img->image = (uint8_t *)malloc(ret_img->tp);
		read_cl_program(prog, mask_id);
		if (strcmp(global.past_func, func) != 0){
			bind_cl_function(func, mask_id);
			strcpy(global.past_func, func);
		}

		set_main_args(image->mem_id, ret_img->mem_id);

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&mask->mem_id);

		ret_img->image = run_kernel(ret_img->tp, *ret_img, ret_img->tp, false);

		if (read)
			ret_img->image = run_kernel(ret_img->tp, *ret_img, ret_img->tp, false);
		else
			run_kernel_no_return(ret_img->tp);
	} else {
		if (image->channels == 3){
			for (int x = 0; x < image->tp/3; x++){
				ret_img->image[x * 3] = image->image[x * 3] * mask->image[x]/255;
				ret_img->image[x*3+1] = image->image[x*3+1] * mask->image[x]/255;
				ret_img->image[x*3+2] = image->image[x*3+2] * mask->image[x]/255;
			}
		} else {
			for (int x = 0; x < image->tp; x++){
				ret_img->image[x] = image->image[x] * mask->image[x] / 255;
			}
		}
	}
	return ret_img;
}

K9_Image *bitwiseNot(K9_Image *ret_img, K9_Image *image, K9_Image *mask, bool read){
	// might need to work on 1:1 channel masks. May be broken with GPU update
	if (mask->channels > 1){
		fprintf(stderr, "\e[1;33mWarning!\e[0m Mask in bitwiseNot() has more than one channel\n");
	}
	// Similar to OpenCV's bitwise_not
	int size = image->width * image->height;
	if (global.enable_gpu == true){
		char prog[] = "./masks/rgb_mask_kernel.cl";
		char func[] = "bitwiseNot";
		uint16_t mask_id = 340;
		if (image->mem_id == NULL)
			update_input_buffer(image);
		if (mask->mem_id == NULL)
			update_input_buffer(mask);
		if (ret_img->mem_id == NULL)
			update_output_buffer(ret_img);
		if (ret_img->image == NULL && read)
			ret_img->image = (uint8_t *)malloc(ret_img->tp);
		read_cl_program(prog, mask_id);
		if (strcmp(global.past_func, func) != 0){
			bind_cl_function(func, mask_id);
			strcpy(global.past_func, func);
		}

		set_main_args(image->mem_id, ret_img->mem_id);

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_mem), (void *)&mask->mem_id);

		if (read)
			ret_img->image = run_kernel(ret_img->tp, *ret_img, ret_img->tp, false);
		else
			run_kernel_no_return(ret_img->tp);
	} else {
		if (image->channels == 3){
			for (int x = 0; x < size; x++){
				if (mask->image[x] == 255){
					ret_img->image[x*3] = 0;
				} else {
					ret_img->image[x * 3] = 	image->image[x * 3];
					ret_img->image[x * 3 + 1] = image->image[x * 3 + 1];
					ret_img->image[x * 3 + 2] = image->image[x * 3 + 2];
				}
			}
		} else {
			for (int x = 0; x < size; x++){
				if (mask->image[x] != 255){
					ret_img->image[x] = image->image[x];
				}
			}
		}
	}
	return ret_img;
}

K9_Image *grayscale_mask(K9_Image *ret_img, K9_Image *image, uint8_t lower_bound, uint8_t higher_bound, bool read){
	if (image->channels > 1){
		fprintf(stderr, "\e[1;31mError!\e[0m Function grayscale_mask() requires single channel image.\n");
		exit(0);
	}
	if (global.enable_gpu == true){
		char prog[] = "./masks/rgb_mask_kernel.cl";
		char func[] = "grayscale_mask";
		uint16_t mask_id = 340;

		mem_check_gpu(image, ret_img, prog, func, mask_id, ret_img->tp, read);

		set_main_args(image->mem_id, ret_img->mem_id);

		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 2, sizeof(cl_uchar), (void *)&lower_bound);
		global.gpu_values.ret = clSetKernelArg(global.gpu_values.kernel, 3, sizeof(cl_uchar), (void *)&higher_bound);

		ret_img->image = run_kernel(ret_img->tp, *ret_img, ret_img->tp, false);
		if (read)
			ret_img->image = run_kernel(ret_img->tp, *ret_img, ret_img->tp, false);
		else
			run_kernel_no_return(ret_img->tp);
	} else {
		for (int g = 0; g < ret_img->tp; g++){
			if (lower_bound <= image->image[g] && higher_bound >= image->image[g])
				ret_img->image[g] = 255;
		}
	}
	return ret_img;
}