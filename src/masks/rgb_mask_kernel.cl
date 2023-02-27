__kernel void rgb_mask(__global const uchar *in_image, __global uchar *out_image, __global int *lower, __global int *higher){
    int x = get_global_id(0);

    out_image[x/3] = 0;
    if (in_image[x] >= lower[x%3] && in_image[x] <= higher[x%3])
        out_image[x/3] = 255;
}

__kernel void bitwiseAnd(__global const uchar *in_image, __global uchar *out_image, __global const uchar *mask){
    int x = get_global_id(0);

    // hacky way to fix buffer overwriting
    out_image[x] = 0;
    if (mask[x/3] == 255){
        out_image[x] = in_image[x];
    }
}

__kernel void bitwiseNot(__global const uchar *in_image, __global uchar *out_image, __global const uchar *mask){
    int x = get_global_id(0);

    // ¯\_(ツ)_/¯ 
    out_image[x] = 0;
    if (mask[x/3] != 255){
        out_image[x] = in_image[x];
    }
}

__kernel void grayscale_mask(__global const uchar *in_image, __global uchar *out_image, uchar low, uchar high){
    int x = get_global_id(0);
    out_image[x] = 0;
    if (in_image[x] >= low && in_image[x] <= high){
        out_image[x] = in_image[x];
    }
}