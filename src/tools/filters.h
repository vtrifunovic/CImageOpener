#include "../render/render.h"
#include "../global.h"
#include "../binaryproc/binaryproc.h"

#define FREE_AND_RETURN(stack) {free(stack->values); free(stack); return;}

typedef struct k9_stack {
    int *values;
    int len;
} K9_Stack;

typedef struct contour {
    int *pixels;
    int length;
    struct contour *next;
} Contour;

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

/*! @brief Detects contours in binary images. Runs depth first search to return
all valid regions. Returns a Contour struct, which is a linked list of all the
contours in the image.
 * 
 * @param image Input image which contour detection is ran on.
 * @param type Type of search to do (N4 or N8 neighbors)
 * @param debug May remove, prints search results to terminal.
 * @return Linked list of Contour structs, one for each contour in image.
 */
Contour *detect_contours(K9_Image *image, int type, bool debug);

/*! @brief Vizualizes the contour by the index of the contour. Re-creates the contour on a 
empty K9_Image. Will automatically stop once last contour is reached.
 *
 * @param original An image with the original shape/size to render the contour onto.
 * @param index Index of the contour to be displayed.
 * @param first The Contour that was returned by detect_contours.
 * @return Image containing only the contour interested in.
 */
K9_Image *viz_contour_by_index(K9_Image *original, int index, Contour *first);

// Frees all data in contour list
void free_contours(Contour *first);