#include "binaryproc.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

K9_Image hit_x_miss(K9_Image image, Kernel kern){
    // center element has to be 1
    // don't cares are defined as anything negative
    if (image.channels > 1){
        fprintf(stderr, "hit_x_miss needs a single channel image\n");
        return image;
    };
    K9_Image ret_img = {
        .channels = 1,
        .height = image.height,
        .width = image.width,
        .name = (char *) malloc(strlen(image.name)+4),
        .image = (uint8_t *) malloc(image.height * image.width)
    };
    strcpy(ret_img.name, "hx_");
    strcat(ret_img.name, image.name);
    int center = kern.width * kern.height/2;
    int length = image.width * image.height;
    int count_trues = 0, nxtline = 0;
    for (int x = 0; x < length; x++){
        for (int k = 0; k < kern.height*kern.width; k++){
            if (k > kern.width)
                nxtline++;
            if (x+k+image.width*nxtline > length) // out of bounds memory check
                continue;
            if (image.image[x+k+image.width*nxtline] == kern.kernel[k] || kern.kernel[k] < 0){
                count_trues++;
            }
        }
        if (count_trues == kern.height*kern.width){       
            ret_img.image[x+kern.width*kern.height/2+image.width*(kern.height/2)] = 255; // accessing center element
        }
        count_trues = 0;
        nxtline = 0;
    }
    return ret_img;
}

K9_Image bin_dilation(K9_Image image, Kernel kern){
    if (image.channels > 1){
        fprintf(stderr, "bin_dilation needs single channel image\n");
        return image;
    }
    K9_Image ret_img = {
        .channels = 1,
        .height = image.height,
        .width = image.width,
        .name = (char *)malloc(strlen(image.name)+4),
        .image = (uint8_t *)malloc(image.height * image.width),
    };
    strcpy(ret_img.name, "dil_");
    strcat(ret_img.name, image.name);
    int center = kern.width * kern.height/ 2;
    int length = image.width * image.height;
    for (int j = 0; j < length; j++){
        if (kern.kernel[center] == image.image[j]){
            ret_img.image[j] = 255;
            int inc = 1;
            while (j-inc >= 0 && kern.kernel[center-inc] == 255 && inc <= kern.width/2){
                ret_img.image[j-inc] = 255;
                inc++;
            }
            inc = 1;
            while (j + inc <= length && kern.kernel[center + inc] == 255 && inc <= kern.width/2){
                ret_img.image[j+inc] = 255;
                inc++;
            }
            inc = 1;
            int side = 1;
            while (j-inc*image.width >= 0 && kern.kernel[center + inc * kern.width] == 255){
                ret_img.image[j - inc*image.width] = 255;
                while (j - inc * image.width - side >= 0 && kern.kernel[center - inc - side * kern.width] == 255)
                {
                    ret_img.image[j - side - inc * image.width] = 255;
                    side++;
                }
                side = 1;
                while (j - inc * image.width - side >= 0 && kern.kernel[center - inc + side * kern.width] == 255)
                {
                    ret_img.image[j + side - inc * image.width] = 255;
                    side++;
                }
                inc++;
            }
            inc = 1;
            side = 1;
            while (j + inc * image.width <= length && kern.kernel[center - inc * kern.width] == 255){
                ret_img.image[j + inc * image.width] = 255;
                while (j + inc * image.width - side >= 0 && kern.kernel[center - inc - side * kern.width] == 255)
                {
                    ret_img.image[j - side + inc * image.width] = 255;
                    side++;
                }
                side = 1;
                while (j + inc * image.width - side >= 0 && kern.kernel[center - inc + side * kern.width] == 255)
                {
                    ret_img.image[j + side + inc * image.width] = 255;
                    side++;
                }
                inc++;
            }
        }
    }
    return ret_img;
}

K9_Image bin_erosion(K9_Image image, Kernel kern){
    // still has an issue w/ malloc() overwriting other memory slots
    if (image.channels > 1)
    {
        fprintf(stderr, "bin_erosion needs single channel image\n");
        return image;
    }
    K9_Image ret_img = {
        .channels = 1,
        .height = image.height,
        .width = image.width,
        .name = (char *)malloc(strlen(image.name) + 7),
        .image = (uint8_t *)malloc(image.height * image.width+1), // +1...this fixed malloc() issue???
    };
    strcpy(ret_img.name, "erode_");
    strcat(ret_img.name, image.name);
    memcpy(ret_img.image, image.image, image.width * image.height);
    int center = kern.width * kern.height / 2;
    int length = image.width * image.height;
    for (int j = 0; j < length; j++)
    {
        if (kern.kernel[center] != image.image[j])
        {
            ret_img.image[j] = 0;
            int inc = 1;
            while (j - inc >= 0 && kern.kernel[center - inc] == 255 && inc <= kern.width/2)
            {
                ret_img.image[j - inc] = 0;
                inc++;
            }
            inc = 0;
            while (j + inc <= length && kern.kernel[center + inc] == 255 && inc <= kern.width/2)
            {
                ret_img.image[j + inc] = 0;
                inc++;
            }
            inc = 1;
            int side = 1;
            while (j - inc * image.width >= 0 && kern.kernel[center + inc * kern.width] == 255)
            {
                ret_img.image[j - inc * image.width] = 0;
                while (j - inc * image.width - side >= 0 && kern.kernel[center - inc - side * kern.width] == 255)
                {
                    ret_img.image[j - side - inc * image.width] = 0;
                    side++;
                }
                side = 1;
                while (j - inc * image.width - side >= 0 && kern.kernel[center - inc + side * kern.width] == 255)
                {
                    ret_img.image[j + side - inc * image.width] = 0;
                    side++;
                }
                inc++;
            }
            side = 1;
            inc = 1;
            while (j + inc * image.width <= length && kern.kernel[center - inc * kern.width] == 255)
            {
                ret_img.image[j + inc * image.width] = 0;
                while (j + inc * image.width - side >= 0 && kern.kernel[center - inc - side * kern.width] == 255)
                {
                    ret_img.image[j - side + inc * image.width] = 0;
                    side++;
                }
                side = 1;
                while (j + inc * image.width - side >= 0 && kern.kernel[center - inc + side * kern.width] == 255)
                {
                    ret_img.image[j + side + inc * image.width] = 0;
                    side++;
                }
                inc++;
            }
        }
    }
    return ret_img;
}

Kernel create_kernel(int *a, size_t size){
    // only works with 1x1, 3x3, 5x5 ... NxN kernels
    Kernel kern = {
        .height = sqrt(size), // fix this
        .width = sqrt(size), // and this
        .kernel = (int *) malloc(sizeof(int)*size), 
    };
    for (int j = 0; j < size; j++){
        kern.kernel[j] = a[j]*255;
    }
    return kern;
}