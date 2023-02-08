#include <GLFW/glfw3.h>

void create_shader(void);
void render_init(void);
void bind_texture(int width, int height);
void main_render(uint8_t *data, int width, int height, int channels);
void render_begin(GLFWwindow *window);
void render_end(GLFWwindow *window);
void render_kill(void);