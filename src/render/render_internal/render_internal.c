#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "render_internal.h"

#define MAX_SOURCE_SIZE (0x5000)

unsigned int VBO;
unsigned int VAO;
unsigned int EBO;
unsigned int vertexShader;
unsigned int fragmentShader;
unsigned int shaderProgram;
unsigned int texture;

void create_shader(void){
    // Loading vert shader
    FILE *fp = fopen("render/render_internal/shader.vert", "r");
    if (!fp){
        printf("File shader.vert not found!\n");
        exit(0);
    }
    char *vertexShaderSource;
    size_t source_size;
    vertexShaderSource = (char *)malloc(MAX_SOURCE_SIZE);
    source_size = fread(vertexShaderSource, 1, MAX_SOURCE_SIZE, fp);
    vertexShaderSource = realloc(vertexShaderSource, strlen(vertexShaderSource) * sizeof(char) + 1);
    fclose(fp);
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char *const *)&vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("Failed to load shader.\nLog: %s\n", infoLog);
    }
    free(vertexShaderSource);

    // Loading fragment shader
    FILE *fp2 = fopen("render/render_internal/shader.frag", "r");
    if (!fp2){
        printf("File shader.frag not found!\n");
        exit(0);
    }
    char *fragmentShaderSource;
    fragmentShaderSource = (char *)malloc(MAX_SOURCE_SIZE);
    source_size = fread(fragmentShaderSource, 1, MAX_SOURCE_SIZE, fp2);
    fragmentShaderSource = realloc(fragmentShaderSource, strlen(fragmentShaderSource) * sizeof(char) + 1);
    fclose(fp2);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const char *const *)&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("Failed to compile frag. Log: %s\n", infoLog);
    }
    free(fragmentShaderSource);

    //Linking programs
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success){
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Failed to link shaders. Log: %s\n", infoLog);
    }
    glUseProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void render_init(void){
    float vertices[] = {
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void bind_texture(int width, int height){
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void main_render(uint8_t *data, int width, int height, int channels){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    if (channels > 1)
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    else
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
}

void render_begin(GLFWwindow *window){
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1, 0.1, 0.1, 0.1);
}

void render_end(GLFWwindow *window){
    glfwSwapBuffers(window);
}

void unbind_texture(void){
    glDeleteTextures(1, &texture);
}

void render_kill(void){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}