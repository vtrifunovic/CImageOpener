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