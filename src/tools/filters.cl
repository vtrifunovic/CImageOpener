static uchar insertion_sort(uchar *window, uchar size){
    int j, key;
    for (uchar i = 1; i < size; i++){
        key = window[i];
        j = i - 1;
        while (j >= 0 && window[j] > key){
            window[j+1] = window[j];
            j = j - 1;
        }
        window[j+1] = key;
    }
    return window[size/2];
}

__kernel void median_filter(__global const uchar *in_image, __global uchar *out_image, __local uchar *window, uchar order, int width, uchar chan){
    int x = get_global_id(0);
    char p_back = order/2;
    char start = p_back-order+1;
    char shift = abs(p_back-order+1);
    for (char y = start; y <= p_back; y++){
        for (char z = start; z <= p_back; z++){
            if (x+z*chan+width*y*chan < 0)
                continue;
            window[order*(y+shift)+(z+shift)] = in_image[x+z*chan+width*y*chan];
        }
    }
    out_image[x] = insertion_sort(window, order*order);
}