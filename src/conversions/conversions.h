#pragma once
#include "../render/render.h"
#include "../linmath.h"

K9_Image rgb_to_gray(K9_Image image);
K9_Image rgb_to_hsv(K9_Image image);
void invert(K9_Image image);