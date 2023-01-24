#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "linmath.h"
#include "global.h"
#include "render/render.h"
#include "masks/masks.h"
#include "conversions/conversions.h"
#include "binaryproc/binaryproc.h"
#include "tools/basic_tools.h"
#include "typename.h"

bool should_quit = false;
int press = 0, held = 0;
// compilation -> gcc main.c render/render.c masks/masks.c conversions/conversions.c -lGL -lglfw -lm
static bool check_close(GLFWwindow *window)
{
    if (glfwWindowShouldClose(window))
        return true;
    int q_key = glfwGetKey(window, 81);
    if (q_key == GLFW_PRESS){
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    int count = 0;
    if (argc < 2){
        printf("Needs 1 image given\n\n");
        exit(0);
    }
    global.enable_gpu = true;
    K9_Image new_img = load_image(argv[1]);
    Kernel kern;
    /*int a[] =  
    {1, 0, 1, 0, 1,
    1, 0, 1, 0, 1, 
    1, 1, 1, 1, 1, 
    1, 0, 1, 0, 1, 
    1, 0, 1, 0, 1};*/
    int a[] = 
    {1, 1, 1,
    1, 1, 1,
    1, 1, 1};
    int lower[] = {150, 150, 150};
    int higher[] = {255, 255, 255};
    kern = create_kernel(a, sizeof(a)/sizeof(int));
    K9_Image mask = rgb_mask(new_img, lower, higher);
    K9_Image combine = bitwiseAnd(new_img, mask);
    K9_Image gray = rgb_to_gray(new_img);
    K9_Image gmask = grayscale_mask(gray, 50, 150);
    K9_Image inv1 = invert(gray);
    K9_Image inv2 = invert(new_img);
    K9_Image conv = rgb_to_hsv(new_img);
    GLFWwindow *window = init_window(new_img);

    while (!should_quit)
    {
        should_quit = check_close(window);
        int n_key = glfwGetKey(window, GLFW_KEY_N);
        if (n_key == 1 && held == 0){
            count += 1;
            held = 1;
        }
        else if (n_key == 0 && held == 1){
            held = 0;
        }
        if (count == 0)
            show_image(window, new_img, false);
        else if (count == 1)
            show_image(window, mask, false);
        else if (count == 2)
            show_image(window, combine, false);
        else if (count == 3)
            show_image(window, gray, false);
        else if (count == 4)
            show_image(window, gmask, false);
        else if (count == 5)
            show_image(window, inv1, false);
        else if (count == 6)
            show_image(window, inv2, false);
        else if (count == 7)
            show_image(window, conv, false);
        glfwPollEvents();
    }
    K9_free_gpu();
    K9_free(mask);
    K9_free(new_img);
    free(kern.kernel);
    return 0;
}
