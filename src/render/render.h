#pragma once
#include <GLFW/glfw3.h>

typedef struct k9_image{
    int width;
    int height;
    int channels;
    uint8_t *image;
    char *name;
} K9_Image;

void show_image(GLFWwindow *window, K9_Image image);
GLFWwindow *init_window(K9_Image image);
K9_Image load_image(char *file);