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
 * K9_BILINEAR (Bilinear)
 *
 * @param ret_img Return image, where resized image will be stored.
 * @param image Input image that will be resized.
 * @param scale X & Y value for resizing, expressed as a multiple of the image (half size would be 0.5)
 * @param type Resizing method to be used
 * @param[in] read Whether to read the data into RAM
 * @return Resized image based on the scale & type given.
 */
K9_Image *resize_img(K9_Image *ret_img, K9_Image *image, vec2 scale, int type, bool read);
/*! @brief Translates an image x&y amount of pixels. +X axis is left, and -Y axis is down.
 *
 * @param ret_img Return image, where translated image will be stored
 * @param image Input image to be translated
 * @param x Amount of pixels to shift image on X axis.
 * @param y Amount of pixels to shift image on Y axis.
 * @param read Wheter to read the data into RAM
 * @return Translated image on X-Y axis.
 */
K9_Image *translate_image(K9_Image *ret_img, K9_Image *image, int x, int y, bool read);
/*! @brief Rotates an image based on amount of degrees given. +deg for clockwise, -deg for counterclockwise. (Does have accuracy issues with non-90degree shifts)
 *
 * @param ret_img Return image, where rotated image is stored.
 * @param image Input image to be rotated
 * @param deg Amount of degrees to rotate
 * @param rsz Whether to resize image to fit all data. False will crop to original size.
 * @param read Whether to read data into RAM
 * @return Rotated image by deg amount of degrees.
 */
K9_Image *rotate_image(K9_Image *ret_img, K9_Image *image, float deg, bool rsz, bool read);
