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

static bool check_close(GLFWwindow *window){
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
    K9_Image *new_img = load_image(argv[1]);
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
    int lower[] = {210, 200, 175};
    int higher[] = {255, 255, 255};
    kern = create_kernel(a, sizeof(a)/sizeof(int));
    
    K9_Image *mask = create_img(new_img->width, new_img->height, 1, "mask");
    mask = rgb_mask(mask, *new_img, lower, higher);

    K9_Image *dil = create_img_template(mask);
    dil = bin_dilation(dil, *mask, kern);

    K9_Image *hxm = create_img_template(mask);
    hxm = hit_x_miss(hxm, mask, kern);

    K9_Image *and = create_img_template(new_img);
    and = bitwiseAnd(and, *new_img, *dil);

    K9_Image *gray = create_img_template(mask);
    gray = rgb_to_gray(gray, *and);

    K9_Image *gray2 = create_img_template(mask);
    gray2 = grayscale_mask(gray2, *gray, 150, 255);

    K9_Image *blr = create_img_template(mask);
    blr = blur(blr, *gray, kern, 2);

    K9_Image *thin = create_img_template(mask);
    thin = thinning(thin, *hxm);
    //K9_Image closing = where_min(hxm, gray2);
    GLFWwindow *window = init_window(*new_img);

    while (!should_quit){
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
            show_image(window, dil, false);
        else if (count == 3)
            show_image(window, hxm, false);
        else if (count == 4)
            show_image(window, and, false);
        else if (count == 5)
            show_image(window, gray, false);
        else if (count == 6)
            show_image(window, gray2, false);
        else if (count == 7)
            show_image(window, blr, false);
        else if (count >= 8)
            show_image(window, thin, false);
        glfwPollEvents();
    }
    K9_free_gpu();
    return 0;
}
