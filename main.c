#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "linmath.h"
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
    if (argc < 3){
        printf("Needs 2 images given\n\n");
        exit(0);
    }
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
    kern = create_kernel(a, sizeof(a)/sizeof(int));
    K9_Image img2 = load_image(argv[2]);
    GLFWwindow *window = init_window(new_img);
    K9_Split chan = split_channels(img2);
    K9_Image mask = rgb_mask(new_img, (vec3){150, 180, 200}, (vec3){255, 255, 220});
    K9_Image gray_img = rgb_to_gray(new_img);
    K9_Image bin = bin_dilation(mask, kern);
    K9_Image hxm = hit_x_miss(mask, kern);
    K9_Image smaller = resize_img(new_img, (vec2){0.5, 0.5}, K9_NEAREST);
    K9_Image blurred = blur(chan.g, kern, 10);
    K9_Image cropped = crop(new_img, (vec2){50, 450}, (vec2){250, 450}, K9_NOFILL);
    invert(chan.r);
    K9_Image merged = merge_channels(chan.r, hxm, chan.b);
    K9_Image stc = blend_img(new_img, chan.r, 0.63254);
    float blend = 0;

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
            show_image(window, img2, false);
        else if (count == 2)
            show_image(window, stc, false);
        else if (count == 3)
            show_image(window, cropped, false);
        else if (count == 4){
            K9_Image fade = blend_img(new_img, img2, blend);
            blend += 0.005;
            show_image(window, fade, false);
            K9_free(fade);
        }
        
        else if (count == 5){
            bitwiseAnd(gray_img, mask);
            show_image(window, gray_img, true);
        } else {
            if (count == 6){
                invert(new_img);
                count++;
            }
            show_image(window, merged, false);
        }
        glfwPollEvents();
    }
    K9_free(mask);
    K9_free(gray_img);
    K9_free(new_img);
    K9_free_split(chan);
    K9_free(blurred);
    K9_free(merged);
    K9_free(smaller);
    K9_free(hxm);
    free(kern.kernel);
    return 0;
}
