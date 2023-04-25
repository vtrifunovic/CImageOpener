#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "linmath.h"
#include "global.h"
#include "render/render.h"
#include "masks/masks.h"
#include "binaryproc/binaryproc.h"
#include "conversions/conversions.h"
#include "tools/basic_tools.h"
#include "tools/filters.h"
#include "typename.h"

int press = 0, held = 0;

int main(int argc, char *argv[]){
    int count = 0;

    // opening images from terminal arguments
    if (argc < 2){
        printf("Needs 1 image given\n\n");
        exit(0);
    }

    // loading input image into memory
    K9_Image *new_img = load_image(argv[1], true);

    // initializes gpu processing
    init_gpu(new_img);

    // creating a 3x3 kernel of all 1's
    Kernel kern;
    int a[] = 
    {1, 1, 1,
    1, 1, 1,
    1, 1, 1};
    kern = create_kernel(a, sizeof(a) / sizeof(int), false);
    //kern = quick_kernel(7, 7);

    // setting upper and lower bound for rgb masking.
    // Creating new image structure to hold the masked values in, then running the mask.
    int lower[] = {210, 150, 175};
    int higher[] = {255, 255, 255};
    
    K9_Image *mask = create_img(new_img->width, new_img->height, 1);
    mask = rgb_mask(mask, new_img, lower, higher, true);

    // converting the input image to grayscale
    // since the output is a single channel image I'm using the mask as a template
    K9_Image *gray = create_img_template(mask, false);
    gray = rgb_to_gray(gray, new_img, true);
   
    // binary dilation
    K9_Image *dil = create_img_template(mask, false);
    dil = bin_dilation(dil, mask, kern, true);

    // binary erosion
    K9_Image *ero = create_img_template(mask, false);
    ero = bin_erosion(ero, mask, kern, true);

    // detecting contours in our mask
    Contour *cnts = detect_contours(dil, K9_N8, false);

    // creating image & window to display contours
    K9_Image *first_c = create_img_template(mask, false);
    GLFWwindow *cviz = init_window(*mask, "Contour viz");
    while (!handle_inputs(cviz)){
        first_c = viz_contour_by_index(first_c, count, cnts);
        show_image(cviz, *first_c, false);
        count++;
        usleep(50000);
    }
    glfwTerminate();
    count = 0;

    // binary hit miss
    K9_Image *hxm = create_img_template(mask, false);
    hxm = hit_x_miss(hxm, mask, kern, true);

    // binary bitwise and
    K9_Image *and = create_img_template(new_img, false);
    and = bitwiseAnd(and, new_img, dil, true);

    // resizing image to half of our original size
    K9_Image *rz = create_img_template(new_img, false);
    rz = resize_img(rz, new_img, (vec2){0.5, 0.5}, K9_BILINEAR, true);

    // running median filter on image
    K9_Image *med = create_img_template(new_img, true);
    med = median_filter(med, new_img, 7, true);
    
    int g[] = {
        1, -2, 7,
        -5, 2, 5, 
        -10, -8, -1
    };
    Kernel k2 = create_kernel(g, sizeof(g) / sizeof(int), true);

    K9_Image *hp = create_img_template(new_img, true);
    convolve(hp, new_img, k2, true);

    // blurring the image
    K9_Image *blr = create_img_template(new_img, false);
    blr = blur(blr, new_img, kern, 10, true);

    // grayscale erosion
    K9_Image *g_dil = create_img_template(new_img, false);
    g_dil = gray_morph(g_dil, new_img, kern, K9_EROSION, true);

    // grayscale dilation
    K9_Image *gray2 = create_img_template(new_img, false);
    gray2 = gray_morph(gray2, new_img, kern, K9_DILATION, true);

    // subtracting erosion from dilation to create edge detect
    K9_Image *e_tec = create_img_template(new_img, false);
    e_tec = subtract(e_tec, gray2, g_dil, true);

    // thinning 
    K9_Image *thin = create_img_template(mask, false);
    thin = thinning(thin, mask, true);

    // initializing glfw window
    GLFWwindow *window = init_window(*new_img, "Engine Showcase");

    // looping and displaying images based on how many times 'n' key is pressed
    while (!handle_inputs(window)){
        int n_key = glfwGetKey(window, GLFW_KEY_N);
        if (n_key == 1 && held == 0){
            count += 1;
            held = 1;
        }
        else if (n_key == 0 && held == 1)
            held = 0;
        if (count == 0)
            show_image(window, *new_img, false);
        else if (count == 1)
            show_image(window, *rz, false);            
        else if (count == 2)
            show_image(window, *med, false);
        else if (count == 3)
            show_image(window, *hp, false);
        else if (count == 4)
            show_image(window, *mask, false);
        else if (count == 5)
            show_image(window, *dil, false);
        else if (count == 6)
            show_image(window, *g_dil, false);
        else if (count == 7)
            show_image(window, *gray2, false);
        else
            show_image(window, *e_tec, false);
    }
    K9_free(new_img);
    K9_free(thin);
    K9_free(dil);
    K9_free(and);
    K9_free(hxm);
    K9_free(blr);
    K9_free(gray);
    K9_free(g_dil);
    K9_free(gray2);
    K9_free(e_tec);
    K9_free_gpu();
    return 0;
}
