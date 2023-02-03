#pragma once
#include "../linmath.h"
#include "../render/render.h"
#include "../binaryproc/binaryproc.h"

typedef struct k9_split {
    K9_Image r;
    K9_Image g;
    K9_Image b;
} K9_Split;

K9_Image *blur(K9_Image *ret_img, K9_Image image, Kernel kern, int iterations);
K9_Image *subtract(K9_Image *ret_img, K9_Image *img1, K9_Image *img2);
K9_Image *add(K9_Image *ret_img, K9_Image img1, K9_Image img2);
K9_Split split_channels(K9_Image image);
K9_Image merge_channels(K9_Image r, K9_Image g, K9_Image b);
K9_Image *crop(K9_Image *ret_img, K9_Image image, vec2 xcrop, vec2 ycrop, char *type);
K9_Image blend_img(K9_Image image, K9_Image b, float alpha);
bool compare(K9_Image img1, K9_Image img2);
void K9_free_split(K9_Split image);