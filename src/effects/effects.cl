#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

__kernel void pixel_lag(__global const uchar *in_image, __global uchar *out_image, int chance, int chunk_size, int rand){
    int x = get_global_id(0);
    rand = rand<<11;
    rand = x ^ (rand >> 19) ^ ( in_image[x+10] ^ (rand >> 8));
    int choice = rand % (chance + 1);
    if (choice == 1){
        for (int i = 0; i < chunk_size; i++)
            out_image[x+i] = in_image[x+i];
    }
}

__kernel void color_decay(__global const uchar *in_image, __global uchar *out_image, uchar dim, int width, uchar chan){
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
    out_image[x] = (totalcolor+in_image[x])/dim;
}

__kernel void buffer_kill(__global const uchar *in_image, __global uchar *out_image, uchar buffer_overwrite){
        int x = get_global_id(0);
        if (buffer_overwrite != 1)
            out_image[x] = 0;
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
        if (cmax > 255)
            cmax = 10;
        if (x%3 == 0){
            if (x-s > 0)
                out_image[x-(int)s] = 255-s/in_image[x-(int)s];
            float pos1 = x-s;
            if ((int)pos1 > 0)
                out_image[(int)pos1] -= 255-s*cmax/h;
            float pos2 = x/1.002;
            if ((int)pos2 > 0 && pos1 > 0)
                out_image[(int)pos2] -= h-in_image[(int)pos1];
        }
}

static float magnitude(double z1, double z2){
    float mag = z1*z1 + z2*z2;
    return sqrt(mag);
}

__kernel void mbzoom(__global uchar *out_image, int width, 
double scale, double cenx, double ceny, int height){
    int x = get_global_id(0);
    //calculate x & y pos of global_id
    int a = x % width;
    int b = x/width;
    double p1 = (a - (double)width/2) * (4/(double)width) * (16/(9*(double)scale)) + cenx;
    double p2 = (b - (double)width/2) * (4/(double)height) * (1/(double)scale) + ceny;
    double z1, z2 = 0;
    int i = 0;
    int bounds = 100000;
    bool isIn = true;
    while (i < 1400 && isIn){
        // zn^2 + c
        double q1 = z1*z1 - z2*z2 + p1;
        double q2 = 2*z1*z2 - p2;
        // try: zn + sin(zn)
        //double q1 = z1 - z2 + p1;
        //double q2 = sin(z1-z2) - p2;
        i++;
        if (magnitude(q1, q2) > bounds)
            isIn = false;
        z1 = q1;
        z2 = q2;
    }
    if (isIn){
        out_image[x*3] = 0;
        out_image[x*3+1] = 0;
        out_image[x*3+2] = 0;
    }
    else{
        out_image[x*3] = i%256;
        out_image[x*3+1] = (i)%(int)z1*height;
        out_image[x*3+2] = (i)%(int)z2*width;//(i*i)%height;
    }
}