#pragma once
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>

// Image struct
typedef struct k9_image{
    int width;
    int height;
    int channels;
    uint8_t *image;
    char *name;
} K9_Image;

// Video data struct
typedef struct k9_video{
    int width;
    int height;
    uint16_t framerate;
    int duration;
    FILE *pipein;
} K9_Video;

/*! @brief Shows the selected image in the chosen window
 *
 *  @param[in] window The window the image will be displayed in
 *  @param[in] image The image stored as as K9_Image structure to be displayed
 *  @param[in] show_fps Under construction. Will eventualy display the fps that the window is running at.
 *  
 */
void show_image(GLFWwindow *window, K9_Image image, bool show_fps);
/*! @brief Displays the current frame if the correct amount of time has passed and returns 0. Otherwise returns a 1 to indicate frame still needs to be on screen.
 *
 * @param[in] window The window to display the frame in.
 * @param[in] image The image/frame to be displayed
 * @param[in] video The video structure containing the video metadata.
 * @return Int of 1 if the frame has not been shown, 0 if the frame has been shown.
 */
int show_video(GLFWwindow *window, K9_Image image, K9_Video video);
/*! @brief Initializes a new window.
 *
 * This function performs the standard GLFW opening a window boilerplate.
 *
 * @param[in] image Doesn't have to be one that will be displayed, just a general K9_Image for the window to be created around
 * @return The created GLFW window
 */
GLFWwindow *init_window(K9_Image image);
/*! @brief Create a image stored in memory as a K9_Image struct.
 * 
 * @param[in] file Path to the file relative to the main file
 * @return K9_Image struct that contains the width, height, channels and image data
 * 
 */
K9_Image *load_image(char *file);
/*! @brief Loads the next frame in the K9_Video struct and returns it as an K9_Image struct.
 * 
 * @param[in] ret_img The image that the frame will be saved in
 * @param[in] video The video file containing video metadata.
 * @param[in] ret Return value if to actually load the next frame. If ret is zero, it will return the ret_img struct.
 * @return If ret is zero, the current frame as a K9_Image structure that can be used with the rest of the functions. Otherwise returns the input image.
 */
K9_Image *load_video_frame(K9_Image *ret_img, K9_Video video, int ret);
/*! @brief Loads a video's metadata into memory as a K9_Video struct.
 * The video stuct is used to decode the frames and display them properly.
 * 
 * @param[in] file Path to the file relative to the main file 
 * @return K9_Video struct that contains the width, height, channels and video path.
 */
K9_Video *load_video(char *file);
/*! @brief To save memory (and fix my memory leak), whenever a new image "object" is needed, a new image should be created. 
 * 
 * The function will take the given parameters and create a new K9_Image struct. 
 * 
 * @param[in] width The width (in pixels) of the image
 * @param[in] height The height (in pixels) of the image
 * @param[in] channels The number of channels the image will have
 * @param[in] name (May be removed) What the name of the structure should be
 * @return K9_Image created with the specified parameters, containing no useful image data.
 */
K9_Image *create_img(int width, int height, int channels, char *name);
/*! @brief Similar to the @ref create_img function, this function creates a new image, but takes in less parameters. It creates a new image "object" based on the dimensions stored in the input struct.
 * 
 * @param[in] image The image whose parameters will be copied into a new structure.
 * @returns New K9_Image struct with same dimensions as the given image. Image data will contain no useful information.
 *
 */
K9_Image *create_img_template(K9_Image *image);
// Frees the structure from memory.
void K9_free(K9_Image *image);