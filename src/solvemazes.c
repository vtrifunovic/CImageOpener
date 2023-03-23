#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include "linmath.h"
#include "global.h"
#include "render/render.h"
#include "masks/masks.h"
#include "conversions/conversions.h"
#include "binaryproc/binaryproc.h"
#include "tools/basic_tools.h"
#include "typename.h"

// use maze6.png

bool should_quit = false;

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
    struct timeval start, stop;
    gettimeofday(&start, NULL);
    K9_Image *new_img = load_image(argv[1], true);
    K9_Image *new_img2 = create_img_template(new_img);
    memcpy(new_img2->image, new_img->image, new_img->height*new_img->width*new_img->channels);
    init_gpu(new_img);
    int lower[] = {210, 200, 175};
    int higher[] = {255, 255, 255};
    int dil[] =
        {1, 1, 1,
         1, 1, 1,
         1, 1, 1};
    int a[] = {
        -1, 1, -1, 
        0, 1, 0,
        0, 0, 0
    };
    int b[] = {
        0, 0, 0,
        0, 1, 0,
        -1, 1, -1,
    };
    int c[] = {
        0, -1, -1,
        0, 1, 1,
        0, -1, -1
    };
    int d[] = {
        -1, -1, 0,
        1, 1, 0,
        -1, -1, 0
    };    
    int e[] = {
        -1, -1, 1,
        0, 1, 0,
        0, 0, 0
    };
    int f[] = {
        1, -1, -1,
        0, 1, 0,
        0, 0, 0
    };
    int g[] = {
        0, 0, 0,
        0, 1, 0,
        1, -1, -1,
    };
    int h[] = {
        0, 0, 0,
        0, 1, 0,
        -1, -1, 1,
    };
    int fx[] = {
        0, 0, 0,
        -1, 0, -1,
        0, 1, 0,
    };
    Kernel kern_x = create_kernel(a, sizeof(a) / sizeof(int));
    Kernel kern_y = create_kernel(b, sizeof(b) / sizeof(int));
    Kernel kern_z = create_kernel(c, sizeof(c) / sizeof(int));
    Kernel kern_a = create_kernel(d, sizeof(d) / sizeof(int));
    Kernel kern_b = create_kernel(e, sizeof(e) / sizeof(int));
    Kernel kern_c = create_kernel(f, sizeof(f) / sizeof(int));
    Kernel kern_d = create_kernel(g, sizeof(g) / sizeof(int));
    Kernel kern_e = create_kernel(h, sizeof(h) / sizeof(int));
    Kernel fix = create_kernel(fx, sizeof(fx) / sizeof(int));
    Kernel kern_dil = create_kernel(dil, sizeof(dil) / sizeof(int));

    K9_Image *mask = create_img(new_img->width, new_img->height, 1);
    rgb_mask(mask, new_img, lower, higher, false);
    thinning(mask, mask, true);

    K9_Image *mask2 = create_img_template(mask);

    K9_Image *past_mask = create_img_template(mask);
    rgb_mask(past_mask, new_img, lower, higher, false);

    K9_Image *dil_img = create_img_template(mask);
    memset(dil_img->image, 255, dil_img->height*dil_img->width);
    dil_img = crop(dil_img, *dil_img, (vec2){5, dil_img->width - 5}, (vec2){5, dil_img->height - 5}, K9_FILL);
    invert(dil_img, dil_img, false);
    add(mask, dil_img, mask, false);

    memcpy(mask2->image, mask->image, mask->channels * mask->height * mask->width);

    K9_Image *m1 = create_img_template(mask);
    GLFWwindow *window = init_window(*new_img, "Solving Mazes");

    bool should_quit = false, show_result = false;
    int loops = 0, rs = 0;

    while (!should_quit){
        should_quit = check_close(window);
        if (loops == 0){
            show_image(window, *new_img, false);
            show_image(window, *new_img, false);
            usleep(500000);
            loops++;
        }
        if (!show_result){
            show_image(window, *mask, false);

            hit_x_miss(m1, mask, kern_x, false);
            subtract(mask2, mask, m1, false);

            hit_x_miss(m1, mask, kern_y, false);
            subtract(mask, mask2, m1, false);

            hit_x_miss(m1, mask, kern_z, false);
            subtract(mask2, mask, m1, false);

            hit_x_miss(m1, mask, kern_a, false);
            subtract(mask, mask2, m1, false);

            hit_x_miss(m1, mask, kern_b, false);
            subtract(mask2, mask, m1, false);

            hit_x_miss(m1, mask, kern_c, false);
            subtract(mask, mask2, m1, false);

            hit_x_miss(m1, mask, kern_d, false);
            subtract(mask2, mask, m1, false);

            hit_x_miss(m1, mask, kern_e, false);
            subtract(mask, mask2, m1, true);

            show_result = compare(*mask, *past_mask);

            memcpy(past_mask->image, mask->image, mask->width * mask->height);
        } else {
            show_image(window, *new_img2, false);
        }
        if (show_result == true && rs == 0){
            dil_img = gray_morph(dil_img, mask, kern_dil, K9_DILATION, true);
            dil_img = crop(dil_img, *dil_img, (vec2){10, dil_img->width-(dil_img->width/25)}, (vec2){10, dil_img->height-15}, K9_FILL);
            new_img2 = bitwiseNot(new_img2, new_img, dil_img, true);
            free(kern_x.kernel);
            free(kern_y.kernel);
            free(kern_z.kernel);
            free(kern_a.kernel);
            free(kern_b.kernel);
            free(kern_c.kernel);
            free(kern_d.kernel);
            free(kern_e.kernel);
            free(kern_dil.kernel);
            free(fix.kernel);
            K9_free(dil_img);
            K9_free(m1);
            K9_free(past_mask);
            K9_free(mask);
            gettimeofday(&stop, NULL);
            double sec = (double)(stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
            sec = sec / 1000000 - 0.5; // -0.5 for the half second usleep()
            printf("Solved in: %f sec\n\n", sec);
            rs++;
        }
        glfwPollEvents();
    }
    K9_free_gpu();
    K9_free(new_img);
}
