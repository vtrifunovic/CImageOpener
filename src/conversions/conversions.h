#pragma once
#include "../render/render.h"
#include "../linmath.h"

/*! @brief Creates a single channel grayscale image from an 3 channel RGB image
 *
 * @param gray The output grayscale image
 * @param image The input RGB image. Must be 3 channels
 * @param[in] read Whether to read the data into RAM
 * @return A single channel grayscaled image.
 */
K9_Image *rgb_to_gray(K9_Image *gray, K9_Image *image, bool read);
/*! @brief Converts a RGB Image into its HSV (hue/saturation/value) values.
 *
 * @param hsv Output HSV image
 * @param image Input RGB image to be converted to HSV
 * @param[in] read Whether to read the data into RAM
 * @return HSV image. The values will be stored in VSH order.
 */
K9_Image *rgb_to_hsv(K9_Image *hsv, K9_Image *image, bool read);
/*! @brief Inverts the image. Negative effect in most editing software.
 *
 * @param ret_img Return image where inverted image will be stored.
 * @param image Input image that will be inverted
 * @param[in] read Whether to read the data into RAM
 * @return Inverted image, 255-(current pixel value).
 */
K9_Image *invert(K9_Image *ret_img, K9_Image *image, bool read);
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