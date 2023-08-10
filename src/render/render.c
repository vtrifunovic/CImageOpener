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
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct timeval start, stop;
float added_delay;
int pastsize = 0;
float zoom = 0;
double xpos, ypos;

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

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
    glfwGetCursorPos(window, &xpos, &ypos);
    int h, w;
    glfwGetWindowSize(window, &h, &w);
    xpos = -(xpos/(h/2)-1);
    ypos = ypos/(w/2)-1;
    zoom += yoffset/10;
    zoom = zoom < 0 ? 0 : zoom;
    zoom = zoom > 100 ? 100 : zoom;
    xpos *= zoom;
    ypos *= zoom;
}

static void reset_values(void){
    zoom = 0;
    xpos = 0;
    ypos = 0;
}

bool handle_inputs(GLFWwindow *window){
    glfwPollEvents();
    glfwSetScrollCallback(window, scroll_callback);
    if (glfwGetKey(window, GLFW_KEY_R))
        reset_values();
    if (glfwWindowShouldClose(window))
        return true;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        return true;
    return false;
}

void show_image(GLFWwindow *window, K9_Image image, bool show_fps){
    if (pastsize != image.width * image.height){
        reset_values();
        unbind_texture();
        bind_texture(image.width, image.height);
        glfwSetWindowSize(window, image.width, image.height);
    }
    glfwMakeContextCurrent(window);
    render_begin(window);
    main_render(image.image, image.width, image.height, image.channels, zoom, xpos, ypos);
    render_end(window);
    pastsize = image.width * image.height;
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
}

GLFWwindow *init_window(K9_Image image, char *name){
    if (!glfwInit()){
        fprintf(stderr, "\e[1;31mGLFW did not initialize\e[0m");
        exit(0);
    }
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(image.width, image.height, name, NULL, NULL);
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

K9_Image *load_image(char *file, bool debug){
    K9_Image *image = malloc(sizeof(K9_Image));
    // load image using stbi
    uint8_t *rgb_image = stbi_load(file, &image->width, &image->height, &image->channels, 3);
    image->channels = 3;
    image->tp = image->width * image->height * image->channels;
    // Check that image exists
    if (!rgb_image){
        fprintf(stderr, "\e[1;31mError!\e[0m In function call load_image(), valid file was not given.\n");
        exit(0);
    }
    image->image = (uint8_t *) malloc(image->width * image->height * image->channels);
    image->mem_id = NULL;
    // ^ allocating memory for the name and image data
    // ⌄ copying data into our image struct
    memcpy(image->image, rgb_image, image->width*image->height*image->channels);
    stbi_image_free(rgb_image);
    if (debug){
        printf("\e[1;32mLoaded: \e[0m%s\n", file);
        printf("Dimensions:\n-->Width: %d, Height: %d, Channels: %d\n", image->width, image->height, image->channels);
    }
    return image;
}

void save_image(K9_Image *save, const char *filename){
    int ret;
    char type[6] = {0};
    for (int i = strlen(filename) - 5; i < strlen(filename); i++){
        type[i - (strlen(filename) - 5)] = filename[i];
    }
    if (strcmp(type, ".jpeg") == 0)
        ret = stbi_write_jpg(filename, save->width, save->height, save->channels, save->image, 100);
    for (int j = 0; j < 5; j++)
        type[j] = type[j+1];
    type[5] = '\0';
    //printf("%s\n", type);
    if (strcmp(type, ".jpg") == 0)
        ret = stbi_write_jpg(filename, save->width, save->height, save->channels, save->image, 100);
    if (strcmp(type, ".png") == 0)
        ret = stbi_write_png(filename, save->width, save->height, save->channels, save->image, save->width*save->channels);
    if (!ret)
        fprintf(stderr, "Failed to save image.\n");
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
    if (ret_img->mem_id != NULL){
        clReleaseMemObject(ret_img->mem_id);
        ret_img->mem_id = NULL;
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

K9_Image *create_img(int width, int height, int channels){
    K9_Image *ret_img = malloc(sizeof(K9_Image));
    ret_img->channels = channels;
    ret_img->height = height;
    ret_img->width = width;
    ret_img->tp = width * height * channels;
    ret_img->mem_id = NULL;
    ret_img->image = (uint8_t *)malloc(width * height * channels);
    return ret_img;
}

K9_Image *create_img_template(K9_Image *image, bool alloc_mem){
    K9_Image *ret_img = malloc(sizeof(K9_Image));
    ret_img->channels = image->channels;
    ret_img->height = image->height;
    ret_img->width = image->width;
    ret_img->tp = image->tp;
    ret_img->mem_id = NULL;
    // No point to allocate data if its going str8 to gpu as a buffer
    if (global.enable_gpu && !alloc_mem)
        ret_img->image = NULL; 
    else
        ret_img->image = (uint8_t *)malloc(image->width * image->height * image->channels);
    return ret_img;
}

void K9_free(K9_Image *image){
    if (image->mem_id != NULL)
        global.gpu_values.ret = clReleaseMemObject(image->mem_id);
    free(image->image);
    free(image);
}
