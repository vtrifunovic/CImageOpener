#pragma once
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct k9_image{
    int width;
    int height;
    int channels;
    uint8_t *image;
    char *name;
} K9_Image;

void show_image(GLFWwindow *window, K9_Image image, bool show_fps);
GLFWwindow *init_window(K9_Image image);
K9_Image *load_image(char *file);
K9_Image *create_img(int width, int height, int channels, char *name);
K9_Image *create_img_template(K9_Image *image);
void K9_free(K9_Image *image);
void K9_free_gpu();