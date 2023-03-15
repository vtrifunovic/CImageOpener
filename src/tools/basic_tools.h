#pragma once
#include "../linmath.h"
#include "../render/render.h"
#include "../binaryproc/binaryproc.h"

typedef struct k9_split {
    K9_Image r;
    K9_Image g;
    K9_Image b;
} K9_Split;

/*! @brief Blurs an image, with pixel mixing depending on the size of the kernel.
 * 
 * @param ret_img Return image, blurred output image is stored here
 * @param image Input image, the image thats going to get blurred
 * @param kern Kernel, values don't matter just the size to determine pixel mixing
 * @param iterations How many times the blurring operation should take place, more iterations, more blurry
 * @return Blurred image stored in the ret_img structure
 */
K9_Image *blur(K9_Image *ret_img, K9_Image image, Kernel kern, int iterations);
/*! @brief Subracts one image from another
 *
 * @param ret_img Return image where the output gets stored
 * @param img1 First image, one being subtracted from
 * @param img2 Second image, one that is subtracting
 * @return Output of img1 - img2
 */
K9_Image *subtract(K9_Image *ret_img, K9_Image *img1, K9_Image *img2);
/*! @brief Adds two images together
 *
 * @param ret_img Return image where the output gets stored
 * @param img1 First image in addition process
 * @param img2 Second image in addition process
 * @return Output of img1 + img2
 */
K9_Image *add(K9_Image *ret_img, K9_Image img1, K9_Image img2);
/*! @brief Splits the image into its red, green, and blue channels.
 * 
 * @param r The image that will hold the "red" or 1st channel.
 * @param g The image that will hold the "green" or 2nd channel.
 * @param b The image that will hold the "blue" or 3rd channel.
 * @param image The image that will be split into its independent channels.
 */
void split_channels(K9_Image *r, K9_Image *g, K9_Image *b, K9_Image image);
/*! @brief Takes three single channel images and merges them into a singular image.
 *
 * @param r Image to be merged into the red channel
 * @param g Image to be merged into the green channel
 * @param b Image to be merged into the blue channel
 * @param destroy Boolean value on whether to free r, g, & b images.
 * @return RGB image containing the r, g, & b images.
 */
K9_Image *merge_channels(K9_Image *ret_img, K9_Image *r, K9_Image *g, K9_Image *b, bool destroy);
/*! @brief Crops image based on x1->x2 and y1-y2 pixels given.
 * Unlike @ref resize_img this takes pixel values and not relative values.
 *
 * Has two types of crops. K9_FILL which fills cropped values with 0's. K9_NOFILL which resizes the image to the cropped dimensions
 * 
 * @param ret_img Return image, this is where the cropped image is stored
 * @param image Input image to be cropped
 * @param xcrop Two int vector of x1 -> x2 values to crop the image on
 * @param ycrop Two int vector of y1 -> y2 values to crop the image on
 * @param type The type of crop to be performed. 
 * @return Cropped image
 */
K9_Image *crop(K9_Image *ret_img, K9_Image image, vec2 xcrop, vec2 ycrop, int type);
/*! @brief Blends two images based on the alpha value. The 1st image is the alpha image and the 2nd one is the beta value.
 *
 * @param image First image to be blended
 * @param b Second image to be blended
 * @param alpha The alpha value to mix the two images
 * @return Output image containing image*alpha + b*(1-alpha)
 */
K9_Image *blend_img(K9_Image *ret_img, K9_Image *image, K9_Image b, float alpha);
/*! @brief Checks to see if two images are the same or not.
 *
 * @param img1 First image for comparison
 * @param img2 Second image for comparison
 * @return Boolean value of true if images are the same, false if they differ
 */
bool compare(K9_Image img1, K9_Image img2);

// Frees the image structure
void K9_free_split(K9_Split image);

/*! @brief Grayscale Morphological Processes, based on the type given will determine what will be used. (Will accept 3 channel images). 
 * Check `type` param to see what acceptable values are.
 *
 * @param ret_img Return image, where output is stored
 * @param image Input image, the image to be processed
 * @param kern Kernel struct determining area of processing
 * @param type K9_EROSION will perform Erosion operation. K9_DILATION will perform dilation.
 * @return Image with requested type of processing.
 */
K9_Image *gray_morph(K9_Image *ret_img, K9_Image *image, Kernel kern, int type);