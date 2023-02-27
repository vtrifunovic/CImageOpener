#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

__kernel void hit_x_miss(__global const uchar *in_image, __global uchar *out_image, __global const short *kern, uchar dim, int width){
    int x = get_global_id(0);
    out_image[x] = 0;
    // assuming square 3x3 kernel for now
    uchar h = sqrt((float)dim)-1;
    for (uchar i = 0; i < h; i++){
        for (uchar j = 0; j < h+1; j++){
            if (i == 0 && j == h)
                continue;
            else if (x-(i*width-i)-j < 0)
                return;
            if (kern[dim/2-i*h-j] != in_image[x-(i*width-i)-j] && kern[dim/2-i*h-j] >= 0)
                return;
            if (kern[dim/2+i*h+j] != in_image[x+(i*width-i)+j] && kern[dim/2+i*h+j] >= 0)
                return;
        }
    }
    out_image[x] = 255;
}

__kernel void bin_dilation(__global const uchar *in_image, __global uchar *out_image, __global const short *kern, uchar dim, int width){
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

__kernel void bin_erosion(__global const uchar *in_image, __global uchar *out_image, __global const short *kern, uchar dim, int width){
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

static char A(char a1, char a2){
    return (!a1 && a2);
}

__kernel void gh_thin(__global const uchar *in_image, __global uchar *out_image, int width, uchar iteration){
    int x = get_global_id(0);

    char p1 = in_image[x]/255;
    char p2 = in_image[x-width]/255;
    char p3 = in_image[x-width+1]/255;
    char p4 = in_image[x+1]/255;
    char p5 = in_image[x+1+width]/255;
    char p6 = in_image[x+width]/255;
    char p7 = in_image[x+width-1]/255;
    char p8 = in_image[x-1]/255;
    char p9 = in_image[x-1-width]/255;
    
    char check1 = p2+p3+p4+p5+p6+p7+p8+p9;
    check1 = check1 >= 2 ? check1 : 0;
    check1 = check1 <= 6 ? check1 : 0;
    char check2, check3;
    if (iteration == 1){
        check2 = p2*p4*p8;
        check3 = p2*p6*p8;
    } else {
        check2 = p2*p4*p6;
        check3 = p4*p6*p8;
    }
    char check4 = A(p2, p3) + A(p3, p4) + A(p4, p5) + A(p5, p6) + A(p6, p7) + A(p7, p8) + A(p8, p9) + A(p9, p2);
    if (check1 != 0 && check2 == 0 && check3 == 0 && check4 == 1){
        out_image[x] = 0;
        return;
    }
    out_image[x] = in_image[x];
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