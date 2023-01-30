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
    gettimeofday(&start, NULL);
}

static void show_gray(GLFWwindow *window, K9_Image image){
    glClearColor (0.1, 0.1, 0.1, 0.1);
    glClear(GL_COLOR_BUFFER_BIT);
    GLubyte displayimg[image.height][image.width];
    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {
         	displayimg[i][j] = image.image[((image.width*image.height-image.width+j)-(i*image.width))];
        }
    }
    glDrawPixels(image.width, image.height, GL_LUMINANCE, GL_UNSIGNED_BYTE, displayimg);
    glfwSwapBuffers(window);
}

void show_image(GLFWwindow *window, K9_Image image, bool show_fps){
    //fps_count(window);
    glfwSetWindowSize(window, image.width, image.height);
    glfwSetWindowTitle(window, image.name);
    if (image.channels == 1){
        show_gray(window, image);
        return;
    }
    glClearColor (0.1, 0.1, 0.1, 0.1);
    GLubyte checkimg[image.height][image.width][image.channels];
    glClear(GL_COLOR_BUFFER_BIT);
    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {
	        for(int c = 0; c < image.channels; c++){
         	    checkimg[i][j][c] = image.image[((image.width*image.height-image.width+j)-(i*image.width))*3+c];
            }
        }
    }
    if (image.channels == 4){
        glDrawPixels(image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, checkimg);
    } else {
	    glDrawPixels(image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, checkimg);
    }
    glfwSwapBuffers(window);
}

GLFWwindow *init_window(K9_Image image){
    if (!glfwInit()){
        fprintf(stderr, "\e[1;31mGLFW did not initialize\e[0m");
        exit(0);
    }
    //glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(image.width, image.height, image.name, NULL, NULL);
    if (!window){
        fprintf(stderr, "\e[1;31mWindow could not be created! \e[0m\n");
        exit(0);
    }
    glfwMakeContextCurrent(window);
    gettimeofday(&start, NULL);
    return window;
}

K9_Image load_image(char *file){
    K9_Image image;
    // load image using stbi
    uint8_t *rgb_image = stbi_load(file, &image.width, &image.height, &image.channels, 0);
    // Check that image exists
    if (!rgb_image){
        fprintf(stderr, "\e[1;31mError!\e[0m In function call load_image(), valid file was not given.\n");
        exit(0);
    }
    image.image = (uint8_t *) malloc(image.width * image.height * image.channels);
    image.name = (char *) malloc(strlen(file));
    // ^ allocating memory for the name and image data
    // ⌄ copying data into our image struct
    memcpy(image.image, rgb_image, image.width*image.height*image.channels);
    strcpy(image.name, file);
    stbi_image_free(rgb_image);
    printf("\e[1;32mLoaded: \e[0m%s\n", image.name);
    printf("Dimensions:\n-->Width: %d, Height: %d, Channels: %d\n", image.width, image.height, image.channels);
    printf("GPU Enabled: ");
    if (global.enable_gpu == true){
        printf("\e[1;32mYes.\e[0m\n\n");
        init_gpu(image);
    } else {
        printf("\e[1;31mNo.\e[0m\n\n");
    }
    return image;
}

void K9_free_gpu(){
    global.gpu_values.ret = clFlush(global.gpu_values.command_queue);
    global.gpu_values.ret = clFinish(global.gpu_values.command_queue);
    global.gpu_values.ret = clReleaseKernel(global.gpu_values.kernel);
    global.gpu_values.ret = clReleaseProgram(global.gpu_values.program);
    global.gpu_values.ret = clReleaseMemObject(global.gpu_values.input_image);
    global.gpu_values.ret = clReleaseMemObject(global.gpu_values.output_image);
    global.gpu_values.ret = clReleaseCommandQueue(global.gpu_values.command_queue);
    global.gpu_values.ret = clReleaseContext(global.gpu_values.context);
}

void K9_free(K9_Image image){
    free(image.name);
    free(image.image);
}