#pragma once
#include "../render/render.h"

typedef struct kernel {
    int height;
    int width;
    int16_t *kernel;
} Kernel;

/*! @brief Performs the hit-miss operation. Values > 0 on the kernel are considered as 1's in the typical hit-miss, 0's are 0's and values < 0 are considered x's or don't care values. More info: https://en.wikipedia.org/wiki/Hit-or-miss_transform
 *
 * @param ret_img Return image, stores the output of the hit miss operation
 * @param image Input image that the hit miss operation gets ran on
 * @param kern Input kernel, this determines what pixels are hits and which are miss
 * @return Single channel image where 255's are hits and 0's are misses.
 */
K9_Image *hit_x_miss(K9_Image *ret_img, K9_Image *image, Kernel kern, bool read);
/*! @brief Performs binary dilation. Dilates based on the shape of the kernel. Values > 0 dilation is performed, values < 0 are ignored, values = 0 the original value of the image is preserved.
 *
 * @param ret_img Return image, stores the output of the dilation
 * @param image Input image that the dilation is performed on
 * @param kern The input kernel used for dilation
 * @return Single channel image of 255's and 0's.
 */
K9_Image *bin_dilation(K9_Image *ret_img, K9_Image *image, Kernel kern, bool read);
/*! @brief Performs binary erosion. Erodes based on the shape of the kernel.
 * Values >= 0 are set to 0, values < 0 are ignored.
 *
 * @param ret_img Return image, stores the output of the erosion
 * @param image Input image that the erosion is performed on
 * @param kern The input kernel used for erosion
 * @return Single channel image of 255's and 0's.
 */
K9_Image *bin_erosion(K9_Image *ret_img, K9_Image *image, Kernel kern, bool read);
/*! @brief Performs the thinning operation. Currently only uses the GuoHall method.
 * More info on GuoHall thinning: https://dl.acm.org/doi/10.1145/62065.62074
 *
 * @param ret_img Return image, stores output of thinning
 * @param image Input image for thinning, must be a binary image 
 * @return Single channel image thinned to one pixel paths.
 */
K9_Image *thinning(K9_Image *ret_img, K9_Image *image, bool read);
/*! @brief Creates a kernel/structuring element. 
 * When creating input array use 1's, 0's and -1's. 
 *
 * @param input Input integer array created by user.
 * @param size Size/length of the input array.
 * @returns Kernel structure that can be used for operations.
 */
Kernel create_kernel(int *input, size_t size);
/*! @brief Creates a wxh sized kernel filled with 1's (255's).
 * This is for when the shape of the kernel isn't important.
 *
 * @param w Width of the kernel
 * @param h Height of the kernel
 * @returns Kernel structure that can be used for operations.
 */
Kernel quick_kernel(int w, int h);