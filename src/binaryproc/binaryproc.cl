#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

__kernel void hit_x_miss(__global const uchar *in_image, __global uchar *out_image, __global const short *kern, uchar dim, int width){
    int x = get_global_id(0);
    out_image[x] = 0;
    if (in_image[x] != kern[dim/2])
        return;
    char order = sqrt((float)dim);
    char p_back = order/2;
    char start = p_back-order+1;
    char shift = abs(p_back-order+1);
    for (char y = start; y <= p_back; y++){
        for (char z = start; z <= p_back; z++){
            if (x+z+width*y < 0)
                continue;
            if (kern[order*(y+shift)+(z+shift)] != in_image[x+z+width*y] && kern[order*(y+shift)+(z+shift)] >= 0)
                return;
        }
    }
    out_image[x] = 255;
}

__kernel void bin_dilation(__global const uchar *in_image, __global uchar *out_image, __global const short *kern, uchar dim, int width){
    int x = get_global_id(0);
    out_image[x] = 0;
    if (kern[dim/2] != in_image[x])
        return;
    char order = sqrt((float)dim);
    char p_back = order/2;
    char start = p_back-order+1;
    char shift = abs(p_back-order+1);
    for (char y = start; y <= p_back; y++){
        for (char z = start; z <= p_back; z++){
            if (x+z+width*y < 0)
                continue;
            out_image[x+z+width*y] = MAX(kern[order*(y+shift)+(z+shift)], in_image[x+z+width*y]);
        }
    }
}

__kernel void bin_erosion(__global const uchar *in_image, __global uchar *out_image, __global const short *kern, uchar dim, int width){
    int x = get_global_id(0);
    out_image[x] = 255;
    if (kern[dim/2] == in_image[x])
        return;
    char order = sqrt((float)dim);
    char p_back = order/2;
    char start = p_back-order+1;
    char shift = abs(p_back-order+1);
    for (char y = start; y <= p_back; y++){
        for (char z = start; z <= p_back; z++){
            if (x+z+width*y < 0)
                continue;
            out_image[x+z+width*y] = kern[order*(y+shift)+(z+shift)] > 0 ? 0 : in_image[x+z+width*y];
        }
    }
}

static char A(char a1, char a2){
    return (!a1 && a2);
}

__kernel void gh_thin(__global const uchar *in_image, __global uchar *out_image, int width, uchar iteration){
    int x = get_global_id(0);
    if (in_image[x] != 255){
        out_image[x] = 0;
        return;
    }
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