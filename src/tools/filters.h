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
    int width;
    int height;
    int x0;
    int y0;
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
/*! @brief Analyzes the contours in the list and returns the same list with the
 * height, width, and (x0, y0) parameters updated.
 *
 * @param image Image which the images were detected on.
 * @param contour_list List of contours detected by detect_contours
 * @returns Updated Contour list.
 */
Contour *analyze_contours(K9_Image *image, Contour *contour_list);
/*! @brief
 * Interpolates the values using two arrays. One for determining cutoffs, the other for determining the values. Works similiar to np.interp()
 * 
 * @param return_size Size of array to be returned (256 for images)
 * @param xp Input array that determines the boundaries of interpolation
 * @param size_xp Length of the xp array
 * @param fp Input array that determines max values in each xp segment
 * @param size_fp Length of fp array
 * @returns Integer array with interpolated values.
 */
int *interp_arrays(int return_size, int *xp, int size_xp, int *fp, int size_fp);
/*! @brief Uses an image and a table to adjust all the pixel values in an image based on the table's values.
 *
 * @param ret_img Return image, this is where output of LUT is stored
 * @param image Input image, this is the image that will be transformed.
 * @param table Table of pixel values. Index is the input image pixel, value is the returned value.
 * @param tablesize Size of the input table
 * @param read Whether to read the data into RAM
 * @returns Image that is scaled based on the input table.
 */
K9_Image *LUT(K9_Image *ret_img, K9_Image *image, int *table, int tablesize, bool read);

// Frees all data in contour list
void K9_free_contours(Contour *first);