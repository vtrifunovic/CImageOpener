#pragma once
#include "../render/render.h"
#include "../linmath.h"

K9_Image rgb_mask(K9_Image image, int *lower_bound, int *higher_bound);
void bitwiseAnd(K9_Image image, K9_Image mask);
void bitwiseNot(K9_Image image, K9_Image mask);
void grayscale_mask(K9_Image image, int lower_bound, int higher_bound);