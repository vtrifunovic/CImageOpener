#pragma once
#include "../render/render.h"
#include "../linmath.h"

K9_Image rgb_mask(K9_Image image, int *lower_bound, int *higher_bound);
K9_Image bitwiseAnd(K9_Image image, K9_Image mask);
K9_Image bitwiseNot(K9_Image image, K9_Image mask);
K9_Image grayscale_mask(K9_Image image, uint8_t lower_bound, uint8_t higher_bound);