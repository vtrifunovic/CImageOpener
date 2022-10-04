#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "linmath.h"
#include "render/render.h"
#include "binaryproc/rgbmask.h"

bool should_quit = false;
// compilation -> gcc main.c render/render.c binaryproc/rgbmask.c -lGL -lglfw -lm
static bool check_close(GLFWwindow *window){
    if (glfwWindowShouldClose(window))
        return true;
    int q_key = glfwGetKey(window, 81);
    if (q_key == GLFW_PRESS)
        return true;
    return false;
}

int main(int argc, char *argv[]){
    if (argc < 2){
        printf("No image selected\n\n");
        exit(0);
    }
    K9_Image new_img = load_image(argv[1]);
    GLFWwindow *window = init_window(new_img);
    rgb_mask(new_img, (vec3){0, 0, 0}, (vec3) {0, 255, 255});
    show_image(window, new_img);
    while (!should_quit){
        should_quit = check_close(window);
        glfwPollEvents();
    }
    free(new_img.image);
    return 0;
}
