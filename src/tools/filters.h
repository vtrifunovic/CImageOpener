#include "../render/render.h"
#include "../global.h"
#include "../binaryproc/binaryproc.h"

struct contour {
    int *pixels;
    int length;
    struct contour *next;
};

/*! @brief Performs the Median filter/blur on a given input image. The order is the size of the blur to be added to the photo.
 * @param[in] ret_img Return image that stores the output
 * @param[in] image Input image that will get filtered
 * @param[in] order The size of the blur (NxN)
 * @param[in] read Whether to read the data into RAM
 * @returns Image that has had a median filter ran on it.
 */
K9_Image *median_filter(K9_Image *ret_img, K9_Image *image, uint8_t order, bool read);

/*! @brief Performs convolution on the image, keeping the same dimensions that the image originally had (similar to 'np.convolve(img, kern, "same")').
 *
 * @param[in] ret_img Return image, where the output is stored
 * @param[in] image Input image that the convolution will be performed on.
 * @param[in] kern Kernel to use for the convolution operation
 * @param[in] read Whether to read the data into RAM
 * @returns Convolved image based on the user's input kernel
 */
K9_Image *convolve(K9_Image *ret_img, K9_Image *image, Kernel kern, bool read);

/*! @brief Performs a gaussian blur on an image.
 *
 * @param ret_img Return image where output is stored
 * @param image Input image that the operation is performed on
 * @param kern The kernel (preferrably a gaussian one)
 * @param[in] read Whether to read the data into RAM
 * @param divisor If you want to override and provide your own external factor. Otherwise provide it K9_IGNORE (0xFFFFFF)
 * @returns Image that has had a gaussian blur performed on it.
 */
K9_Image *gaussian_blur(K9_Image *ret_img, K9_Image *image, Kernel kern, bool read, int divisor);

void detect_contours(K9_Image *image, bool debug);

K9_Image *viz_contour_by_index(K9_Image *original, int index);