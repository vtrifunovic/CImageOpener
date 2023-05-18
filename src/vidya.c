#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "linmath.h"
#include "global.h"
#include "render/render.h"
#include "typename.h"
#include "tools/filters.h"
#include "binaryproc/binaryproc.h"
#include "effects/effects.h"

// Horrible prototype function just to play audio with my video.
// May integrate actual audio handling into video processing
static void *audio_thread(void *vargp){
    //usleep(15000);
    // hardcoded in, using badapple as my test video
    //system("ffplay -nodisp a.mp4");
}

int main(int argc, char *argv[]){
    int count = 0;
    if (argc < 2){
        printf("Needs 1 video given\n\n");
        exit(0);
    }

    int a[] = {
        1, -2, 7,
        -5, 2, 5,
        -10, -8, -1};
    // load video data in
    K9_Video *vid = load_video(argv[1]);

    // create image templates with video metadata
    K9_Image *f = create_img(vid->width, vid->height, 3);
    K9_Image *frame = create_img(vid->width, vid->height, 3);
    K9_Image *frame2 = create_img(vid->width, vid->height, 3);
    K9_Image *frame3 = create_img(vid->width, vid->height, 3);

    // Initialize gpu processing
    init_gpu(f);

    // Creating new kernel of all 1's
    Kernel kern = create_kernel(a, 9, false);
    // Initializing display window
    GLFWwindow *window = init_window(*frame, argv[1]);

    int ret;
    //int high[] = {115, 110, 75};
    //int low[] = {0, 0, 0};
    int high[] = {255, 255, 255};
    int low[] = {150, 150, 150};
    // looping and doing some basic processing on video while it is playing
    size_t totalpixels = f->width * f->height * f->channels;
    while (!handle_inputs(window)){
        f = load_video_frame(f, *vid, ret);
        if (ret == 0){
            //color_decay(frame, f, 3, 10, false);
            pixel_lag(frame, f, 1, 1, false);
            buffer_kill(frame2, frame, 0, true);
            //frame2 = gray_morph(frame2, frame, kern, K9_DILATION, true);
        }
        if (f->channels == 0){
            break;
        }
        ret = show_video(window, *frame2, *vid);
    }
    glfwTerminate();
    K9_free(f);
    K9_free(frame);
    K9_free(frame2);
    K9_free_gpu();
    return 0;
}
