#pragma once
#include "../render/render.h"

typedef struct kernel {
    int height;
    int width;
    int *kernel;
} Kernel;

K9_Image hit_x_miss(K9_Image, Kernel kern);
K9_Image bin_dilation(K9_Image image, Kernel kern);
K9_Image bin_erosion(K9_Image image, Kernel kern);
Kernel create_kernel(int *input, size_t size);