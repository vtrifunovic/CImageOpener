__kernel void blur(__global const uchar *in_image, __global uchar *out_image, uchar dim, uchar width){
    int x = get_global_id(0);
    int totalcolor = 0;
    uchar h = sqrt((float)dim)-1;
    for (uchar i = 0; i < h; i++){
        for (uchar j = 0; j < h+1; j++){
            if (i == 0 && j == h)
                continue;
            totalcolor = in_image[x-(i*width-i)-j];
            totalcolor = in_image[x+(i*width-i)+j];
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