#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "linmath.h"
#include "render/render.h"
#include "masks/masks.h"
#include "conversions/conversions.h"

bool should_quit = false;
int press = 0, held = 0;
// compilation -> gcc main.c render/render.c masks/masks.c conversions/conversions.c -lGL -lglfw -lm
static bool check_close(GLFWwindow *window){
    if (glfwWindowShouldClose(window))
        return true;
    int q_key = glfwGetKey(window, 81);
    if (q_key == GLFW_PRESS){
        return true;
    }
    return false;
}

int main(int argc, char *argv[]){
    int count = 0;
    if (argc < 2){
        printf("No image selected\n\n");
        exit(0);
    }
    K9_Image new_img = load_image(argv[1]);
    GLFWwindow *window = init_window(new_img);
    K9_Image hsv_img = rgb_to_hsv(new_img);
    K9_Image mask = rgb_mask(new_img, (vec3){0, 0, 0}, (vec3) {150, 200, 50});
    K9_Image gray_img = rgb_to_gray(new_img);
    while (!should_quit){
        should_quit = check_close(window);
        int n_key = glfwGetKey(window, GLFW_KEY_N);
        if (n_key == 1 && held == 0){
            count += 1;
            held = 1;
        } else if (n_key == 0 && held == 1){
            held = 0;
        }
        if (count == 0)
            show_image(window, new_img, false);
        else if (count == 1)
            show_image(window, hsv_img, false);
        else if (count == 2)
            show_image(window, gray_img, false);
        else if (count == 3)
            show_image(window, mask, false);
        else if (count == 4){
            grayscale_mask(gray_img, 170, 255);
            show_image(window, gray_img, false);
        } else if (count == 5){
            bitwiseAnd(new_img, mask);
            show_image(window, new_img, false);
        }
        glfwPollEvents();
    }
    free(new_img.image);
    free(mask.image);
    free(gray_img.image);
    free(hsv_img.image);
    return 0;
}
