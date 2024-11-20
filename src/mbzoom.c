#include "render/render.h"
#include "effects/effects.h"

should_quit = false;

int main(){
    K9_Image *img = load_image("test.JPG");
    GLFWwindow *window = init_window(*img);
    while(!should_quit){
        mandelbrot_zoom(img, 1.0, 0, 0);
        show_image(window, *img, false);
        glfwPollEvents();
    }
    return 0;
}