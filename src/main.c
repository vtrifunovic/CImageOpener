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

    // Creating interpolated table of values for LUT function
    int xp[] = {0, 64, 128, 192, 255};
    int fp[] = {0, 16, 128, 240, 255};
    int *table = interp_arrays(256, xp, 5, fp, 5);

    // Applying LUT function to contrast-stretch image
    K9_Image *luttest = create_img_template(new_img, true);
    LUT(luttest, new_img, table, 256, true);

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

    // converting the input image to HSV
    K9_Image *gray = create_img_template(new_img, false);
    // Converting image to hsv and back. This should result in same image
    convert_channels(gray, new_img, K9_RGB2HSV, true);
    convert_channels(new_img, gray, K9_HSV2RGB, true);
   
    // binary dilation
    K9_Image *dil = create_img_template(mask, false);
    dil = bin_dilation(dil, mask, kern, true);

    // binary erosion
    K9_Image *ero = create_img_template(mask, false);
    ero = bin_erosion(ero, mask, kern, true);

    // detecting contours in our mask
    Contour *cnts = detect_contours(dil, K9_N4, false);

    // creating image & window to display contours
    K9_Image *first_c = create_img_template(mask, true);
    analyze_contours(dil, cnts);
    GLFWwindow *cviz = init_window(*mask, "Contour viz");
    while (!handle_inputs(cviz)){
        int n_key = glfwGetKey(cviz, GLFW_KEY_N);
        if (n_key == 1 && held == 0){
            count += 1;
            held = 1;
        }
        else if (n_key == 0 && held == 1)
            held = 0;
        viz_contour_by_index(first_c, count, cnts);
        show_image(cviz, *first_c, false);
        //viz_all_contours(new_img, cnts);
        //show_image(cviz, *new_img, false);
    }
    glfwTerminate();
    count = 0;

    K9_free_contours(cnts);

    // binary hit miss
    K9_Image *hxm = create_img_template(mask, false);
    hxm = hit_x_miss(hxm, mask, kern, true);

    // binary bitwise and
    K9_Image *and = create_img_template(new_img, false);
    and = bitwiseAnd(and, new_img, dil, true);

    // resizing image to half of our original size
    K9_Image *rz = create_img_template(new_img, false);
    rz = resize_img(rz, new_img, (vec2){0.89, 0.89}, K9_BILINEAR, true);

    //save_image(rz, "test.jpeg");

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
    K9_Image *g_ero = create_img_template(new_img, false);
    g_ero = gray_morph(g_ero, new_img, kern, K9_EROSION, true);

    // grayscale dilation
    K9_Image *g_dil = create_img_template(new_img, false);
    g_dil = gray_morph(g_dil, new_img, kern, K9_DILATION, true);

    // subtracting erosion from dilation to create edge detect
    K9_Image *e_tec = create_img_template(new_img, false);
    e_tec = subtract(e_tec, g_dil, g_ero, true);

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
            show_image(window, *new_img, true);
        else if (count == 1)
            show_image(window, *rz, false); 
        else if (count == 2)
            show_image(window, *luttest, false);            
        else if (count == 3)
            show_image(window, *med, false);
        else if (count == 4)
            show_image(window, *hp, false);
        else if (count == 5)
            show_image(window, *mask, false);
        else if (count == 6)
            show_image(window, *dil, false);
        else if (count == 7)
            show_image(window, *ero, false);
        else if (count == 8)
            show_image(window, *and, false);
        else if (count == 9)
            show_image(window, *g_dil, false);
        else if (count == 10)
            show_image(window, *g_ero, false);
        else if (count == 11)
            show_image(window, *thin, false);
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
    K9_free(g_ero);
    K9_free(g_dil);
    K9_free(e_tec);
    K9_free_gpu();
    return 0;
}