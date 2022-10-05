#include <stdio.h>
#include <stdlib.h>
#include "masks.h"
#include "../linmath.h"
#include "../render/render.h"

K9_Image rgb_mask(K9_Image image, vec3 lower_bound, vec3 higher_bound){
	// Similar to OpenCV's inRange() function
	if (image.channels == 1){
		fprintf(stderr, "rgb_mask needs 3 channels\n");
		return image;
	}
	int psize = image.width * image.height;
	K9_Image mask = {
        .channels = 1,
        .name = (char *) malloc(strlen(image.name)+5),
        .height = image.height,
        .width = image.width,
        .image = malloc(psize),
    };
	mask.name = strcpy(mask.name, "mask_");
    mask.name = strcat(mask.name, image.name);
	for (int x = 0; x < psize; x++){
		for (int c = 0; c < 3; c++){
			if (lower_bound[c] <= image.image[x*3+c] && higher_bound[c] >= image.image[x*3+c])
				mask.image[x] = 255;
		}
	}
	return mask;
}

void bitwiseAnd(K9_Image image, K9_Image mask){
	// Similar to OpenCV's bitwise_and
	int size = image.width * image.height;
	for (int x = 0; x < size; x++){
		if (mask.image[x] != 255){
			image.image[x*3] = 0;
			image.image[x*3+1] = 0;
			image.image[x*3+2] = 0;
		}
	}
}

void bitwiseNot(K9_Image image, K9_Image mask){
	// Similar to OpenCV's bitwise_not
	int size = image.width * image.height;
	for (int x = 0; x < size; x++){
		if (mask.image[x] == 255){
			image.image[x*3] = 0;
			image.image[x*3+1] = 0;
			image.image[x*3+2] = 0;
		}
	}
}

void grayscale_mask(K9_Image image, int lower_bound, int higher_bound){
	if (image.channels > 1){
		fprintf(stderr, "Image has not been converted to grayscale\n");
		exit(0);
	}
	int totalpixels = image.width * image.height;
	for (int g = 0; g < totalpixels; g++){
		if (lower_bound <= image.image[g] && higher_bound >= image.image[g])
			image.image[g] = 0;
	}
}