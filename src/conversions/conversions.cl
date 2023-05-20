#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define GET_PIXEL(x, y, z, width, chan) ((x*width+y)*chan+z)

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
    
    if (x%3 == 0){
        float r, g, b, cmax, cmin, cdiff;
        float h = -1, s = -1, v;
        r = (float) in_image[x]/255.0;
        g = (float) in_image[x+1]/255.0; 
        b = (float) in_image[x+2]/255.0; 
        cmax = MAX(r, MAX(g, b));
        cmin = MIN(r, MIN(g, b));
        cdiff = cmax - cmin;
        if (cmax == cmin){
            h = 0;
        }
        else if (cmax == r)
            h = fmod(60 * ((g - b)/cdiff) + 360, 360);
        else if (cmax == g)
            h = fmod(60 * ((b - r)/cdiff) + 120, 360);
        else if (cmax == b)
            h = fmod(60 * ((r - g)/cdiff) + 240, 360);
        if (cmax == 0)
            s = 0;
        else
            s = (1 - cmin/cmax)*255;
        out_image[x] = cmax * 255;
        out_image[x+1] = s;
        out_image[x+2] = h/2;
    }
}

__kernel void hsv_to_rgb(__global const uchar *in_image, __global uchar *out_image){
    int x = get_global_id(0);

    if (x%3 == 0){
        float hh, p, q, t, ff;
        int i;
        if (in_image[x+1] == 0){
            out_image[x] =   in_image[x];
            out_image[x+1] = in_image[x];
            out_image[x+2] = in_image[x];
        }
        hh = in_image[x+2]*2;
        if (hh >= 360.0)
            hh = 0;
        hh /= 60.0;
        i = (int)hh;
        ff = hh - i;
        p = in_image[x] * (1.0 - (float)in_image[x+1]/255);
        q = in_image[x] * (1.0 - ((float)in_image[x+1]/255 * ff));
        t = in_image[x] * (1.0 - ((float)in_image[x+1]/255 * (1.0 - ff)));
        switch(i) {
            case 0:
                out_image[x] = in_image[x];
                out_image[x+1] = t;
                out_image[x+2] = p;
                break;
            case 1:
                out_image[x] = q;
                out_image[x+1] = in_image[x];
                out_image[x+2] = p;
                break;
            case 2:
                out_image[x] = p;
                out_image[x+1] = in_image[x];
                out_image[x+2] = t;
                break;

            case 3:
                out_image[x] = p;
                out_image[x+1] = q;
                out_image[x+2] = in_image[x];
                break;
            case 4:
                out_image[x] = t;
                out_image[x+1] = p;
                out_image[x+2] = in_image[x];
                break;
            case 5:
            default:
                out_image[x] = in_image[x];
                out_image[x+1] = p;
                out_image[x+2] = q;
                break;
        }
    }
}

// full color range rgb to yuv:
// https://web.archive.org/web/20180423091842/http://www.equasys.de/colorconversion.html
__kernel void rgb_to_yuv(__global const uchar *in_image, __global uchar *out_image){
    int x = get_global_id(0);

    if (x%3 == 0){
        float r = (float)in_image[x];
        float g = (float)in_image[x+1];
        float b = (float)in_image[x+2];
        out_image[x] = 0.299*r+0.587*g+0.114*b;
        out_image[x+1] = -0.169*r-0.331*g+0.500*b+128;
        out_image[x+2] = 0.500*r-0.419*g-0.081*b+128;
    }
}

__kernel void yuv_to_rgb(__global const uchar *in_image, __global uchar *out_image){
    int x = get_global_id(0);

    if (x%3 == 0){
        float y = (float)in_image[x];
        float u = (float)in_image[x+1]-128;
        float v = (float)in_image[x+2]-128;

        out_image[x] = 1.000*y+1.400*v;
        out_image[x+1] = 1.000*y-0.343*u-0.711*v;
        out_image[x+2] = 1.000*y+1.765*u;
    }
}

// calculating where the x position of our pixel would be if we used a 2d array
int calculate_pos_x(int in, int width, int height){
    return (in/width)%height;
}

// calculating where the y position of our pixel would be if we used a 2d array
int calculate_pos_y(int in, int width){
    return in%width;
}

__kernel void resize_img_nearest(__global const uchar *in_image, __global uchar *out_image, __global const double *scale, __global const int *sizes){
    int x = get_global_id(0);
    // x_loc refers to x position in x down y left type grid
    // SCALE: sizex = 0 :: sizey = 1 
    // SIZES: ret.width = 0 :: ret.height = 1 :: ret.chan = 2 :: image.width = 3
    if (x%sizes[2] == 0){
        int x_loc = calculate_pos_x(x/sizes[2], sizes[0], sizes[1]);
        int y_loc = calculate_pos_y(x/sizes[2], sizes[0]);
        double posx = floor(y_loc*scale[0]);
        double posy = floor(x_loc*scale[1]);
        for (uchar z = 0; z < sizes[2]; z++){
            out_image[(x_loc*sizes[0]+y_loc)*sizes[2]+z] = in_image[(int)(posy*sizes[3]+posx)*sizes[2]+z];
        }
    }
}

__kernel void resize_img_billinear(__global const uchar *in_image, __global uchar *out_image, __global const double *scale, __global const int *sizes){
    int x = get_global_id(0);

    // Scales are swapped for this one
    if (x%sizes[2] == 0){
        int x_loc = calculate_pos_x(x/sizes[2], sizes[0], sizes[1]);
        int y_loc = calculate_pos_y(x/sizes[2], sizes[0]);
        float sizex = (x_loc + 0.5) * (scale[1])-0.5;
        float sizey = (y_loc + 0.5) * (scale[0])-0.5;
        int xpoi = (int)sizex;
        int ypoi = (int)sizey;
        float x_dist = sizex - xpoi;
        float y_dist = sizey - ypoi;
        for (uchar z = 0; z < sizes[2]; z++){
            uchar a = in_image[(int)(xpoi*sizes[3]+ypoi)*sizes[2]+z];
            uchar b = in_image[(int)((xpoi+1)*sizes[3]+ypoi)*sizes[2]+z];
            uchar c = in_image[(int)(xpoi*sizes[3]+(ypoi+1))*sizes[2]+z];
            uchar d = in_image[(int)((xpoi+1)*sizes[3]+(ypoi+1))*sizes[2]+z];

            out_image[(x_loc*sizes[0]+y_loc)*sizes[2]+z] =  a*(1-x_dist)*(1-y_dist) +b*(x_dist)*(1-y_dist) + c*(1-x_dist)*(y_dist) + d*(x_dist)*y_dist;
        }
    }
}