#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

__kernel void rgb_to_gray(__global const uchar *in_image, __global uchar *out_image){
    int x = get_global_id(0);

    out_image[x/3] = (in_image[x]+in_image[x+1]+in_image[x+2])/3;
}

__kernel void invert(__global const uchar *in_image, __global uchar *out_image){
    int x = get_global_id(0);

    out_image[x] = 255 - in_image[x];
}

__kernel void rgb_to_hsv(__global const uchar *in_image, __global uchar *out_image){
    int x = get_global_id(0);

    double r, g, b, cmax, cmin, cdiff;
    double h = -1, s = -1, v;
    
    // will come back to make this work better
    if (x%3 == 0){
        r = (double) in_image[x]/255.0;
        g = (double) in_image[x+1]/255.0; 
        b = (double) in_image[x+2]/255.0; 
        cmax = MAX(r, MAX(g, b));
        cmin = MIN(r, MIN(g, b));
        cdiff = cmax - cmin;
        if (cmax == cmin){
            h = 0;
        }
        else if (cmax == r){
            h = fmod(60 * ((g - b)/cdiff) + 360, 360);
        }
        else if (cmax == g){
            h = fmod(60 * ((b - r)/cdiff) + 120, 360);
        }
        else if (cmax == b){
            h = fmod(60 * ((r - g)/cdiff) + 240, 360);
        }
        if (cmax == 0){
            s = 0;
        }
        else{
            s = (1 - cmin/cmax)*255;
        }
        out_image[x] = cmax * 255;
        out_image[x+1] = s;
        out_image[x+2] = h/2;
    }
}
