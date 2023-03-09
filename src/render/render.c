#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../linmath.h"
#include "render.h"
#include "render_internal/render_internal.h"
#include "../global.h"
#include "../opencl_support/gpu_setup.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct timeval start, stop;
GLuint texture = 0;
float added_delay;

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
    glViewport(0, 0, width, height);
}

// prototype for future
static void fps_count(GLFWwindow *window){
    gettimeofday(&stop, NULL);
    double sec = (double)(stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
    int fps = 1 / (sec / 1000000);
    char dfps[20];
    sprintf(dfps, "%d", fps);
    glfwSetWindowTitle(window, dfps);
    gettimeofday(&start, NULL);
}

void show_image(GLFWwindow *window, K9_Image image, bool show_fps){
    glfwMakeContextCurrent(window);
    render_begin(window);
    main_render(image.image, image.width, image.height, image.channels);
    render_end(window);
}

int show_video(GLFWwindow *window, K9_Image image, K9_Video video){
    gettimeofday(&stop, NULL);
    double sec = (double)(stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
    double vid_tim = (1 / (double)video.framerate) * 1000000;
    if (vid_tim < sec+added_delay){
        gettimeofday(&start, NULL);
        show_image(window, image, false);
        added_delay = sec - vid_tim;
        return 0;
    }
    return 1;
    //printf("Vid time: %f :: Current time %f\n", vid_tim, sec);
    //usleep(vid_tim - sec - added_delay);
    //gettimeofday(&start, NULL);
    //show_image(window, image, false);
}

GLFWwindow *init_window(K9_Image image){
    if (!glfwInit()){
        fprintf(stderr, "\e[1;31mGLFW did not initialize\e[0m");
        exit(0);
    }
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(image.width, image.height, image.name, NULL, NULL);
    
    if (!window){
        fprintf(stderr, "\e[1;31mWindow could not be created! \e[0m\n");
        exit(0);
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit()){
        fprintf(stderr, "\e[1;31mGLEW Failed to initialize\e[0m\n");
        exit(0);
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    create_shader();
    render_init();
    bind_texture(image.width, image.height);
    gettimeofday(&start, NULL);
    return window;
}

K9_Image *load_image(char *file){
    K9_Image *image = malloc(sizeof(K9_Image));
    // load image using stbi
    uint8_t *rgb_image = stbi_load(file, &image->width, &image->height, &image->channels, 3);
    image->channels = 3;
    // Check that image exists
    if (!rgb_image){
        fprintf(stderr, "\e[1;31mError!\e[0m In function call load_image(), valid file was not given.\n");
        exit(0);
    }
    image->image = (uint8_t *) malloc(image->width * image->height * image->channels);
    image->name = (char *) malloc(strlen(file));
    // ^ allocating memory for the name and image data
    // ⌄ copying data into our image struct
    memcpy(image->image, rgb_image, image->width*image->height*image->channels);
    strcpy(image->name, file);
    stbi_image_free(rgb_image);
    printf("\e[1;32mLoaded: \e[0m%s\n", image->name);
    printf("Dimensions:\n-->Width: %d, Height: %d, Channels: %d\n", image->width, image->height, image->channels);
    return image;
}

K9_Image *load_video_frame(K9_Image *ret_img, K9_Video video, int ret){
    if (ret == 1)
        return ret_img;
    int count = fread(ret_img->image, 1, video.width*video.height*3, video.pipein);
    if (count != video.width*video.height*3){
        fflush(video.pipein);
        pclose(video.pipein);
        ret_img->channels = 0;
    }
    return ret_img;
}

static char *shift_buffers(char buffer[], uint8_t len){
    for (int i = 0; i < len-1; i++){
        buffer[i] = buffer[i+1];
    }
    return buffer;
}

K9_Video *load_video(char *file){
    K9_Video *vid = malloc(sizeof(K9_Video));
    vid->height = 0;
    vid->width = 0;
    vid->framerate = 0;
    vid->duration = 0;
    char start[] = "ffprobe -show_streams -loglevel error -show_format ";
    char *ffmpegdata = (char *) malloc(strlen(start)+strlen(file)+1);
    strcpy(ffmpegdata, start);
    strcat(ffmpegdata, file);
    FILE *vdata = popen(ffmpegdata, "r");
    bool f_width, f_height, f_frames, f_duration;
    char ch;
    char *beginning_buffer = (char *) malloc(20);
    char *end_buffer = (char *) malloc(10);
    memset(end_buffer, ' ', 10);
    while (ch != EOF){
        ch = (char)fgetc(vdata);
        if (f_width && f_height && f_duration && f_frames){
            break;
        }
        if (ch != '=' && ch != '\n'){
            beginning_buffer = shift_buffers(beginning_buffer, 20);
            beginning_buffer[18] = ch;
            beginning_buffer[19] = '\0';
        } else if (ch == '='){
            while (beginning_buffer[0] == ' ')
                beginning_buffer = shift_buffers(beginning_buffer, 20);
            f_width = !strcmp(beginning_buffer, "width");
            if (f_width && vid->width == 0){
                while (vid->width == 0){
                    ch = (char)fgetc(vdata);
                    if (ch == '\n'){
                        while(end_buffer[0] == ' ')
                            end_buffer = shift_buffers(end_buffer, 10);
                        vid->width = atoi(end_buffer);
                        memset(end_buffer, ' ', 10);
                        memset(beginning_buffer, ' ', 20);
                        if (vid->width == 0)
                            vid->width = 420;
                    }
                    end_buffer = shift_buffers(end_buffer, 10);
                    end_buffer[8] = ch;
                    end_buffer[9] = '\0';
                }
            }
            f_height = !strcmp(beginning_buffer, "height");
            if (f_height && vid->height == 0){
                while (vid->height == 0){
                    ch = (char)fgetc(vdata);
                    if (ch == '\n'){
                        while (end_buffer[0] == ' ')
                            end_buffer = shift_buffers(end_buffer, 10);
                        vid->height = atoi(end_buffer);
                        memset(end_buffer, ' ', 10);
                        memset(beginning_buffer, ' ', 20);
                        if (vid->height == 0)
                            vid->height = 420;
                    }
                    end_buffer = shift_buffers(end_buffer, 10);
                    end_buffer[8] = ch;
                    end_buffer[9] = '\0';
                }
            }
            f_frames = !strcmp(beginning_buffer, "avg_frame_rate");
            if (f_frames && vid->framerate == 0){
                while (vid->framerate == 0){
                    ch = (char)fgetc(vdata);
                    if (ch == '\n'){
                        while (end_buffer[0] == ' ')
                            end_buffer = shift_buffers(end_buffer, 10);
                        vid->framerate = atoi(end_buffer);
                        memset(end_buffer, ' ', 10);
                        memset(beginning_buffer, ' ', 20);
                        if (vid->framerate == 0)
                            vid->framerate = 420;
                    }
                    end_buffer = shift_buffers(end_buffer, 10);
                    end_buffer[8] = ch;
                    end_buffer[9] = '\0';
                }
            }
            f_duration = !strcmp(beginning_buffer, "duration_ts");
            if (f_duration && vid->duration == 0){
                while (vid->duration == 0){
                    ch = (char)fgetc(vdata);
                    if (ch == '\n'){
                        while (end_buffer[0] == ' ')
                            end_buffer = shift_buffers(end_buffer, 10);
                        vid->duration = atoi(end_buffer);
                        memset(end_buffer, ' ', 10);
                        memset(beginning_buffer, ' ', 20);
                        if (vid->duration == 0)
                            vid->duration = 420;
                    }
                    end_buffer = shift_buffers(end_buffer, 10);
                    end_buffer[8] = ch;
                    end_buffer[9] = '\0';
                }
            }
        } else if (ch = '\n'){
            memset(beginning_buffer, ' ', 20);
        }
    }
    fclose(vdata);
    char beginning[] = "ffmpeg -i ";
    char end[] = " -loglevel error -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -c:a libfdk_aac -";
    char *pipedata = (char *) malloc(strlen(beginning) + strlen(file) + strlen(end));
    strcpy(pipedata, beginning);
    strcat(pipedata, file);
    strcat(pipedata, end);
    vid->pipein = popen(pipedata, "r");
    printf("\e[1;32mLoaded: \e[0m%s\n", file);
    printf("-->Width: %d, Height: %d, Frame Rate: %d, Duration: %d\n", vid->width, vid->height, vid->framerate, vid->duration);
    free(pipedata);
    free(ffmpegdata);
    return vid;
}

K9_Image *create_img(int width, int height, int channels, char *name){
    K9_Image *ret_img = malloc(sizeof(K9_Image));
    ret_img->channels = channels;
    ret_img->height = height;
    ret_img->width = width;
    ret_img->name = (char *)malloc(strlen(name));
    strcpy(ret_img->name, name);
    ret_img->image = (uint8_t *)malloc(width * height * channels);
    return ret_img;
}

K9_Image *create_img_template(K9_Image *image){
    K9_Image *ret_img = malloc(sizeof(K9_Image));
    ret_img->channels = image->channels;
    ret_img->height = image->height;
    ret_img->width = image->width;
    ret_img->name = (char *)malloc(strlen(image->name));
    strcpy(ret_img->name, image->name);
    ret_img->image = (uint8_t *)malloc(image->width * image->height * image->channels);
    return ret_img;
}

void K9_free(K9_Image *image){
    free(image->name);
    free(image->image);
    free(image);
}
