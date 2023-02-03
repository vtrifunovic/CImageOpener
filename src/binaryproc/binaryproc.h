#pragma once
#include "../render/render.h"

typedef struct kernel {
    int height;
    int width;
    int16_t *kernel;
} Kernel;

K9_Image *hit_x_miss(K9_Image *ret_img, K9_Image *image, Kernel kern);
K9_Image *bin_dilation(K9_Image *ret_img, K9_Image image, Kernel kern);
K9_Image *bin_erosion(K9_Image *ret_img, K9_Image image, Kernel kern);
K9_Image *thinning(K9_Image *ret_img, K9_Image image);
Kernel create_kernel(int *input, size_t size);
Kernel quick_kernel(int w, int h);