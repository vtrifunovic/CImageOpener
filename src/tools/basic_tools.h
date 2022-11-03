#pragma once
#include "../render/render.h"
#include "../binaryproc/binaryproc.h"

typedef struct k9_split {
    K9_Image r;
    K9_Image g;
    K9_Image b;
} K9_Split;

K9_Image blur(K9_Image image, Kernel kern, int iterations);
K9_Split split_channels(K9_Image image);
K9_Image merge_channels(K9_Image r, K9_Image g, K9_Image b);
void K9_free_split(K9_Split image);