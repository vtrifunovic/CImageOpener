#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "../linmath.h"
#include "render.h"
#include "../global.h"
#include "../opencl_support/gpu_setup.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct timeval start, stop;

// prototype for future
static void fps_count(GLFWwindow *window){
    gettimeofday(&stop, NULL);
    double sec = (double)(stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
    int fps = 1 / (sec / 1000000);
    char dfps[20];
    sprintf(dfps, "%d", fps);
    glfwSetWindowTitle(window, dfps);
    gettimeofday(&start, NULL);
}

static void show_gray(GLFWwindow *window, K9_Image *image){
    GLubyte displayimg[image->height][image->width];
    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
         	displayimg[i][j] = image->image[((image->width*image->height-image->width+j)-(i*image->width))];
        }
    }
    glDrawPixels(image->width, image->height, GL_LUMINANCE, GL_UNSIGNED_BYTE, displayimg);
    glFinish();
    glfwSwapBuffers(window);
}


void show_image(GLFWwindow *window, K9_Image *image, bool show_fps){
    //fps_count(window);
    glfwMakeContextCurrent(window);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1, 0.1, 0.1, 0.1);
    if (image->channels == 1){
        show_gray(window, image);
        return;
    }
    GLubyte checkimg[image->height][image->width][image->channels];
    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
	        for(int c = 0; c < image->channels; c++){
         	    checkimg[i][j][c] = image->image[((image->width*image->height-image->width+j)-(i*image->width))*3+c];
            }
        }
    }
    if (image->channels == 4){
        glDrawPixels(image->width, image->height, GL_RGBA, GL_UNSIGNED_BYTE, checkimg);
    } else {
	    glDrawPixels(image->width, image->height, GL_RGB, GL_UNSIGNED_BYTE, checkimg);
    }
    glfwSetWindowTitle(window, image->name);
    glFinish();
    glfwSwapBuffers(window);
}

GLFWwindow *init_window(K9_Image image){
    if (!glfwInit()){
        fprintf(stderr, "\e[1;31mGLFW did not initialize\e[0m");
        exit(0);
    }
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(image.width, image.height, image.name, NULL, NULL);
    //glfwSetWindowPos(window, 150, 100);
    if (!window){
        fprintf(stderr, "\e[1;31mWindow could not be created! \e[0m\n");
        exit(0);
    }
    gettimeofday(&start, NULL);
    glfwMakeContextCurrent(window);
    return window;
}

K9_Image *load_image(char *file){
    K9_Image *image = malloc(sizeof(K9_Image));
    // load image using stbi
    uint8_t *rgb_image = stbi_load(file, &image->width, &image->height, &image->channels, 3);
    image->channels = 3;
    // Check that image exists
    if (!rgb_image){
        fprintf(stderr, "\e[1;31mError!\e[0m In function call load_image(), valid file was not given.\n");
        exit(0);
    }
    image->image = (uint8_t *) malloc(image->width * image->height * image->channels);
    image->name = (char *) malloc(strlen(file));
    // ^ allocating memory for the name and image data
    // ⌄ copying data into our image struct
    memcpy(image->image, rgb_image, image->width*image->height*image->channels);
    strcpy(image->name, file);
    stbi_image_free(rgb_image);
    printf("\e[1;32mLoaded: \e[0m%s\n", image->name);
    printf("Dimensions:\n-->Width: %d, Height: %d, Channels: %d\n", image->width, image->height, image->channels);
    printf("GPU Enabled: ");
    if (global.enable_gpu == true){
        printf("\e[1;32mYes.\e[0m\n\n");
        init_gpu(*image);
    } else {
        printf("\e[1;31mNo.\e[0m\n\n");
    }
    return image;
}

K9_Image *create_img(int width, int height, int channels, char *name){
    K9_Image *ret_img = malloc(sizeof(K9_Image));
    ret_img->channels = channels;
    ret_img->height = height;
    ret_img->width = width;
    ret_img->name = (char *)malloc(strlen(name));
    strcpy(ret_img->name, name);
    ret_img->image = (uint8_t *)malloc(width * height * channels);
    return ret_img;
}

K9_Image *create_img_template(K9_Image *image){
    K9_Image *ret_img = malloc(sizeof(K9_Image));
    ret_img->channels = image->channels;
    ret_img->height = image->height;
    ret_img->width = image->width;
    ret_img->name = (char *)malloc(strlen(image->name));
    strcpy(ret_img->name, image->name);
    ret_img->image = (uint8_t *)malloc(image->width * image->height * image->channels);
    return ret_img;
}

void K9_free(K9_Image *image){
    free(image->name);
    free(image->image);
    free(image);
}