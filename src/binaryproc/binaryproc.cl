#define MAX(a,b) (((a)>(b))?(a):(b))

__kernel void hit_x_miss(__global const uchar *in_image, __global uchar *out_image, __global const uchar *kern, uchar dim, int width){
    int x = get_global_id(0);
    out_image[x] = 0;
    // assuming square kernel for now
    uchar h = sqrt((float)dim)-1;
    for (uchar i = 0; i < h; i++){
        for (uchar j = 0; j < h+1; j++){
            if (i == 0 && j == h)
                continue;
            else if (x-(i*width-i)-j < 0)
                    continue;
            if (kern[dim/2-i*h-j] != in_image[x-(i*width-i)-j])
                return;
            if (kern[dim/2+i*h+j] != in_image[x+(i*width-i)+j])
                return;
        }
    }
    out_image[x] = 255;
}

__kernel void bin_dilation(__global const uchar *in_image, __global uchar *out_image, __global const uchar *kern, uchar dim, int width){
    int x = get_global_id(0);
    out_image[x] = 0;
    uchar h = sqrt((float)dim)-1;
    if (kern[dim/2] == in_image[x]){ // matching center elements
        for (uchar i = 0; i < h; i++){
            for (uchar j = 0; j < h+1; j++){
                if (i == 0 && j == h)
                    continue;
                else if (x-(i*width-i)-j < 0)
                    continue;
                out_image[x-(i*width-i)-j] = MAX(kern[dim/2-i*h-j], in_image[x-(i*width-i)-j]);
                out_image[x+(i*width-i)+j] = MAX(kern[dim/2+i*h+j], in_image[x+(i*width-i)+j]);
            }
        }
    }
}


__kernel void bin_erosion(__global const uchar *in_image, __global uchar *out_image, __global const uchar *kern, uchar dim, int width){
    int x = get_global_id(0);
    out_image[x] = 255;
    uchar h = sqrt((float)dim)-1;
    if (kern[dim/2] != in_image[x]){ // not matching center elements
        for (uchar i = 0; i < h; i++){
            for (uchar j = 0; j < h+1; j++){
                if (i == 0 && j == h)
                    continue;
                else if (x-(i*width-i)-j < 0) // OOB Check
                    continue;
                out_image[x-(i*width-i)-j] = 0;
                out_image[x+(i*width-i)+j] = 0;
            }
        }
    }
}