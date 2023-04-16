#pragma once
#include "../render/render.h"
#include "../linmath.h"

/*! @brief Masks the image based on RGB lower and RGB higher bounds. If the R,G, or B pixel is in the range given, then the output mask contains a 255 or white pixel to indicate that the value was inside the range.
 *
 * @param[in] mask This is the output single channel image where the mask values will be stored
 * @param[in] image Input RGB image that will be masked.
 * @param[in] lower_bound 3 integer array, in RGB order of the lower boundary.
 * @param[in] higher_bound 3 integer array, in RGB order of the upper boundary.
 * @param[in] read Whether to read the data into RAM
 * @returns Masked, single channel image with 0's and 255's
 */
K9_Image *rgb_mask(K9_Image *mask, K9_Image *image, int *lower_bound, int *higher_bound, bool read);
/*! @brief Performs a bitwiseAnd operation between a 3 channel image and a single channel mask. Keeping the RGB pixels of the input image only where the mask is white or 255;
 *
 * @param[in] image The 3 channel image that will be masked
 * @param[in] mask The single channel mask that will be used to determine what pixels to keep
 * @param[in] read Whether to read the data into RAM
 * @return Output 3 channel image where values not in the mask are changed to 0's
 */
K9_Image *bitwiseAnd(K9_Image *ret_img, K9_Image *image, K9_Image *mask, bool read);
/*! @brief Performs a bitwiseNot operation between a 3 channel image and a single channel mask. Keeping the RGB pixels of the input image only where the mask is black or 0.
 *
 * @param[in] image The 3 channel image that will be masked
 * @param[in] mask The single channel mask that will be used to determine what pixels to keep
 * @param[in] read Whether to read the data into RAM
 * @return Output 3 channel image where values that were in the mask are changed to 0's
 */
K9_Image *bitwiseNot(K9_Image *ret_img, K9_Image *image, K9_Image *mask, bool read);
/*! @brief Performs a mask on a grayscale image, keeping pixels that are withing the given bounds.
 *
 * @param image Input image that will be masked. Must be single channel image
 * @param lower_bound The lowest pixel value that is allowed inside the mask
 * @param higher_bound The highest pixel value that is allowed inside the mask
 * @param[in] read Whether to read the data into RAM
 * @return Grayscale image where unwanted pixel values are changed to 0's
 */
K9_Image *grayscale_mask(K9_Image *ret_img, K9_Image *image, uint8_t lower_bound, uint8_t higher_bound, bool read);