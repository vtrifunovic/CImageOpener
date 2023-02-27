__kernel void blur(__global const uchar *in_image, __global uchar *out_image, uchar dim, int width, uchar channels){
    int x = get_global_id(0);
    int totalcolor = 0;
    uchar h = sqrt((float)dim)-1;
    for (uchar i = 0; i < h; i++){
        for (uchar j = 0; j < h+1; j++){
            if (i == 0 && j == h)
                continue;
            totalcolor += in_image[x-(i*width-i)*channels-j*channels];
            totalcolor += in_image[x+(i*width-i)*channels+j*channels];
        }
    }
    out_image[x] = (totalcolor - in_image[x])/dim;
}

__kernel void add(__global const uchar *in_image, __global uchar *out_image, __global const uchar *img2){
    int x = get_global_id(0);
    out_image[x] = 0;
    if (in_image[x] + img2[x] > 255)
        out_image[x] = 255;
    else
        out_image[x] = in_image[x] + img2[x];
}

__kernel void subtract(__global const uchar *in_image, __global uchar *out_image, __global const uchar *img2){
    int x = get_global_id(0);
    if (in_image[x] - img2[x] < 0)
        out_image[x] = 0;
    else
        out_image[x] = in_image[x] - img2[x];
}

__kernel void g_dilated(__global const uchar *in_image, __global uchar *out_image, __global const short *kern, uchar dim, int width, uchar channels){
    int x = get_global_id(0);
    uchar max = 0;
    uchar h = sqrt((float)dim)-1;
    for (uchar i = 0; i < h; i++){
        for (uchar j = 0; j < h+1; j++){
            if (i == 0 && j == h)
                continue;
            if (x-(i*width-i)*channels-j*channels < 0)
                continue;
            if (in_image[x-(i*width-i)*channels-j*channels] > max)
                max = in_image[x-(i*width-i)*channels-j*channels];
            if (in_image[x+(i*width-i)*channels+j*channels] > max)
                max = in_image[x+(i*width-i)*channels+j*channels];
        }
    }
    out_image[x] = max;
}

__kernel void g_eroded(__global const uchar *in_image, __global uchar *out_image, __global const short *kern, uchar dim, int width, uchar channels){
    int x = get_global_id(0);
    short min = 255;
    uchar h = sqrt((float)dim)-1;
    for (uchar i = 0; i < h; i++){
        for (uchar j = 0; j < h+1; j++){
            if (i == 0 && j == h)
                continue;
            if (x-(i*width-i)*channels-j*channels < 0)
                continue;
            if (in_image[x-(i*width-i)*channels-j*channels] < min)
                min = in_image[x-(i*width-i)*channels-j*channels];
            if (in_image[x+(i*width-i)*channels+j*channels] < min)
                min = in_image[x+(i*width-i)*channels+j*channels];
        }
    }
    out_image[x] = min;
}