#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "linmath.h"
#include "global.h"
#include "render/render.h"
#include "masks/masks.h"
#include "conversions/conversions.h"
#include "binaryproc/binaryproc.h"
#include "tools/basic_tools.h"
#include "typename.h"

// Horrible prototype function just to play audio with my video.
// May integrate actual audio handling into video processing
static void *audio_thread(void *vargp){
    //usleep(15000);
    // hardcoded in, using badapple as my test video
    system("ffplay -nodisp badapple.mp4");
}

bool should_quit = false;
int press = 0, held = 0;

static bool check_close(GLFWwindow *window){
    if (glfwWindowShouldClose(window))
        return true;
    int q_key = glfwGetKey(window, 81);
    if (q_key == GLFW_PRESS){
        return true;
    }
    return false;
}

int main(int argc, char *argv[]){
    int count = 0;
    if (argc < 2){
        printf("Needs 1 video given\n\n");
        exit(0);
    }

    int a[] = {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1,
    };
    // load video data in
    K9_Video *vid = load_video(argv[1]);

    // create image templates with video metadata
    K9_Image *frame = create_img(vid->width, vid->height, 3, "BadApple");
    K9_Image *frame2 = create_img(vid->width, vid->height, 3, "BadApple");

    // Initialize gpu processing
    init_gpu(*frame);

    // Creating new kernel of all 1's
    Kernel kern = create_kernel(a, 9);
    // Initializing display window
    GLFWwindow *window = init_window(*frame);

    // Creating thread to play audio
    pthread_t audio_th;
    int error = pthread_create(&audio_th, NULL, &audio_thread, NULL);
    if (error != 0){
        printf("Failed to create thread\n");
    }
    int ret;

    // looping and doing some basic processing on video while it is playing
    while (!should_quit){
        should_quit = check_close(window);
        frame = load_video_frame(frame, *vid, ret);
        if (ret == 0){
            frame = gray_morph(frame, frame, kern, K9_DILATION);
            frame2 = gray_morph(frame2, frame, kern, K9_EROSION);
            frame = subtract(frame, frame, frame2);
            frame = gray_morph(frame, frame, kern, K9_DILATION);
        }
        if (frame->channels == 0){
            break;
        }
        ret = show_video(window, *frame, *vid);
        glfwPollEvents();
    }
    glfwTerminate();
    // Thread won't join as ffmpeg keeps going
    pthread_join(audio_th, NULL);
    K9_free_gpu();
    return 0;
}
