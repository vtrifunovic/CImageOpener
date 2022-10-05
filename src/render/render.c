#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "render.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static void show_gray(GLFWwindow *window, K9_Image image){
    GLubyte displayimg[image.height][image.width][4];
    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {
         	displayimg[i][j][0] = image.image[((image.width*image.height-image.width+j)-(i*image.width))];
         	displayimg[i][j][1] = image.image[((image.width*image.height-image.width+j)-(i*image.width))];
         	displayimg[i][j][2] = image.image[((image.width*image.height-image.width+j)-(i*image.width))];
         	displayimg[i][j][3] = image.image[((image.width*image.height-image.width+j)-(i*image.width))];
        }
    }
    glDrawPixels(image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, displayimg);
    glfwSwapBuffers(window);
}

void show_image(GLFWwindow *window, K9_Image image, bool show_fps){
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
        fprintf(stderr, "GLFW did not initialize");
        exit(0);
    }
    GLFWwindow *window = glfwCreateWindow(image.width, image.height, image.name, NULL, NULL);
    if (!window){
        fprintf(stderr, "Window could not be created! \n");
        exit(0);
    }
    glfwMakeContextCurrent(window);
    return window;
}

K9_Image load_image(char *file){
    K9_Image image;
    uint8_t *rgb_image = stbi_load(file, &image.width, &image.height, &image.channels, 3);
    /* Error check for imcompatible images
    & incorrect filenames */
    image.image = rgb_image;
    image.name = file;
    //stbi_image_free(rgb_image);
    printf("Loaded: %s\n", image.name);
    printf("Dimensions:\n-->Width: %d, Height: %d, Channels: %d\n\n", image.width, image.height, image.channels);
    return image;
}
