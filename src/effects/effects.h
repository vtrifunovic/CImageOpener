#include "../render/render.h"
#include "../global.h"

K9_Image *pixel_lag(K9_Image *ret_img, K9_Image *image, int chance, int chunk_size, bool read);

K9_Image *color_decay(K9_Image *ret_img, K9_Image *image, int kern, int iterations, bool read);

K9_Image *buffer_kill(K9_Image *ret_img, K9_Image *image, uint8_t buffer_overwrite, bool read);