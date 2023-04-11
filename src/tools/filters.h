#include "../render/render.h"
#include "../global.h"

K9_Image *median_filter(K9_Image *ret_img, K9_Image *image, uint8_t order, bool read);