#pragma once
#include "../render/render.h"
#include "../linmath.h"

/*! @brief Converts the channels in an image to new desired type.
 * K9_RGB2HSV for rgb to hsv.
 * K9_RGB2GRAY for rgb to gray.
 * K9_INVERT for inverted colors.
 * 
 * @param ret_img Image where output will be stored
 * @param image Input image to convert
 * @param type Type of conversion to perform
 * @param read Whether to read data into RAM
 * @returns Image with channels converted to desired type.
 */
K9_Image *convert_channels(K9_Image *ret_img, K9_Image *image, int type, bool read);
/*! @brief Resizes a given image based on a scaling value.
 * Given types: K9_NEAREST (Nearest Neighbor)
 *
 * @param ret_img Return image, where resized image will be stored.
 * @param image Input image that will be resized.
 * @param scale X & Y value for resizing, expressed as a multiple of the image (half size would be 0.5)
 * @param type Resizing method to be used
 * @param[in] read Whether to read the data into RAM
 * @return Resized image based on the scale & type given.
 */
K9_Image *resize_img(K9_Image *ret_img, K9_Image *image, vec2 scale, int type, bool read);