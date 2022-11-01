#include "basic_tools.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

K9_Image blur(K9_Image image, Kernel kern, int iterations){
    int totalpixels = image.width * image.height * image.channels;
    K9_Image ret_img = {
        .height = image.height,
        .width = image.width,
        .channels = image.channels,
        .image = (uint8_t *) malloc(totalpixels),
        .name = (char *) malloc(strlen(image.name)+6),
    };
    strcpy(ret_img.name, "blur_");
    strcat(ret_img.name, image.name);
    // creating temporary image to store data so we can do multiple iterations
    K9_Image tmpimg = { 
        .width = image.width,
        .height = image.height,
        .channels = image.channels,
        .name = (char *) malloc(4),
        .image = (uint8_t *) malloc(totalpixels),
    };
    strcpy(tmpimg.name, "tmp");
    memcpy(tmpimg.image, image.image, totalpixels);
    if (tmpimg.channels > 3)
        tmpimg.channels = 3;
    int comp_iters = 0;
    while (comp_iters != iterations){
        for (int x = 0; x < totalpixels; x++){
            int avg_col = 0, loops = 0;
            for (int y = 0; y < kern.width; y++){
                for (int z = 0; z < kern.height; z++){
                    int curr_val = x + (y - (kern.width / 2))*tmpimg.channels + ((z - (kern.height / 2)) * tmpimg.width)*tmpimg.channels;
                    if (curr_val > totalpixels || curr_val < 0) //OOB MEM CHECK
                        continue; 
                    avg_col += tmpimg.image[curr_val];
                    loops++;
                }
            }
            if (loops == 0)
                ret_img.image[x] = 0;
            else
                ret_img.image[x] = avg_col/loops;
        }
        if (comp_iters != iterations)
            memcpy(tmpimg.image, ret_img.image, totalpixels);
        comp_iters++;
    }
    K9_free(tmpimg);
    return ret_img;
}



K9_Split split_channels(K9_Image image){
    int totalpixels = image.width*image.height;
    if (image.channels < 3)
        fprintf(stderr, "\e[1;31mCannot split less than 3 channels\e[0m\n");
    K9_Split ret_img = {
        .r = {
            .width = image.width,
            .height = image.height,
            .channels = 1,
            .image = (uint8_t *)malloc(totalpixels),
            .name = (char *)malloc(2)
        },
        .g = {
            .width = image.width,
            .height = image.height,
            .channels = 1,
            .image = (uint8_t *)malloc(totalpixels),
            .name = (char *)malloc(2)
        },
        .b = {
            .width = image.width,
            .height = image.height,
            .channels = 1,
            .image = (uint8_t *)malloc(totalpixels),
            .name = (char *)malloc(2)
        },

    };
    strcpy(ret_img.r.name, "r");
    strcpy(ret_img.g.name, "g");
    strcpy(ret_img.b.name, "b");
    for (int x = 0; x < totalpixels; x++){
        ret_img.r.image[x] = image.image[x*3];
        ret_img.g.image[x] = image.image[x*3+1];
        ret_img.b.image[x] = image.image[x*3+2];
    }
    return ret_img;
}

K9_Image merge_channels(K9_Image r, K9_Image g, K9_Image b){
    if (r.height != g.height || g.height != b.height || r.width != g.width || g.width != b.width)
        fprintf(stderr,"\e[1;33mWarning!!!\e[0m In function call merge_channels() channel sizes do not match.\n");
    if (r.channels > 1 || g.channels > 1 || b.channels > 1){
        fprintf(stderr, "\e[1;31mAll images must be single channel\e[0m\n");
        exit(0);
    }
    int totalpixels = r.height * r.width;
    K9_Image ret_img = {
        .height = r.height,
        .width = r.width,
        .channels = 3,
        .name = (char *)malloc(7),
        .image = (uint8_t *)malloc(totalpixels*3),
    };
    strcpy(ret_img.name, "Merged");
    for (int x = 0; x < totalpixels; x++){
        ret_img.image[x*3] = r.image[x];
        ret_img.image[x*3+1] = g.image[x];
        ret_img.image[x*3+2] = b.image[x];
    }
    return ret_img;
}

// @todo find a better way to do this
void K9_free_split(K9_Split image){
    free(image.r.name);
    free(image.r.image);
    free(image.g.name);
    free(image.g.image);
    free(image.b.name);
    free(image.b.image);
}