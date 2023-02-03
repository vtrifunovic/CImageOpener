#pragma once
#include "../render/render.h"
#include "../linmath.h"

K9_Image *rgb_to_gray(K9_Image *gray, K9_Image image);
K9_Image *rgb_to_hsv(K9_Image *hsv, K9_Image image);
K9_Image *invert(K9_Image *ret_img, K9_Image image);
K9_Image *resize_img(K9_Image *ret_img, K9_Image image, vec2 scale, char *type);