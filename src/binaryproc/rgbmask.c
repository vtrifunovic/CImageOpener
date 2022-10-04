#include <stdio.h>
#include "rgbmask.h"
#include "../linmath.h"
#include "../render/render.h"

void rgb_mask(K9_Image image, vec3 lower_bound, vec3 higher_bound){
	// Similar to OpenCV's inRange() function
	// test position of pixel to determine bound
	long totalpixels = image.width * image.height * 3;
	for (int c = 0; c < 3; c++){
		for (int x = 0; x < totalpixels; x+=3){
			if (lower_bound[c] < image.image[x+c] && higher_bound[c] > image.image[x+c])
				image.image[x+c] = 0;
		}
	}
}
