__kernel void blur(__global const uchar *in_image, __global uchar *out_image, uchar dim, int width, uchar chan){
    int x = get_global_id(0);
    int totalcolor = 0;
    char order = sqrt((float)dim);
    char p_back = order/2;
    char start = p_back-order+1;
    char shift = abs(p_back-order+1);
    for (char y = start; y <= p_back; y++){
        for (char z = start; z <= p_back; z++){
            if (x+z*chan+width*y*chan < 0)
                continue;
            totalcolor += in_image[x+z*chan+width*y*chan];
        }
    }
    out_image[x] = totalcolor/dim;
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

__kernel void g_dilated(__global const uchar *in_image, __global uchar *out_image, __global const short *kern, uchar dim, int width, uchar chan){
    int x = get_global_id(0);
    uchar max = 0;
    char order = sqrt((float)dim);
    char p_back = order/2;
    char start = p_back-order+1;
    char shift = abs(p_back-order+1);
    for (char y = start; y <= p_back; y++){
        for (char z = start; z <= p_back; z++){
            if (x+z*chan+width*y*chan < 0)
                continue;
            if (in_image[x+z*chan+width*y*chan] > max)
                max = in_image[x+z*chan+width*y*chan];
        }
    }
    out_image[x] = max;
}

__kernel void g_eroded(__global const uchar *in_image, __global uchar *out_image, __global const short *kern, uchar dim, int width, uchar chan){
    int x = get_global_id(0);
    short min = 255;
    char order = sqrt((float)dim);
    char p_back = order/2;
    char start = p_back-order+1;
    char shift = abs(p_back-order+1);
    for (char y = start; y <= p_back; y++){
        for (char z = start; z <= p_back; z++){
            if (x+z*chan+width*y*chan < 0)
                continue;
            if (in_image[x+z*chan+width*y*chan] < min)
                min = in_image[x+z*chan+width*y*chan];
        }
    }
    out_image[x] = min;
}