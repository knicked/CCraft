#ifndef UTIL_H
#define UTIL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

char *load_file(const char *path);
GLuint make_shader(GLenum type, const char *source);
GLuint load_shader(GLenum type, const char *path);
GLuint make_program(GLuint shader1, GLuint shader2);
GLuint load_program(const char *path1, const char *path2);
void load_png_texture(const char *file_name);

#endif