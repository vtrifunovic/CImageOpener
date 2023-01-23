__kernel void rgb_mask(__global const uchar *in_image, __global uchar *out_image, __global int *lower, __global int *higher){
    int x = get_global_id(0);

    if (in_image[x] > lower[x%3] && in_image[x] < higher[x%3])
        out_image[x/3] = 255;
}